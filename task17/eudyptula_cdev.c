#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>

static DECLARE_WAIT_QUEUE_HEAD(wee_wait);
static struct task_struct *eudyptula_kthr;

int eudyptula_kthr_fn(void *data)
{
	if (wait_event_interruptible(wee_wait, kthread_should_stop() != 0))
		return -ERESTARTSYS;
	return 0;
}

#define MY_ID "my_id"
#define MY_ID_SIZE sizeof(MY_ID)

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
	.write = eudyptula_write,
};

static struct miscdevice eudyptula_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "eudyptula",
	.fops = &eudyptula_fops,
	.mode = 0222,
};

static int __init eudyptula_init(void)
{
	int err;

	eudyptula_kthr = kthread_run(eudyptula_kthr_fn, NULL, "eudyptula");
	if (IS_ERR(eudyptula_kthr))
		return PTR_ERR(eudyptula_kthr);

	err = misc_register(&eudyptula_dev);
	if (err) {
		pr_err("misc_register() failed! (%d)\n", err);
		kthread_stop(eudyptula_kthr);
	}

	return err;
}

static void __exit eudyptula_exit(void)
{
	kthread_stop(eudyptula_kthr);
	misc_deregister(&eudyptula_dev);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Eudyptula chardev (task 17)");
