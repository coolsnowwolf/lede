/*
  * Copyright (c) 2016 MediaTek Inc.  All rights reserved.
  *
  * This software is available to you under a choice of one of two
  * licenses.  You may choose to be licensed under the terms of the GNU
  * General Public License (GPL) Version 2, available from the file
  * COPYING in the main directory of this source tree, or the
  * BSD license below:
  *
  *     Redistribution and use in source and binary forms, with or
  *     without modification, are permitted provided that the following
  *     conditions are met:
  *
  *      - Redistributions of source code must retain the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer.
  *
  *      - Redistributions in binary form must reproduce the above
  *        copyright notice, this list of conditions and the following
  *        disclaimer in the documentation and/or other materials
  *        provided with the distribution.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  */

#ifdef WIFI_DIAG
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/bitops.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include "rt_config.h"
#include "rtmp.h"
#include "rtmp_comm.h"
#include "dot11_base.h"
#include "diag.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#define GET_PRIV_DATA_FROM_INODE(inode)		PDE_DATA(inode)
#define GET_PRIV_DATA_FROM_FILE(file)		PDE_DATA(file_inode(file))
#else
#define GET_PRIV_DATA_FROM_INODE(inode)		(PDE(inode)->data)
#define GET_PRIV_DATA_FROM_FILE(file)		(PDE(file->f_path.dentry->d_inode)->data)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
#define OS_FILE_SIZE(fd) (fd->f_dentry->d_inode->i_size)
#else
#define OS_FILE_SIZE(fd) (fd->f_path.dentry->d_inode->i_size)
#endif

#define DIAG_PROC_PATH "ctcwifi"
#define DIAG_VAR_PATH	"/var/ctcwifi"
#define DIAG_VAR_ASSOC_ERROR_2G_PATH	"/var/ctcwifi/association_errors_2G"
#define DIAG_VAR_LOG_PATH	"/var/ctcwifi/diag_log_2G"
#define DIAG_STAT	"Stat2G"
#define DIAG_DURA	"diag_duration_2G"
#define DIAG_EN		"diag_enable_2G"
#define DIAG_CH_OC	"channel_occupancy_2G"
#define DIAG_STA_ST	"stats_2G"

#define DIAG_STATUS_FILE	"/var/ctcwifi/mt_wifi_diag"

#define DIAG_MAX_PROCESS_NUM			(8)
#define DIAG_LOG_MAX_FRAME_SIZE			(2048)
#define DIAG_LOG_ENTRY_NUM				(1000)
#define DIAG_ASSOC_ERROR_ENTRY_NUM		(100)
#define DIAG_LOG_FILE_MAX_SIZE			(1024 * 1024) /* 1M bytes */

struct diag_wifi_process_entry {
	UCHAR name[RTMP_OS_TASK_NAME_LEN];
	INT32 pid;
	UCHAR is_process;
};

struct diag_wifi_process_info {
	UCHAR result;
	UCHAR errDesc[32];
	UCHAR num;
	struct diag_wifi_process_entry entry[DIAG_MAX_PROCESS_NUM];
};

struct diag_log_entry {
	struct rtc_time tm;
	UCHAR isTX;
	UCHAR ssid[32];
	UCHAR ssid_len;
	UCHAR band;
	UCHAR frame_type[32];
	UCHAR sta_addr[6];
	UINT32 data_len;
	UCHAR data[DIAG_LOG_MAX_FRAME_SIZE];
};

struct diag_assoc_error_entry {
	struct rtc_time tm;
	UCHAR StaAddr[6];
	UCHAR Ssid[32];
	ENUM_DIAG_CONN_ERROR_CODE errCode;
	UINT32 reason;
};

struct diag_proc_ch_occupancy {
	UINT32 ch;
	UINT32 band;
	UINT32 busy_time;
	UINT32 total_time;
};

struct diag_ctrl {
	UINT32  inited;
	UINT32	diag_duration;	/* default 60s, max 300s */
	UINT32	diag_enable;
	ULONG	diag_enable_time;
	struct diag_proc_ch_occupancy ch_occupancy;

	struct diag_wifi_process_info process_info;

	UINT32 diag_log_file_offset;
	struct diag_log_entry diag_log_entry[DIAG_LOG_ENTRY_NUM];
	UINT32 diag_log_read_idx;
	UINT32 diag_log_write_idx;
	NDIS_SPIN_LOCK diag_log_lock;

	UCHAR diag_log_tmp_buf[2*DIAG_LOG_MAX_FRAME_SIZE + 128];

	UINT32 assoc_error_file_offset;
	struct diag_assoc_error_entry assoc_error_entry[DIAG_ASSOC_ERROR_ENTRY_NUM];
	UINT32 assoc_error_read_idx;
	UINT32 assoc_error_write_idx;
	NDIS_SPIN_LOCK assoc_error_lock;
};


#define DIAG_INITED() ((NULL != diag_ctrl_db) && (diag_ctrl_db->inited == 1))

#define DIAG_ENABLE() ((NULL != diag_ctrl_db) && (diag_ctrl_db->inited == 1) && \
	((1 == diag_ctrl_db->diag_enable) || (2 == diag_ctrl_db->diag_enable)))

#define DIAG_ENABLE_CNTL_FRAME() (1 == diag_ctrl_db->diag_enable)

#define DIAG_IN_DURATION(now) \
	(RTMP_TIME_BEFORE(now, diag_ctrl_db->diag_enable_time + ((diag_ctrl_db->diag_duration) * OS_HZ)))

const PSTRING DIAG_CONN_ERR_INFO[DIAG_CONN_ERROR_MAX] = {
	"WiFi frames losing. reason code ",
	"Capability checking failed.\n",
	"Authentication failed. reason code ",
	"Rejected by blacklist.\n",
	"Rejected by STA limiation.\n",
	"De-authed. reason code ",
	"Association ignored by band steering.\n"
};

const UCHAR MAC_ADDR_ZERO[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct diag_ctrl *diag_ctrl_mem;
struct diag_ctrl *diag_ctrl_db;


static int stat2G_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)m->private;
	ULONG txCount = 0;
	ULONG rxCount = 0;
	ULONG txFails = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s pAd:%p\n", __func__, pAd));

	txCount = pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
	rxCount = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
	if (pAd->chipCap.hif_type == HIF_MT)
		txFails = pAd->WlanCounters.FailedCount.u.LowPart;
	else
		txFails = pAd->WlanCounters.RetryCount.u.LowPart + pAd->WlanCounters.FailedCount.u.LowPart;

	seq_printf(m, "tx_packets:%ld\n", txCount);
	seq_printf(m, "rx_packets:%ld\n", rxCount);
	seq_printf(m, "tx_fails:%ld\n", txFails);

	return 0;
}

