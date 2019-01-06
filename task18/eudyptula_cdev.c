#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>

struct identity {
	char name[20];
	int id;
	bool busy;
	struct list_head list;
};

static LIST_HEAD(identities);
static DEFINE_MUTEX(identities_mutex);

static int identity_create(char *name, int id)
{
	struct identity *identity;

	identity = kmalloc(sizeof(struct identity), GFP_KERNEL);
	if (!identity)
		return -ENOMEM;

	strlcpy(identity->name, name, sizeof(identity->name));
	identity->id = id;
	identity->busy = false;

	if (mutex_lock_interruptible(&identities_mutex)) {
		kfree(identity);
		return -ERESTARTSYS;
	}

	list_add(&identity->list, &identities);

	mutex_unlock(&identities_mutex);

	return 0;
}

static struct identity *identity_get(void)
{
	struct identity *identity;

	if (mutex_lock_interruptible(&identities_mutex))
		return NULL;

	identity = list_first_entry_or_null(&identities, struct identity, list);
	if (identity)
		list_del(&identity->list);

	mutex_unlock(&identities_mutex);

	return identity;
}

static DECLARE_WAIT_QUEUE_HEAD(wee_wait);
static struct task_struct *eudyptula_kthr;

int eudyptula_kthr_fn(void *data)
{
	struct identity *identity;

	while (1) {
		if (wait_event_interruptible(wee_wait,
					     kthread_should_stop() ||
						     !list_empty(&identities)))
			return -ERESTARTSYS;

		if (kthread_should_stop())
			return 0;

		if (mutex_lock_interruptible(&identities_mutex))
			continue;
		if (!list_empty(&identities)) {
			mutex_unlock(&identities_mutex);

			identity = identity_get();
			msleep_interruptible(5000);
			if (identity) {
				printk(KERN_DEBUG "name=%s, id=%d",
				       identity->name, identity->id);
				kfree(identity);
			}
		}
	}
}

static ssize_t eudyptula_write(struct file *filp, const char __user *buf,
			       size_t count, loff_t *f_pos)
{
	static int id = 1;

	char kbuf[20];
	size_t cnt = (count > 19) ? 19 : count;
	int err;

	if (copy_from_user(kbuf, buf, cnt))
		return -EIO;

	kbuf[cnt] = '\0';
	err = identity_create(kbuf, id++);
	if (err)
		return err;

	wake_up_interruptible(&wee_wait);

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

	while (!list_empty(&identities)) {
		struct identity *identity = identity_get();
		kfree(identity);
	}
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Eudyptula chardev (task 18)");
