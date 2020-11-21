/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: whnat
	whnat_proc.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include "woe.h"

/*define proc*/
#define PROC_ROOT_DIR	"whnat_ctrl"	/*global dir for whnat*/
#define PROC_TRACE_DIR	"tracer"		/*tracer */
#define PROC_WHNAT_DIR	"whnat"			/*specific for each whnat*/
#define PROC_CFG_DIR	"cfg"			/*all of related configure*/
#define PROC_CR_DIR		"cr"			/*get specific domain cr*/
#define PROC_CTRL_DIR	"ctrl"			/*get/set control whnat*/
#define PROC_STAT_DIR	"stat"			/*all of status for debug*/
#define PROC_TX_DIR		"tx"			/*tx top information*/
#define PROC_RX_DIR		"rx"			/*rx top information*/
#define PROC_WED_DIR	"wed"			/*wed specific information*/
#define PROC_WDMA_DIR	"wdma"			/*wdma specic information*/

/*
* WDMA
*/
static int wdma_proc_show(struct seq_file *seq, void *v)
{
	struct wdma_entry *wdma = (struct wdma_entry *)seq->private;

	WHNAT_DBG(WHNAT_DBG_LOU, "wdma: %p\n", wdma);
	WHNAT_DBG(WHNAT_DBG_OFF, "WDMA_PROC_BASIC\t: echo 0 > wdma\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WDMA_PROC_RX_CELL\t: echo 1 [ringid] [idx] > wdma\n");
	return 0;
}

/*
*
*/
static int wdma_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wdma_proc_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t wdma_proc_write(struct file *file, const char __user *buff,
							   size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char choice;
	struct wdma_entry *wdma = (struct wdma_entry *)PDE_DATA(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		choice = whnat_str_tol(value, &end, 10);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Choice=%d\n", __func__, choice);
		wdma_proc_handle(wdma, choice, value);
	}

	return len1;
}


/*
 * WED proc operation
*/
/*
*
*/
static int wed_proc_show(struct seq_file *seq, void *v)
{
	struct wed_entry *wed = (struct wed_entry *)seq->private;

	WHNAT_DBG(WHNAT_DBG_LOU, "wed: %p\n", wed);
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_RING_BASIC\t: echo 0 > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_BUF_BASIC\t: echo 1 > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_BUF_INFO\t: echo 2 [tkid] > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_RING_CELL\t: echo 3 [ringid] [idx] > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_RING_RAW\t: echo 4 [ringid] [idx] > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_DBG_INFO\t: echo 5 > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_DYNAMIC_FREE\t: echo 6 > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_DYNAMIC_ALLOC\t: echo 7 > wed\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "WED_PROC_TX_FREE_CNT\t: echo 8 > wed\n");
	return 0;
}

/*
*
*/
static int wed_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, wed_proc_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t wed_proc_write(struct file *file, const char __user *buff,
							  size_t len1, loff_t *ppos)
{
	char value[64];
	char *end;
	char choice;
	struct wed_entry *wed = (struct wed_entry *)PDE_DATA(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		choice = whnat_str_tol(value, &end, 10);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): Choice=%d\n", __func__, choice);
		wed_proc_handle(wed, choice, value);
	}

	return len1;
}

/*
* WHNAT
*/
/*
*
*/
static int whnat_proc_cr_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;
	/*usage*/
	WHNAT_DBG(WHNAT_DBG_OFF, "WED\t: base addr=%lx\n", whnat->wed.base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "WDMA\t: base addr=%lx\n", whnat->wdma.base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "WIFI\t: base addr=%lx\n", whnat->wifi.base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "echo [0:WED|1:WDMA|2:WIFI] [0:READ|1:WRITE] [ADDR] {VALUE} > cr\n");
	return 0;
}

/*
*
*/
static int whnat_proc_cr_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_cr_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t whnat_proc_cr_write(struct file *file, const char __user *buff,
								 size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char *token;
	char is_write;
	char type;
	char *s = value;
	unsigned long addr;
	unsigned int cr_value = 0;
	struct whnat_entry *whnat = (struct whnat_entry *)PDE_DATA(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		type = whnat_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		is_write = whnat_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		addr = whnat_str_tol(token, &end, 16);

		if (is_write) {
			token = strsep(&s, " ");
			cr_value = whnat_str_tol(token, &end, 16);
		}

		whnat_hal_cr_handle(whnat, type, is_write, addr, &cr_value);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s():Type=%d,IsWrite=%d,Addr=%lx, Value=%x\n",
				__func__, type, is_write, addr, cr_value);
	}

	return len1;
}

/*
* CTRL operate
*/
enum {
	TYPE_HW_TX_EN=0,
	TYPE_CR_MIRROR_EN=1,
};

