#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/textsearch.h>

#define MY_ID "my_id"

struct ts_config *conf;

static unsigned int hook_fn(void *priv, struct sk_buff *skb,
			    const struct nf_hook_state *state)
{
	unsigned int pos;

	pos = skb_find_text(skb, 0, UINT_MAX, conf);
	if (pos != UINT_MAX)
		printk(KERN_DEBUG "found " MY_ID " at position %u", pos);

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
	conf = textsearch_prepare("kmp", MY_ID, sizeof(MY_ID) - 1, GFP_KERNEL,
				  TS_AUTOLOAD);
	if (IS_ERR(conf))
		return PTR_ERR(conf);

	return nf_register_net_hook(&init_net, &nfho);
}

static void __exit pf_exit(void)
{
	textsearch_destroy(conf);
	nf_unregister_net_hook(&init_net, &nfho);
}

module_init(pf_init);
module_exit(pf_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Packet filter");