static int stat2G_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, stat2G_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static int diag_duration_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)m->private;

	DBGPRINT(RT_DEBUG_TRACE, ("%s pAd:%p\n", __func__, pAd));
	seq_printf(m, "%d\n", diag_ctrl_db->diag_duration);
	return 0;
}

static int diag_duration_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_duration_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static ssize_t diag_duration_proc_write(struct file *file, const char __user *buffer,
	size_t count, loff_t *pos)
{
	unsigned int val = 0;
	UCHAR buf[10];

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, min(sizeof(buf), count)))
		return -EFAULT;

	val = simple_strtol(buf, NULL, 10);
	if ((val <= 300) && (val >= 60)) {
		diag_ctrl_db->diag_duration = val;
		DBGPRINT(RT_DEBUG_TRACE, ("%s set duraion %d.\n", __func__, val));
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("%s rang error, valid rang is [60,300].\n", __func__));

	return count;
}

static int diag_enable_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)m->private;

	DBGPRINT(RT_DEBUG_TRACE, ("%s pAd:%p\n", __func__, pAd));
	seq_printf(m, "%d\n", diag_ctrl_db->diag_enable);
	return 0;
}

static int diag_enable_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, diag_enable_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

#ifdef CONFIG_SNIFFER_SUPPORT
static INT diag_set_monitor_mode(
	IN RTMP_ADAPTER	*pAd,
	IN UCHAR MonitorMode)
{
	if (!pAd)
		return FALSE;

	pAd->monitor_ctrl.current_monitor_mode = MonitorMode;
	if (pAd->monitor_ctrl.current_monitor_mode > MONITOR_MODE_FULL ||
		pAd->monitor_ctrl.current_monitor_mode < MONITOR_MODE_OFF)
		pAd->monitor_ctrl.current_monitor_mode = MONITOR_MODE_OFF;
		DBGPRINT(RT_DEBUG_TRACE,
			("set Current Monitor Mode = %d , range(%d ~ %d)\n",
			pAd->monitor_ctrl.current_monitor_mode, MONITOR_MODE_OFF, MONITOR_MODE_FULL));

	switch (pAd->monitor_ctrl.current_monitor_mode) {
	case MONITOR_MODE_OFF:			/*reset to normal */
		pAd->ApCfg.BssType = BSS_INFRA;
		AsicSetRxFilter(pAd);
		break;
	case MONITOR_MODE_REGULAR_RX:	/*report probe_request only , normal rx filter */
		pAd->ApCfg.BssType = BSS_MONITOR;
		break;
	case MONITOR_MODE_FULL:			/*fully report, Enable Rx with promiscuous reception */
		pAd->ApCfg.BssType = BSS_MONITOR;
		AsicSetRxFilter(pAd);
		break;
	}
	return TRUE;
}
#endif /* CONFIG_SNIFFER_SUPPORT */

static ssize_t diag_enable_proc_write(struct file *file, const char __user *buffer,
			       size_t count, loff_t *pos)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	unsigned int val = 0;
	UCHAR buf[10];
#ifdef CONFIG_SNIFFER_SUPPORT
	PRTMP_ADAPTER pAd = NULL;
#endif

	if (!file || !buffer)
		return 0;

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, min(sizeof(buf), count)))
		return -EFAULT;

	/* get diag_enable */
	val = simple_strtol(buf, NULL, 10);
	if ((0 == val) || (1 == val) || (2 == val)) {
		/* stop diag_log handling */
		diag_ctrl_db->diag_enable = 0;
		OS_WAIT(500);

#ifdef CONFIG_SNIFFER_SUPPORT
		pAd = (PRTMP_ADAPTER)GET_PRIV_DATA_FROM_FILE(file);
		DBGPRINT(RT_DEBUG_ERROR, ("7603 %s pAd:%p\n", __func__, pAd));
		if ((val == 2) || (val == 0)) { /* normal mode */
			diag_set_monitor_mode(pAd, MONITOR_MODE_OFF);
		} else { /* sniffer mode */
			diag_set_monitor_mode(pAd, MONITOR_MODE_FULL);
		}
#endif

		if (val !=  0) {
			/* when enable, flush diag_log_queue & /var/.../diag_log file here */
			DBGPRINT(RT_DEBUG_ERROR, ("Flush diag_log file\n"));
			memset(diag_ctrl_db->diag_log_entry, 0, DIAG_LOG_ENTRY_NUM*sizeof(struct diag_log_entry));

			RtmpOSFSInfoChange(&osFSInfo, TRUE);
			fd = RtmpOSFileOpen(DIAG_VAR_LOG_PATH, O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
			if (!IS_FILE_OPEN_ERR(fd))
				RtmpOSFileClose(fd);
			else
				DBGPRINT(RT_DEBUG_ERROR, ("Open file %s ERROR", DIAG_VAR_LOG_PATH));
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			diag_ctrl_db->diag_log_file_offset = 0;

			NdisGetSystemUpTime(&diag_ctrl_db->diag_enable_time);
		}

		diag_ctrl_db->diag_enable = val;
	}

	return count;
}

static int ch_occupancy_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)m->private;
	UINT32 ch_occ = (100 * diag_ctrl_db->ch_occupancy.busy_time)/1000;

	DBGPRINT(RT_DEBUG_TRACE, ("%s pAd:%p\n", __func__, pAd));

	seq_printf(m, "Channel\tBand\tOccupancy\n");
	seq_printf(m, "%-7d\t%-4s\t%-9d\n", pAd->CommonCfg.Channel, "2G", ch_occ);

	return 0;
}

static int ch_occupancy_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ch_occupancy_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

