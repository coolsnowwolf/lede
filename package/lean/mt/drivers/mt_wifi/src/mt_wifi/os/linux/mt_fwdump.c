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

	Module Name:
	mt_fwdump.c
*/

#include "rt_config.h"

#ifdef FW_DUMP_SUPPORT

#define FW_DUMP_NAME	"fwdump_"
#define FW_DUMP_DIR		"MTxxxx"

static int fwdumpopen(struct inode *inode, struct file *fp)
{
	struct _RTMP_ADAPTER *pAd = 0;
#if (KERNEL_VERSION(3, 10, 0) > LINUX_VERSION_CODE)
	pAd = (struct _RTMP_ADAPTER *)PDE(fp->f_dentry->d_inode)->data;
#else
	pAd = (struct _RTMP_ADAPTER *)PDE_DATA(file_inode(fp));
#endif
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s fw_dump_size=%d\n", __func__, pAd->fw_dump_size));
	pAd->fw_dump_read = 0;
	return 0;
}

static ssize_t fwdumpread(struct file *fp, char __user *buf, size_t cnt, loff_t *pos)
{
	UINT32 len = cnt;
	struct _RTMP_ADAPTER *pAd = 0;
#if (KERNEL_VERSION(3, 10, 0) > LINUX_VERSION_CODE)
	pAd = (struct _RTMP_ADAPTER *)PDE(fp->f_dentry->d_inode)->data;
#else
	pAd = (struct _RTMP_ADAPTER *)PDE_DATA(file_inode(fp));
#endif

	if ((pAd->fw_dump_read + cnt) > pAd->fw_dump_size)
		len = (pAd->fw_dump_size - pAd->fw_dump_read);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("cnt=%d len=%d fw_dump_read=%d\n", cnt, len, pAd->fw_dump_read));
	os_move_mem(buf, pAd->fw_dump_buffer + pAd->fw_dump_read, len);
	pAd->fw_dump_read += len;
	return len;
}

static ssize_t fwdumpwrite(struct file *fp, const char __user *buf, size_t cnt, loff_t *pos)
{
	return 0;
}

static const struct file_operations fops_dump = {
	.owner = THIS_MODULE,
	.open = fwdumpopen,
	.read = fwdumpread,
	.write = fwdumpwrite,
};

BOOLEAN FWDumpProcInit(VOID *ptr)
{
	struct _RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ptr;
	POS_COOKIE pCookie;

	pCookie = (POS_COOKIE) pAd->OS_Cookie;
	os_move_mem(pCookie->fwdump_dir_name, FW_DUMP_DIR, sizeof(FW_DUMP_DIR));
	return TRUE;
}

BOOLEAN FWDumpProcCreate(VOID *ptr, PCHAR suffix)
{
	struct _RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ptr;
	POS_COOKIE pCookie;
	CHAR fwdump_file_name[20];

	pCookie = (POS_COOKIE) pAd->OS_Cookie;

	if (!pCookie->proc_fwdump_dir) {
		pCookie->proc_fwdump_dir = proc_mkdir(pCookie->fwdump_dir_name, NULL);

		if (pCookie->proc_fwdump_dir)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Create dir /proc/%s successfully\n", __func__, FW_DUMP_DIR));
		else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Create %s fail\n", __func__, FW_DUMP_DIR));
			return FALSE;
		}
	}

	if (!pCookie->proc_fwdump_file) {
		strcpy(fwdump_file_name, FW_DUMP_NAME);
		strcat(fwdump_file_name, pAd->net_dev->name);
		strcat(fwdump_file_name, suffix);
		pCookie->proc_fwdump_file = proc_create_data(fwdump_file_name, 0644, pCookie->proc_fwdump_dir, &fops_dump, (VOID *)pAd);

		if (pCookie->proc_fwdump_file)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Create file /proc/%s/%s successfully\n", __func__, pCookie->fwdump_dir_name, fwdump_file_name));
		else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Create %s fail\n", __func__, fwdump_file_name));
			return FALSE;
		}
	}

	return TRUE;
}

VOID FWDumpProcRemove(VOID *ptr)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)ptr;
	CHAR fwdump_file_name[20];
	POS_COOKIE pCookie;

	pCookie = (POS_COOKIE) pAd->OS_Cookie;
	strcpy(fwdump_file_name, FW_DUMP_NAME);
	strcat(fwdump_file_name, pAd->net_dev->name);

	if (pCookie->proc_fwdump_file)
		remove_proc_entry(fwdump_file_name, pCookie->proc_fwdump_dir);

	if (pCookie->proc_fwdump_dir)
		remove_proc_entry(pCookie->fwdump_dir_name, NULL);
}

INT os_set_fwdump_path(VOID *ptr1, VOID *ptr2)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)ptr1;
	RTMP_STRING *arg = (RTMP_STRING *)ptr2;
	POS_COOKIE pCookie;

	pCookie = (POS_COOKIE) pAd->OS_Cookie;
	os_move_mem(pCookie->fwdump_dir_name, arg, 10);
	return TRUE;
}

#endif

