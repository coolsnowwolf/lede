// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/debugfs.h>
#include <linux/iosys-map.h>
#include <linux/dma-buf.h>
#include "phytium_npu.h"
#include "phytium_npu_mmu.h"
#include "linux/phytium_npu_dma_buf_heap.h"
#include "phytium_npu_pd2408_reg.h"

#define DEBUG_BAND_COUNT_RESET	0x3fffffff
#define DEBUG_BAND_EN	0x1
#define DEBUG_PERFORMANCE_COUNT_RESET	0x1
#define DEBUG_PERFORMANCE_EN	0x1
#define DEBUG_B_OR_C_STREAM 0x1
#define DEBUG_B_OR_C_LAYER 0x2
#define DEBUG_B_OR_C_PASS 0x3
#define DEBUG_PERF_S 0x4
#define DEBUG_PERF_L 0x8
#define DEBUG_PERF_P 0xC
#define DEBUG_FILE_MODE		(DEBUG_TYPE_BAND_STREAM | DEBUG_TYPE_BAND_LAYER \
				| DEBUG_TYPE_BAND_PASS | DEBUG_TYPE_PERF_LAYER \
				| DEBUG_TYPE_PERF_PASS | DEBUG_TYPE_PERF_STREAM)
#define DEBUG_FILE_CRC_MODE (DEBUG_TYPE_CRC_STREAM | DEBUG_TYPE_CRC_LAYER \
							| DEBUG_TYPE_CRC_PASS)
#define DEBUG_FILE_MODE_BAND (DEBUG_TYPE_BAND_STREAM | DEBUG_TYPE_BAND_LAYER \
							| DEBUG_TYPE_BAND_PASS)
#define DEBUG_FILE_MODE_PERF (DEBUG_TYPE_PERF_STREAM | DEBUG_TYPE_PERF_LAYER \
							| DEBUG_TYPE_PERF_PASS)
int is_debug_get_time_span;

void phytium_npu_get_time_span(struct timespec64 *start,
			       struct timespec64 *end,
				struct timespec64 *span)
{
	if (!is_debug_get_time_span)
		return;
	if (!start || !end || !span)
		return;
	span->tv_sec = end->tv_sec - start->tv_sec;
	if (span->tv_sec > 0) {
		span->tv_sec--;
		end->tv_nsec += 1000000000;
	}
	/* Calculate the nanoseconds span. */
	span->tv_nsec = end->tv_nsec - start->tv_nsec;
	if (span->tv_nsec > 1000000000) {
		span->tv_sec++;
		span->tv_nsec -= 1000000000;
	}
	pr_info("@@@@ scheduled  span: %llu (time-ns)\n",
		(uint64_t)span->tv_sec * 1000000000ULL +
		(uint64_t)span->tv_nsec);
}

int phytiun_npu_check_debug_fs_cfg(struct phytium_npu_session *sess)
{
	struct phytium_npu_debugfs *dbgfs = &sess->dbgfs;
	struct npu_mctx_map *ncmap;
	struct npu_unified_heap_buffer *buffer;

	if (dbgfs->debug_mode <= DEBUG_MODE_REGISTER)
		return 0;
	if (dbgfs->debug_mode == DEBUG_MODE_MEMORY_FILE) {
		if (dbgfs->debug_type & DEBUG_FILE_MODE) {
			ncmap = phytium_npu_find_mmu_ctx_map(sess, dbgfs->debug_fd);
			if (!ncmap)	{
				pr_err("Not find debug fd %d from debug command!", dbgfs->debug_fd);
				return -EFAULT;
			}
			buffer = ncmap->dma_buf->priv;
			if (buffer) {
				dbgfs->debug_size = buffer->req_len;
				dbgfs->debug_addr = ncmap->virt_addr;
			} else {
				pr_err("not find buffer structure");
			}
			pr_debug("Get debug file info debug_size:%d, debug_addr:%llx ",
				 dbgfs->debug_size, dbgfs->debug_addr);
		}
		if (dbgfs->debug_type & DEBUG_FILE_CRC_MODE) {
			ncmap = phytium_npu_find_mmu_ctx_map(sess, dbgfs->debug_crc_fd);
			if (!ncmap)	{
				pr_err("Not find debug crc fd %d from debug command!",
				       dbgfs->debug_fd);
				return -EFAULT;
			}
			dbgfs->debug_crc_addr = ncmap->virt_addr;
			pr_debug("Get debug crc file info debug_crc_addr:%llx ",
				 dbgfs->debug_crc_addr);
		}
		phytium_npu_debugfs_session_init(sess->npu_dev, sess);
	}
	return 0;
}