static int stat_proc_show(struct seq_file *m, void *v)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)m->private;
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT i = 0;
	UINT32 pecent;
	UINT32 deci;

	DBGPRINT(RT_DEBUG_TRACE, ("%s pAd:%p\n", __func__, pAd));

	seq_printf(m, "STA:             \tSNR  \tRSSI   \tTXSUCC\n");
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (pEntry->EntryType != ENTRY_CLIENT)
			continue;
		seq_printf(m, "%02X:%02X:%02X:%02X:%02X:%02X", PRINT_MAC(pEntry->Addr));
		seq_printf(m, "\t%d", RTMPMinSnr(pAd, pEntry->RssiSample.AvgSnr[0], pEntry->RssiSample.AvgSnr[1]));
		seq_printf(m, "\t%d", RTMPAvgRssi(pAd, &pEntry->RssiSample));
		if ((pEntry->diag_tx_count > 0) && (pEntry->diag_tx_succ_count <= pEntry->diag_tx_count)) {
			pecent = (UINT32)((100 * pEntry->diag_tx_succ_count)/pEntry->diag_tx_count);
			deci = (UINT32)((100 * pEntry->diag_tx_succ_count)%pEntry->diag_tx_count);
			deci = (UINT32)((100 * deci)/pEntry->diag_tx_count);
		} else {
			pecent = 100;
			deci = 0;
		}
		seq_printf(m, "\t%d.%02d\n", pecent, deci);
	}
	seq_printf(m, "RXCRCERR: %d\n", pAd->WlanCounters.FCSErrorCount.u.LowPart);

	return 0;
}

static int stat_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, stat_proc_show, GET_PRIV_DATA_FROM_INODE(inode));
}

void DiagAssocErrorFileWrite(void)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR buf[256];
	UINT32 buf_size = 256;
	struct diag_assoc_error_entry *log_entry;
	UINT32 write_size;

	if (!DIAG_INITED())
		return;

	memset(buf, 0, sizeof(buf));
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_VAR_ASSOC_ERROR_2G_PATH, O_WRONLY, 0);
	if (IS_FILE_OPEN_ERR(fd)) {
		fd = RtmpOSFileOpen(DIAG_VAR_ASSOC_ERROR_2G_PATH, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (IS_FILE_OPEN_ERR(fd)) {
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			return;
		}
		diag_ctrl_db->assoc_error_file_offset = 0;
	}

	RtmpOSFileSeek(fd, diag_ctrl_db->assoc_error_file_offset);

	while (diag_ctrl_db->assoc_error_write_idx != diag_ctrl_db->assoc_error_read_idx) {

		if (!DIAG_INITED())
			break;

		log_entry = &diag_ctrl_db->assoc_error_entry[diag_ctrl_db->assoc_error_read_idx];

		snprintf(buf + strlen(buf),
			buf_size -  strlen(buf),
			"%04d-%02d-%02d %02d:%02d:%02d",
			log_entry->tm.tm_year+1900,
			log_entry->tm.tm_mon+1,
			log_entry->tm.tm_mday,
			log_entry->tm.tm_hour,
			log_entry->tm.tm_min,
			log_entry->tm.tm_sec);

		snprintf(buf + strlen(buf),
			buf_size -  strlen(buf),
			" %02x:%02x:%02x:%02x:%02x:%02x %s: %s",
			PRINT_MAC(log_entry->StaAddr),
			log_entry->Ssid,
			DIAG_CONN_ERR_INFO[log_entry->errCode]);

		if (DIAG_CONN_DEAUTH == log_entry->errCode ||
			DIAG_CONN_AUTH_FAIL == log_entry->errCode ||
			DIAG_CONN_FRAME_LOST == log_entry->errCode)
			snprintf(buf + strlen(buf),
				buf_size -  strlen(buf),
				"%d\n", log_entry->reason);

		write_size = strlen(buf);
		if ((diag_ctrl_db->assoc_error_file_offset + write_size) > DIAG_LOG_FILE_MAX_SIZE) {
			diag_ctrl_db->assoc_error_file_offset = 0; /* overwrite from file start */
			RtmpOSFileSeek(fd, 0);
		}
		RtmpOSFileWrite(fd, buf, write_size);
		diag_ctrl_db->assoc_error_file_offset += write_size;

		memset(buf, 0, buf_size);
		memset(log_entry, 0, sizeof(struct diag_assoc_error_entry));

		diag_ctrl_db->assoc_error_read_idx =
			(diag_ctrl_db->assoc_error_read_idx + 1) % DIAG_ASSOC_ERROR_ENTRY_NUM;
	}

	RtmpOSFileClose(fd);
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}

void DiagConnError(PRTMP_ADAPTER pAd, UCHAR apidx, UCHAR *addr,
	ENUM_DIAG_CONN_ERROR_CODE Code, UINT32 Reason)
{
	ENUM_DIAG_CONN_ERROR_CODE errCode = 0;
	struct timex  txc;
	struct diag_assoc_error_entry *entry;
	ULONG flags;

	if (!DIAG_INITED())
		return;

	if (!pAd || !addr)
		return;

	if (!(VALID_MBSS(pAd, apidx) && (apidx < pAd->ApCfg.BssidNum)))
		return;

	if (Code >= DIAG_CONN_ERROR_MAX) {
		switch (Reason) {
		case REASON_4_WAY_TIMEOUT:
		case REASON_GROUP_KEY_HS_TIMEOUT:
		case REASON_MIC_FAILURE:
		case REASON_STA_REQ_ASSOC_NOT_AUTH:
		case REASON_8021X_AUTH_FAIL:
		case REASON_IE_DIFFERENT:
		case REASON_MCIPHER_NOT_VALID:
		case REASON_UCIPHER_NOT_VALID:
		case REASON_AKMP_NOT_VALID:
		case REASON_UNSUPPORT_RSNE_VER:
		case REASON_CIPHER_SUITE_REJECTED:
			errCode = DIAG_CONN_AUTH_FAIL;
			break;
		case REASON_NO_LONGER_VALID:
			errCode = DIAG_CONN_FRAME_LOST;
			break;
		case REASON_INVALID_IE:
		case REASON_INVALID_RSNE_CAP:
			errCode = DIAG_CONN_CAP_ERROR;
			break;
		case REASON_DECLINED:
			errCode = DIAG_CONN_ACL_BLK;
			break;
		default:
			errCode = DIAG_CONN_DEAUTH;
			break;
		}
	} else
		errCode = Code;

	RTMP_IRQ_LOCK(&diag_ctrl_db->assoc_error_lock, flags);

	if (((diag_ctrl_db->assoc_error_write_idx + 1) % DIAG_ASSOC_ERROR_ENTRY_NUM) !=
		diag_ctrl_db->assoc_error_read_idx) {

		entry = &diag_ctrl_db->assoc_error_entry[diag_ctrl_db->assoc_error_write_idx];
		memset(entry, 0, sizeof(*entry));

		do_gettimeofday(&(txc.time));
	    txc.time.tv_sec -= sys_tz.tz_minuteswest * 60;
	    rtc_time_to_tm(txc.time.tv_sec, &entry->tm);

		memcpy(entry->StaAddr, addr, 6);
		if (pAd->ApCfg.MBSSID[apidx].SsidLen > 0 && pAd->ApCfg.MBSSID[apidx].SsidLen <= 32)
			memcpy(entry->Ssid, pAd->ApCfg.MBSSID[apidx].Ssid, pAd->ApCfg.MBSSID[apidx].SsidLen);
		entry->errCode = errCode;
		entry->reason = Reason;

		/* update write_idx */
		diag_ctrl_db->assoc_error_write_idx =
			(diag_ctrl_db->assoc_error_write_idx + 1) % DIAG_ASSOC_ERROR_ENTRY_NUM;
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, assoc_error_entry array full\n", __func__));
	}

	RTMP_IRQ_UNLOCK(&diag_ctrl_db->assoc_error_lock, flags);
}

