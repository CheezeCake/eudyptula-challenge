#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>

struct identity {
	char name[20];
	int id;
	bool busy;
	struct list_head list;
};

static LIST_HEAD(identities);

static int identity_create(char *name, int id)
{
	struct identity *identity;

	identity = kmalloc(sizeof(struct identity), GFP_KERNEL);
	if (!identity)
		return -ENOMEM;

	strlcpy(identity->name, name, sizeof(identity->name));
	identity->id = id;
	identity->busy = false;
	list_add(&identity->list, &identities);

	return 0;
}

static struct identity *identity_find(int id)
{
	struct list_head *it;
	struct identity *entry;

	list_for_each (it, &identities) {
		entry = list_entry(it, struct identity, list);
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

static void identity_destroy(int id)
{
	struct identity *identity;

	identity = identity_find(id);
	if (identity) {
		list_del(&identity->list);
		kfree(identity);
	}
}

static int __init identity_list_init(void)
{
	struct identity *temp;
	int err;

	err = identity_create("Alice", 1);
	if (err)
		return err;
	err = identity_create("Bob", 2);
	if (err)
		goto fail_2;
	err = identity_create("Dave", 3);
	if (err)
		goto fail_3;
	err = identity_create("Gena", 10);
	if (err)
		goto fail_10;

	temp = identity_find(3);
	pr_info("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_info("id 42 not found\n");

	identity_destroy(42);

	identity_destroy(10);
fail_10:
	identity_destroy(3);
fail_3:
	identity_destroy(2);
fail_2:
	identity_destroy(1);

	return 0;
}

static void __exit identity_list_exit(void)
{
}

module_init(identity_list_init);
module_exit(identity_list_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Linked list");
