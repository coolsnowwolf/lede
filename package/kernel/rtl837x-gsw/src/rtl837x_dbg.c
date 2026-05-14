/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 StarField Xu <air_jinkela@163.com>
 */

#include <linux/uaccess.h>
#include <linux/trace_seq.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/u64_stats_sync.h>

#include "./rtl837x_common.h"

#include "./rtk-api/rtk_error.h"
#include "./rtk-api/rtk_switch.h"
#include "./rtk-api/dal/rtl8373/rtl8373_asicdrv.h"

static struct proc_dir_entry *proc_reg_dir;
static struct proc_dir_entry *proc_sds_reg;
static struct proc_dir_entry *proc_sds_page5;

#define N_RTK_GSW "rtk_gsw"
#define N_SDS_REG "sds_reg"
#define N_SDS_PAGE5 "sds_page5"

static int sds_reg_read(struct seq_file *seq, void *v)
{
	unsigned int v3;

	rtk_rtl8373_getAsicReg(RTL8373_SDS_MODE_SEL_ADDR, &v3);
	seq_printf(seq, "reg 0x7b20: %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x21u, 0x10u, &v3);
	seq_printf(seq, "sds page 0x21  reg 0x10; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x21u, 0x13u, &v3);
	seq_printf(seq, "sds page 0x21  reg 0x13; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x21u, 0x18u, &v3);
	seq_printf(seq, "sds page 0x21  reg 0x18; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x21u, 0x1Bu, &v3);
	seq_printf(seq, "sds page 0x21  reg 0x1b; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x21u, 0x1Du, &v3);
	seq_printf(seq, "sds page 0x21  reg 0x1d; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x36u, 0x1Cu, &v3);
	seq_printf(seq, "sds page 0x36  reg 0x1c; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x36u, 0x14u, &v3);
	seq_printf(seq, "sds page 0x36  reg 0x14; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x36u, 0x10u, &v3);
	seq_printf(seq, "sds page 0x36  reg 0x10; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 4u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x04; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 6u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x06; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 7u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x07; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 9u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x09; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0xBu, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x0b; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0xCu, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x0c; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0xDu, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x0d; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0x15u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x15; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0x16u, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x16; data = %#x\n", v3);
	rtk_rtl8373_sds_reg_read(0, 0x2Eu, 0x1Du, &v3);
	seq_printf(seq, "sds page 0x2e  reg 0x1d; data = %#x\n", v3);
	return 0LL;
}

static int sds_page5_read(struct seq_file *seq, void *v)
{
	unsigned int v3;

	rtk_rtl8373_sds_regbits_read(0LL, 5LL, 0LL, 1LL, &v3);
	seq_printf(seq, "sds page 5  reg 0; bit0 = %#x\n", v3);
	rtk_rtl8373_sds_regbits_read(0LL, 5LL, 1LL, 255LL, &v3);
	seq_printf(seq, "sds page 5  reg 1; bit7:0 = %#x\n", v3);
	return 0LL;
}



static int sds_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, sds_reg_read, 0);
}

static int sds_page5_open(struct inode *inode, struct file *file)
{
	return single_open(file, sds_page5_read, 0);
}

static const struct proc_ops proc_sds_reg_fops = {
	.proc_open = sds_reg_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release
};

static const struct proc_ops proc_sds_page5_fops = {
	.proc_open = sds_page5_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release
};

int rtl837x_debug_proc_init(void)
{

	if (!proc_reg_dir)
		proc_reg_dir = proc_mkdir(N_RTK_GSW, NULL);

	proc_sds_reg =
	proc_create(N_SDS_REG, 0, proc_reg_dir, &proc_sds_reg_fops);
	if (!proc_sds_reg)
		pr_err("!! FAIL to create %s PROC !!\n", N_SDS_REG);

	proc_sds_page5 =
	proc_create(N_SDS_PAGE5, 0, proc_reg_dir, &proc_sds_page5_fops);
	if (!proc_sds_page5)
		pr_err("!! FAIL to create %s PROC !!\n", N_SDS_PAGE5);

	return 0;
}

int rtl837x_debug_proc_deinit(void)
{

	proc_remove(proc_sds_page5);
	proc_remove(proc_sds_reg);
	proc_remove(proc_reg_dir);

	return 0;
}