void diag_get_frame_info(
	PHEADER_802_11 pHeader, UCHAR isTx,
	UCHAR **pp_frame_type, UCHAR **pp_sta_addr)
{
	if  (!pHeader || !pp_frame_type || !pp_sta_addr)
		return;

	*pp_frame_type = NULL;
	*pp_sta_addr = NULL;

	if (pHeader->FC.Type == FC_TYPE_MGMT) {

		if (isTx)
			*pp_sta_addr = pHeader->Addr1;
		else
			*pp_sta_addr = pHeader->Addr2;

		switch (pHeader->FC.SubType) {
		case SUBTYPE_BEACON:
			/* just record our beacon */
			if (isTx)
				*pp_frame_type = "Beacon";
			break;

		case SUBTYPE_PROBE_REQ:
			/* just record peer's probe request */
			if (!isTx)
				*pp_frame_type = "Probe Request";
			break;

		case SUBTYPE_PROBE_RSP:
			/* just record our probe response */
			if (isTx)
				*pp_frame_type = "Probe Response";
			break;

		case SUBTYPE_ATIM:
			*pp_frame_type = "ATIM";
			break;

		case SUBTYPE_DISASSOC:
			*pp_frame_type = "Disassociation";
			break;

		case SUBTYPE_DEAUTH:
			*pp_frame_type = "Deauthentication";
			break;

		case SUBTYPE_ASSOC_REQ:
			/* just record peer's Association request */
			if (!isTx)
				*pp_frame_type = "Association Request";
			break;

		case SUBTYPE_ASSOC_RSP:
			/* just record our Association response */
			if (isTx)
				*pp_frame_type = "Association Response";
			break;

		case SUBTYPE_REASSOC_REQ:
			/* just record peer's Reassociation request */
			if (!isTx)
				*pp_frame_type = "Reassociation Request";
			break;

		case SUBTYPE_REASSOC_RSP:
			/* just record our Reassociation response */
			if (isTx)
				*pp_frame_type = "Reassociation Response";
			break;

		case SUBTYPE_AUTH:
			*pp_frame_type = "Authentication";
			break;

		default:
			DBGPRINT(RT_DEBUG_TRACE, ("%s, not handled MGMT frme, subtype=%d\n",
				__func__, pHeader->FC.SubType));
			break;
		}
	} else if (pHeader->FC.Type == FC_TYPE_CNTL) {

		switch (pHeader->FC.SubType) {
		case SUBTYPE_RTS:
			*pp_frame_type = "RTS";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			else
				*pp_sta_addr = pHeader->Addr2;
			break;

		case SUBTYPE_CTS:
			*pp_frame_type = "CTS";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			break;

		case SUBTYPE_ACK:
			*pp_frame_type = "ACK";
			if (isTx)
				*pp_sta_addr = pHeader->Addr1;
			break;

		default:
			DBGPRINT(RT_DEBUG_TRACE, ("%s, not handled CNTL frme, subtype=%d\n",
				__func__, pHeader->FC.SubType));
			break;
		}
	}
}

