# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/init.h>
# include <linux/fs.h>
# include <linux/time.h>

MODULE_LICENSE( "GPL" );

#define MYVFS_MAGIC_NUMBER 0x13131313

static void myvfs_put_super(struct super_block *sb)
{
	printk(KERN_DEBUG "My VFS super block destroyed !\n");
}

static struct super_operations const myvfs_super_ops = {
	.put_super = myvfs_put_super,
	.statfs = simple_statfs,
	.drop_inode = generic_delete_inode,
};

static struct inode *myvfs_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);
	if (ret)	
	{
		inode_init_owner(ret, NULL, mode);
		ret->i_size = PAGE_SIZE;
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);//время последнего доступа(изменения) к файлу(ctime - изменения индекса)
	}
	return ret;
}


static int myvfs_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = MYVFS_MAGIC_NUMBER;
	sb->s_op = &myvfs_super_ops;
	root = myvfs_make_inode(sb, S_IFDIR | 0755);
	if (!root)
	{
		printk(KERN_ERR "My VFS inode allocation failed!\n");
		return - ENOMEM;
	}
	root->i_op = &simple_dir_inode_operations;
	root->i_fop = &simple_dir_operations;
	sb->s_root = d_make_root(root);
	if (!sb->s_root)
	{
		printk(KERN_ERR "My VFS root creation failed!\n");
		iput(root);
		return -ENOMEM;
	}
	return 0;
}


static struct dentry *myvfs_mount(struct file_system_type *type, int flags, char const *dev, void *data)
{
	struct dentry *const entry = mount_bdev(type, flags, dev, data, myvfs_fill_sb);
	if(IS_ERR(entry))
		printk(KERN_ERR "My VFS mounting failed!\n");
	else
		printk(KERN_DEBUG "My VFS mounted!\n");
	return entry;
}

static struct file_system_type myvfs_type = {
	.owner = THIS_MODULE,  
	.name = "myvfs",  
	.mount = myvfs_mount, 
	.kill_sb = kill_block_super,  
};

static int __init myvfs_init(void)
{
	int ret = register_filesystem(&myvfs_type);
	if (ret != 0)
	{
		printk(KERN_ERR "MYVFS_MODULE cannot register filesystem !\n");
		return ret;
	}
	printk(KERN_DEBUG "MYVFS_MODULE loaded !\n");
	return 0;
}
static void __exit myvfs_exit(void)
{
	int ret = unregister_filesystem(&myvfs_type);
	if (ret != 0)
		printk(KERN_ERR "MYVFS_MODULE cannot unregister filesystem !\n");
	printk(KERN_DEBUG "MYVFS_MODULE unloaded !\n");
}

module_init(myvfs_init);
module_exit(myvfs_exit);