/*
*
*/
static void whnat_proc_ctrl_handle(
	struct whnat_entry *entry,
	char type,
	char is_true)
{
	struct whnat_cfg *cfg = &entry->cfg;
	switch(type) {
	case TYPE_HW_TX_EN:
		if (cfg->hw_tx_en != is_true) {
			cfg->hw_tx_en = is_true;
		}
		break;
	case TYPE_CR_MIRROR_EN:
		if (cfg->cr_mirror_en != is_true) {
			wifi_chip_cr_mirror_set(&entry->wifi, is_true);
			cfg->cr_mirror_en = is_true;
		}
		break;
	default:
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): set wrong type: %d!\n", __func__, type);
		break;
	}
}

/*
*
*/
static int whnat_proc_ctrl_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;
	/*usage*/
	WHNAT_DBG(WHNAT_DBG_OFF, "HW_TX_EN\t: %s\n",
	whnat->cfg.hw_tx_en ? "TRUE" : "FALSE");
	WHNAT_DBG(WHNAT_DBG_OFF, "CR_MIRROR_EN\t: %s\n",
	whnat->cfg.cr_mirror_en ? "TRUE" : "FALSE");
	WHNAT_DBG(WHNAT_DBG_OFF, "echo [0:HW_TX_EN |1:CR_MIRROR_EN] [0:FALSE|1:TRUE] > ctrl\n");
	return 0;
}

/*
*
*/
static int whnat_proc_ctrl_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_ctrl_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t whnat_proc_ctrl_write(struct file *file, const char __user *buff,
								 size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char *token;
	char is_true;
	char type;
	char *s = value;
	struct whnat_entry *whnat = (struct whnat_entry *)PDE_DATA(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		type = whnat_str_tol(token, &end, 10);
		token = strsep(&s, " ");
		is_true = whnat_str_tol(token, &end, 10);

		whnat_proc_ctrl_handle(whnat, type, is_true);
		WHNAT_DBG(WHNAT_DBG_OFF, "%s():Type=%d,IsTrue=%d\n",
				__func__, type, is_true);
	}

	return len1;
}

/*
 * WHNAT trace tool operation
*/

/*
*
*/
static int whnat_proc_trace_show(struct seq_file *seq, void *v)
{
	struct whnat_ctrl *ctrl = (struct whnat_ctrl *)seq->private;
	struct whnat_cputracer *tracer = &ctrl->hif_cfg.tracer;

	WHNAT_DBG(WHNAT_DBG_OFF, "CFG:\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "Irq ID: %d\n", tracer->irq);
	WHNAT_DBG(WHNAT_DBG_OFF, "BaseAddr: %lx\n", tracer->base_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "Address: 0x%x\n", tracer->trace_addr);
	WHNAT_DBG(WHNAT_DBG_OFF, "Mask: 0x%x\n", tracer->trace_mask);
	WHNAT_DBG(WHNAT_DBG_OFF, "echo 0 [ 0 | 1 ] > trace, for enable or disable trace\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "echo 1 [ phy_addr] > trace, for set watch address\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "echo 2 [ mask ] > trace, for set watch mask\n");
	return 0;
}


/*
*
*/
static int whnat_proc_trace_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_trace_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t whnat_proc_trace_write(struct file *file, const char __user *buff,
									size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char choice;
	char *token;
	char *s = value;
	unsigned int tmp;
	struct whnat_ctrl *ctrl = (struct whnat_ctrl *)PDE_DATA(file_inode(file));
	struct whnat_cputracer *tracer = &ctrl->hif_cfg.tracer;

	if (buff && !copy_from_user(value, buff, len1)) {
		token = strsep(&s, " ");
		choice = whnat_str_tol(token, &end, 10);
		token = strsep(&s, " ");

		switch (choice) {
		case WHNAT_TRACE_EN:
			tmp = whnat_str_tol(token, &end, 10);
			tracer->trace_en = tmp;
			break;

		case WHNAT_TRACE_ADDR:
			tmp = whnat_str_tol(token, &end, 16);
			tracer->trace_addr = tmp;
			break;

		case WHNAT_TRACE_MSK:
			tmp = whnat_str_tol(token, &end, 16);
			tracer->trace_mask = tmp;
			break;
		}

		WHNAT_DBG(WHNAT_DBG_OFF, "Choice:%d\n", choice);
		WHNAT_DBG(WHNAT_DBG_OFF, "EN:%d\n", tracer->trace_en);
		WHNAT_DBG(WHNAT_DBG_OFF, "ADDR:%x\n", tracer->trace_addr);
		WHNAT_DBG(WHNAT_DBG_OFF, "MASK:%x\n", tracer->trace_mask);
		whnat_hal_trace_set(tracer);
	}

	return len1;
}

/*
*
*/
static int whnat_proc_state_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;

	whnat_proc_handle(whnat);
	WHNAT_DBG(WHNAT_DBG_OFF, "echo [LOG_LEVEL] > stat\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "LOG_LEVEL:\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "(0):WHNAT_DBG_OFF\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "(1):WHNAT_DBG_ERR\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "(2):WHNAT_DBG_INF\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "(3):WHNAT_DBG_LOU\n");
	return 0;
}

/*
*
*/
static int whnat_proc_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_state_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static ssize_t whnat_proc_state_write(struct file *file, const char __user *buff,
									size_t len1, loff_t *ppos)

{
	char value[64];
	char *end;
	char log;
	struct whnat_entry *whnat = (struct whnat_entry *)PDE_DATA(file_inode(file));

	if (buff && !copy_from_user(value, buff, len1)) {
		log = whnat_str_tol(value, &end, 10);

#ifdef ERR_RECOVERY
		switch (log) {
		case 10:
			whnat_hal_ser_trigger(whnat);
			return len1;
		case 11:
			wed_ser_dump(&whnat->wed);
			return len1;
		}
#endif /*ERR_RECOVERY*/
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): LogLevel=%d\n", __func__, log);
		whnat_log_set(log);
	}

	return len1;
}