void DiagFrameCache(DIAG_FRAME_INFO *info)
{
	ULONG now;
	struct diag_log_entry *entry;
	UCHAR *p_frame_type = NULL;
	UCHAR *p_sta_addr = NULL;
	struct timex  txc;
	ULONG flags;

	NdisGetSystemUpTime(&now);
	if (!DIAG_ENABLE() || !DIAG_IN_DURATION(now))
		return;

	if (!info)
		return;

	diag_get_frame_info((PHEADER_802_11)info->pData, info->isTX, &p_frame_type, &p_sta_addr);
	if (!p_frame_type)
		return;

	RTMP_IRQ_LOCK(&diag_ctrl_db->diag_log_lock, flags);

	if (((diag_ctrl_db->diag_log_write_idx + 1) % DIAG_LOG_ENTRY_NUM)
		!= diag_ctrl_db->diag_log_read_idx) {

		entry = &diag_ctrl_db->diag_log_entry[diag_ctrl_db->diag_log_write_idx];
		memset(entry, 0, sizeof(*entry));

		do_gettimeofday(&(txc.time));
	    txc.time.tv_sec -= sys_tz.tz_minuteswest * 60;
	    rtc_time_to_tm(txc.time.tv_sec, &entry->tm);

		entry->isTX = info->isTX;
		if (info->ssid_len <= 32)
			entry->ssid_len = info->ssid_len;
		else
			entry->ssid_len = 32;
		memcpy(entry->ssid, info->ssid, entry->ssid_len);

		entry->band = info->band;

		if (strlen(p_frame_type) < 32)
			memcpy(entry->frame_type, p_frame_type, strlen(p_frame_type));
		else
			memcpy(entry->frame_type, p_frame_type, 32-1);

		if (p_sta_addr)
			memcpy(entry->sta_addr, p_sta_addr, 6);

		entry->data_len =
			(info->dataLen > DIAG_LOG_MAX_FRAME_SIZE) ? DIAG_LOG_MAX_FRAME_SIZE : info->dataLen;
		memcpy(entry->data, info->pData, entry->data_len);

		/* update write_idx */
		diag_ctrl_db->diag_log_write_idx =
			(diag_ctrl_db->diag_log_write_idx + 1) % DIAG_LOG_ENTRY_NUM;
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_log_entry array full\n", __func__));

	RTMP_IRQ_UNLOCK(&diag_ctrl_db->diag_log_lock, flags);
}

void DiagLogFileWrite(void)
{
	RTMP_OS_FD fd = NULL;
	RTMP_OS_FS_INFO osFSInfo;
	UCHAR *buf = NULL;
	UINT32 buf_size = 0;
	struct diag_log_entry *log_entry;
	UINT32 write_size;

	UCHAR *pData = NULL;
	UINT32 dataLen = 0, index = 0;
	UINT32 log_line_len = 0;

	if (!DIAG_ENABLE())
		return;

	buf = diag_ctrl_db->diag_log_tmp_buf;
	buf_size = 2*DIAG_LOG_MAX_FRAME_SIZE + 128;
	memset(buf, 0, buf_size);

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_VAR_LOG_PATH, O_WRONLY, 0);
	if (IS_FILE_OPEN_ERR(fd)) {
		fd = RtmpOSFileOpen(DIAG_VAR_LOG_PATH, O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (IS_FILE_OPEN_ERR(fd)) {
			RtmpOSFSInfoChange(&osFSInfo, FALSE);
			return;
		}
		diag_ctrl_db->diag_log_file_offset = 0;
	}

	RtmpOSFileSeek(fd, diag_ctrl_db->diag_log_file_offset);

	while (diag_ctrl_db->diag_log_write_idx != diag_ctrl_db->diag_log_read_idx) {

		if (!DIAG_ENABLE())
			break;

		log_entry = &diag_ctrl_db->diag_log_entry[diag_ctrl_db->diag_log_read_idx];

		/* pData include 80211 header, because CNTL frame just include FCS except the 80211 header */
		pData = log_entry->data;
		dataLen = log_entry->data_len;

		/* check diag_log_buf is enough */
		log_line_len =
			(strlen("YYYY-MM-DD HH:MM:SS ")) +
			(log_entry->ssid_len) +
			((log_entry->isTX) ? strlen(" sends ") : strlen(" receives ")) +
			(strlen(log_entry->frame_type)) +
			((log_entry->isTX) ? strlen(" to ") : strlen(" from ")) +
			((!MAC_ADDR_EQUAL(log_entry->sta_addr, MAC_ADDR_ZERO))
			? strlen("112233445566") : strlen("No-STA-Addr")) +
			(strlen("[]")) +
			(dataLen * 2) +
			(strlen("\n"));

		if ((buf_size - 1) < log_line_len) {
			memset(log_entry, 0, sizeof(struct diag_log_entry));
			memset(buf, 0, buf_size);
			diag_ctrl_db->diag_log_read_idx =
				(diag_ctrl_db->diag_log_read_idx + 1) % DIAG_LOG_ENTRY_NUM;
			DBGPRINT(RT_DEBUG_ERROR, ("%s, buf_size is not enough, log_line_len=%d!!!\n",
				__func__, log_line_len));
			continue;
		}

		/* time */
		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf),
				buf_size -  strlen(buf),
				"%04d-%02d-%02d %02d:%02d:%02d ",
				log_entry->tm.tm_year+1900,
				log_entry->tm.tm_mon+1,
				log_entry->tm.tm_mday,
				log_entry->tm.tm_hour,
				log_entry->tm.tm_min,
				log_entry->tm.tm_sec);

		/* SSID */
		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf), buf_size -  strlen(buf), log_entry->ssid);

		/* sends/receives */
		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf),
				buf_size -  strlen(buf), (log_entry->isTX) ?  " sends " : " receives ");

		/* frame type */
		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf), buf_size -  strlen(buf), log_entry->frame_type);

		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf), buf_size -  strlen(buf), (log_entry->isTX) ?  " to " : " from ");

		/* STA MAC address */
		if (strlen(buf) < (buf_size - 1)) {
			if (!MAC_ADDR_EQUAL(log_entry->sta_addr, MAC_ADDR_ZERO))
				snprintf(buf + strlen(buf),
					buf_size -  strlen(buf),
					"%02X%02X%02X%02X%02X%02X",
					log_entry->sta_addr[0], log_entry->sta_addr[1],
					log_entry->sta_addr[2], log_entry->sta_addr[3],
					log_entry->sta_addr[4], log_entry->sta_addr[5]);
			else
				snprintf(buf + strlen(buf),
					buf_size -  strlen(buf),
					"%s", "No-STA-Addr");
		}

		/* frame body */
		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf), buf_size -  strlen(buf), "[");

		for (index = 0; index < dataLen; index++) {
			if (strlen(buf) < (buf_size - 1))
				snprintf(buf + strlen(buf), buf_size -  strlen(buf), "%02X", pData[index]);
		}

		if (strlen(buf) < (buf_size - 1))
			snprintf(buf + strlen(buf), buf_size -  strlen(buf), "]\n");

		write_size = strlen(buf);
		if ((diag_ctrl_db->diag_log_file_offset + write_size) > DIAG_LOG_FILE_MAX_SIZE) {
			diag_ctrl_db->diag_log_file_offset = 0; /* overwrite from file start */
			RtmpOSFileSeek(fd, 0);
		}
		RtmpOSFileWrite(fd, buf, write_size);
		diag_ctrl_db->diag_log_file_offset += write_size;

		memset(buf, 0, buf_size);
		memset(log_entry, 0, sizeof(struct diag_log_entry));

		/* update read_idx */
		diag_ctrl_db->diag_log_read_idx =
			(diag_ctrl_db->diag_log_read_idx + 1) % DIAG_LOG_ENTRY_NUM;
	}

	RtmpOSFileClose(fd);
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
}

void DiagMlmeEnqueue(PRTMP_ADAPTER pAd)
{
	MlmeEnqueue(pAd, WIFI_DAIG_STATE_MACHINE, 0, 0, NULL, 0);
}

void DiagApMlmeOneSecProc(PRTMP_ADAPTER pAd)
{
	UINT32	value;

	if (!pAd)
		return;

	if (!DIAG_INITED()) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s DIAG NOT INITED\n", __func__));
		return;
	}

	value = pAd->OneSecChBusyTime;
	diag_ctrl_db->ch_occupancy.busy_time = value>>10; /* translate us to ms */
	DBGPRINT(RT_DEBUG_INFO, ("%s update busy_time %d ms\n",
		__func__, diag_ctrl_db->ch_occupancy.busy_time));

	DiagMlmeEnqueue(pAd);
}

