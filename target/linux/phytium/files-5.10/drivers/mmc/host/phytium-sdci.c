// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SD Card Interface dirver
 *
 * Copyright (c) 2019-2023, Phytium Technology Co.,Ltd.
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/acpi.h>
#include <linux/timer.h>

#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>

#include "phytium-sdci.h"

static const u32 cmd_ints_mask = SDCI_SDCI_NORMAL_ISER_ECC_EN | SDCI_SDCI_NORMAL_ISER_EEI_EN;
static const u32 data_ints_mask = SDCI_BD_ISER_ETRS_EN;
static const u32 err_ints_mask = SDCI_ERROR_ISER_ECTE_EN | SDCI_ERROR_ISR_CCRCE_EN |
				  SDCI_ERROR_ISR_CIR_EN | SDCI_ERROR_ISR_CNR_EN;

static void hotplug_timer_func(struct timer_list *t);
static bool phytium_sdci_private_send_cmd(struct phytium_sdci_host *host,
					  u32 cmd, u32 resp_type, u32 arg);
static bool phytium_sdci_cmd_done(struct phytium_sdci_host *host, int events,
				  struct mmc_request *mrq,
				  struct mmc_command *cmd);
static bool phytium_sdci_data_xfer_done(struct phytium_sdci_host *host,
					u32 events, struct mmc_request *mrq,
					struct mmc_data *data);
static void phytium_sdci_cmd_next(struct phytium_sdci_host *host,
				  struct mmc_request *mrq,
				  struct mmc_command *cmd);

static int phytium_sdci_cmd13_process(struct phytium_sdci_host *host,
				       struct mmc_request *mrq,
				       struct mmc_data *data,
				       u32 wait_timeout_ms,
				       u32 send_once_time_ms);

static int phytium_sd_error(struct phytium_sdci_host *host)
{
	int temp;

	temp = readl(host->base + SDCI_NORMAL_ISR);
	dev_err(host->dev, "[%s %d]SDCI_NORMAL_ISR:%x\n", __func__, __LINE__, temp);
	temp = readl(host->base + SDCI_BD_ISR);
	temp = readl(host->base + SDCI_ERROR_ISR);
	dev_err(host->dev, "[%s %d]SDCI_ERROR_ISR:%x\n", __func__, __LINE__, temp);
	temp = readl(host->base + SDCI_BD_ISR);
	dev_err(host->dev, "[%s %d]SDCI_BD_ISR:%x\n", __func__, __LINE__, temp);
	temp = readl(host->base + SDCI_RESP0);
	dev_err(host->dev, "[%s %d]SDCI_RESP0:%x\n", __func__, __LINE__, temp);

	return 0;
}

static void sdr_set_bits(void __iomem *reg, u32 bs)
{
	u32 val;

	val = readl(reg);
	val |= bs;

	writel(val, reg);
}

static void sdr_clr_bits(void __iomem *reg, u32 bs)
{
	u32 val;

	val = readl(reg);
	val &= ~bs;

	writel(val, reg);
}

static void phytium_sdci_reset_hw(struct phytium_sdci_host *host)
{
	sdr_set_bits(host->base + SDCI_SOFTWARE,
		     SDCI_SOFTWARE_SRST);
	sdr_clr_bits(host->base + SDCI_SOFTWARE,
		     SDCI_SOFTWARE_SRST);
	while (!(readl(host->base + SDCI_STATUS) & SDCI_STATUS_IDIE))
		cpu_relax();
}

static void phytium_sdci_prepare_data(struct phytium_sdci_host *host,
				      struct mmc_request *mrq)
{
	struct mmc_data *data = mrq->data;
	bool read;

	read = (data->flags & MMC_DATA_READ) != 0;
	data->sg_count = dma_map_sg(host->dev, data->sg, data->sg_len,
				    read ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

static void phytium_sdci_unprepare_data(struct phytium_sdci_host *host,
					struct mmc_request *mrq)
{
	bool read;
	struct mmc_data *data = mrq->data;

	read = (data->flags & MMC_DATA_READ) != 0;
	dma_unmap_sg(host->dev, data->sg, data->sg_len,
		     read ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

static void phytium_sdci_set_clk(struct phytium_sdci_host *host,
				 struct mmc_ios *ios)
{
	unsigned long clk_rate;
	u32 div = 0xffffffff, div_reg;

	if (ios->clock) {
		clk_rate = host->clk_rate;
		div = ((clk_rate / (2 * ios->clock)) - 1);
		div_reg = readl(host->base + SDCI_CLOCK_D);
		if (div_reg == div)
			return;
		writel(div, host->base + SDCI_CLOCK_D);
		writel(0, host->base + SDCI_SD_DRV);
		writel(5, host->base + SDCI_SD_SAMP);

		sdr_set_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_SRST);
		sdr_clr_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_SRST);
		while (!(readl(host->base + SDCI_STATUS) & SDCI_STATUS_IDIE))
			cpu_relax();
		dev_dbg(host->dev, "host->clk_rate: %ld, ios->clock: %d\n",
			host->clk_rate, ios->clock);
	}
}


static inline u32 phytium_sdci_cmd_find_resp(struct phytium_sdci_host *host,
					     struct mmc_request *mrq,
					     struct mmc_command *cmd)
{
	u32 resp;

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1:
		resp = 0x2;
		break;
	case MMC_RSP_R1B:
		resp = 0x2;
		break;
	case MMC_RSP_R2:
		resp = 0x1;
		break;
	case MMC_RSP_R3:
		resp = 0x3;
		break;
	case MMC_RSP_NONE:
	default:
		resp = 0x0;
		break;
	}

	return resp;
}

static inline u32 phytium_sdci_cmd_prepare_raw_cmd(struct phytium_sdci_host *host,
		struct mmc_request *mrq, struct mmc_command *cmd)
{
	/*
	 * rawcmd :
	 *   trty << 14 | opcode << 8 | cmdw << 6 | cice << 4 | crce << 3 | resp
	 */
	u32 resp, rawcmd;
	u32 opcode = cmd->opcode;

	resp = phytium_sdci_cmd_find_resp(host, mrq, cmd);
	rawcmd = ((opcode << 8) | resp);

