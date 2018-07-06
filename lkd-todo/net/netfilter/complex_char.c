#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/kdev_t.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/interrupt.h>    /* We want an interrupt */
 
#define PHY_IO_ADD 0x101e8000
#define HW_BUF_SIZE 64
#define NUMOBJS 100
#define BLOCKSIZE 128
#define MYNAME "driver/my_proc_file"
 
DECLARE_MUTEX(mt);
 
struct mydevicestatus
{
    int r1;
    int r2;
    int r3;
};
 
static char *clipstack[100];
static int clipstop = -1;
 
static int acme_count = 1;
static dev_t acme_dev;
 
static struct cdev *acme_cdev;
 
static struct proc_dir_entry *myproc = NULL;
struct kmem_cache *clips;
mempool_t *pool;
static char msg[100];
 
static ssize_t
acme_read(struct file *file, char __user *buf,size_t count,loff_t *ppos)
{
    int len;
    if(*ppos > 0)
    {
        return 0;
    }
    len=strlen(clipstack[clipstop]);
    down(&mt);
   if (copy_to_user(buf /* to */, clipstack[clipstop] /* from */, len)) {
       up(&mt);
      return -EFAULT;
   } else {
       mempool_free(clipstack[clipstop--],pool);
       *ppos = len;
       up(&mt);
       return len;
   }
}
 
static ssize_t
acme_write(struct file *file1,const char __user *buf,size_t count,loff_t *ppos)
{
    down(&mt);
   clipstack[++clipstop] = mempool_alloc(pool,GFP_KERNEL);
   if (copy_from_user(clipstack[clipstop], buf , count)) {
       up(&mt);
      return -EFAULT;
   } else {
       up(&mt);
      return count;
   }
 
}
 
static int device_ioctl(struct inode *inode,struct file*file,unsigned int num,
        unsigned long param)
{
    unsigned int r;
    int (*fptr)(int);
    int fr;
    struct mydevicestatus st;
    switch(num)
    {
        case 10:
                __get_user(r,(int *)param);
                printk("ioctl num=%u int pointer=%u\n",num,r);
                break;
        case 20:
                fptr = (int(*)(int))param;
                fr=fptr(100);
                printk("res=%d\n",fr);
                //mempool_resize(pool,param,0);
                break;
        case 30:
                copy_from_user(&st,(void *)param,sizeof(struct mydevicestatus));
                printk("ioctl struct r1=%d, r2=%d, r3=%d\n",st.r1,st.r2,st.r3);
                break;
    }
    return 0;
}
 
static int device_mmap(struct file *file,struct vm_area_struct* vma)
{
    int size;
    size=vma->vm_end - vma->vm_start;
    if(remap_pfn_range(vma,vma->vm_start,PHY_IO_ADD>>PAGE_SHIFT,size,vma->vm_page_prot))
        return -EAGAIN;
    return 0;
}
 
static struct file_operations acme_fops =
{
    .owner = THIS_MODULE,
    .read = acme_read,
    .write = acme_write,
    .ioctl = device_ioctl,
    .mmap = device_mmap
};
 
int myread(char *page,char **start,off_t off,int count,int *eof,void *data)
{
    int len =0;
    if(off <= 0){
        len=sprintf(page,"msg=%s\n",msg);
    }
    return len;
}
 
int mywrite(struct file *file, const char *buffer, unsigned long count,
           void *data)
{
    if ( copy_from_user(msg, buffer, count) ) {
        return -EFAULT;
    }
    msg[count] = 0;
    return count;
}
 
static int
hello_init (void)
{
    clips=kmem_cache_create("clips",BLOCKSIZE,BLOCKSIZE,0,NULL);
    pool = mempool_create(NUMOBJS,mempool_alloc_slab,mempool_free_slab,clips);
 
    acme_dev = MKDEV(237,0);
    register_chrdev_region(acme_dev,1,"acme 2");
 
    acme_cdev=cdev_alloc();
    if(!acme_cdev)
    {
        printk (KERN_INFO "cdev alloc error.\n");
         return -1;
    }
    acme_cdev->ops = &acme_fops;
    acme_cdev->owner = THIS_MODULE;
 
    if(cdev_add(acme_cdev,acme_dev,acme_count))
    {
        printk (KERN_INFO "cdev add error.\n");
         return -1;
    }
      myproc = create_proc_entry(MYNAME,0,NULL);
      myproc->read_proc = myread;
      myproc->write_proc = mywrite;
 
  return 0;
 
}
 
static void
hello_cleanup (void)
{
    cdev_del(acme_cdev);
    unregister_chrdev_region(acme_dev, acme_count);
    remove_proc_entry(MYNAME,NULL);
    kmem_cache_destroy(clips);
    mempool_destroy(pool);
}
 
module_init (hello_init);
module_exit (hello_cleanup);
MODULE_LICENSE("GPL");
