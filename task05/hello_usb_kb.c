#include <linux/errno.h>
#include <linux/hid.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>

static int hello_usb_probe(struct usb_interface *intf,
			   const struct usb_device_id *id)
{
	printk(KERN_INFO "%s\n", __func__);
	return 0;
}

static void hello_usb_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "%s\n", __func__);
}

static const struct usb_device_id usb_ids[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
			     USB_INTERFACE_SUBCLASS_BOOT,
			     USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{}
};

MODULE_DEVICE_TABLE(usb, usb_ids);

static struct usb_driver hello_driver = {
	.name = "hello-usb-keyboard",
	.id_table = usb_ids,
	.probe = hello_usb_probe,
	.disconnect = hello_usb_disconnect,
};

static int __init hello_init(void)
{
	int err;

	err = usb_register(&hello_driver);
	if (err)
		printk(KERN_ERR "usb_register failed (%d)", err);

	return err;
}

static void __exit hello_exit(void)
{
	usb_deregister(&hello_driver);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("EN");
MODULE_DESCRIPTION("Hello USB keyboard");