	if ((cmd->flags & MMC_CMD_MASK) == MMC_CMD_ADTC)
		rawcmd = (rawcmd | (SDCI_CMD_TYPE_ADTC << 14));

	return rawcmd;
}

static void
phytium_sdci_unexpected_error_handler(struct phytium_sdci_host *host,
				       struct mmc_request *mrq,
				       struct mmc_data *data,
				       int err_type)
{
	unsigned long flags;
	int error;

	spin_lock_irqsave(&host->lock, flags);
	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	if (err_type & ERR_CARD_ABSENT) {
		host->mmc->detect_change = 1;
		dev_dbg(host->dev, "SD is absent when send cmd:%d\n", mrq->cmd->opcode);
	}

	switch (err_type) {
	case ERR_CARD_ABSENT:
		error = -ENOMEDIUM;
		break;
	case ERR_TIMEOUT:
		error = -ETIMEDOUT;
		break;
	case ERR_CMD_RESPONED:
		error = -EIO;
		break;
	default:
		error = -ETIMEDOUT;
		break;
	}

	if (data) {
		data->error = error;
		phytium_sdci_unprepare_data(host, mrq);

		if ((data->flags & MMC_DATA_READ) == MMC_DATA_READ ||
		    (data->flags & MMC_DATA_WRITE) == MMC_DATA_WRITE)
			phytium_sdci_data_xfer_done(host, SDCI_BD_ISR_TRS_R, mrq, data);
	} else {
		mrq->cmd->error = error;
	}

	mmc_request_done(host->mmc, mrq);
}

static bool phytium_sdci_start_data(struct phytium_sdci_host *host, struct mmc_request *mrq,
			    struct mmc_command *cmd, struct mmc_data *data)
{
	bool read, res;
	u32 sg_dma_addrh, sg_dma_addrl;
	u32 sd_block_addrh, sd_block_addrl;
	u32 temp, timeout, sd_status;
	u32 block_cnt = 0;
	u32 sd_block_addr = cmd->arg;
	u32 private_cmd, resp_type, arg;
	u32 j, dma_len;
	unsigned long deadline_time;
	dma_addr_t dma_address;
	struct scatterlist *sg;
	int ret;

	WARN_ON(host->cmd);
	host->cmd = cmd;

	WARN_ON(host->data);
	host->data = data;
	read = data->flags & MMC_DATA_READ;

	for_each_sg(data->sg, sg, data->sg_count, j) {
		writel(0, host->base + SDCI_COMMAND);

		dma_address = sg_dma_address(sg);
		sg_dma_addrh = (u32) (dma_address >> 32);
		sg_dma_addrl = (u32) dma_address;

		dma_len = sg_dma_len(sg);
		block_cnt = (dma_len / SD_BLOCK_SIZE);

		sd_block_addrh = 0;
		sd_block_addrl = sd_block_addr;

		sdr_set_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_BDRST);
		sdr_clr_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_BDRST);
		writel(block_cnt, host->base + SDCI_BLK_CNT);

		if ((mrq->data->flags & MMC_DATA_READ) == MMC_DATA_READ) {
			writel(sg_dma_addrl, host->base + SDCI_BD_RX);
			writel(sg_dma_addrh, host->base + SDCI_BD_RX);
			writel(sd_block_addrl, host->base + SDCI_BD_RX);
			writel(sd_block_addrh, host->base + SDCI_BD_RX);
			timeout = 100 * block_cnt;
		} else {
			timeout = 250 * block_cnt;
			ret = phytium_sdci_cmd13_process(host, mrq, data, timeout, 1);
			if (ret != SDCI_CMD13_OK)
				return false;

			writel(sg_dma_addrl, host->base + SDCI_BD_TX);
			writel(sg_dma_addrh, host->base + SDCI_BD_TX);
			writel(sd_block_addrl, host->base + SDCI_BD_TX);
			writel(sd_block_addrh, host->base + SDCI_BD_TX);
		}

		deadline_time = jiffies + msecs_to_jiffies(timeout);

		temp = readl(host->base + SDCI_BD_ISR);
		if ((mrq->data->flags & MMC_DATA_READ) == MMC_DATA_READ) {
			while ((temp & SDCI_BD_ISR_TRS_R) != SDCI_BD_ISR_TRS_R) {
				sd_status = readl(host->base + SDCI_STATUS);
				if (sd_status & SDCI_STATUS_CDSL) {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_CARD_ABSENT);
					if (temp & SDCI_BD_ISR_DAIS)
						writel(1, host->base + SDCI_BD_ISR);
					return false;
				}

				temp = readl(host->base + SDCI_BD_ISR);
				if (time_after(jiffies, deadline_time)) {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_TIMEOUT);
					dev_err(host->dev,
						"Read Data timeout:jiffies:0x%lx,dt_jiffies: 0x%lx, BD_isr_reg:0x%x,cmd:%d, REG_D0:0x%x\n",
						jiffies, jiffies - deadline_time, temp,
						cmd->opcode, readl(host->base + SDCI_STATUS));

					return false;
				}
			}
		} else {
			while ((temp & SDCI_BD_ISR_TRS_W) != SDCI_BD_ISR_TRS_W) {
				sd_status = readl(host->base + SDCI_STATUS);
				if (sd_status & SDCI_STATUS_CDSL) {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_CARD_ABSENT);
					dev_err(host->dev, "[%s][%d]: Card absent ! cmd(%d)\n",
						__func__, __LINE__, mrq->cmd->opcode);
					return false;
				}

				temp = readl(host->base + SDCI_BD_ISR);
				if (time_after(jiffies, deadline_time)) {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_TIMEOUT);
					dev_err(host->dev,
						"Write Date timeout: jiffies:0x%lx,dt_jiffies: 0x%lx,BD_isr_reg:0x%x\n",
						jiffies, jiffies - deadline_time, temp);
					return false;
				}
			}
		}
		writel(1, host->base + SDCI_BD_ISR);
		writel(1, host->base + SDCI_NORMAL_ISR);
		sd_block_addr = sd_block_addr + block_cnt;

		if (j < (data->sg_count - 1) && 1 < block_cnt) {
			private_cmd = MMC_STOP_TRANSMISSION;
			resp_type = 0x2;
			arg = 0;
			res = phytium_sdci_private_send_cmd(host, private_cmd,
							    resp_type, arg);
			if (!res) {
				sd_status = readl(host->base + SDCI_STATUS);
				if (sd_status & SDCI_STATUS_CDSL) {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_CARD_ABSENT);
					writel(1, host->base + SDCI_BD_ISR);
					dev_err(host->dev,
						"[%s][%d]:Card absent ! private_cmd(%d)\n",
						__func__, __LINE__, private_cmd);
				} else {
					phytium_sdci_unexpected_error_handler(host, mrq, data,
									      ERR_CMD_RESPONED);
					dev_err(host->dev,
						"[%s][%d] cmd(%d) response errored\n",
						__func__, __LINE__, mrq->cmd->opcode);
					phytium_sd_error(host);
				}
				writel(1, host->base + SDCI_NORMAL_ISR);
				return false;
			}
			writel(1, host->base + SDCI_NORMAL_ISR);
		}
	}

	host->is_multi_rw_only_one_blkcnt = false;

	if ((cmd->opcode == MMC_READ_MULTIPLE_BLOCK && block_cnt == 1) ||
	    (cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK && block_cnt == 1))
		host->is_multi_rw_only_one_blkcnt = true;

	phytium_sdci_cmd_done(host, SDCI_NORMAL_ISR_CC, mrq, cmd);
	if ((mrq->data->flags & MMC_DATA_READ) == MMC_DATA_READ)
		phytium_sdci_data_xfer_done(host, SDCI_BD_ISR_TRS_R,
					    mrq, data);
	else
		phytium_sdci_data_xfer_done(host, SDCI_BD_ISR_TRS_W,
					    mrq, data);

	return true;
}

