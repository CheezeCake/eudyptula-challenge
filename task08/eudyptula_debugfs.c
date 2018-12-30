#include <linux/debugfs.h>
#include <linux/file.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MY_ID "my_id"
#define MY_ID_SIZE sizeof(MY_ID)

static ssize_t id_read(struct file *filp, char __user *buf, size_t count,
		       loff_t *f_pos)
{
	ssize_t size = MY_ID_SIZE;

	if (*f_pos != 0)
		return 0;
	if (count < size)
		return -EINVAL;
	if (copy_to_user(buf, MY_ID, size))
		return -EIO;

	*f_pos += size;

	return size;
}

static ssize_t id_write(struct file *filp, const char __user *buf, size_t count,
			loff_t *f_pos)
{
	char kbuf[MY_ID_SIZE];

	if (count != MY_ID_SIZE)
		return -EINVAL;
	if (copy_from_user(kbuf, buf, count))
		return -EIO;
	if (strncmp(kbuf, MY_ID, count - 1))
		return -EINVAL;

	return count;
}

static struct file_operations id_fops = {
	.read = id_read,
	.write = id_write,
};

static void *page = NULL;
size_t written = 0;
static DEFINE_MUTEX(foo_mutex);

static ssize_t foo_read(struct file *filp, char __user *buf, size_t count,
			loff_t *f_pos)
{
	ssize_t ret;

	if (*f_pos != 0)
		return 0;

	mutex_lock(&foo_mutex);

	if (count > written)
		count = written;

	if (copy_to_user(buf, page, count)) {
		ret = -EIO;
	} else {
		ret = count;
		*f_pos += count;
	}

	mutex_unlock(&foo_mutex);

	return ret;
}

static ssize_t foo_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *f_pos)
{
	ssize_t ret;

	if (count > PAGE_SIZE)
		return -EINVAL;

	mutex_lock(&foo_mutex);

	if (copy_from_user(page, buf, count)) {
		written = 0;
		ret = -EIO;
	} else {
		written = count;
		ret = count;
		*f_pos += count;
	}

	mutex_unlock(&foo_mutex);

	return ret;
}

static struct file_operations foo_fops = {
	.read = foo_read,
	.write = foo_write,
};

static struct dentry *dir = NULL;

static int __init eudyptula_init(void)
{
	struct dentry *id;
	struct dentry *jiffies_entry;
	struct dentry *foo;
	int err;

	dir = debugfs_create_dir("eudyptula", NULL);
	if (IS_ERR(dir)) {
		pr_err("debugfs_create_dir() failed! (%ld)\n", PTR_ERR(dir));
		return PTR_ERR(dir);
	}
	id = debugfs_create_file("id", 0666, dir, NULL, &id_fops);
	if (IS_ERR(id)) {
		pr_err("debugfs_create_file() failed! (%ld)\n", PTR_ERR(id));
		err = PTR_ERR(id);
		goto fail;
	}
	jiffies_entry =
		debugfs_create_u64("jiffies", 0444, dir, (u64 *)&jiffies);
	if (IS_ERR(jiffies_entry)) {
		pr_err("debugfs_create_u64() failed! (%ld)\n",
		       PTR_ERR(jiffies_entry));
		err = PTR_ERR(jiffies_entry);
		goto fail;
	}
	page = (void *)get_zeroed_page(GFP_KERNEL);
	if (IS_ERR(page)) {
		pr_err("get_zeroed_page() failed! (%ld)\n", PTR_ERR(page));
		err = PTR_ERR(page);
		goto fail_page;
	}
	foo = debugfs_create_file("foo", 0644, dir, NULL, &foo_fops);
	if (IS_ERR(foo)) {
		pr_err("debugfs_create_file() failed! (%ld)\n", PTR_ERR(foo));
		err = PTR_ERR(foo);
		goto fail_page;
	}

	return 0;

fail_page:
	free_page((unsigned long)page);
fail:
	debugfs_remove_recursive(dir);

	return err;
}

static void __exit eudyptula_exit(void)
{
	debugfs_remove_recursive(dir);
	free_page((unsigned long)page);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Eudyptula debugfs");
