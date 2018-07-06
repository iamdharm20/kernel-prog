#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/string.h>
#include<linux/vmalloc.h>
#include<linux/mm.h>
#include<linux/init.h>
#include<linux/proc_fs.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/seq_file.h>
#include<linux/slab.h>

static char *str = NULL;

static void
show_map_vma(struct seq_file *m){
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *ivma;
	down_read(&mm->mmap_sem);
	for(ivma=mm->mmap;ivma;ivma=ivma->vm_next){
		seq_printf(m,"0x%lx 0x%lx %c%c%c%c\n",
				ivma->vm_start,
				ivma->vm_end,
				ivma->vm_flags & VM_READ ? 'r' : '-',
				ivma->vm_flags & VM_WRITE ? 'w' : '-',
				ivma->vm_flags & VM_EXEC ? 'x' : '-',
				ivma->vm_flags & VM_MAYSHARE ? 's' : 'p');
	}
	up_read(&mm->mmap_sem);
}

static struct page *find_physical_page(struct vm_area_struct *vma,unsigned long addr){
	pud_t *pud;
	pmd_t *pmd;
	pgd_t *pgd;
	pte_t *pte;
	spinlock_t *ptl;
	struct page *page = NULL;
	struct mm_struct *mm = vma->vm_mm;
	pgd=pgd_offset(mm,addr);
	if(pgd_none(*pgd)||unlikely(pgd_bad(*pgd))){
		goto out;
	}
	pud=pud_offset(pgd,addr);
	if(pud_none(*pud)||unlikely(pud_bad(*pud))){
		goto out;
	}
	pmd = pmd_offset(pud,addr);
	if(pmd_none(*pmd)||unlikely(pmd_bad(*pmd))){
		goto out;
	}
	pte = pte_offset_map_lock(mm,pmd,addr,&ptl);
	if(!pte)
		goto out;
	if(!pte_present(*pte))
		goto unlock;
	page = pfn_to_page(pte_pfn(*pte));
	if(!page)
		goto unlock;
	get_page(page);
unlock:
	pte_unmap_unlock(pte,ptl);
out:
	return page;
}

static void
find_page(struct seq_file *m,unsigned long addr){
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	struct page *page;
	unsigned long kernel_addr;
	
	down_read(&mm->mmap_sem);
	vma = find_vma(mm,addr);
	page=find_physical_page(vma,addr);
	if(!page){
		seq_printf(m,"translation not found.\n");
		goto out;
	}
	kernel_addr=(unsigned long)page_address(page);
	kernel_addr+=(addr&~PAGE_MASK);
	seq_printf(m,"0x%lx\n",kernel_addr);
out:
	up_read(&mm->mmap_sem);
}

static void
write_val(struct seq_file *m,unsigned long addr,int val){
	struct vm_area_struct *vma;
	struct mm_struct *mm = current->mm;
	struct page *page;
	unsigned long kernel_addr;
	down_read(&mm->mmap_sem);
	vma = find_vma(mm,addr);
	if(vma&&addr>=vma->vm_start&&(addr+sizeof(val))<vma->vm_end){
		if(!(vma->vm_flags & VM_WRITE))
			goto out;
		page = find_physical_page(vma,addr);
		if(!page)
			goto out;
		kernel_addr = (unsigned long)page_address(page);
		kernel_addr+=(addr&~PAGE_MASK);
		*(int*)kernel_addr = val;
		put_page(page);
		seq_printf(m,"write %d into 0x%lx\n",*(int*)kernel_addr,kernel_addr);
	}
out:
	up_read(&mm->mmap_sem);
}

static ssize_t my_proc_write(struct file* file,const char __user *buffer,size_t count,loff_t *f_pos){
	char *tmp = kzalloc((count+1),GFP_KERNEL);
	if(!tmp)return -ENOMEM;
	if(copy_from_user(tmp,buffer,count)){
		kfree(tmp);
		return EFAULT;
	}
	kfree(str);
	str=tmp;
	return count;
}

static int my_proc_show(struct seq_file *m,void *v){
	//seq_printf(m,"hello from proc file\n");
	//seq_printf(m,"%s\n",str);
	unsigned long val;
	int val2;
	if(memcmp(str,"listvma",7)==0){
		show_map_vma(m);
	}else if(memcmp(str,"findpage",8)==0){
		if(sscanf(str+8,"%lx",&val)==1){
			find_page(m,val);
		}
	}else if(memcmp(str,"writeval",8)==0){
		if(sscanf(str+8,"%lx %d",&val,&val2)==2){
			write_val(m,val,val2);
		}
	}
	return 0;
}

static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}

static struct file_operations my_fops={
	.owner = THIS_MODULE,
	.open = my_proc_open,
	.release = single_release,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = my_proc_write
};

static int __init hello_init(void){
	struct proc_dir_entry *entry;
	entry = proc_create("mtest",0777,NULL,&my_fops);
	if(!entry){
		return -1;	
	}else{
		printk(KERN_INFO "create proc file successfully\n");
	}
	return 0;
}

static void __exit hello_exit(void){
	remove_proc_entry("mtest",NULL);
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