static int phytium_sdci_auto_cmd_done(struct phytium_sdci_host *host,
				      int events, struct mmc_command *cmd)
{
	u32 *rsp = cmd->resp;

	rsp[0] = readl(host->base + SDCI_RESP0);

	if (events & SDCI_NORMAL_ISR_CC)
		cmd->error = 0;
	else {
		phytium_sdci_reset_hw(host);
		dev_err(host->dev,
			"%s: AUTO_CMD%d arg=%08X; rsp %08X; cmd_error=%d\n",
			__func__, cmd->opcode, cmd->arg, rsp[0], cmd->error);
	}

	return cmd->error;
}

static void phytium_sdci_track_cmd_data(struct phytium_sdci_host *host,
					struct mmc_command *cmd,
					struct mmc_data *data)
{
	if (host->error)
		dev_dbg(host->dev, "%s: cmd=%d arg=%08X; host->error=0x%08X\n",
			__func__, cmd->opcode, cmd->arg, host->error);
}

static void phytium_sdci_request_done(struct phytium_sdci_host *host,
				      struct mmc_request *mrq)
{
	unsigned long flags;

	dev_dbg(host->dev,
		"%s_%d:mrq->cmd->opcode:%d, mrq->cmd->arg:0x%x resp 0x%x 0x%x 0x%x 0x%x\n",
		__func__, __LINE__, mrq->cmd->opcode, mrq->cmd->arg,
		mrq->cmd->resp[0], mrq->cmd->resp[1], mrq->cmd->resp[2],
		mrq->cmd->resp[3]);

	spin_lock_irqsave(&host->lock, flags);
	host->mrq = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	phytium_sdci_track_cmd_data(host, mrq->cmd, mrq->data);
	if (mrq->data)
		phytium_sdci_unprepare_data(host, mrq);
	mmc_request_done(host->mmc, mrq);
}

static bool
phytium_sdci_auto_command_done(struct phytium_sdci_host *host, int events,
			       struct mmc_request *mrq, struct mmc_command *cmd)
{
	u32 *rsp = cmd->resp;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->cmd = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	sdr_clr_bits(host->base + SDCI_NORMAL_ISER, cmd_ints_mask);

	rsp[0] = 0x900;
	phytium_sdci_request_done(host, mrq);
	return true;
}

/* returns true if command is fully handled; returns false otherwise */
static bool phytium_sdci_cmd_done(struct phytium_sdci_host *host, int events,
				  struct mmc_request *mrq,
				  struct mmc_command *cmd)
{
	bool done = false;
	bool sbc_error;
	unsigned long flags;
	u32 *rsp = cmd->resp;

	if (mrq->sbc && cmd == mrq->cmd &&
	    (events & SDCI_NORMAL_ISR_CC))
		phytium_sdci_auto_cmd_done(host, events, mrq->sbc);

	sbc_error = mrq->sbc && mrq->sbc->error;

	if (!sbc_error && !(events & (SDCI_NORMAL_ISR_CC |
				      SDCI_NORMAL_ISR_CR |
				      SDCI_NORMAL_ISR_TIMEOUT)))
		return done;

	spin_lock_irqsave(&host->lock, flags);
	done = !host->cmd;
	host->cmd = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	if (done)
		return true;

	sdr_clr_bits(host->base + SDCI_NORMAL_ISER, cmd_ints_mask);

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136) {
			rsp[0] = readl(host->base + SDCI_RESP0);
			rsp[1] = readl(host->base + SDCI_RESP1);
			rsp[2] = readl(host->base + SDCI_RESP2);
			rsp[3] = readl(host->base + SDCI_RESP3);
		} else
			rsp[0] = readl(host->base + SDCI_RESP0);

		if (cmd->opcode == SD_SEND_RELATIVE_ADDR)
			host->current_rca = rsp[0] & 0xFFFF0000;
	}

	if (!sbc_error &&
	    !(events & SDCI_NORMAL_ISR_CC) &&
	    (events & SDCI_NORMAL_ISR_TIMEOUT))
		cmd->error = -ETIMEDOUT;

	if (cmd->error)
		dev_dbg(host->dev,
			"%s: cmd=%d arg=%08X; rsp %08X; cmd_error=%d\n",
			__func__, cmd->opcode, cmd->arg, rsp[0],
			cmd->error);

	phytium_sdci_cmd_next(host, mrq, cmd);

	return true;
}