VOID DiagGetProcessInfo(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	struct diag_wifi_process_info process_info;
	UCHAR index = 0;

	if (!wrq)
		return;

	if (!DIAG_INITED())
		return;

	memset(&process_info, 0, sizeof(process_info));
	for (index = 0; index < DIAG_MAX_PROCESS_NUM; index++) {
		if (diag_ctrl_db->process_info.entry[index].pid != -1) {
			memcpy(&process_info.entry[process_info.num],
				&diag_ctrl_db->process_info.entry[index], sizeof(struct diag_wifi_process_entry));
			process_info.num++;
		}
	}

	wrq->u.data.length = sizeof(process_info);
	if (copy_to_user(wrq->u.data.pointer, &process_info, wrq->u.data.length))
		DBGPRINT(RT_DEBUG_ERROR, ("%s: copy_to_user() fail\n", __func__));
}

void DiagAddPid(INT pid, PUCHAR name)
{
	UCHAR index = 0;

	if (DIAG_INITED()) {
		for (index = 0; index < DIAG_MAX_PROCESS_NUM; index++) {
			if (diag_ctrl_db->process_info.entry[index].pid == -1) {
				diag_ctrl_db->process_info.entry[index].is_process = 0;
				diag_ctrl_db->process_info.entry[index].pid = pid;
				memcpy(diag_ctrl_db->process_info.entry[index].name, name,
					(strlen(name) > (RTMP_OS_TASK_NAME_LEN-1))
					? (RTMP_OS_TASK_NAME_LEN-1) : strlen(name));
				diag_ctrl_db->process_info.num++;
				diag_ctrl_db->process_info.result = 0;
				DBGPRINT(RT_DEBUG_ERROR, ("%s, add PID=%d, name=%s, total_num=%d\n",
					__func__, diag_ctrl_db->process_info.entry[index].pid,
					diag_ctrl_db->process_info.entry[index].name,
					diag_ctrl_db->process_info.num));
				break;
			}
		}
	}
}

void DiagDelPid(INT32 pid)
{
	UCHAR index = 0;

	if (DIAG_INITED()) {
		for (index = 0; index < DIAG_MAX_PROCESS_NUM; index++) {
			if (diag_ctrl_db->process_info.entry[index].pid == pid) {
				diag_ctrl_db->process_info.entry[index].is_process = 0;
				diag_ctrl_db->process_info.entry[index].pid = -1;
				memset(diag_ctrl_db->process_info.entry[index].name,
					0, RTMP_OS_TASK_NAME_LEN);
				diag_ctrl_db->process_info.num--;
				diag_ctrl_db->process_info.result = 0;
				DBGPRINT(RT_DEBUG_ERROR, ("%s, delete PID=%d, total_num=%d\n",
					__func__, pid, diag_ctrl_db->process_info.num));
				break;
			}
		}

	}
}

void DiagMiniportMMRequest(PRTMP_ADAPTER pAd, UCHAR *pData, UINT Length)
{
	PHEADER_802_11 pHdr = NULL;
	BSS_STRUCT *pMbssEntry = NULL;
	MAC_TABLE_ENTRY *pMacTblEntry = NULL;
	DIAG_FRAME_INFO diag_info;

	if (!DIAG_ENABLE())
		return;

	if (!pAd || !pData || (Length == 0))
		return;

	memset(&diag_info, 0, sizeof(diag_info));
	pHdr = (PHEADER_802_11) pData;
	pMacTblEntry = MacTableLookup(pAd, pHdr->Addr1);

	if (IS_VALID_ENTRY(pMacTblEntry) && IS_ENTRY_CLIENT(pMacTblEntry) &&
		VALID_MBSS(pAd, pMacTblEntry->func_tb_idx) &&
		(pMacTblEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbssEntry = &pAd->ApCfg.MBSSID[pMacTblEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pHdr->Addr2, MAC_ADDR_LEN)) {
					pMbssEntry = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbssEntry == NULL) && (first_up_idx != 0xFF))
			pMbssEntry = &pAd->ApCfg.MBSSID[first_up_idx];
	}
	if (pMbssEntry) {
		diag_info.ssid = pMbssEntry->Ssid;
		diag_info.ssid_len = pMbssEntry->SsidLen;
		diag_info.isTX = 1;
		diag_info.band = (pAd->LatchRfRegs.Channel > 14) ? 1 : 0;
		diag_info.pData = pData;
		diag_info.dataLen = Length;
		DiagFrameCache(&diag_info);
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("%s, Error, no UP ap interface!!!\n", __func__));
}

void DiagDevRxMgmtFrm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	DIAG_FRAME_INFO diag_info;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_STRUCT *pMbss = NULL;
	HEADER_802_11 *pHeader;

	if (!DIAG_ENABLE())
		return;

	if (!pAd || !pRxBlk)
		return;

	pHeader = pRxBlk->pHeader;

	/* frame type & subtype check */
	if ((pHeader->FC.Type != FC_TYPE_MGMT) || (
		(pHeader->FC.SubType != SUBTYPE_BEACON) &&
		(pHeader->FC.SubType != SUBTYPE_PROBE_REQ) &&
		(pHeader->FC.SubType != SUBTYPE_PROBE_RSP) &&
		(pHeader->FC.SubType != SUBTYPE_ATIM) &&
		(pHeader->FC.SubType != SUBTYPE_DISASSOC) &&
		(pHeader->FC.SubType != SUBTYPE_DEAUTH) &&
		(pHeader->FC.SubType != SUBTYPE_ASSOC_REQ) &&
		(pHeader->FC.SubType != SUBTYPE_ASSOC_RSP) &&
		(pHeader->FC.SubType != SUBTYPE_REASSOC_REQ) &&
		(pHeader->FC.SubType != SUBTYPE_REASSOC_RSP) &&
		(pHeader->FC.SubType != SUBTYPE_AUTH))) {
		return;
	}

	memset(&diag_info, 0, sizeof(diag_info));
	diag_info.isTX = 0;
	diag_info.band = (pAd->LatchRfRegs.Channel > 14) ? 1 : 0;
	diag_info.pData = pRxBlk->pData;
	diag_info.dataLen = pRxBlk->DataSize;

	if (VALID_WCID(pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if ((IS_VALID_ENTRY(pEntry)) && (IS_ENTRY_CLIENT(pEntry))
		&& VALID_MBSS(pAd, pEntry->func_tb_idx)
		&& (pEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					pHeader->Addr1, MAC_ADDR_LEN)) {
					pMbss = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbss == NULL) && (first_up_idx != 0xFF))
			pMbss = &pAd->ApCfg.MBSSID[first_up_idx];
	}
	if (pMbss) {
		diag_info.ssid = pMbss->Ssid;
		diag_info.ssid_len = pMbss->SsidLen;
		DiagFrameCache(&diag_info);
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("%s, Error, no UP ap interface!!!\n", __func__));
}