void phytium_npu_debug_show_register(struct phytium_npu_dev *npu, struct phytium_npu_session *sess)
{
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;

	if (dbg->debug_mode != DEBUG_MODE_REGISTER)
		return;

	if (dbg->debug_type & DEBUG_TYPE_REG_BAND) {
		pr_info("****  debug band info ****");
		pr_info("request count enable %#x", REGREAD32(npu, NPU_CACHE_REQ_CNT_EN));
		pr_info("stream read request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CMDREQ_RD),
			REGREAD32(npu, NPU_CACHE_CMDREQ_RD_WORD));
		pr_info("stream bck write request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CMDBCK_REQ_WR),
			REGREAD32(npu, NPU_CACHE_CMDBCK_REQ_WR_WORD));
		pr_info("stream crc write request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CMDCRC_REQ_WR),
			REGREAD32(npu, NPU_CACHE_CMDCRC_REQ_WR_WORD));
		pr_info("stream dbg request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CMDDBG_REQ_WR),
			REGREAD32(npu, NPU_CACHE_CMDDBG_REQ_WR_WORD));
		pr_info("stream fence request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CMDREQ_FENCE),
			REGREAD32(npu, NPU_CACHE_CREQ_FENCE_WORD));
		pr_info("input read request0 counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_IREQ0_RD),
			REGREAD32(npu, NPU_CACHE_IREQ0_RD_WORD));
		pr_info("mem read request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_MM_REQ_RD),
			REGREAD32(npu, NPU_CACHE_MM_REQ_RD_WORD));
		pr_info("mem write request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_MM_REQ_WR),
			REGREAD32(npu, NPU_CACHE_MM_REQ_WR_WORD));
		pr_info("coefficient read request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_CREQ_RD),
			REGREAD32(npu, NPU_CACHE_CREQ_RD_WORD));
		pr_info("a segment read request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_AREQ_RD),
			REGREAD32(npu, NPU_CACHE_AREQ_RD_WORD));
		pr_info("opk write request counter %d, word %d",
			REGREAD32(npu, NPU_CACHE_OPK_REQ_WR),
			REGREAD32(npu, NPU_CACHE_OPK_REQ_WR_WORD));
	}

	if (dbg->debug_type & DEBUG_TYPE_REG_PERF) {
		pr_info("**** debug performance info ****");
		pr_info("write data to memory count:%d", REGREAD32(npu, NPU_PERF_WRITE_DS));
		pr_info("read from memory count: %d, write %d",
			REGREAD32(npu, NPU_PERF_READ),
			REGREAD32(npu, NPU_PERF_WRITE));
		pr_info("read stream from memory count:%d, write:%d",
			REGREAD32(npu, NPU_PERF_READS),
			REGREAD32(npu, NPU_PERF_WRITES));
		pr_info("read id from memory count:%d, write %d",
			REGREAD32(npu, NPU_PERF_READ_IS),
			REGREAD32(npu, NPU_PERFWRITE_IS));
		pr_info("read burst s1 count:%d, write %d",
			REGREAD32(npu, NPU_PERF_RD_BS1),
			REGREAD32(npu, NPU_PERF_WR_BS1));
		pr_info("read burst s2 count:%d write %d",
			REGREAD32(npu, NPU_PERF_RD_BS2),
			REGREAD32(npu, NPU_PERF_WR_BS2));
		pr_info("read burst s3 count:%d write %d",
			REGREAD32(npu, NPU_PERF_RD_BS3),
			REGREAD32(npu, NPU_PERF_WR_BS3));
		pr_info("read burst s4 count:%d write %d",
			REGREAD32(npu, NPU_PERF_RD_BS4),
			REGREAD32(npu, NPU_PERF_WR_BS4));
	}
}

