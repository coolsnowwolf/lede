#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/sysctl.h>

#include "af_log.h"
int af_log_lvl = 1;
int af_test_mode = 0;
// todo: rename af_log.c
int g_oaf_enable __read_mostly = 0;

/* 
	cat /proc/sys/oaf/debug
*/
static struct ctl_table oaf_table[] = {
	{
		.procname	= "debug",
		.data		= &af_log_lvl,
		.maxlen 	= sizeof(int),
		.mode		= 0666,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "test_mode",
		.data		= &af_test_mode,
		.maxlen 	= sizeof(int),
		.mode		= 0666,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "enable",
		.data		= &g_oaf_enable,
		.maxlen 	= sizeof(int),
		.mode		= 0666,
		.proc_handler	= proc_dointvec,
	},
	{
	}
};

static struct ctl_table oaf_root_table[] = {
	{
		.procname	= "oaf",
		.mode		= 0555,
		.child		= oaf_table,
	},
	{}
};
static struct ctl_table_header *oaf_table_header;


static int af_init_log_sysctl(void)
{
	struct ctl_table_header *hdr;
	oaf_table_header = register_sysctl_table(oaf_root_table);
	if (oaf_table_header == NULL){
		printk("init log sysctl...failed\n");
		return -ENOMEM;
	}
	printk("init oaf sysctl...ok\n");
	return 0;
}

static int af_fini_log_sysctl(void)
{
	if (oaf_table_header)
		unregister_sysctl_table(oaf_table_header);
	return 0;
}

int af_log_init(void){
	af_init_log_sysctl();
}

int af_log_exit(void){
	af_fini_log_sysctl();
}