#ifdef CONFIG_SNIFFER_SUPPORT
void DiagDevRxCntlFrm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	DIAG_FRAME_INFO diag_info;
	MAC_TABLE_ENTRY *pEntry = NULL;
	BSS_STRUCT *pMbss = NULL;
	HEADER_802_11 *pHeader;

	if (!DIAG_ENABLE() || !DIAG_ENABLE_CNTL_FRAME())
		return;

	if (!pAd || !pRxBlk)
		return;

	pHeader = pRxBlk->pHeader;

	/* frame type & subtype check */
	if ((pHeader->FC.Type != FC_TYPE_CNTL) || (
		(pHeader->FC.SubType != SUBTYPE_RTS) &&
		(pHeader->FC.SubType != SUBTYPE_CTS) &&
		(pHeader->FC.SubType != SUBTYPE_ACK))) {
		return;
	}

	memset(&diag_info, 0, sizeof(diag_info));
	diag_info.isTX = 0;
	diag_info.band = (pAd->LatchRfRegs.Channel > 14) ? 1 : 0;
	diag_info.pData = pRxBlk->pData;
	diag_info.dataLen = pRxBlk->DataSize;

	if (VALID_WCID(pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	else
		pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if ((IS_VALID_ENTRY(pEntry)) && (IS_ENTRY_CLIENT(pEntry))
		&& VALID_MBSS(pAd, pEntry->func_tb_idx)
		&& (pEntry->func_tb_idx < pAd->ApCfg.BssidNum))
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	else {
		UCHAR apidx, first_up_idx = 0xFF;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))) {
				if (RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid,
					pHeader->Addr1, MAC_ADDR_LEN)) {
					pMbss = &pAd->ApCfg.MBSSID[apidx];
					break;
				}
				if (first_up_idx == 0xFF)
					first_up_idx = apidx;
			}
		}
		if ((pMbss == NULL) && (first_up_idx != 0xFF))
			pMbss = &pAd->ApCfg.MBSSID[first_up_idx];
	}
	if (pMbss) {
		diag_info.ssid = pMbss->Ssid;
		diag_info.ssid_len = pMbss->SsidLen;
		DiagFrameCache(&diag_info);
	} else
		DBGPRINT(RT_DEBUG_ERROR, ("%s, Error, no UP ap interface!!!\n", __func__));
}

#endif

void DiagBcnTx(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, UCHAR *pBeaconFrame, ULONG FrameLen)
{
	DIAG_FRAME_INFO diag_info;

	if (!DIAG_ENABLE())
		return;

	if (!pAd || !pMbss || !pBeaconFrame || (FrameLen == 0))
		return;

	memset(&diag_info, 0, sizeof(diag_info));
	diag_info.isTX = 1;
	diag_info.band = (pAd->LatchRfRegs.Channel > 14) ? 1 : 0;
	diag_info.ssid = pMbss->Ssid;
	diag_info.ssid_len = pMbss->SsidLen;
	diag_info.pData = pBeaconFrame;
	diag_info.dataLen = FrameLen;
	DiagFrameCache(&diag_info);
}


static const struct file_operations stat2G_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= stat2G_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations diag_duration_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_duration_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= diag_duration_proc_write,
};

static const struct file_operations diag_enable_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= diag_enable_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= diag_enable_proc_write,
};

static const struct file_operations ch_occupancy_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= ch_occupancy_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static const struct file_operations stat_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= stat_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

BOOLEAN diag_proc_entry_init(void *pAd)
{
	proc_create_data(DIAG_PROC_PATH"/"DIAG_STAT, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
			 &stat2G_proc_fops, pAd);
	proc_create_data(DIAG_PROC_PATH"/"DIAG_DURA, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
			 &diag_duration_proc_fops, pAd);
	proc_create_data(DIAG_PROC_PATH"/"DIAG_EN, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
			 &diag_enable_proc_fops, pAd);
	proc_create_data(DIAG_PROC_PATH"/"DIAG_CH_OC, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
			 &ch_occupancy_proc_fops, pAd);
	proc_create_data(DIAG_PROC_PATH"/"DIAG_STA_ST, S_IRWXU | S_IRWXG | S_IRWXO, NULL,
			 &stat_proc_fops, pAd);

	return TRUE;
}

void DiagProcDirInit(void)
{
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_OS_FD fd = NULL;
	UCHAR buf[64];
	UCHAR buf_size = 64;
	UCHAR fsize = 0;
	UCHAR tmpbuf[64];
	UCHAR diag_2G = 0, diag_5G = 0;

	DBGPRINT(RT_DEBUG_ERROR, ("7603 %s, start\n", __func__));

	memset(buf, 0, 64);
	memset(tmpbuf, 0, 64);

	/*test the DIAG_PROC_PATH dir exist*/

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, open DIAG_STATUS_FILE for read failed\n", __func__));
	} else {
		fsize = (UCHAR)OS_FILE_SIZE(fd);
		DBGPRINT(RT_DEBUG_ERROR, ("%s, DIAG_STATUS_FILE size is %d\n", __func__, fsize));
		if (buf_size > (fsize + 1))
			buf_size = fsize + 1;
		if ((fsize > 0) && (RtmpOSFileRead(fd, buf, buf_size - 1) > 0)) {
			if (RTMPGetKeyParameter("2G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_2G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_2G=%d\n", __func__, diag_2G));
			}
			if (RTMPGetKeyParameter("5G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_5G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_5G=%d\n", __func__, diag_5G));
			}
		}
		RtmpOSFileClose(fd);
	}

	if (diag_2G == 0 && diag_5G == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("7603 %s  dir %s  may not exist, create it\n", __func__, DIAG_PROC_PATH));
		proc_mkdir(DIAG_PROC_PATH, NULL);
	}

	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, open DIAG_STATUS_FILE for write failed\n", __func__));
	} else {
		if (diag_5G == 0)
			RtmpOSFileWrite(fd,
				"Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=0\n",
				strlen("Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=0\n"));
		else
			RtmpOSFileWrite(fd,
				"Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=1\n",
				strlen("Default\n2G_WIFI_DIAG=1\n5G_WIFI_DIAG=1\n"));
		RtmpOSFileClose(fd);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);

}