static bool set_databus_width(struct phytium_sdci_host *host)
{
	bool res;
	u32 cmd, resp_type, arg;

	cmd = SD_APP_SET_BUS_WIDTH;
	resp_type = 0x2;
	arg = 0x2;
	res = phytium_sdci_private_send_cmd(host, cmd, resp_type, arg);
	if (!res)
		return false;

	cmd = MMC_APP_CMD;
	resp_type = 0x2;
	arg = host->current_rca;
	res = phytium_sdci_private_send_cmd(host, cmd, resp_type, arg);
	if (!res)
		return false;

	return true;
}


static void phytium_sdci_start_command(struct phytium_sdci_host *host,
				       struct mmc_request *mrq,
				       struct mmc_command *cmd)
{
	u32 rawcmd;
	struct mmc_data *data = mrq->data;
	dma_addr_t dma_adtc_buf;
	u32 dma_bufh, dma_bufl, block_cnt = 0;

	WARN_ON(host->cmd);
	host->cmd = cmd;

	cmd->error = 0;
	rawcmd = phytium_sdci_cmd_prepare_raw_cmd(host, mrq, cmd);
	if (cmd->opcode == MMC_STOP_TRANSMISSION ||
	    cmd->opcode == MMC_SEND_STATUS)
		writel(1, host->base + SDCI_ERROR_ISR);
	sdr_set_bits(host->base + SDCI_NORMAL_ISER, cmd_ints_mask);
	writel(rawcmd, host->base + SDCI_COMMAND);

	if ((cmd->flags & MMC_CMD_MASK) == MMC_CMD_ADTC) {
		WARN_ON(host->data);
		host->data = data;

		dma_adtc_buf = host->dma_rx.bd_addr;
		dma_bufh = (u32) (dma_adtc_buf >> 32);
		dma_bufl = (u32) dma_adtc_buf;
		block_cnt = mrq->data->blocks;
		sdr_set_bits(host->base + SDCI_BD_ISER, data_ints_mask);
		writel(block_cnt, host->base + SDCI_BLK_CNT);

		if ((mrq->data->flags & MMC_DATA_READ) == MMC_DATA_READ) {
			writel(dma_bufl, host->base + SDCI_BD_RX);
			writel(dma_bufh, host->base + SDCI_BD_RX);
			writel(cmd->arg, host->base + SDCI_BD_RX);
			writel(0, host->base + SDCI_BD_RX);
		} else {
			writel(dma_bufl, host->base + SDCI_BD_TX);
			writel(dma_bufh, host->base + SDCI_BD_TX);
			writel(cmd->arg, host->base + SDCI_BD_TX);
			writel(0, host->base + SDCI_BD_TX);
		}
	} else {
		writel(cmd->arg, host->base + SDCI_ARGUMENT);
	}
}

static void phytium_sdci_cmd_next(struct phytium_sdci_host *host,
				  struct mmc_request *mrq,
				  struct mmc_command *cmd)
{
	if (cmd->error || (mrq->sbc && mrq->sbc->error))
		phytium_sdci_request_done(host, mrq);
	else if (cmd == mrq->sbc)
		phytium_sdci_start_command(host, mrq, mrq->cmd);
	else if (!cmd->data)
		phytium_sdci_request_done(host, mrq);
}

static int phytium_sdci_cmd13_process(struct phytium_sdci_host *host,
				       struct mmc_request *mrq,
				       struct mmc_data *data,
				       u32 wait_timeout_ms,
				       u32 send_once_time_ms)
{
	u32 private_cmd, resp_type, arg, temp, sd_status;
	unsigned long deadline_time;
	bool res;

	deadline_time = jiffies + msecs_to_jiffies(wait_timeout_ms);

	do {
		private_cmd = MMC_SEND_STATUS;
		resp_type = 0x2;
		arg = host->current_rca;

		res = phytium_sdci_private_send_cmd(host, private_cmd, resp_type, arg);
		if (!res) {
			sd_status = readl(host->base + SDCI_STATUS);
			if (sd_status & SDCI_STATUS_CDSL) {
				phytium_sdci_unexpected_error_handler(host, mrq, data,
								      ERR_CARD_ABSENT);
				dev_err(host->dev,
					"[%s][%d] Card absent! private_cmd(%d)\n",
					__func__, __LINE__, private_cmd);
			} else {
				phytium_sdci_unexpected_error_handler(host, mrq, data,
								      ERR_CMD_RESPONED);

				dev_err(host->dev,
					"[%s][%d] private_cmd(%d) response errored\n",
					__func__, __LINE__, private_cmd);
				phytium_sd_error(host);
			}
			writel(1, host->base + SDCI_BD_ISR);
			return SDCI_CMD13_FAILED;
		}

		temp = readl(host->base + SDCI_RESP0);

		if (time_after(jiffies, deadline_time)) {

			if (mrq->cmd->opcode == MMC_SEND_STATUS)
				return SDCI_CMD13_OK;

			dev_err(host->dev,
				"SD card is not in transfer mode,timeout:%d,rsp[0]:%x\n",
				wait_timeout_ms, temp);

			phytium_sdci_unexpected_error_handler(host, mrq, data,
							      ERR_TIMEOUT);
			phytium_sd_error(host);
			return SDCI_CMD13_FAILED;
		}

		writel(1, host->base + SDCI_NORMAL_ISR);

		if (CARD_TRAN_STATE != (temp & CARD_CURRENT_STATE) && send_once_time_ms)
			mdelay(send_once_time_ms);

	} while (CARD_TRAN_STATE != (temp & CARD_CURRENT_STATE));

	return SDCI_CMD13_OK;
}

static void phytium_sdci_ops_request(struct mmc_host *mmc,
				     struct mmc_request *mrq)
{
	struct phytium_sdci_host *host = mmc_priv(mmc);
	unsigned long flags;
	bool res;
	u32 status_sd;
	int res_cmd13;

	host->error = 0;
	WARN_ON(host->mrq);
	host->mrq = mrq;

	dev_dbg(host->dev, "%s: mrq->cmd->opcode: %d, mrq->cmd->arg: 0x%x\n",
		__func__, mrq->cmd->opcode, mrq->cmd->arg);