void phytium_npu_debug_set_hw_register(struct phytium_npu_dev *npu,
				       struct phytium_npu_session *sess)
{
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;
	int band_dbg = 0, perf_dbg = 0;

	if (dbg->debug_mode == DEBUG_MODE_NONE)
		return;

	if (dbg->debug_mode == DEBUG_MODE_REGISTER) {
		if (dbg->debug_type & DEBUG_TYPE_REG_BAND) {
			REGWRITE32(npu, NPU_CACHE_RESET, DEBUG_BAND_COUNT_RESET);
			REGWRITE32(npu, NPU_CACHE_REQ_CNT_EN, DEBUG_BAND_EN);
		} else if (dbg->debug_type & DEBUG_TYPE_REG_PERF) {
			REGWRITE32(npu, NPU_PERF_RESET, DEBUG_PERFORMANCE_COUNT_RESET);
			REGWRITE32(npu, NPU_PERF_ENABLE, DEBUG_PERFORMANCE_EN);
		} else {
			pr_err("Err:Not set debug type [%s]", __func__);
		}
	} else if (dbg->debug_mode == DEBUG_MODE_MEMORY_FILE) {
		if (dbg->debug_type & DEBUG_FILE_MODE_BAND) {
			switch (dbg->debug_type & 0xF0) {
			case DEBUG_TYPE_BAND_STREAM:
					band_dbg |= DEBUG_B_OR_C_STREAM;
				break;
			case DEBUG_TYPE_BAND_LAYER:
					band_dbg |= DEBUG_B_OR_C_LAYER;
				break;
			case DEBUG_TYPE_BAND_PASS:
					band_dbg |= DEBUG_B_OR_C_PASS;
				break;
			}
		}

		if (dbg->debug_type & DEBUG_FILE_MODE_PERF) {
			if (dbg->debug_type & DEBUG_TYPE_PERF_STREAM) {
				switch (dbg->debug_type & 0xF00) {
				case DEBUG_TYPE_PERF_STREAM:
					perf_dbg |= DEBUG_PERF_S;
				break;
				case DEBUG_TYPE_PERF_LAYER:
					perf_dbg |= DEBUG_PERF_L;
				break;
				case DEBUG_TYPE_PERF_PASS:
					perf_dbg |= DEBUG_PERF_P;
				break;
				}
			}
		}

		if (dbg->debug_type & DEBUG_FILE_MODE) {
			REGWRITE32(npu, NPU_CH0_DEBUG_ADDRESS, dbg->debug_addr);
			REGWRITE32(npu, NPU_CH0_DEBUG_SIZE, dbg->debug_size);
			REGWRITE32(npu, NPU_CH0_DEBUG_CONTROL, perf_dbg | band_dbg);
			pr_debug("set npu hw debug addr:%llx, size:%u, control :%x\n",
				 dbg->debug_addr, dbg->debug_size, perf_dbg | band_dbg);
		}

		if (dbg->debug_type & DEBUG_FILE_CRC_MODE) {
			band_dbg = 0;
			if (dbg->debug_type & DEBUG_FILE_CRC_MODE) {
				switch (dbg->debug_type & 0xF000) {
				case DEBUG_TYPE_CRC_STREAM:
					band_dbg |= DEBUG_B_OR_C_STREAM;
				break;
				case DEBUG_TYPE_CRC_LAYER:
					band_dbg |= DEBUG_B_OR_C_LAYER;
				break;
				case DEBUG_TYPE_CRC_PASS:
					band_dbg |= DEBUG_B_OR_C_PASS;
				break;
				}
				REGWRITE32(npu, NPU_CH0_CRC_CONTROL, band_dbg);
				REGWRITE32(npu, NPU_CH0_CRC_ADDRESS, dbg->debug_crc_addr);
				pr_debug("set npu hw debug addr:%llx, control :%x\n",
					 dbg->debug_crc_addr, band_dbg);
			}
		}
	}
}