/*
*
*/
static int whnat_proc_cfg_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;

	whnat_dump_cfg(whnat);
	return 0;
}

/*
*
*/
static int whnat_proc_cfg_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_cfg_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static int whnat_proc_tx_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;

	whnat_dump_txinfo(whnat);
	return 0;
}

/*
*
*/
static int whnat_proc_tx_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_tx_show, PDE_DATA(file_inode(file)));
}

/*
*
*/
static int whnat_proc_rx_show(struct seq_file *seq, void *v)
{
	struct whnat_entry *whnat = (struct whnat_entry *)seq->private;

	whnat_dump_rxinfo(whnat);
	return 0;
}

/*
*
*/
static int whnat_proc_rx_open(struct inode *inode, struct file *file)
{
	return single_open(file, whnat_proc_rx_show, PDE_DATA(file_inode(file)));
}

/*
 * global file operation
*/
static const struct file_operations proc_whnat_trace_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_trace_open,
	.write  = whnat_proc_trace_write,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_whnat_cr_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_cr_open,
	.write  = whnat_proc_cr_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_whnat_stat_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_state_open,
	.write  = whnat_proc_state_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_whnat_cfg_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_cfg_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_whnat_tx_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_tx_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_whnat_rx_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_rx_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_wed_fops = {
	.owner = THIS_MODULE,
	.open = wed_proc_open,
	.write  = wed_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_wdma_fops = {
	.owner = THIS_MODULE,
	.open = wdma_proc_open,
	.write  = wdma_proc_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations proc_ctrl_fops = {
	.owner = THIS_MODULE,
	.open = whnat_proc_ctrl_open,
	.write  = whnat_proc_ctrl_write,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
 *  proc register/unregister
*/
int wdma_entry_proc_init(struct whnat_entry *whnat, struct wdma_entry *wdma)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat->proc;
	struct proc_dir_entry *proc;

	proc = proc_create_data(PROC_WDMA_DIR, 0, root, &proc_wdma_fops, wdma);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!\n", PROC_WDMA_DIR);
		return -1;
	}

	wdma->proc = (void *)proc;
	WHNAT_DBG(WHNAT_DBG_INF, "create %s ok!\n", PROC_WDMA_DIR);
	return 0;
}

/*
*
*/
void wdma_entry_proc_exit(struct whnat_entry *whnat, struct wdma_entry *wdma)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)wdma->proc;

	if (proc && root)
		remove_proc_entry(PROC_WDMA_DIR, root);
}

/*
*
*/
int wed_entry_proc_init(struct whnat_entry *whnat, struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat->proc;
	struct proc_dir_entry *proc;

	proc = proc_create_data(PROC_WED_DIR, 0, root, &proc_wed_fops, wed);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!\n", PROC_WED_DIR);
		return -1;
	}

	wed->proc = (void *)proc;
	WHNAT_DBG(WHNAT_DBG_OFF, "create %s ok!!!\n", PROC_WED_DIR);
	return 0;
}

/*
*
*/
void wed_entry_proc_exit(struct whnat_entry *whnat, struct wed_entry *wed)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)wed->proc;

	if (proc && root)
		remove_proc_entry(PROC_WED_DIR, root);
}