void DiagProcDirExit(void)
{
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_OS_FD fd = NULL;
	UCHAR buf[64];
	UCHAR buf_size = 64;
	UCHAR fsize = 0;
	UCHAR tmpbuf[64];
	UCHAR diag_2G = 0, diag_5G = 0;

	DBGPRINT(RT_DEBUG_ERROR, ("7603 %s, start\n", __func__));

	memset(buf, 0, 64);
	memset(tmpbuf, 0, 64);

	/*test the DIAG_PROC_PATH dir exist*/
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, open DIAG_STATUS_FILE failed\n", __func__));
	} else {
		fsize = (UCHAR)OS_FILE_SIZE(fd);
		if (buf_size > (fsize + 1))
			buf_size = fsize + 1;
		if ((fsize > 0) && (RtmpOSFileRead(fd, buf, buf_size - 1) > 0)) {
			if (RTMPGetKeyParameter("2G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_2G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_2G=%d\n", __func__, diag_2G));
			}
			if (RTMPGetKeyParameter("5G_WIFI_DIAG", tmpbuf, 64, buf, TRUE)) {
				diag_5G = (UCHAR)simple_strtol(tmpbuf, 0, 10);
				DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_5G=%d\n", __func__, diag_5G));
			}
		}
		RtmpOSFileClose(fd);
	}

	if (diag_2G == 1 && diag_5G == 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("7603 %s, remove proc/ctcwifi\n", __func__));
		remove_proc_entry(DIAG_PROC_PATH, NULL);
	}

	fd = RtmpOSFileOpen(DIAG_STATUS_FILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, open DIAG_STATUS_FILE for write failed\n", __func__));
	} else {
		if (diag_5G == 0)
			RtmpOSFileWrite(fd,
				"Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=0\n",
				strlen("Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=0\n"));
		else
			RtmpOSFileWrite(fd,
				"Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=1\n",
				strlen("Default\n2G_WIFI_DIAG=0\n5G_WIFI_DIAG=1\n"));
		RtmpOSFileClose(fd);
	}

	RtmpOSFSInfoChange(&osFSInfo, FALSE);

}

BOOLEAN DiagProcMemAllocate(void)
{
	if (diag_ctrl_mem != NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, diag_ctrl_mem already allocated\n", __func__));
		return FALSE;
	}

	os_alloc_mem(NULL, (UCHAR **)&diag_ctrl_mem, sizeof(struct diag_ctrl));
	if (diag_ctrl_mem == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s allocate memory failed\n", __func__));
		return FALSE;
	}
	memset(diag_ctrl_mem, 0, sizeof(struct diag_ctrl));
	return TRUE;
}

BOOLEAN DiagProcMemFree(void)
{
	if (diag_ctrl_mem)
		os_free_mem(NULL, diag_ctrl_mem);
	return TRUE;
}

BOOLEAN DiagProcInit(PRTMP_ADAPTER pAd)
{
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_OS_FD fd = NULL;
	UCHAR index = 0;
	int ret = 0;

	if (!pAd)
		return FALSE;

	if (DIAG_INITED()) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s already inited\n", __func__));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("7603 %s pAd:%p\n", __func__, pAd));

	diag_ctrl_db = diag_ctrl_mem;
	memset(diag_ctrl_db, 0, sizeof(struct diag_ctrl));

	NdisAllocateSpinLock(pAd, &(diag_ctrl_db->diag_log_lock));
	NdisAllocateSpinLock(pAd, &(diag_ctrl_db->assoc_error_lock));

	/* create var sub directory */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	ret = sys_access(DIAG_VAR_PATH, 0);
	if (ret != 0) {
		ret = sys_mkdir(DIAG_VAR_PATH, 0777);
		if (ret < 0)
			DBGPRINT(RT_DEBUG_ERROR, ("%s, Error create directory %s\n", __func__, DIAG_VAR_PATH));
	}
	fd = RtmpOSFileOpen(DIAG_VAR_ASSOC_ERROR_2G_PATH, O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd))
		DBGPRINT(RT_DEBUG_ERROR, ("%s, Error create %s\n", __func__, DIAG_VAR_ASSOC_ERROR_2G_PATH));
	else
		RtmpOSFileClose(fd);

	fd = RtmpOSFileOpen(DIAG_VAR_LOG_PATH, O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_FILE_OPEN_ERR(fd))
		DBGPRINT(RT_DEBUG_ERROR, ("%s, Error create %s\n", __func__, DIAG_VAR_LOG_PATH));
	else
		RtmpOSFileClose(fd);
	RtmpOSFSInfoChange(&osFSInfo, FALSE);

	/* proc */
	DiagProcDirInit();
	diag_proc_entry_init(pAd);

	/* process info */
	for (index = 0; index < DIAG_MAX_PROCESS_NUM; index++)
		diag_ctrl_db->process_info.entry[index].pid = -1;

	diag_ctrl_db->diag_enable = 0;
	diag_ctrl_db->diag_duration = 60;

	diag_ctrl_db->inited = 1;

	return TRUE;
}

BOOLEAN DiagProcExit(PRTMP_ADAPTER pAd)
{
	if (!DIAG_INITED())
		return FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("%s pAd:%p 7603\n", __func__, pAd));

	diag_ctrl_db->inited = 0;

	/* stop diag_log handling */
	diag_ctrl_db->diag_enable = 0;
	OS_WAIT(300);

	remove_proc_entry(DIAG_PROC_PATH"/"DIAG_STAT, 0);
	remove_proc_entry(DIAG_PROC_PATH"/"DIAG_DURA, 0);
	remove_proc_entry(DIAG_PROC_PATH"/"DIAG_EN, 0);
	remove_proc_entry(DIAG_PROC_PATH"/"DIAG_CH_OC, 0);
	remove_proc_entry(DIAG_PROC_PATH"/"DIAG_STA_ST, 0);
	DiagProcDirExit();

	NdisFreeSpinLock(&diag_ctrl_db->diag_log_lock);
	NdisFreeSpinLock(&diag_ctrl_db->assoc_error_lock);

	diag_ctrl_db = NULL;

	return TRUE;
}

#endif