	if (mrq->cmd->opcode == MMC_SEND_STATUS &&
	    (mrq->cmd->flags & MMC_CMD_MASK) != MMC_CMD_ADTC) {
		u32 status = readl(host->base + SDCI_STATUS);

		if (status & SDCI_STATUS_CDSL) {
			phytium_sdci_unexpected_error_handler(host, mrq, NULL,
							      ERR_CARD_ABSENT);
			return;
		}

		res_cmd13 = phytium_sdci_cmd13_process(host, mrq, NULL, 400, 5);
		if (res_cmd13 == SDCI_CMD13_FAILED)
			return;
	} else if (mrq->cmd->opcode == MMC_STOP_TRANSMISSION) {
		status_sd = readl(host->base + SDCI_STATUS);
		if (status_sd & SDCI_STATUS_CDSL) {
			phytium_sdci_unexpected_error_handler(host, mrq, NULL,
							      ERR_CARD_ABSENT);
			return;
		}
	}

	if (mrq->data) {
		phytium_sdci_prepare_data(host, mrq);
		if (mrq->cmd->opcode == MMC_READ_MULTIPLE_BLOCK ||
		    mrq->cmd->opcode == MMC_READ_SINGLE_BLOCK ||
		    mrq->cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK ||
		    mrq->cmd->opcode == MMC_WRITE_BLOCK) {
			host->adtc_type = BLOCK_RW_ADTC;
			phytium_sdci_start_data(host, mrq,
						mrq->cmd, mrq->data);
			return;
		}
		host->adtc_type = COMMOM_ADTC;
	}

	if (mrq->cmd->opcode == SD_IO_RW_DIRECT ||
	    mrq->cmd->opcode == SD_IO_SEND_OP_COND) {
		spin_lock_irqsave(&host->lock, flags);
		host->mrq = NULL;
		host->cmd = NULL;
		spin_unlock_irqrestore(&host->lock, flags);
		mrq->cmd->error = -EINVAL;
		mmc_request_done(host->mmc, mrq);

		return;
	}

	if (mrq->cmd->opcode == SD_APP_SEND_SCR) {
		res = set_databus_width(host);
		if (!res) {
			phytium_sdci_unexpected_error_handler(host, mrq, NULL, ERR_CMD_RESPONED);
			return;
		}
	}

	/* if SBC is required, we have HW option and SW option.
	 * if HW option is enabled, and SBC does not have "special" flags,
	 * use HW option,  otherwise use SW option
	 */
	if (mrq->sbc &&
	    (!mmc_card_mmc(mmc->card) || (mrq->sbc->arg & 0xFFFF0000)))
		phytium_sdci_start_command(host, mrq, mrq->sbc);
	else
		phytium_sdci_start_command(host, mrq, mrq->cmd);
}

static void phytium_sdci_data_xfer_next(struct phytium_sdci_host *host,
					struct mmc_request *mrq,
					struct mmc_data *data)
{
	if (mmc_op_multi(mrq->cmd->opcode) &&
	    mrq->stop && !mrq->stop->error &&
	    !mrq->sbc && host->is_multi_rw_only_one_blkcnt) {
		host->is_multi_rw_only_one_blkcnt = false;
		phytium_sdci_auto_command_done(host, SDCI_NORMAL_ISR_CC, mrq, mrq->stop);
	} else if (mmc_op_multi(mrq->cmd->opcode) &&
		   mrq->stop && !mrq->stop->error &&
		   !mrq->sbc)
		phytium_sdci_start_command(host, mrq, mrq->stop);
	else
		phytium_sdci_request_done(host, mrq);
}

static inline void get_data_buffer(struct mmc_data *data,
				   u32 *bytes, u32 **pointer)
{
	struct scatterlist *sg;

	sg = &data->sg[0];
	*bytes = sg->length;
	*pointer = sg_virt(sg);
}

static bool phytium_sdci_data_xfer_done(struct phytium_sdci_host *host,
					u32 events, struct mmc_request *mrq,
					struct mmc_data *data)
{
	struct mmc_command *stop = data->stop;
	unsigned long flags;
	bool done;
	unsigned int check_data;
	u32 sg_length, i;
	u32 *sg_virt_addr;

	check_data = events & (SDCI_BD_ISR_TRS_R | SDCI_BD_ISR_TRS_W | SDCI_BD_ISR_EDTE);

	spin_lock_irqsave(&host->lock, flags);
	done = !host->data;
	if (check_data)
		host->data = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	if (done)
		return true;

	if (check_data || (stop && stop->error)) {
		sdr_clr_bits(host->base + SDCI_BD_ISER, data_ints_mask);
		dev_dbg(host->dev, "DMA stop\n");

		if (((events & SDCI_BD_ISR_TRS_R) ||
		     (events & SDCI_BD_ISR_TRS_W)) &&
		     (!stop || !stop->error)) {
			if ((mrq->cmd->flags & MMC_CMD_MASK) == MMC_CMD_ADTC &&
			    (host->adtc_type == COMMOM_ADTC)) {
				get_data_buffer(data, &sg_length,
						&host->sg_virt_addr);
				sg_virt_addr = host->sg_virt_addr;

				for (i = 0; i < (sg_length/4); i++) {
					*sg_virt_addr = host->dma_rx.buf[i];
					sg_virt_addr++;
				}
			}
			data->bytes_xfered = data->blocks * data->blksz;
		} else {
			dev_dbg(host->dev, "interrupt events: %x\n", events);
			phytium_sdci_reset_hw(host);
			data->bytes_xfered = 0;
			dev_dbg(host->dev, "%s: cmd=%d; blocks=%d",
				__func__, mrq->cmd->opcode, data->blocks);
			dev_dbg(host->dev, "data_error=%d xfer_size=%d\n",
				(int)data->error, data->bytes_xfered);
		}

		phytium_sdci_data_xfer_next(host, mrq, data);
		done = true;
	}

	return done;
}


static int phytium_sdci_card_busy(struct mmc_host *mmc)
{
	struct phytium_sdci_host *host = mmc_priv(mmc);
	u32 status;

	/* check if any pin between dat[0:3] is low */
	status = readl(host->base + SDCI_STATUS);
	if (((status >> 20) & 0xf) != 0xf)
		return 1;

	return 0;
}