/*
*
*/
int whnat_entry_proc_init(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat_ctrl->proc;
	struct proc_dir_entry *proc;
	char name[64] = "";

	snprintf(name, sizeof(name), "%s%d", PROC_WHNAT_DIR, whnat->idx);
	proc = proc_mkdir(name, root);
	whnat->proc = (void *)proc;
	proc = proc_create_data(PROC_STAT_DIR, 0, whnat->proc, &proc_whnat_stat_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_STAT_DIR);
		goto err1;
	}

	whnat->proc_stat = (void *)proc;
	proc = proc_create_data(PROC_CR_DIR, 0, whnat->proc, &proc_whnat_cr_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_CR_DIR);
		goto err2;
	}

	whnat->proc_cr = (void *)proc;
	proc = proc_create_data(PROC_CFG_DIR, 0, whnat->proc, &proc_whnat_cfg_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_CFG_DIR);
		goto err3;
	}

	whnat->proc_cfg = (void *)proc;
	proc = proc_create_data(PROC_TX_DIR, 0, whnat->proc, &proc_whnat_tx_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_TX_DIR);
		goto err4;
	}

	whnat->proc_tx = (void *)proc;
	proc = proc_create_data(PROC_RX_DIR, 0, whnat->proc, &proc_whnat_rx_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_RX_DIR);
		goto err4;
	}

	whnat->proc_rx = (void *)proc;
	proc = proc_create_data(PROC_CTRL_DIR, 0, whnat->proc, &proc_ctrl_fops, whnat);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_CTRL_DIR);
		goto err5;
	}

	whnat->proc_ctrl = (void *)proc;
	WHNAT_DBG(WHNAT_DBG_INF, "create %s ok!!!\n", name);
	return 0;
err5:
	remove_proc_entry(PROC_RX_DIR, whnat->proc);
err4:
	remove_proc_entry(PROC_TX_DIR, whnat->proc);
err3:
	remove_proc_entry(PROC_CR_DIR, whnat->proc);
err2:
	remove_proc_entry(PROC_STAT_DIR, whnat->proc);
err1:
	remove_proc_entry(name, root);
	return -1;
}

/*
*
*/
void whnat_entry_proc_exit(struct whnat_ctrl *whnat_ctrl, struct whnat_entry *whnat)
{
	struct proc_dir_entry *root = (struct proc_dir_entry *)whnat_ctrl->proc;
	struct proc_dir_entry *proc = (struct proc_dir_entry *)whnat->proc;
	char name[64] = "";

	if (whnat->proc_cfg) {
		remove_proc_entry(PROC_CFG_DIR, proc);
		whnat->proc_cfg = NULL;
	}

	if (whnat->proc_tx) {
		remove_proc_entry(PROC_TX_DIR, proc);
		whnat->proc_tx = NULL;
	}

	if (whnat->proc_rx) {
		remove_proc_entry(PROC_RX_DIR, proc);
		whnat->proc_rx = NULL;
	}

	if (whnat->proc_cr) {
		remove_proc_entry(PROC_CR_DIR, proc);
		whnat->proc_cr = NULL;
	}

	if (whnat->proc_stat) {
		remove_proc_entry(PROC_STAT_DIR, proc);
		whnat->proc_stat = NULL;
	}

	if (whnat->proc_ctrl) {
		remove_proc_entry(PROC_CTRL_DIR, proc);
		whnat->proc_ctrl = NULL;
	}

	snprintf(name, sizeof(name), "%s%d", PROC_WHNAT_DIR, whnat->idx);

	if (proc) {
		remove_proc_entry(name, root);
		whnat->proc = NULL;
	}
}

/*
*
*/
int whnat_ctrl_proc_init(struct whnat_ctrl *whnat_ctrl)
{
	struct proc_dir_entry *proc;

	proc = proc_mkdir(PROC_ROOT_DIR, NULL);

	if (!proc) {
		WHNAT_DBG(WHNAT_DBG_ERR, "create %s failed!!!\n", PROC_ROOT_DIR);
		return -1;
	}

	whnat_ctrl->proc = (void *)proc;
	proc = proc_create_data(PROC_TRACE_DIR, 0, whnat_ctrl->proc, &proc_whnat_trace_fops, whnat_ctrl);

	if (!proc)
		goto err1;

	whnat_ctrl->proc_trace = proc;
	WHNAT_DBG(WHNAT_DBG_OFF, "create %s ok!!!\n", PROC_ROOT_DIR);
	return 0;
err1:
	remove_proc_entry(PROC_ROOT_DIR, whnat_ctrl->proc);
	return 0;
}

/*
*
*/
void whnat_ctrl_proc_exit(struct whnat_ctrl *whnat_ctrl)
{
	struct proc_dir_entry *proc = (struct proc_dir_entry *)whnat_ctrl->proc;

	if (proc) {
		remove_proc_entry(PROC_TRACE_DIR, proc);
		remove_proc_entry(PROC_ROOT_DIR, NULL);
	}
}
