#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x1ec61cfe, "module_layout" },
	{ 0xa51505f8, "class_unregister" },
	{ 0x3742e597, "device_destroy" },
	{ 0xf6e396a0, "class_destroy" },
	{ 0x4dd6c422, "device_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x585e908e, "__class_create" },
	{ 0x6306677b, "__register_chrdev" },
	{ 0x3b825fc1, "commit_creds" },
	{ 0xc5850110, "printk" },
	{ 0x611bf0f1, "prepare_creds" },
	{ 0x37a0cba, "kfree" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "8DBC5C36F76E5FCCD24F00C");
