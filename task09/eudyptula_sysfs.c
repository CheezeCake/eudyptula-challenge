#include <linux/file.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define MY_ID "my_id"
#define MY_ID_SIZE sizeof(MY_ID)

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		       char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s", MY_ID);
}

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t size)
{
	return (size == MY_ID_SIZE && strncmp(buf, MY_ID, size - 1) == 0) ?
		       size :
		       -EINVAL;
}

static struct kobj_attribute id_attr = __ATTR(id, 0660, id_show, id_store);

static void *page = NULL;
size_t written = 0;
static DEFINE_MUTEX(foo_mutex);

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	if (mutex_lock_interruptible(&foo_mutex))
		return -ERESTARTSYS;

	memcpy(buf, page, written);

	mutex_unlock(&foo_mutex);

	return written;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t size)
{
	if (mutex_lock_interruptible(&foo_mutex))
		return -ERESTARTSYS;

	memcpy(page, buf, size);
	written = size;

	mutex_unlock(&foo_mutex);

	return size;
}

static struct kobj_attribute foo_attr = __ATTR(foo, 0644, foo_show, foo_store);

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%lu", jiffies);
}

static struct kobj_attribute jiffies_attr =
	__ATTR(jiffies, 0444, jiffies_show, NULL);

static struct attribute *attrs[] = {
	&id_attr.attr,
	&foo_attr.attr,
	&jiffies_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *dir;

static int __init eudyptula_init(void)
{
	int err;

	page = (void *)get_zeroed_page(GFP_KERNEL);
	if (IS_ERR(page))
		return PTR_ERR(page);

	dir = kobject_create_and_add("eudyptula", kernel_kobj);
	if (!dir)
		return -ENOMEM;

	err = sysfs_create_group(dir, &attr_group);
	if (err)
		kobject_put(dir);

	return err;
}

static void __exit eudyptula_exit(void)
{
	kobject_put(dir);
	free_page((unsigned long)page);
}

module_init(eudyptula_init);
module_exit(eudyptula_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Eudyptula sysfs");
