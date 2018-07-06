//https://jlmedina123.wordpress.com/2015/04/14/mmap-driver-implementation/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/mm.h>  /* mmap related stuff */

/* Memory mapping: provides user programs with direct access to device memory
 * Mapped area must be multiple of PAGE_SIZE, and starting address aligned to
 * PAGE_SIZE
 *
 * syscall: mmap(caddr_t addr, size_t len, int ptro, int flags, int fd, off_t off)
 * file operation: int (*mmap)(struct file *f, struct vm_area_struct *vma)
 *
 * Driver needs to: build page tables for address range, and replace vma->vm_ops 
 * Building page tables:
 *    - all at once: remap_page_range
 *    - one page at a time: nopage method. Finds correct page for address, and 
 *      increments its reference cout. Must be implemented if driver supports
 *      mremap syscall
 *
 * fields in struct vm_area_struct:
 *     unsigned long vm_start, vm_end: virtual address range covered
 *                   by VMA
 *     struct file *vm_file: file associated with VMA
 *     struct vm_operations_struct *vm_ops: functions that kernel
 *                   will invoke to operate in VMA
 *     void *vm_private_data: used by driver to store its own information
 *
 * VMA operations:
 *     void (*open)(struct vm_area_struct *vma): invoked when a new reference
 *                   to the VMA is made, except when the VMA is first created,
 *                   when mmap is called
 *     struct page *(*nopage)(struct vm_area_struct *area,
 *                            unsigned long address, int write_access): 
 *                   invoked by page fault handler when process tries to access 
 *                   valid page in VMA, but not currently in memory
 */

#define DEV_NAME "mmap_example"
#define PRINTFUNC() printk(KERN_ALERT DEV_NAME ": %s called\n", __func__)
#define PRINT(a) printk(KERN_ALERT DEV_NAME ": %s: %s\n", __func__, a)

struct dentry  *file1;

struct mmap_info {
	char *data;	/* the data */
	int reference;       /* how many times it is mmapped */  	
};


/* keep track of how many times it is mmapped */
void mmap_open(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
		
	PRINTFUNC();

	info->reference++;
}

void mmap_close(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	
	PRINTFUNC();
	
	info->reference--;
}


/* nopage is called the first time a memory area is accessed which is not in memory,
 * it does the actual mapping between kernel and user space memory
 */
int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;
	unsigned long address = (unsigned long)vmf->virtual_address;

	PRINTFUNC();

	/* is the address valid? */
	if (address > vma->vm_end) {
		printk("invalid address\n");
		return VM_FAULT_SIGBUS;
	}
	/* the data is in vma->vm_private_data */
	info = (struct mmap_info *)vma->vm_private_data;
	if (!info->data) {
		printk("no data\n");
		return VM_FAULT_SIGBUS;	
	}

	/* get the page */
	page = virt_to_page(info->data);
	
	/* increment the reference count of this page */
	get_page(page);

	/* type is the page fault type */
	vmf->page = page;

	return 0;
}

struct vm_operations_struct mmap_vm_ops = {
	.open 	= mmap_open,
	.close	= mmap_close,
	.fault	= mmap_fault,
};

int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	PRINTFUNC();
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
	/* assign the file private data to the vm private data */
	vma->vm_private_data = filp->private_data;
	mmap_open(vma);
	return 0;
}

int my_close(struct inode *inode, struct file *filp)
{
	struct mmap_info *info = filp->private_data;
	
	PRINTFUNC();
	
	/* obtain new memory */
	free_page((unsigned long)info->data);
    	kfree(info);
	filp->private_data = NULL;
	return 0;
}

int my_open(struct inode *inode, struct file *filp)
{
	struct mmap_info *info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	
	PRINTFUNC();
	
	/* obtain new memory */
    	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	
	/* writing something to it */
	memcpy(info->data, "hello from kernel this is file: ", 32);
	memcpy(info->data + 32, filp->f_path.dentry->d_name.name, strlen(filp->f_path.dentry->d_name.name));
	/* assign this info struct to the file */
	filp->private_data = info;
	return 0;
}

static const struct file_operations my_fops = {
	.open = my_open,
	.release = my_close,
	.mmap = my_mmap,
};

static int __init mmapexample_module_init(void)
{
	PRINTFUNC();
	file1 = debugfs_create_file("mmap_example", 0644, NULL, NULL, &my_fops);
	return 0;
}

static void __exit mmapexample_module_exit(void)
{
	PRINTFUNC();
	debugfs_remove(file1);

}

module_init(mmapexample_module_init);
module_exit(mmapexample_module_exit);
MODULE_LICENSE("GPL");
