#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#define MY_ID "my_id"

static unsigned int hook_fn(void *priv, struct sk_buff *skb,
			    const struct nf_hook_state *state)
{
	if (strnstr(skb->data, MY_ID, skb->len))
		printk(KERN_DEBUG MY_ID);

	return NF_ACCEPT;
}

static struct nf_hook_ops nfho = {
	.hook = hook_fn,
	.hooknum = NF_INET_LOCAL_IN,
	.pf = NFPROTO_IPV4,
	.priority = NF_IP_PRI_LAST,
};

static int __init pf_init(void)
{
	return nf_register_net_hook(&init_net, &nfho);
}

static void __exit pf_exit(void)
{
	nf_unregister_net_hook(&init_net, &nfho);
}

module_init(pf_init);
module_exit(pf_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Packet filter");