static void phytium_sdci_request_timeout(struct work_struct *work)
{
	struct phytium_sdci_host *host;

	host = container_of(work, struct phytium_sdci_host, req_timeout.work);
	dev_err(host->dev, "%s: aborting cmd/data/mrq\n", __func__);
	if (host->mrq) {
		dev_err(host->dev, "%s: aborting mrq=%p cmd=%d\n", __func__,
			host->mrq, host->mrq->cmd->opcode);
		if (host->cmd) {
			dev_err(host->dev, "%s: aborting cmd=%d\n",
				__func__, host->cmd->opcode);
			phytium_sdci_cmd_done(host, SDCI_NORMAL_ISR_TIMEOUT,
					      host->mrq, host->cmd);
		} else if (host->data) {
			dev_err(host->dev, "%s: abort data: cmd%d; %d blocks\n",
				__func__, host->mrq->cmd->opcode,
				host->data->blocks);
			phytium_sdci_data_xfer_done(host, SDCI_BD_ISR_EDTE,
						    host->mrq, host->data);
		}
	}
}

static void hotplug_timer_func(struct timer_list *t)
{
	struct phytium_sdci_host *host;
	u32 status;

	host = from_timer(host, t, hotplug_timer);
	if (!host)
		dev_err(host->dev, "%s: Not find host!\n", __func__);
	status = readl(host->base + SDCI_STATUS);

	if (status & SDCI_STATUS_CDSL) { /* card absent */
		if (host->mmc->card) {
			cancel_delayed_work(&host->mmc->detect);
			mmc_detect_change(host->mmc,
					msecs_to_jiffies(100));
		}
	} else { /* card insert */
		cancel_delayed_work(&host->mmc->detect);
		mmc_detect_change(host->mmc, msecs_to_jiffies(200));
	}
}

static irqreturn_t phytium_sdci_irq(int irq, void *dev_id)
{
	struct phytium_sdci_host *host = (struct phytium_sdci_host *) dev_id;
	unsigned long flags;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	u32 events;

	if (!host)
		return IRQ_NONE;

	spin_lock_irqsave(&host->lock, flags);
	events = readl(host->base + SDCI_NORMAL_ISR);
	/* clear interrupts */
	writel(1, host->base + SDCI_NORMAL_ISR);

	mrq = host->mrq;
	cmd = host->cmd;
	spin_unlock_irqrestore(&host->lock, flags);

	if (events & (SDCI_NORMAL_ISR_CR | SDCI_NORMAL_ISR_CI)) {
		mod_timer(&host->hotplug_timer,
			  jiffies + usecs_to_jiffies(30000));
		goto irq_out;
	}

	if (!(events & cmd_ints_mask))
		goto irq_out;

	if (!mrq) {
		dev_err(host->dev, "%s: MRQ=NULL; events=%08X\n",
			__func__, events);
		WARN_ON(1);
		goto irq_out;
	}

	dev_dbg(host->dev, "%s: events=%08X\n", __func__, events);

	if (cmd)
		phytium_sdci_cmd_done(host, events, mrq, cmd);

irq_out:
	return IRQ_HANDLED;
}

static irqreturn_t phytium_sdci_dma_irq(int irq, void *dev_id)
{
	struct phytium_sdci_host *host = (struct phytium_sdci_host *) dev_id;
	unsigned long flags;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	u32 events;

	spin_lock_irqsave(&host->lock, flags);
	events = readl(host->base + SDCI_BD_ISR);
	writel(1, host->base + SDCI_BD_ISR);

	mrq = host->mrq;
	cmd = host->cmd;
	data = host->data;
	spin_unlock_irqrestore(&host->lock, flags);

	if (!(events & data_ints_mask))
		goto dma_irq_out;

	if (!mrq) {
		dev_err(host->dev,
			"%s: MRQ=NULL; events=%08X\n",
			__func__, events);
		goto dma_irq_out;
	}

	dev_dbg(host->dev, "%s: events=%08X\n", __func__, events);

	if (data)
		phytium_sdci_data_xfer_done(host, events, mrq, data);

dma_irq_out:
	return IRQ_HANDLED;
}

static irqreturn_t phytium_sdci_err_irq(int irq, void *dev_id)
{
	struct phytium_sdci_host *host = (struct phytium_sdci_host *) dev_id;
	unsigned long flags;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	u32 events;

	if (!host)
		return IRQ_NONE;

	spin_lock_irqsave(&host->lock, flags);
	events = readl(host->base + SDCI_ERROR_ISR);
	mrq = host->mrq;
	cmd = host->cmd;
	data = host->data;
	spin_unlock_irqrestore(&host->lock, flags);

	if (!(events&err_ints_mask))
		goto err_irq_out;

	if (!mrq) {
		sdr_clr_bits(host->base + SDCI_NORMAL_ISER, SDCI_NORMAL_ISR_EI);
		writel(1, host->base + SDCI_ERROR_ISR);
		dev_err(host->dev, "%s: MRQ=NULL; events=%08X\n", __func__, events);
		goto err_irq_out;
	}
	sdr_clr_bits(host->base + SDCI_NORMAL_ISER, SDCI_NORMAL_ISR_EI);
	if (data) {
		dev_err(host->dev,
			"[%s][%d]:  cmd(%d); %d read blocks, status:%x,flag:%x\n",
			__func__, __LINE__, mrq->cmd->opcode, data->blocks, events, data->flags);
		data->error = -ETIMEDOUT;
		if ((data->flags & MMC_DATA_READ) == MMC_DATA_READ ||
		    (data->flags & MMC_DATA_WRITE) == MMC_DATA_WRITE)
			phytium_sdci_data_xfer_done(host, SDCI_BD_ISR_EDTE | SDCI_BD_ISR_TRS_R,
						    mrq, data);
		mrq->cmd->error = -ETIMEDOUT;
		mmc_request_done(host->mmc, mrq);
	} else if (cmd) {
		phytium_sdci_cmd_done(host, SDCI_NORMAL_ISR_TIMEOUT, mrq, cmd);
	}

	writel(1, host->base + SDCI_NORMAL_ISR);
	writel(1, host->base + SDCI_ERROR_ISR);
err_irq_out:
	return IRQ_HANDLED;
}

