#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#define MY_ID "my_id"
#define MY_ID_SIZE sizeof(MY_ID)

static ssize_t eudyptula_read(struct file *filp, char __user *buf, size_t count,
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

static ssize_t eudyptula_write(struct file *filp, const char __user *buf,
			       size_t count, loff_t *f_pos)
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

static struct file_operations eudyptula_fops = {
	.read = eudyptula_read,
	.write = eudyptula_write,
};

static struct miscdevice eudyptula_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "eudyptula",
	.fops = &eudyptula_fops,
};

static int __init eudyptula_init(void)
{
	int err;

	err = misc_register(&eudyptula_dev);
	if (err)
		pr_err("misc_register() failed! (%d)\n", err);

	return err;
}

static void __exit eudyptula_exit(void)
{
	misc_deregister(&eudyptula_dev);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Eudyptula chardev");