void phytium_npu_debugfs_init(struct phytium_npu_dev *npu)
{
	npu->dbgfs_root_dir = NULL;
#ifdef CONFIG_DEBUG_FS
	/* create npu0 directory in /sys/kernel/debug/ */
	npu->dbgfs_root_dir = debugfs_create_dir(npu->miscdev.name, NULL);
	if (!npu->dbgfs_root_dir) {
		pr_err("%s: Failed to create debug %s directory", __func__, npu->miscdev.name);
		return;
	}
	debugfs_create_x32("load", 0444, npu->dbgfs_root_dir,
			   &npu->load_status);
	debugfs_create_x32("config_cluster", 0444, npu->dbgfs_root_dir,
			   &npu->power_status);
	debugfs_create_x32("clk", 0444, npu->dbgfs_root_dir,
			   &npu->clock_freq);
	debugfs_create_x32("voltage", 0444, npu->dbgfs_root_dir,
			   &npu->voltage_val);
#endif
}

void phytium_npu_debugfs_remove(struct phytium_npu_dev *npu)
{
#ifdef CONFIG_DEBUG_FS
	pr_debug("remove debugfs root directory");
	debugfs_remove_recursive(npu->dbgfs_root_dir);
#endif
}

static ssize_t
phytium_npu_debug_perf_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	struct phytium_npu_session  *sess = (struct phytium_npu_session *)file->private_data;
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;
	struct dma_buf *pbuf;
	struct npu_unified_heap_buffer *buffer;

	if (dbg->debug_mode != DEBUG_MODE_MEMORY_FILE)
		return 0;
	if (!(dbg->debug_type & DEBUG_FILE_MODE))
		return 0;
	pr_debug("%s[%d]: user read file debug_perf.bin", __func__, __LINE__);
	pbuf = phytium_npu_check_and_get_dma_buf(sess, dbg->debug_fd);
	if (pbuf && !dbg->debug_map.vaddr) {
		if (dma_buf_vmap(pbuf, &dbg->debug_map)) {
			pr_err("%s: Failed to vmap debug dma_buf", __func__);
			return 0;
		}
	} else {
		return 0;
	}
	buffer = pbuf->priv;
	pr_debug("dbg->debug_map.vaddr:%#llx %lld, ppos:%lld, readlen:%ld",
		 (u64)dbg->debug_map.vaddr, (u64)buffer->req_len, *ppos, len);
	pr_debug("%s, fd:%d,count:%ld", __func__, dbg->debug_fd, file_count(pbuf->file));
	return simple_read_from_buffer(buf, len, ppos, dbg->debug_map.vaddr, buffer->req_len);
}

static int phytium_npu_debug_perf_release(struct inode *inode, struct file *file)
{
	struct phytium_npu_session  *sess = (struct phytium_npu_session *)file->private_data;
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;
	struct dma_buf *pbuf;

	pbuf = phytium_npu_check_and_get_dma_buf(sess, dbg->debug_fd);
	if (pbuf) {
		dma_buf_vunmap(pbuf, &dbg->debug_map);
		pr_debug("%s[%d]: vunmap debug dma_buf fd:%d", __func__, __LINE__, dbg->debug_fd);
		pr_info("%s[%d]: vunmap debug dma_buf fd:%d", __func__, __LINE__, dbg->debug_fd);
	}
	pr_debug("%s, fd:%d,count:%ld", __func__, dbg->debug_fd, file_count(pbuf->file));
	memset(&dbg->debug_map, 0, sizeof(dbg->debug_map));
	return 0;
}

static const struct file_operations _npu_debug_perf_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = phytium_npu_debug_perf_read,
	.release = phytium_npu_debug_perf_release,
};