static void phytium_sdci_init_hw(struct phytium_sdci_host *host)
{
	u32 val;

	/* Reset */
	phytium_sdci_reset_hw(host);

	val = SDCI_SEN_CREFR_VAL | SDCI_SEN_DEBNCE_VAL;
	writel(val, host->base + SDCI_SD_SEN);

	/* Disable and clear all interrupts */
	writel(0, host->base + SDCI_NORMAL_ISER);
	writel(0, host->base + SDCI_ERROR_ISER);
	writel(0, host->base + SDCI_BD_ISER);

	writel(1, host->base + SDCI_NORMAL_ISR);
	writel(1, host->base + SDCI_ERROR_ISR);
	writel(1, host->base + SDCI_BD_ISR);

	sdr_set_bits(host->base + SDCI_NORMAL_ISER,
		     SDCI_SDCI_NORMAL_ISER_ECI|SDCI_SDCI_NORMAL_ISER_ECR);
	/* Configure default cmd timeout to 0.1(s)s = val/25M */
	val = SDCI_F_MAX / 10;
	writel(val, host->base + SDCI_TIMEOUT_CMD);
	writel(SDCI_TIMEOUT_DATA_VALUE, host->base + SDCI_TIMEOUT_DATA);

	val = 0x0F00;
	writel(val, host->base + SDCI_CONTROLLER);

	dev_dbg(host->dev, "init hardware done!");
}

static void phytium_sdci_deinit_hw(struct phytium_sdci_host *host)
{
	/* Disable and clear all interrupts */
	writel(0, host->base + SDCI_NORMAL_ISER);
	writel(0, host->base + SDCI_ERROR_ISER);
	writel(0, host->base + SDCI_BD_ISER);

	writel(0, host->base + SDCI_NORMAL_ISR);
	writel(0, host->base + SDCI_ERROR_ISR);
	writel(0, host->base + SDCI_BD_ISR);
}

static void phytium_sdci_ops_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct phytium_sdci_host *host = mmc_priv(mmc);

	if (ios->bus_width == MMC_BUS_WIDTH_4)
		mmc->caps = mmc->caps & (~MMC_CAP_4_BIT_DATA);

	/* Suspend/Resume will do power off/on */
	switch (ios->power_mode) {
	case MMC_POWER_UP:
		writel(SDCI_POWER_ON, host->base + SDCI_POWER);
		break;
	case MMC_POWER_ON:
		phytium_sdci_set_clk(host, ios);
		break;
	case MMC_POWER_OFF:
		writel(SDCI_POWER_OFF, host->base + SDCI_POWER);
		break;
	default:
		break;
	}
}

static int phytium_sdci_get_cd(struct mmc_host *mmc)
{
	struct phytium_sdci_host *host = mmc_priv(mmc);
	u32 status = readl(host->base + SDCI_STATUS);

	if (((status >> 19) & 0x1) == 0x1)
		return 0;

	return 1;
}

static void phytium_sdci_hw_reset(struct mmc_host *mmc)
{
	struct phytium_sdci_host *host = mmc_priv(mmc);

	sdr_set_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_SRST);
	sdr_clr_bits(host->base + SDCI_SOFTWARE, SDCI_SOFTWARE_SRST);
	while (!(readl(host->base + SDCI_STATUS) & SDCI_STATUS_IDIE))
		cpu_relax();
}

static struct mmc_host_ops phytium_sdci_ops = {
	.request = phytium_sdci_ops_request,
	.set_ios = phytium_sdci_ops_set_ios,
	.get_cd = phytium_sdci_get_cd,
	.card_busy = phytium_sdci_card_busy,
	.hw_reset = phytium_sdci_hw_reset,
};

static bool phytium_sdci_private_send_cmd(struct phytium_sdci_host *host,
					  u32 cmd, u32 resp_type, u32 arg)
{
	u32 temp, sd_cmd, sd_arg, sd_status;
	unsigned long deadline_time;

	writel(1, host->base + SDCI_NORMAL_ISR);
	writel(1, host->base + SDCI_ERROR_ISR);

	sd_cmd = (cmd << 8) | resp_type;
	sd_arg = arg;
	writel(sd_cmd, host->base + SDCI_COMMAND);
	writel(sd_arg, host->base + SDCI_ARGUMENT);

	if (cmd == MMC_STOP_TRANSMISSION)
		deadline_time = jiffies + msecs_to_jiffies(1000);
	else
		deadline_time = jiffies + msecs_to_jiffies(100);

	temp = readl(host->base + SDCI_NORMAL_ISR);
	while ((temp & SDCI_NORMAL_ISR_CC) != SDCI_NORMAL_ISR_CC) {
		sd_status = readl(host->base + SDCI_STATUS);
		if (sd_status & SDCI_STATUS_CDSL)
			return false;

		temp = readl(host->base + SDCI_NORMAL_ISR);
		if (time_after(jiffies, deadline_time))
			return false;

		if (cmd == MMC_STOP_TRANSMISSION)
			mdelay(1);
	}

	return true;
}

