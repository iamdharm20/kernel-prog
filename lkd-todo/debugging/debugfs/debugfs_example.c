/*
	debugfs for easy debug 
	you can also find good example code here
	drivers/net/ehternet/amd/xgbe/xgbe-debugfs.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include "../include/debugfs_types.h"

const char symbol = 'z';


struct dfs_test {
	struct dentry* root;
	struct dfs_pack pack;
};

struct dfs_test		*dfs_data;
int data_pool[1];

void set_dfs_data(struct dfs_test* box, struct dentry* root)
{
	struct dfs_pack* pack;

	box->root = root;
	pack = &box->pack;
	pack->symbol = symbol;
	pack->type = -1;
	pack->status = 0;
}

void update_dfs_data(void)
{
	static int loop;
	
	dfs_data->pack.status = loop++;
}

static ssize_t mem_dev_read(struct file *filep, char __user *user_buf, size_t count,
				loff_t *ppos)
{
	ssize_t ret;

	//ret = simple_read_from_buffer(user_buf, count, ppos, &dfs_data->pack, sizeof(struct dfs_pack));
	update_dfs_data();
	ret = copy_to_user(user_buf, &dfs_data->pack, count);
	if(ret)
		ret = -EAGAIN;
	else
		ret = sizeof(struct dfs_pack);
	
	return ret;
}

static ssize_t mem_dev_write(struct file *filep, const char __user *user_buf, size_t count,
				loff_t *ppos)
{
	ssize_t ret;
	unsigned char kernel_data;

	ret = copy_from_user(&kernel_data, user_buf, 1);
	if(ret) {
		return -EAGAIN;
	}
	else
		ret = 1;

	data_pool[0] += kernel_data;
	printk("jk@debug - %u, %d\n", kernel_data, data_pool[0]);

	return ret;		//# of written
}

static ssize_t cpu_dev_read(struct file *filp, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	ssize_t ret;
	const char *str = "hello world!!";
	const int len = strlen(str) + 1;

	if(count != len) {
		return -EINVAL;
	}

	ret = copy_to_user(user_buf, str, len);	//include null
	if(ret) {
		ret = -EAGAIN;
	}
	else 
		ret = len;

	return ret;
}

static const struct file_operations mem_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = mem_dev_read,
	.write = mem_dev_write,
};

static const struct file_operations cpu_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = cpu_dev_read,
};

struct dentry *rtp;

static int debugfs_drv_init(void)
{
	int 			ret;
	struct dentry 		*root_path;
	struct dentry		*pfile;

	
	dfs_data = (struct dfs_test*)vzalloc(sizeof(struct dfs_test));
	if(dfs_data == NULL) {
		printk("error: could not allocate memory\n");
		return -ENOMEM;
	}

	root_path = debugfs_create_dir(dfs_module_name, NULL);
	if(IS_ERR(root_path)) {
		printk("error: could not create debugfs\n");
		ret = -ENODEV;
		goto init_err1;
	}

	set_dfs_data(dfs_data, root_path);

	pfile = debugfs_create_file(dfs_dev_mem_name, 0600, root_path, dfs_data, &mem_fops);
	if(!pfile) {
		printk("error could not create mem fops file\n");	
		ret = -ENODEV;
		goto init_err2;
	}

	pfile = debugfs_create_file(dfs_dev_cpu_name, 0600, root_path, dfs_data, &cpu_fops);
	if(!pfile) {
		printk("error could not create cpu fops file\n");	
		ret = -ENODEV;
		goto init_err2;
	}

	return 0;

init_err2:
	debugfs_remove_recursive(dfs_data->root);

init_err1:
	vfree(dfs_data);

	return ret;
}

static void debugfs_drv_exit(void)
{
	debugfs_remove_recursive(dfs_data->root);

	vfree(dfs_data);
}

module_init(debugfs_drv_init);
module_exit(debugfs_drv_exit);

MODULE_LICENSE("GPL");