static ssize_t
phytium_npu_debug_perf_crc_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	struct phytium_npu_session  *sess = (struct phytium_npu_session *)file->private_data;
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;
	struct dma_buf *pbuf;
	struct npu_unified_heap_buffer *buffer;

	if (dbg->debug_mode != DEBUG_MODE_MEMORY_FILE)
		return 0;
	if (!(dbg->debug_type & DEBUG_FILE_CRC_MODE))
		return 0;

	pr_debug("%s[%d]: user read file debug_perf.crc", __func__, __LINE__);
	pbuf = phytium_npu_check_and_get_dma_buf(sess, dbg->debug_crc_fd);
	if (pbuf) {
		if (!dbg->debug_crc_map.vaddr) {
			if (dma_buf_vmap(pbuf, &dbg->debug_crc_map)) {
				pr_err("%s: Failed to vmap debug crc dma_buf", __func__);
				return 0;
			}
		}
	} else {
		return 0;
	}
	buffer = pbuf->priv;
	pr_debug("%s dbg->debug_map.vaddr:%#llx %ld, ppos:%lld, readlen:%ld", __func__,
		 (u64)dbg->debug_crc_map.vaddr, buffer->req_len, (u64)*ppos, len);
	pr_debug("%s, fd:%d,count:%ld", __func__, dbg->debug_crc_fd, file_count(pbuf->file));
	return simple_read_from_buffer(buf, len, ppos, dbg->debug_crc_map.vaddr, buffer->req_len);
}

static int phytium_npu_debug_perf_crc_release(struct inode *inode, struct file *file)
{
	struct phytium_npu_session  *sess = (struct phytium_npu_session *)file->private_data;
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;
	struct dma_buf *pbuf;

	pbuf = phytium_npu_check_and_get_dma_buf(sess, dbg->debug_crc_fd);
	if (pbuf) {
		dma_buf_vunmap(pbuf, &dbg->debug_crc_map);
		pr_debug("%s[%d]: vunmap debug crc dma_buf fd:%d", __func__,
			 __LINE__, dbg->debug_crc_fd);
	}
	pr_info("%s, fd:%d,count:%ld", __func__, dbg->debug_crc_fd, file_count(pbuf->file));
	memset(&dbg->debug_crc_map, 0, sizeof(dbg->debug_crc_map));
	return 0;
}

static const struct file_operations _npu_debug_perf_crc_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = phytium_npu_debug_perf_crc_read,
	.release = phytium_npu_debug_perf_crc_release,
};

void phytium_npu_debugfs_session_init(struct phytium_npu_dev *npu, struct phytium_npu_session *sess)
{
#ifdef CONFIG_DEBUG_FS
	char sess_dir_name[20];
	struct phytium_npu_debugfs *dbg = &sess->dbgfs;

	if (dbg->debug_mode != DEBUG_MODE_MEMORY_FILE)
		return;

	snprintf(sess_dir_name, sizeof(sess_dir_name) - 1, "session%d", sess->id);
	/* create sessionX directory in /sys/kernel/debug/npu0 */
	if (npu->dbgfs_root_dir) {
		dbg->sess_dbgfs_dir = debugfs_create_dir(sess_dir_name, npu->dbgfs_root_dir);
		if (!dbg->sess_dbgfs_dir) {
			pr_err("%s: Failed to create debug %s directory", __func__, sess_dir_name);
			return;
		}

		if (dbg->debug_type & DEBUG_FILE_MODE) {
			if (!debugfs_create_file("debug_perf.bin", 0444, dbg->sess_dbgfs_dir,
						 sess, &_npu_debug_perf_fops))
				pr_err("%s: Failed to create debug_perf.bin file", __func__);
		}
		if (dbg->debug_type & DEBUG_FILE_CRC_MODE) {
			if (!debugfs_create_file("debug_perf.crc", 0444, dbg->sess_dbgfs_dir,
						 sess, &_npu_debug_perf_crc_fops))
				pr_err("%s: Failed to create debug_perf.bin file", __func__);
		}
	}
#endif
}

void phytium_npu_debugfs_session_remove(struct phytium_npu_session *sess)
{
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(sess->dbgfs.sess_dbgfs_dir);
	pr_debug("%s[%d]:will remove the directory of debugfs", __func__, __LINE__);
#endif
}