static int phytium_sdci_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct phytium_sdci_host *host;
	struct resource *res;
	int ret;
	const struct acpi_device_id *match;
	struct device *dev = &pdev->dev;

	/* Allocate MMC host for this device */
	mmc = mmc_alloc_host(sizeof(struct phytium_sdci_host), &pdev->dev);
	if (!mmc)
		return -ENOMEM;

	host = mmc_priv(mmc);
	ret = mmc_of_parse(mmc);
	if (ret)
		goto host_free;

	if (dev->of_node) {
		host->src_clk = devm_clk_get(&pdev->dev, "phytium_sdc_clk");
		if (IS_ERR(host->src_clk)) {
			ret = PTR_ERR(host->src_clk);
			goto host_free;
		}

		host->clk_rate = clk_get_rate(host->src_clk);
		if (device_property_read_bool(dev, "no-dma-coherent"))
			dev->dma_coherent = false;
	} else if (has_acpi_companion(dev)) {
		match = acpi_match_device(dev->driver->acpi_match_table, dev);
		if (!match) {
			dev_err(dev, "Error ACPI match data is missing\n");
			return -ENODEV;
		}

		acpi_dma_configure(dev, DEV_DMA_NOT_SUPPORTED);

		host->clk_rate = 600000000;
	} else {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	dma_set_mask(dev, DMA_BIT_MASK(40));
	dma_set_coherent_mask(dev, DMA_BIT_MASK(40));

	timer_setup(&host->hotplug_timer, hotplug_timer_func, 0);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	host->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(host->base)) {
		ret = PTR_ERR(host->base);
		goto host_free;
	}

	host->irq = platform_get_irq(pdev, 1);
	if (host->irq < 0) {
		ret = -EINVAL;
		goto host_free;
	}

	host->irq_err = platform_get_irq(pdev, 2);
	if (host->irq_err < 0) {
		ret = -EINVAL;
		goto host_free;
	}

	host->irq_bd = platform_get_irq(pdev, 0);
	if (host->irq_bd < 0) {
		ret = -EINVAL;
		goto host_free;
	}

	host->dev = &pdev->dev;
	host->mmc = mmc;

	if ((4 * SDCI_F_MAX) > host->clk_rate)
		host->clk_div	= 1;
	else
		host->clk_div	= ((host->clk_rate / (2 * SDCI_F_MAX)) - 1);

	/* Set host parameters to mmc */
	mmc->f_min = SDCI_F_MIN;
	mmc->f_max = (host->clk_rate / ((host->clk_div + 1) * 2));
	mmc->ops = &phytium_sdci_ops;
	mmc->ocr_avail	= MMC_VDD_32_33 | MMC_VDD_33_34;

	mmc->caps |= host->caps;
	/* MMC core transfer sizes tunable parameters */
	mmc->max_segs = MAX_BD_NUM;
	mmc->max_seg_size = 512 * 1024;
	mmc->max_blk_size = 512;
	mmc->max_req_size = 512 * 1024;
	mmc->max_blk_count = mmc->max_req_size / 512;

	host->dma_rx.buf = dma_alloc_coherent(&pdev->dev,
					       MAX_BD_NUM,
					       &host->dma_rx.bd_addr,
					       GFP_KERNEL);
	if (!host->dma_rx.buf) {
		ret = -ENOMEM;
		goto release_mem;
	}

	host->cmd_timeout = msecs_to_jiffies(100);
	host->data_timeout = msecs_to_jiffies(250);

	INIT_DELAYED_WORK(&host->req_timeout, phytium_sdci_request_timeout);
	spin_lock_init(&host->lock);

	platform_set_drvdata(pdev, mmc);
	phytium_sdci_init_hw(host);

	ret = devm_request_irq(&pdev->dev, host->irq, phytium_sdci_irq,
			       IRQF_SHARED, pdev->name, host);
	if (ret)
		goto release;

	ret = devm_request_irq(&pdev->dev, host->irq_err, phytium_sdci_err_irq,
			       IRQF_SHARED, pdev->name, host);
	if (ret)
		goto release;

	ret = devm_request_irq(&pdev->dev, host->irq_bd, phytium_sdci_dma_irq,
			       IRQF_SHARED, pdev->name, host);
	if (ret)
		goto release;

	ret = mmc_add_host(mmc);
	if (ret)
		goto release;

	return 0;

release:
	platform_set_drvdata(pdev, NULL);
	phytium_sdci_deinit_hw(host);
release_mem:
	if (host->dma_rx.buf)
		dma_free_coherent(&pdev->dev, MAX_BD_NUM,
				  host->dma_rx.buf,
				  host->dma_rx.bd_addr);
host_free:
	mmc_free_host(mmc);

	return ret;
}

static int phytium_sdci_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct phytium_sdci_host *host;

	mmc = platform_get_drvdata(pdev);
	host = mmc_priv(mmc);

	cancel_delayed_work_sync(&host->req_timeout);
	platform_set_drvdata(pdev, NULL);
	mmc_remove_host(host->mmc);
	phytium_sdci_deinit_hw(host);

	if (host->dma_rx.buf)
		dma_free_coherent(&pdev->dev, MAX_BD_NUM,
				  host->dma_rx.buf, host->dma_rx.bd_addr);

	mmc_free_host(host->mmc);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phytium_sdci_suspend(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_sdci_host *host = mmc_priv(mmc);

	phytium_sdci_deinit_hw(host);
	return 0;
}

static int phytium_sdci_resume(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_sdci_host *host = mmc_priv(mmc);

	phytium_sdci_init_hw(host);
	mmc->caps = mmc->caps | MMC_CAP_4_BIT_DATA;

	return 0;
}
#endif

#ifdef CONFIG_PM
static int phytium_sdci_runtime_suspend(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_sdci_host *host = mmc_priv(mmc);

	phytium_sdci_deinit_hw(host);

	return 0;
}

static int phytium_sdci_runtime_resume(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_sdci_host *host = mmc_priv(mmc);

	phytium_sdci_init_hw(host);

	return 0;
}

static const struct dev_pm_ops phytium_sdci_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_sdci_suspend,
				phytium_sdci_resume)
	SET_RUNTIME_PM_OPS(phytium_sdci_runtime_suspend,
			   phytium_sdci_runtime_resume, NULL)
};
#else
#define phytium_sdci_dev_pm_ops NULL
#endif

static const struct of_device_id phytium_sdci_of_ids[] = {
	{ .compatible = "phytium,sdci", },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_sdci_of_ids);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_sdci_acpi_ids[] = {
	{ .id = "PHYT0005" },
	{ }
};

MODULE_DEVICE_TABLE(acpi, phytium_sdci_acpi_ids);
#else
#define phytium_sdci_acpi_ids NULL
#endif

static struct platform_driver phytium_sdci_driver = {
	.probe = phytium_sdci_probe,
	.remove = phytium_sdci_remove,
	.driver = {
		.name = "sdci-phytium",
		.of_match_table = phytium_sdci_of_ids,
		.acpi_match_table = phytium_sdci_acpi_ids,
		.pm = &phytium_sdci_dev_pm_ops,
	},
};

module_platform_driver(phytium_sdci_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium SD Card Interface driver");
