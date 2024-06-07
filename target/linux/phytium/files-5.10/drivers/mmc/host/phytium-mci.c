// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Phytium Multimedia Card Interface
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
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
#include <linux/swab.h>
#include <linux/pci.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/delay.h>
#include "phytium-mci.h"

static const u32 cmd_ints_mask = MCI_INT_MASK_RE | MCI_INT_MASK_CMD | MCI_INT_MASK_RCRC |
				 MCI_INT_MASK_RTO | MCI_INT_MASK_HTO | MCI_RAW_INTS_HLE;

static const u32 data_ints_mask = MCI_INT_MASK_DTO | MCI_INT_MASK_DCRC | MCI_INT_MASK_DRTO |
				  MCI_INT_MASK_SBE_BCI;
static const u32 cmd_err_ints_mask = MCI_INT_MASK_RTO | MCI_INT_MASK_RCRC | MCI_INT_MASK_RE |
				     MCI_INT_MASK_DCRC | MCI_INT_MASK_DRTO |
				     MCI_MASKED_INTS_SBE_BCI;

static const u32 dmac_ints_mask = MCI_DMAC_INT_ENA_FBE | MCI_DMAC_INT_ENA_DU |
				  MCI_DMAC_INT_ENA_NIS | MCI_DMAC_INT_ENA_AIS;
static const u32 dmac_err_ints_mask = MCI_DMAC_INT_ENA_FBE | MCI_DMAC_INT_ENA_DU |
				      MCI_DMAC_INT_ENA_AIS;

static void phytium_mci_cmd_next(struct phytium_mci_host *host,
				  struct mmc_request *mrq,
				  struct mmc_command *cmd);
static void phytium_mci_adma_reset(struct phytium_mci_host *host);
static void phytium_mci_send_cmd(struct phytium_mci_host *host, u32 cmd, u32 arg);
static bool phytium_mci_data_xfer_done(struct phytium_mci_host *host, u32 events,
					struct mmc_request *mrq, struct mmc_data *data);
static void phytium_mci_init_adma_table(struct phytium_mci_host *host,
					 struct phytium_mci_dma *dma);
static void phytium_mci_init_hw(struct phytium_mci_host *host);
static int phytium_mci_get_cd(struct mmc_host *mmc);
static int phytium_mci_err_irq(struct phytium_mci_host *host, u32 dmac_events, u32 events);

static void sdr_set_bits(void __iomem *reg, u32 bs)
{
	u32 val = readl(reg);

	val |= bs;
	writel(val, reg);
}

static void sdr_clr_bits(void __iomem *reg, u32 bs)
{
	u32 val = readl(reg);

	val &= ~bs;
	writel(val, reg);
}

static void phytium_mci_reset_hw(struct phytium_mci_host *host)
{
	sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_FIFO_RESET | MCI_CNTRL_DMA_RESET);

	while (readl(host->base + MCI_CNTRL) & (MCI_CNTRL_FIFO_RESET | MCI_CNTRL_DMA_RESET))
		cpu_relax();
	phytium_mci_send_cmd(host, MCI_CMD_UPD_CLK, 0);
}

static void phytium_mci_update_external_clk(struct phytium_mci_host *host, u32 uhs_reg_value)
{
	writel(0, host->base + MCI_UHS_REG_EXT);
	writel(uhs_reg_value, host->base + MCI_UHS_REG_EXT);
	while (!(readl(host->base + MCI_CCLK_RDY) & 0x1))
		cpu_relax();
}

static void phytium_mci_prepare_data(struct phytium_mci_host *host,
				      struct mmc_request *mrq)
{
	struct mmc_data *data = mrq->data;

	if (!(data->host_cookie & MCI_PREPARE_FLAG)) {
		data->host_cookie |= MCI_PREPARE_FLAG;
		data->sg_count = dma_map_sg(host->dev, data->sg, data->sg_len,
					    mmc_get_dma_dir(data));
	}
}

static void phytium_mci_unprepare_data(struct phytium_mci_host *host,
					struct mmc_request *mrq)
{
	struct mmc_data *data = mrq->data;

	if (data->host_cookie & MCI_ASYNC_FLAG)
		return;

	if (data->host_cookie & MCI_PREPARE_FLAG) {
		dma_unmap_sg(host->dev, data->sg, data->sg_len, mmc_get_dma_dir(data));
		data->host_cookie &= ~MCI_PREPARE_FLAG;
	}
}

static void phytium_mci_send_cmd(struct phytium_mci_host *host, u32 cmd, u32 arg)
{
	int rc;
	u32 data;

	writel(arg, host->base + MCI_CMDARG);
	wmb(); /* drain writebuffer */

	rc = readl_relaxed_poll_timeout(host->base + MCI_STATUS,
					 data,
					 !(data & MCI_STATUS_CARD_BUSY),
					 0, 100 * 1000);
	if (rc == -ETIMEDOUT)
		pr_debug("%s %d, timeout mci_status: 0x%08x\n", __func__, __LINE__, data);

	writel(MCI_CMD_START | cmd, host->base + MCI_CMD);

	rc = readl_relaxed_poll_timeout(host->base + MCI_CMD,
					 data,
					 !(data & MCI_CMD_START),
					 0, 100 * 1000);
	if (rc == -ETIMEDOUT)
		pr_debug("%s %d, timeout mci_cmd: 0x%08x\n", __func__, __LINE__, data);
}

static void phytium_mci_update_cmd11(struct phytium_mci_host *host, u32 cmd)
{
	writel(MCI_CMD_START | cmd, host->base + MCI_CMD);

	while (readl(host->base + MCI_CMD) & MCI_CMD_START)
		cpu_relax();
}

static void phytium_mci_set_clk(struct phytium_mci_host *host, struct mmc_ios *ios)
{
	u32 div = 0xff, drv = 0, sample = 0;
	unsigned long clk_rate;
	u32 mci_cmd_bits = MCI_CMD_UPD_CLK;
	u32 cmd_reg;
	u32 cur_cmd_index;
	u32 first_uhs_div, tmp_ext_reg;

	cmd_reg = readl(host->base + MCI_CMD);
	cur_cmd_index = cmd_reg & 0x3F;

	if (cur_cmd_index == SD_SWITCH_VOLTAGE)
		mci_cmd_bits |= MCI_CMD_VOLT_SWITCH;
	if (ios->clock) {
		if (host->current_ios_clk == ios->clock)
			return;

		dev_dbg(host->dev, "will change clock, host->clk_rate: %ld, ios->clock: %d\n",
			host->clk_rate, ios->clock);

		if (ios->clock >= 25000000)
			tmp_ext_reg = 0x202;
		else if (ios->clock == 400000)
			tmp_ext_reg = 0x502;
		else
			tmp_ext_reg = 0x302;

		phytium_mci_update_external_clk(host, tmp_ext_reg);
		sdr_clr_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);

		if (cur_cmd_index == SD_SWITCH_VOLTAGE)
			phytium_mci_update_cmd11(host, mci_cmd_bits | cmd_reg);
		else
			phytium_mci_send_cmd(host, mci_cmd_bits, 0);

		clk_rate = host->clk_rate;
		first_uhs_div = 1 + ((tmp_ext_reg >> 8)&0xFF);
		div = clk_rate / (2 * first_uhs_div * ios->clock);
		if (div > 2) {
			sample = div / 2 + 1;
			drv = sample - 1;
			writel((sample << 16) | (drv << 8) | (div & 0xff),
			       host->base + MCI_CLKDIV);
		} else if (div == 2) {
			drv = 0;
			sample = 1;
			writel((drv << 8) | (sample << 16) | (div & 0xff),
			       host->base + MCI_CLKDIV);
		}

		dev_dbg(host->dev, "UHS_REG_EXT ext: %x, CLKDIV: %x\n",
			readl(host->base + MCI_UHS_REG_EXT), readl(host->base + MCI_CLKDIV));

		sdr_set_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);

		if (cur_cmd_index == SD_SWITCH_VOLTAGE)
			phytium_mci_update_cmd11(host, mci_cmd_bits | cmd_reg);
		else
			phytium_mci_send_cmd(host, mci_cmd_bits, 0);

		host->current_ios_clk = ios->clock;

		dev_dbg(host->dev, "host->clk_rate: %ld, ios->clock: %d\n",
			host->clk_rate, ios->clock);
	} else {
		host->current_ios_clk = 0;
		sdr_clr_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);

		if (cur_cmd_index == SD_SWITCH_VOLTAGE)
			phytium_mci_update_cmd11(host, mci_cmd_bits | cmd_reg);
		else
			phytium_mci_send_cmd(host, mci_cmd_bits, 0);

		sdr_clr_bits(host->base + MCI_UHS_REG_EXT, MCI_EXT_CLK_ENABLE);
		dev_dbg(host->dev, "host->clk_rate: %ld, ios->clock: %d\n",
			host->clk_rate, ios->clock);
	}
}

static inline u32
phytium_mci_cmd_find_resp(struct phytium_mci_host *host,
			  struct mmc_request *mrq,
			  struct mmc_command *cmd)
{
	u32 resp;

	switch (mmc_resp_type(cmd)) {
	case MMC_RSP_R1:
	case MMC_RSP_R1B:
		resp = 0x5;
		break;

	case MMC_RSP_R2:
		resp = 0x7;
		break;

	case MMC_RSP_R3:
		resp = 0x1;
		break;

	case MMC_RSP_NONE:
	default:
		resp = 0x0;
		break;
	}

	return resp;
}

static inline
u32 phytium_mci_cmd_prepare_raw_cmd(struct phytium_mci_host *host,
				     struct mmc_request *mrq,
				     struct mmc_command *cmd)
{
	u32 opcode = cmd->opcode;
	u32 resp = phytium_mci_cmd_find_resp(host, mrq, cmd);
	u32 rawcmd = ((opcode & 0x3f) | ((resp & 0x7) << 6));

	if (opcode == MMC_GO_INACTIVE_STATE ||
	    (opcode == SD_IO_RW_DIRECT && ((cmd->arg >> 9) & 0x1FFFF) == SDIO_CCCR_ABORT))
		rawcmd |= (0x1 << 14);
	else if (opcode == SD_SWITCH_VOLTAGE)
		rawcmd |= (0x1 << 28);

	if (test_and_clear_bit(MCI_CARD_NEED_INIT, &host->flags))
		rawcmd |= (0x1 << 15);

	if (cmd->data) {
		struct mmc_data *data = cmd->data;

		rawcmd |= (0x1 << 9);

		if (data->flags & MMC_DATA_WRITE)
			rawcmd |= (0x1 << 10);
	}

	return (rawcmd | (0x1 << 29) | (0x1 << 31));
}

static inline void
phytium_mci_adma_write_desc(struct phytium_mci_host *host,
			     struct phytium_adma2_64_desc *desc,
			     dma_addr_t addr, u32 len, u32 attribute)
{
	desc->attribute = attribute;
	desc->len = len;
	desc->addr_lo = lower_32_bits(addr);
	desc->addr_hi = upper_32_bits(addr);
	dev_dbg(host->dev, "%s %d:addr_lo:0x%x ddr_hi:0x%x\n", __func__,
		__LINE__,  desc->addr_lo, desc->addr_hi);

	if ((attribute == 0x80000004) || (attribute == 0x8000000c)) {
		desc->desc_lo = 0;
		desc->desc_hi = 0;
	}
}

static void
phytium_mci_data_sg_write_2_admc_table(struct phytium_mci_host *host, struct mmc_data *data)
{
	struct phytium_adma2_64_desc *desc;
	u32 dma_len, i;
	dma_addr_t dma_address;
	struct scatterlist *sg;

	phytium_mci_init_adma_table(host, &host->dma);

	desc = host->dma.adma_table;
	for_each_sg(data->sg, sg, data->sg_count, i) {
		dma_address = sg_dma_address(sg);
		dma_len = sg_dma_len(sg);

		if (i == 0) {
			if (sg_is_last(sg) || (data->sg_count == 1 && dma_len == SD_BLOCK_SIZE))
				phytium_mci_adma_write_desc(host, desc, dma_address,
							     dma_len, 0x8000000c);
			else
				phytium_mci_adma_write_desc(host, desc, dma_address,
							     dma_len, 0x8000001a);
		} else if (sg_is_last(sg)) {
			phytium_mci_adma_write_desc(host, desc, dma_address,
						     dma_len, 0x80000004);
		} else {
			phytium_mci_adma_write_desc(host, desc, dma_address,
						     dma_len, 0x80000012);
		}

		desc++;
	}
}

static void
phytium_mci_data_sg_write_2_fifo(struct phytium_mci_host *host, struct mmc_data *data)
{
	struct scatterlist *sg;
	u32 dma_len, i, j;
	u32 *virt_addr;

	if (mmc_get_dma_dir(data) == DMA_TO_DEVICE) {
		writel(0x1<<10, host->base + MCI_CMD);
		for_each_sg(data->sg, sg, data->sg_count, i) {
			dma_len = sg_dma_len(sg);
			virt_addr = sg_virt(data->sg);
			for (j = 0; j < (dma_len / 4); j++) {
				writel(*virt_addr, host->base + MCI_DATA);
				virt_addr++;
			}
		}
	}
}

static void phytium_mci_restart_clk(struct phytium_mci_host *host)
{
	u32 clk_div, uhs;

	while (readl(host->base + MCI_CMD) & MCI_CMD_START)
		cpu_relax();
	sdr_clr_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);
	clk_div = readl(host->base + MCI_CLKDIV);
	uhs = readl(host->base + MCI_UHS_REG_EXT);
	writel(0, host->base + MCI_UHS_REG_EXT);
	writel(uhs, host->base + MCI_UHS_REG_EXT);
	while (!(readl(host->base + MCI_CCLK_RDY) & 0x1))
		cpu_relax();

	writel(clk_div, host->base + MCI_CLKDIV);
	sdr_set_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);
	writel(MCI_CMD_START | MCI_CMD_UPD_CLK, host->base + MCI_CMD);
	while (readl(host->base + MCI_CMD) & MCI_CMD_START)
		cpu_relax();
}

static int
phytim_mci_start_multiple_write(struct phytium_mci_host *host,
				struct mmc_request *mrq, u32 cnts, u32 offset)
{
	u32 rawcmd, cmd_status;
	struct mmc_command *cmd = mrq->cmd;
	u32 *rsp = cmd->resp;
	unsigned long deadline_time;

	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) && readl(host->base + MCI_CARD_DETECT))
		return -ESHUTDOWN;

	while ((readl(host->base + MCI_STATUS) & (MCI_STATUS_CARD_BUSY)))
		cpu_relax();

	writel(0xffffe, host->base + MCI_RAW_INTS);
	rawcmd = phytium_mci_cmd_prepare_raw_cmd(host, mrq, cmd);
	writel(mrq->data->blksz, host->base + MCI_BLKSIZ);
	writel(cnts * mrq->data->blksz, host->base + MCI_BYTCNT);
	writel(cmd->arg + offset, host->base + MCI_CMDARG);
	writel(rawcmd, host->base + MCI_CMD);
	deadline_time = jiffies + msecs_to_jiffies(200);

	cmd_status = readl(host->base + MCI_RAW_INTS);
	while (!(cmd_status & MCI_MASKED_INTS_CMD)) {
		if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) &&
		    readl(host->base + MCI_CARD_DETECT))
			return -ESHUTDOWN;

		cmd_status = readl(host->base + MCI_RAW_INTS);
		if (cmd_err_ints_mask & cmd_status)
			return -ESHUTDOWN;

		if (cmd_status & MCI_MASKED_INTS_CMD)
			break;

		if (time_after(jiffies, deadline_time))
			return -ESHUTDOWN;
	}

	if (cmd_status & MCI_MASKED_INTS_CMD) {
		if (cmd->flags & MMC_RSP_136) {
			rsp[3] = readl(host->base + MCI_RESP0);
			rsp[2] = readl(host->base + MCI_RESP1);
			rsp[1] = readl(host->base + MCI_RESP2);
			rsp[0] = readl(host->base + MCI_RESP3);
		} else {
			rsp[0] = readl(host->base + MCI_RESP0);
		}
	}
	deadline_time = jiffies + msecs_to_jiffies(1000);
	while (!(cmd_status & MCI_MASKED_INTS_DTO)) {
		if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) &&
		    readl(host->base + MCI_CARD_DETECT))
			return -ESHUTDOWN;
		cmd_status = readl(host->base + MCI_RAW_INTS);
		if (cmd_err_ints_mask & cmd_status)
			return -ESHUTDOWN;
		if (cmd_status & MCI_MASKED_INTS_DTO)
			return 0;
		if (time_after(jiffies, deadline_time))
			return -ESHUTDOWN;
	}
	return 0;
}

static int
phytium_mci_start_sbc_stop_cmd(struct phytium_mci_host *host, struct mmc_request *mrq,
			       struct mmc_command *cmd, u32 arg)
{
	u32 rawcmd, cmd_status;
	u32 *rsp = cmd->resp;
	unsigned long deadline_time;

	writel(0xffffe, host->base + MCI_RAW_INTS);

	while ((readl(host->base + MCI_STATUS) & (MCI_STATUS_CARD_BUSY)))
		cpu_relax();

	rawcmd = phytium_mci_cmd_prepare_raw_cmd(host, mrq, cmd);
	writel(arg, host->base + MCI_CMDARG);
	writel(rawcmd, host->base + MCI_CMD);

	deadline_time = jiffies + msecs_to_jiffies(200);
	cmd_status = readl(host->base + MCI_RAW_INTS);
	while (!(cmd_status & MCI_MASKED_INTS_CMD)) {
		if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) &&
		    readl(host->base + MCI_CARD_DETECT))
			return -ENOMEDIUM;

		cmd_status = readl(host->base + MCI_RAW_INTS);
		if (cmd_err_ints_mask & cmd_status)
			return -ETIMEDOUT;

		if (cmd_status & MCI_MASKED_INTS_CMD)
			break;

		if (time_after(jiffies, deadline_time))
			return -ETIMEDOUT;
	}

	if (cmd_status & MCI_MASKED_INTS_CMD) {
		if (cmd->flags & MMC_RSP_136) {
			rsp[3] = readl(host->base + MCI_RESP0);
			rsp[2] = readl(host->base + MCI_RESP1);
			rsp[1] = readl(host->base + MCI_RESP2);
			rsp[0] = readl(host->base + MCI_RESP3);
		} else {
			rsp[0] = readl(host->base + MCI_RESP0);
		}
	}

	if (cmd_err_ints_mask & cmd_status)
		return -ETIMEDOUT;

	return 0;
}

static void
phytium_mci_start_write_multiple_non_dma(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	struct mmc_data *data = mrq->data;
	u32 write_cnts, last_cnts;
	u32 i, j, k, send_cnt_one_sg, block_offset;
	int ret = 0, dma_len;
	struct scatterlist *sg;
	u32 *virt_addr = NULL;

	write_cnts = data->blocks / 4;
	(data->blocks % 4) ? write_cnts++ : write_cnts;
	last_cnts = data->blocks % 4;
	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) && readl(host->base + MCI_CARD_DETECT)) {
		ret = -ENOMEDIUM;
		goto write_err;
	}

	dev_dbg(host->dev, "%s: cmd:%d, block counts:%d\n",
		__func__, mrq->cmd->opcode, data->blocks);

	sdr_clr_bits(host->base + MCI_CNTRL, MCI_CNTRL_USE_INTERNAL_DMAC);
	sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_FIFO_RESET);
	while (readl(host->base + MCI_CNTRL) & MCI_CNTRL_FIFO_RESET)
		cpu_relax();
	sdr_clr_bits(host->base + MCI_BUS_MODE, MCI_BUS_MODE_DE);

	if (mmc_get_dma_dir(data) == DMA_TO_DEVICE) {
		block_offset = 0;
		for_each_sg(data->sg, sg, data->sg_count, i) {
			/* Each SG data transfor starts */
			dma_len = sg_dma_len(sg);
			send_cnt_one_sg = (dma_len / MCI_MAX_FIFO_CNT) + 1;
			virt_addr = sg_virt(sg);
			for (k = 0; k < send_cnt_one_sg; k++) {
				if (dma_len && dma_len >= MCI_MAX_FIFO_CNT) {
					/*first write sbc cmd*/
					ret = phytium_mci_start_sbc_stop_cmd(host, mrq,
									     mrq->sbc, 4);
					if (ret)
						goto write_err;
					writel(0x1 << 10, host->base + MCI_CMD);
					for (j = 0; j < (MCI_MAX_FIFO_CNT / 4); j++) {
						writel(*virt_addr, host->base + MCI_DATA);
						virt_addr++;
					}

					/*second write cmd25 here*/
					ret = phytim_mci_start_multiple_write(host, mrq, 4,
									      block_offset);
					if (ret)
						goto write_err;
					block_offset += 4;
					dma_len -= MCI_MAX_FIFO_CNT;
				} else if (dma_len > 0) {
					/*first write sbc cmd*/
					last_cnts = dma_len / 512;
					ret = phytium_mci_start_sbc_stop_cmd(host, mrq, mrq->sbc,
									     last_cnts);
					if (ret)
						goto write_err;
					writel(0x1 << 10, host->base + MCI_CMD);
					for (j = 0; j < (dma_len / 4); j++) {
						writel(*virt_addr, host->base + MCI_DATA);
						virt_addr++;
					}
					/*second write cmd25 here*/
					ret = phytim_mci_start_multiple_write(host, mrq, last_cnts,
									      block_offset);
					if (ret)
						goto write_err;
					block_offset += last_cnts;
					dma_len = 0;
				} else {
					dev_dbg(host->dev, "%s: sg %d end\n", __func__, i);
					break;
				}
			}
		}
	}

write_err:
	host->data = NULL;
	host->cmd = NULL;
	host->mrq = NULL;
	writel(0xffffe, host->base + MCI_RAW_INTS);
	if (ret) {
		data->bytes_xfered = 0;
		if (ret == -ESHUTDOWN) {
			sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_FIFO_RESET);
			while (readl(host->base + MCI_CNTRL) & MCI_CNTRL_FIFO_RESET)
				cpu_relax();

			sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_CONTROLLER_RESET);
			while (readl(host->base + MCI_STATUS) & MCI_STATUS_CARD_BUSY)
				sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_CONTROLLER_RESET);
			phytium_mci_restart_clk(host);
			phytium_mci_start_sbc_stop_cmd(host, mrq, mrq->stop, mrq->stop->arg);
		}
		data->error = -ETIMEDOUT;
		mrq->cmd->error = -ETIMEDOUT;
		mmc_request_done(host->mmc, mrq);
		return;
	}
	data->bytes_xfered = data->blocks * data->blksz;
	mmc_request_done(host->mmc, mrq);
}

static void
phytium_mci_start_data(struct phytium_mci_host *host, struct mmc_request *mrq,
			struct mmc_command *cmd, struct mmc_data *data)
{
	bool read;
	u32 rawcmd;
	unsigned long flags;


	WARN_ON(host->cmd);
	host->cmd = cmd;
	cmd->error = 0;
	WARN_ON(host->data);
	host->data = data;
	read = data->flags & MMC_DATA_READ;

	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) && readl(host->base + MCI_CARD_DETECT)) {
		phytium_mci_err_irq(host, 0, MCI_INT_MASK_RTO);
		return;
	}
	/* clear interrupts */
	writel(0xffffe, host->base + MCI_RAW_INTS);

	sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_FIFO_RESET | MCI_CNTRL_DMA_RESET);

	while (readl(host->base + MCI_CNTRL) & (MCI_CNTRL_FIFO_RESET | MCI_CNTRL_DMA_RESET))
		cpu_relax();

	if (host->adtc_type == COMMOM_ADTC)
		sdr_clr_bits(host->base + MCI_CNTRL, MCI_CNTRL_USE_INTERNAL_DMAC);
	else
		sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_USE_INTERNAL_DMAC);
	wmb(); /* drain writebuffer */
	sdr_clr_bits(host->base + MCI_CNTRL, MCI_CNTRL_INT_ENABLE);

	rawcmd = phytium_mci_cmd_prepare_raw_cmd(host, mrq, cmd);
	if (host->is_use_dma && host->adtc_type == BLOCK_RW_ADTC)
		phytium_mci_data_sg_write_2_admc_table(host, data);
	else
		phytium_mci_data_sg_write_2_fifo(host, data);

	spin_lock_irqsave(&host->lock, flags);
	sdr_set_bits(host->base + MCI_INT_MASK, cmd_ints_mask | data_ints_mask);
	if (host->is_use_dma && host->adtc_type == BLOCK_RW_ADTC) {
		sdr_set_bits(host->base + MCI_DMAC_INT_ENA, dmac_ints_mask);
		/* Enable the IDMAC */
		sdr_set_bits(host->base + MCI_BUS_MODE, MCI_BUS_MODE_DE);
		writel((u32)host->dma.adma_addr, host->base + MCI_DESC_LIST_ADDRL);
		writel((u32)(host->dma.adma_addr >> 32), host->base + MCI_DESC_LIST_ADDRH);
	}
	writel(mrq->data->blksz, host->base + MCI_BLKSIZ);
	writel(mrq->data->blocks * mrq->data->blksz, host->base + MCI_BYTCNT);
	sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_INT_ENABLE);
	writel(cmd->arg, host->base + MCI_CMDARG);
	wmb(); /* drain writebuffer */
	writel(rawcmd, host->base + MCI_CMD);
	spin_unlock_irqrestore(&host->lock, flags);

	mod_timer(&host->timeout_timer,
		jiffies + msecs_to_jiffies(MMC_REQ_TIMEOUT_MS));
}

static void phytium_mci_track_cmd_data(struct phytium_mci_host *host,
					struct mmc_command *cmd,
					struct mmc_data *data)
{
	if (host->error)
		dev_dbg(host->dev, "%s: cmd=%d arg=%08X; host->error=0x%08X\n",
			__func__, cmd->opcode, cmd->arg, host->error);
}

static void phytium_mci_request_done(struct phytium_mci_host *host, struct mmc_request *mrq)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	del_timer(&host->timeout_timer);
	host->mrq = NULL;
	if (host->cmd)
		host->cmd = NULL;
	spin_unlock_irqrestore(&host->lock, flags);
	phytium_mci_track_cmd_data(host, mrq->cmd, mrq->data);

	if (mrq->data)
		phytium_mci_unprepare_data(host, mrq);

	mmc_request_done(host->mmc, mrq);
}

static bool phytium_mci_cmd_done(struct phytium_mci_host *host, int events,
				 struct mmc_request *mrq, struct mmc_command *cmd)
{
	bool done = false;
	unsigned long flags;
	u32 *rsp = cmd->resp;

	if (!(events & (MCI_RAW_INTS_RCRC | MCI_RAW_INTS_RE | MCI_RAW_INTS_CMD |
	      MCI_RAW_INTS_RTO | MCI_INT_MASK_HTO))) {
		dev_err(host->dev, "No interrupt generation:h%x\n", events);
		return done;
	}

	spin_lock_irqsave(&host->lock, flags);
	done = !host->cmd;
	host->cmd = NULL;
	if (done) {
		spin_unlock_irqrestore(&host->lock, flags);
		return true;
	}
	sdr_clr_bits(host->base + MCI_INT_MASK, cmd_ints_mask);
	spin_unlock_irqrestore(&host->lock, flags);

	if (cmd->flags & MMC_RSP_PRESENT) {
		if (cmd->flags & MMC_RSP_136) {
			rsp[3] = readl(host->base + MCI_RESP0);
			rsp[2] = readl(host->base + MCI_RESP1);
			rsp[1] = readl(host->base + MCI_RESP2);
			rsp[0] = readl(host->base + MCI_RESP3);
		} else {
			/*
			 * Sometimes get ACMD41 cmd done irq but the respose index still the APP_CMD,
			 * so polling the mci status entill the respose index change.
			 */
			if (cmd->opcode == SD_APP_OP_COND) {
				int polling_cnt = 20;
				while (MMC_APP_CMD == MCI_STATUS_RESPOSE_INDEX(readl(host->base + MCI_STATUS))) {
					udelay(100);
					polling_cnt --;
					if (polling_cnt == 0) {
						dev_info(host->dev, "hw respose index not equal cmd opcode, respose value may error\n");
						break;
					}
				}
			}
			rsp[0] = readl(host->base + MCI_RESP0);
		}

		if (cmd->opcode == SD_SEND_RELATIVE_ADDR)
			host->current_rca = rsp[0] & 0xFFFF0000;
	}
	if (!(events & (MCI_RAW_INTS_CMD | MCI_INT_MASK_HTO))) {
		if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) && (events & MCI_RAW_INTS_RTO)
		   && readl(host->base + MCI_CARD_DETECT)) {
			cmd->error = -ENOMEDIUM;
			rsp[0] = 0;
		} else if (events & MCI_RAW_INTS_RTO ||
		    (cmd->opcode != MMC_SEND_TUNING_BLOCK &&
		     cmd->opcode != MMC_SEND_TUNING_BLOCK_HS200)) {
			cmd->error = -ETIMEDOUT;
		} else if (events & MCI_RAW_INTS_RCRC) {
			cmd->error = -EILSEQ;
		} else {
			cmd->error = -ETIMEDOUT;
		}
	}
	phytium_mci_cmd_next(host, mrq, cmd);
	return true;
}

static void phytium_mci_start_command(struct phytium_mci_host *host,
				       struct mmc_request *mrq,
				       struct mmc_command *cmd)
{
	u32 rawcmd;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	WARN_ON(host->cmd);
	host->cmd = cmd;
	cmd->error = 0;
	writel(0xffffe, host->base + MCI_RAW_INTS);

	rawcmd = phytium_mci_cmd_prepare_raw_cmd(host, mrq, cmd);
	spin_unlock_irqrestore(&host->lock, flags);

	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE) && readl(host->base + MCI_CARD_DETECT)) {
		phytium_mci_cmd_done(host, MCI_RAW_INTS_RTO, mrq, cmd);
		return;
	}

	spin_lock_irqsave(&host->lock, flags);
	sdr_set_bits(host->base + MCI_INT_MASK, cmd_ints_mask);
	writel(cmd->arg, host->base + MCI_CMDARG);
	writel(rawcmd, host->base + MCI_CMD);
	spin_unlock_irqrestore(&host->lock, flags);

	mod_timer(&host->timeout_timer,
		jiffies + msecs_to_jiffies(MMC_REQ_TIMEOUT_MS));
}

static void
phytium_mci_cmd_next(struct phytium_mci_host *host, struct mmc_request *mrq,
		      struct mmc_command *cmd)
{
	if ((cmd->error && !(cmd->opcode == MMC_SEND_TUNING_BLOCK ||
		cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200)) ||
		(mrq->sbc && mrq->sbc->error)) {
		phytium_mci_request_done(host, mrq);
	} else if (cmd == mrq->sbc) {
		if ((mrq->cmd->opcode == MMC_READ_MULTIPLE_BLOCK) ||
		    (mrq->cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) ||
		    (mrq->cmd->opcode == MMC_READ_SINGLE_BLOCK) ||
		    (mrq->cmd->opcode == MMC_WRITE_BLOCK)) {
			dev_dbg(host->dev, "%s %d:sbc done and next cmd :%d length:%d\n",
				__func__, __LINE__, mrq->cmd->opcode, mrq->data->sg->length);
			phytium_mci_prepare_data(host, mrq);
			if (host->is_use_dma)
				host->adtc_type = BLOCK_RW_ADTC;
			else
				host->adtc_type = COMMOM_ADTC;
			phytium_mci_start_data(host, mrq, mrq->cmd, mrq->data);
		} else {
			dev_err(host->dev, "%s %d:ERROR: cmd %d followers the SBC\n",
				__func__, __LINE__, cmd->opcode);
		}
	} else if (!cmd->data) {
		phytium_mci_request_done(host, mrq);
	}
}

static void phytium_mci_ops_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	u32 data;
	int rc;

	host->error = 0;
	WARN_ON(host->mrq);
	host->mrq = mrq;

	rc = readl_relaxed_poll_timeout(host->base + MCI_STATUS,
					 data,
					 !(data & MCI_STATUS_CARD_BUSY),
					 0, 500 * 1000);
	if (rc == -ETIMEDOUT)
		pr_debug("%s %d, timeout mci_status: 0x%08x\n", __func__, __LINE__, data);

	dev_dbg(host->dev, "%s %d: cmd:%d arg:0x%x\n", __func__, __LINE__,
		mrq->cmd->opcode, mrq->cmd->arg);

	if (host->is_device_x100 && mrq->sbc && mrq->cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) {
		phytium_mci_start_write_multiple_non_dma(mmc, mrq);
		return;
	}

	if (mrq->sbc) {
		phytium_mci_start_command(host, mrq, mrq->sbc);
		return;
	}
	if (mrq->data) {
		phytium_mci_prepare_data(host, mrq);

		if ((mrq->data->sg->length >= 512) && host->is_use_dma &&
			((mrq->cmd->opcode == MMC_READ_MULTIPLE_BLOCK) ||
			(mrq->cmd->opcode == MMC_READ_SINGLE_BLOCK) ||
			(mrq->cmd->opcode == MMC_WRITE_MULTIPLE_BLOCK) ||
			(mrq->cmd->opcode == MMC_WRITE_BLOCK) ||
			(mrq->cmd->opcode == SD_IO_RW_EXTENDED)))

			host->adtc_type = BLOCK_RW_ADTC;
		else
			host->adtc_type = COMMOM_ADTC;

		phytium_mci_start_data(host, mrq, mrq->cmd, mrq->data);
		return;
	}
	phytium_mci_start_command(host, mrq, mrq->cmd);
}

static void phytium_mci_pre_req(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	struct mmc_data *data = mrq->data;

	if (!data)
		return;

	phytium_mci_prepare_data(host, mrq);
	data->host_cookie |= MCI_ASYNC_FLAG;
}

static void phytium_mci_post_req(struct mmc_host *mmc, struct mmc_request *mrq,
				  int err)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	struct mmc_data *data = mrq->data;

	if (!data)
		return;

	if (data->host_cookie & MCI_ASYNC_FLAG) {
		data->host_cookie &= ~MCI_ASYNC_FLAG;
		phytium_mci_unprepare_data(host, mrq);
	}
}

static void phytium_mci_data_read_without_dma(struct phytium_mci_host *host,
					      struct mmc_data *data)
{
	u32 length, i, data_val, dma_len, tmp = 0;
	u32 *virt_addr;
	unsigned long flags;
	struct scatterlist *sg;

	length = data->blocks * data->blksz;

	if (mmc_get_dma_dir(data) == DMA_FROM_DEVICE) {
		spin_lock_irqsave(&host->lock, flags);
		if (data->host_cookie & MCI_ASYNC_FLAG) {
			tmp = MCI_ASYNC_FLAG;
			phytium_mci_post_req(host->mmc, data->mrq, 0);
		} else {
			phytium_mci_unprepare_data(host, data->mrq);
		}

		for_each_sg(data->sg, sg, data->sg_count, i) {
			dma_len = sg_dma_len(sg);
			virt_addr = sg_virt(data->sg);

			for (i = 0; i < (dma_len / 4); i++) {
				data_val = readl(host->base + MCI_DATA);
				memcpy(virt_addr, &data_val, 4);
				++virt_addr;
			}
		}

		if (tmp & MCI_ASYNC_FLAG)
			phytium_mci_pre_req(host->mmc, data->mrq);
		else
			phytium_mci_prepare_data(host, data->mrq);

		spin_unlock_irqrestore(&host->lock, flags);
	}
	data->bytes_xfered = length;
}

static void phytium_mci_data_xfer_next(struct phytium_mci_host *host,
					struct mmc_request *mrq,
					struct mmc_data *data)
{
	if (mmc_op_multi(mrq->cmd->opcode) && mrq->stop &&
	    (data->error || !mrq->sbc)) {
		while ((readl(host->base + MCI_STATUS) & (MCI_STATUS_CARD_BUSY)))
			cpu_relax();
		phytium_mci_start_command(host, mrq, mrq->stop);
	} else {
		phytium_mci_request_done(host, mrq);
	}
}

static bool phytium_mci_data_xfer_done(struct phytium_mci_host *host, u32 events,
					struct mmc_request *mrq, struct mmc_data *data)
{
	unsigned long flags;
	bool done;

	unsigned int check_data = events & (MCI_RAW_INTS_DTO | MCI_RAW_INTS_RCRC |
					    MCI_RAW_INTS_DCRC | MCI_RAW_INTS_RE |
					    MCI_RAW_INTS_DRTO | MCI_RAW_INTS_EBE |
					    MCI_DMAC_STATUS_AIS | MCI_DMAC_STATUS_DU |
					    MCI_RAW_INTS_SBE_BCI | MCI_INT_MASK_RTO);

	spin_lock_irqsave(&host->lock, flags);
	done = !host->data;

	if (check_data || host->data)
		host->data = NULL;
	spin_unlock_irqrestore(&host->lock, flags);

	if (done)
		return true;
	if (check_data) {
		spin_lock_irqsave(&host->lock, flags);
		sdr_clr_bits(host->base + MCI_DMAC_INT_ENA, dmac_ints_mask);
		sdr_clr_bits(host->base + MCI_INT_MASK, data_ints_mask);
		/* Stop the IDMAC running */
		sdr_clr_bits(host->base + MCI_BUS_MODE, MCI_BUS_MODE_DE);
		dev_dbg(host->dev, "DMA stop\n");
		spin_unlock_irqrestore(&host->lock, flags);

		if (events & MCI_RAW_INTS_DTO) {
			if (!host->is_use_dma ||
			    (host->is_use_dma && host->adtc_type == COMMOM_ADTC &&
			     (mrq->cmd->flags & MMC_CMD_MASK) == MMC_CMD_ADTC))
				phytium_mci_data_read_without_dma(host, data);
			else
				data->bytes_xfered = data->blocks * data->blksz;
		} else {
			data->bytes_xfered = 0;
			if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE)
			    && readl(host->base + MCI_CARD_DETECT)
			    && (events & cmd_err_ints_mask)) {
				data->error = -ENOMEDIUM;
				data->mrq->cmd->error =  -ENOMEDIUM;
			} else if (events & (MCI_RAW_INTS_DCRC | MCI_RAW_INTS_EBE |
					   MCI_RAW_INTS_SBE_BCI)) {
				data->error = -EILSEQ;
				host->cmd = NULL;
			} else {
				data->error = -ETIMEDOUT;
				host->cmd = NULL;
			}
		}

		phytium_mci_data_xfer_next(host, mrq, data);
		done = true;
	}
	return done;
}

static int phytium_mci_card_busy(struct mmc_host *mmc)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	u32 status;

	status = readl(host->base + MCI_STATUS);

	return !!(status & MCI_STATUS_CARD_BUSY);
}

static void __phytium_mci_enable_sdio_irq(struct phytium_mci_host *host, int enable)
{
	if (enable)
		sdr_set_bits(host->base + MCI_INT_MASK, MCI_INT_MASK_SDIO);
	else
		sdr_clr_bits(host->base + MCI_INT_MASK, MCI_INT_MASK_SDIO);
}

static void phytium_mci_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct phytium_mci_host *host  = mmc_priv(mmc);

	__phytium_mci_enable_sdio_irq(host, enable);
}

static void hotplug_timer_func(struct timer_list *t)
{
	struct phytium_mci_host *host;
	u32 status;

	host = from_timer(host, t, hotplug_timer);
	if (!host)
		return;

	status = readl(host->base + MCI_CARD_DETECT);

	if (status & 0x1) {
		if (host->mmc->card) {
			cancel_delayed_work(&host->mmc->detect);
			mmc_detect_change(host->mmc, msecs_to_jiffies(100));
		}
	} else {
		cancel_delayed_work(&host->mmc->detect);
		mmc_detect_change(host->mmc, msecs_to_jiffies(200));
	}
}

static void phytium_mci_request_timeout(struct timer_list *t)
{
	struct phytium_mci_host *host = from_timer(host, t, timeout_timer);

	dev_err(host->dev, "request timeout\n");
	if (host->mrq) {
		dev_err(host->dev, "aborting mrq=%p cmd=%d\n",
				host->mrq, host->mrq->cmd->opcode);
		if (host->cmd) {
			dev_err(host->dev, "aborting cmd=%d\n", host->cmd->opcode);
			phytium_mci_cmd_done(host, MCI_RAW_INTS_RTO, host->mrq, host->cmd);
		} else if (host->data) {
			dev_err(host->dev, "abort data: cmd%d; %d blocks\n",
					host->mrq->cmd->opcode, host->data->blocks);
			phytium_mci_data_xfer_done(host, MCI_RAW_INTS_RTO, host->mrq,
					host->data);
		}
	}
}

static int phytium_mci_err_irq(struct phytium_mci_host *host, u32 dmac_events, u32 events)
{
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;

	mrq = host->mrq;
	cmd = host->cmd;
	data = host->data;

	if (cmd && (cmd == mrq->sbc)) {
		phytium_mci_cmd_done(host, MCI_RAW_INTS_RTO, mrq, mrq->sbc);
	} else if (cmd && (cmd == mrq->stop)) {
		phytium_mci_cmd_done(host, MCI_RAW_INTS_RTO, mrq, mrq->stop);
	} else if (data) {
		data->error = -ETIMEDOUT;
		if ((data->flags & MMC_DATA_READ) == MMC_DATA_READ ||
		    (data->flags & MMC_DATA_WRITE) == MMC_DATA_WRITE)
			phytium_mci_data_xfer_done(host, events | dmac_events, mrq, data);
	} else if (cmd) {
		phytium_mci_cmd_done(host, MCI_RAW_INTS_RTO, mrq, mrq->cmd);
	}

	return 0;
}

static irqreturn_t phytium_mci_irq(int irq, void *dev_id)
{
	struct phytium_mci_host *host = (struct phytium_mci_host *) dev_id;
	unsigned long flags;
	struct mmc_request *mrq;
	struct mmc_command *cmd;
	struct mmc_data *data;
	u32 events, event_mask, dmac_events, dmac_evt_mask;

	if (!host)
		return IRQ_NONE;
	writel(0, host->base + 0xfd0);

	spin_lock_irqsave(&host->lock, flags);
	events = readl(host->base + MCI_RAW_INTS);
	dmac_events = readl(host->base + MCI_DMAC_STATUS);
	event_mask = readl(host->base + MCI_INT_MASK);
	dmac_evt_mask = readl(host->base + MCI_DMAC_INT_ENA);
	if ((!events) && (!(dmac_events&0x1fff))) {
		spin_unlock_irqrestore(&host->lock, flags);
		return IRQ_NONE;
	}
	dev_dbg(host->dev, "%s:events:%x,mask:0x%x,dmac_events:%x,dmac_mask:0x%x,cmd:%d\n",
		__func__, events, event_mask, dmac_events, dmac_evt_mask,
		host->mrq ? host->mrq->cmd->opcode : 255);

	mrq = host->mrq;
	cmd = host->cmd;
	data = host->data;

#if 0
	if (((events & event_mask) & MCI_RAW_INTS_SDIO) &&
	    ((events == 0x10001) || (events == 0x10000) || (events == 0x10040))) {
		writel(events, host->base + MCI_RAW_INTS);
		__phytium_mci_enable_sdio_irq(host, 0);
		sdio_signal_irq(host->mmc);
		spin_unlock_irqrestore(&host->lock, flags);
		goto irq_out;
	}
#endif
	if ((events & event_mask) & MCI_RAW_INTS_SDIO) {
		__phytium_mci_enable_sdio_irq(host, 0);
	}


	writel(events, host->base + MCI_RAW_INTS);
	writel(dmac_events, host->base + MCI_DMAC_STATUS);
	spin_unlock_irqrestore(&host->lock, flags);

	if ((events & event_mask) & MCI_RAW_INTS_SDIO) {
		sdio_signal_irq(host->mmc);
	}

	if (((events & event_mask) == 0) && ((dmac_evt_mask & dmac_events) == 0))
		goto irq_out;

	if (((events & event_mask) & MCI_RAW_INTS_CD) &&
	    !(host->mmc->caps & MMC_CAP_NONREMOVABLE)) {
		mod_timer(&host->hotplug_timer, jiffies + usecs_to_jiffies(20000));
		dev_dbg(host->dev, "sd status changed here ! status:[%d] [%s %d]",
			readl(host->base + MCI_CARD_DETECT), __func__, __LINE__);

		if ((events & event_mask) == MCI_RAW_INTS_CD)
			goto irq_out;
	}

	if (!mrq) {
		if (events & MCI_RAW_INTS_HLE)
			dev_dbg(host->dev,
				"%s: MRQ=NULL and HW write locked, events=%08x,event_mask=%08x\n",
				__func__, events, event_mask);
		else
			dev_dbg(host->dev, "%s: MRQ=NULL events:%08X evt_mask=%08X,sd_status:%d\n",
				__func__, events, event_mask, readl(host->base + MCI_CARD_DETECT));
		goto irq_out;
	}

	if ((dmac_events & dmac_err_ints_mask) || (events & cmd_err_ints_mask)) {
		dev_dbg(host->dev, "ERR:events:%x,mask:0x%x,dmac_evts:%x,dmac_mask:0x%x,cmd:%d\n",
			events, event_mask, dmac_events, dmac_evt_mask, mrq->cmd->opcode);
		phytium_mci_err_irq(host, dmac_events & dmac_err_ints_mask,
						    events & cmd_err_ints_mask);
		goto irq_out;
	}

	if ((events & MCI_MASKED_INTS_DTO) && (events & MCI_MASKED_INTS_CMD)) {
		phytium_mci_cmd_done(host, events, mrq, cmd);
		phytium_mci_data_xfer_done(host, (events & data_ints_mask) |
					    (dmac_events & dmac_ints_mask), mrq, data);
	} else if (events & MCI_MASKED_INTS_CMD ||
		  ((events & MCI_INT_MASK_HTO) && (cmd->opcode == SD_SWITCH_VOLTAGE))) {
		phytium_mci_cmd_done(host, events, mrq, cmd);
	} else if (events & MCI_MASKED_INTS_DTO) {
		phytium_mci_data_xfer_done(host, (events & data_ints_mask) |
					    (dmac_events & dmac_ints_mask), mrq, data);
	}

irq_out:
	return IRQ_HANDLED;
}

static void phytium_mci_init_hw(struct phytium_mci_host *host)
{
	u32 val;
	int uhs_reg_value = 0x502;

	writel(MCI_SET_FIFOTH(0x2, 0x7, 0x100), host->base + MCI_FIFOTH);
	writel(0x800001, host->base + MCI_CARD_THRCTL);
	sdr_clr_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);
	phytium_mci_update_external_clk(host, uhs_reg_value);

	sdr_set_bits(host->base + MCI_PWREN, MCI_PWREN_ENABLE);
	sdr_set_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);
	sdr_set_bits(host->base + MCI_UHS_REG_EXT, MCI_EXT_CLK_ENABLE);
	sdr_clr_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_VOLT);

	phytium_mci_reset_hw(host);

	if (host->mmc->caps & MMC_CAP_NONREMOVABLE)
		sdr_set_bits(host->base + MCI_CARD_RESET, MCI_CARD_RESET_ENABLE);
	else
		sdr_clr_bits(host->base + MCI_CARD_RESET, MCI_CARD_RESET_ENABLE);

	writel(0, host->base + MCI_INT_MASK);
	val = readl(host->base + MCI_RAW_INTS);
	writel(val, host->base + MCI_RAW_INTS);
	writel(0, host->base + MCI_DMAC_INT_ENA);
	val = readl(host->base + MCI_DMAC_STATUS);
	writel(val, host->base + MCI_DMAC_STATUS);
	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE))
		writel(MCI_INT_MASK_CD, host->base + MCI_INT_MASK);

	sdr_set_bits(host->base + MCI_CNTRL, MCI_CNTRL_INT_ENABLE |
		     MCI_CNTRL_USE_INTERNAL_DMAC);

	writel(0xFFFFFFFF, host->base + MCI_TMOUT);
	dev_info(host->dev, "init hardware done!");

}

void phytium_mci_deinit_hw(struct phytium_mci_host *host)
{
	u32 val;

	sdr_clr_bits(host->base + MCI_PWREN, MCI_PWREN_ENABLE);
	sdr_clr_bits(host->base + MCI_CLKENA, MCI_CLKENA_CCLK_ENABLE);
	sdr_clr_bits(host->base + MCI_UHS_REG_EXT, MCI_EXT_CLK_ENABLE);
	sdr_clr_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_VOLT);
	writel(0, host->base + MCI_INT_MASK);
	val = readl(host->base + MCI_RAW_INTS);
	writel(val, host->base + MCI_RAW_INTS);
	writel(0, host->base + MCI_DMAC_INT_ENA);
	val = readl(host->base + MCI_DMAC_STATUS);
	writel(val, host->base + MCI_DMAC_STATUS);
	if (!(host->mmc->caps & MMC_CAP_NONREMOVABLE))
		writel(MCI_INT_MASK_CD, host->base + MCI_INT_MASK);
}
EXPORT_SYMBOL_GPL(phytium_mci_deinit_hw);

static void phytium_mci_adma_reset(struct phytium_mci_host *host)
{
	u32 bmod = readl(host->base + MCI_BUS_MODE);

	bmod |= MCI_BUS_MODE_SWR;
	writel(bmod, host->base + MCI_BUS_MODE);
}

static void phytium_mci_init_adma_table(struct phytium_mci_host *host,
					 struct phytium_mci_dma *dma)
{
	struct phytium_adma2_64_desc *adma_table = dma->adma_table;
	dma_addr_t dma_addr;
	int i;

	memset(adma_table, 0, sizeof(struct phytium_adma2_64_desc) * MAX_BD_NUM);

	for (i = 0; i < (MAX_BD_NUM - 1); i++) {
		dma_addr = dma->adma_addr + sizeof(*adma_table) * (i + 1);
		adma_table[i].desc_lo = lower_32_bits(dma_addr);
		adma_table[i].desc_hi = upper_32_bits(dma_addr);
		adma_table[i].attribute = 0;
		adma_table[i].NON1 = 0;
		adma_table[i].len = 0;
		adma_table[i].NON2 = 0;
	}

	phytium_mci_adma_reset(host);
}

static void phytium_mci_set_buswidth(struct phytium_mci_host *host, u32 width)
{
	u32 val;

	switch (width) {
	case MMC_BUS_WIDTH_1:
		val = MCI_BUS_1BITS;
		break;

	case MMC_BUS_WIDTH_4:
		val = MCI_BUS_4BITS;
		break;

	case MMC_BUS_WIDTH_8:
		val = MCI_BUS_8BITS;
		break;
	default:
		val = MCI_BUS_4BITS;
		break;
	}
	writel(val, host->base + MCI_CTYPE);
	dev_dbg(host->dev, "Bus Width = %d, set value:0x%x\n", width, val);
}

static void phytium_mci_ops_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct phytium_mci_host *host = mmc_priv(mmc);

	if (ios->timing == MMC_TIMING_MMC_DDR52 || ios->timing == MMC_TIMING_UHS_DDR50)
		sdr_set_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_DDR);
	else
		sdr_clr_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_DDR);

	phytium_mci_set_buswidth(host, ios->bus_width);

	switch (ios->power_mode) {
	case MMC_POWER_UP:
		set_bit(MCI_CARD_NEED_INIT, &host->flags);
		writel(MCI_POWER_ON, host->base + MCI_PWREN);
		break;

	case MMC_POWER_ON:
		break;

	case MMC_POWER_OFF:
		writel(MCI_POWER_OFF, host->base + MCI_PWREN);
		break;

	default:
		break;
	}
	phytium_mci_set_clk(host, ios);
}

static void phytium_mci_ack_sdio_irq(struct mmc_host *mmc)
{
	unsigned long flags;
	struct phytium_mci_host *host = mmc_priv(mmc);

	spin_lock_irqsave(&host->lock, flags);
	__phytium_mci_enable_sdio_irq(host, 1);
	spin_unlock_irqrestore(&host->lock, flags);
}

static int phytium_mci_get_cd(struct mmc_host *mmc)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	u32 status;

	if (mmc->caps & MMC_CAP_NONREMOVABLE)
		return 1;

	status = readl(host->base + MCI_CARD_DETECT);

	if ((status & 0x1) == 0x1)
		return 0;

	return 1;
}

static int phytium_mci_ops_switch_volt(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	unsigned int is_voltage_180 = 0;

	is_voltage_180 = readl(host->base + MCI_UHS_REG);
	if ((mmc->caps & MMC_CAP_NONREMOVABLE) && (ios->signal_voltage != MMC_SIGNAL_VOLTAGE_180))
		return -EINVAL;

	if ((ios->signal_voltage == MMC_SIGNAL_VOLTAGE_330) && (is_voltage_180 & 0x1))
		sdr_clr_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_VOLT);
	else if ((ios->signal_voltage == MMC_SIGNAL_VOLTAGE_180) && (!(is_voltage_180 & 0x1)))
		sdr_set_bits(host->base + MCI_UHS_REG, MCI_UHS_REG_VOLT);
	else if (ios->signal_voltage == MMC_SIGNAL_VOLTAGE_120)
		return -EINVAL;
	return 0;
}

static void phytium_mci_hw_reset(struct mmc_host *mmc)
{
	struct phytium_mci_host *host = mmc_priv(mmc);
	u32 reset_flag;

	if (host->is_use_dma) {
		reset_flag = MCI_CNTRL_FIFO_RESET | MCI_CNTRL_DMA_RESET;
		phytium_mci_adma_reset(host);
		sdr_set_bits(host->base + MCI_CNTRL, reset_flag);
	} else {
		reset_flag = MCI_CNTRL_FIFO_RESET;
		sdr_set_bits(host->base + MCI_CNTRL, reset_flag);
	}

	while (readl(host->base + MCI_CNTRL) & reset_flag)
		cpu_relax();

	sdr_clr_bits(host->base + MCI_CARD_RESET, MCI_CARD_RESET_ENABLE);
	udelay(5);
	sdr_set_bits(host->base + MCI_CARD_RESET, MCI_CARD_RESET_ENABLE);
	usleep_range(200, 300);
}

#ifdef CONFIG_PM_SLEEP
int phytium_mci_suspend(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_mci_host *host = mmc_priv(mmc);

	phytium_mci_deinit_hw(host);
	return 0;
}
EXPORT_SYMBOL(phytium_mci_suspend);

int phytium_mci_resume(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_mci_host *host = mmc_priv(mmc);

	phytium_mci_init_hw(host);
	return 0;
}
EXPORT_SYMBOL(phytium_mci_resume);

#endif

#ifdef CONFIG_PM
int phytium_mci_runtime_suspend(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_mci_host *host = mmc_priv(mmc);

	phytium_mci_deinit_hw(host);
	return 0;
}
EXPORT_SYMBOL(phytium_mci_runtime_suspend);

int phytium_mci_runtime_resume(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	struct phytium_mci_host *host = mmc_priv(mmc);

	phytium_mci_init_hw(host);
	return 0;
}
EXPORT_SYMBOL(phytium_mci_runtime_resume);

#endif

static struct mmc_host_ops phytium_mci_ops = {
	.post_req = phytium_mci_post_req,
	.pre_req = phytium_mci_pre_req,
	.request = phytium_mci_ops_request,
	.set_ios = phytium_mci_ops_set_ios,
	.get_cd = phytium_mci_get_cd,
	.enable_sdio_irq = phytium_mci_enable_sdio_irq,
	.ack_sdio_irq = phytium_mci_ack_sdio_irq,
	.card_busy = phytium_mci_card_busy,
	.start_signal_voltage_switch = phytium_mci_ops_switch_volt,
	.hw_reset = phytium_mci_hw_reset,
};

int phytium_mci_common_probe(struct phytium_mci_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct device *dev = host->dev;
	int ret;

	dma_set_mask(dev, DMA_BIT_MASK(64));
	dma_set_coherent_mask(dev, DMA_BIT_MASK(64));

	timer_setup(&host->hotplug_timer, hotplug_timer_func, 0);
	timer_setup(&host->timeout_timer, phytium_mci_request_timeout, 0);

	mmc->f_min = MCI_F_MIN;
	if (!mmc->f_max)
		mmc->f_max = MCI_F_MAX;

	mmc->ops = &phytium_mci_ops;
	mmc->ocr_avail_sdio = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->ocr_avail_sd = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->ocr_avail_mmc = MMC_VDD_165_195;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	mmc->caps |= host->caps;

	if (mmc->caps & MMC_CAP_SDIO_IRQ) {
		mmc->caps2 |= MMC_CAP2_SDIO_IRQ_NOTHREAD;
		dev_dbg(host->dev, "%s %d: MMC_CAP_SDIO_IRQ\n", __func__, __LINE__);
	}
	mmc->caps2 |= host->caps2;
	if (host->is_use_dma) {
		/* MMC core transfer sizes tunable parameters */
		mmc->max_segs = MAX_BD_NUM;
		mmc->max_seg_size = 4 * 1024;
		mmc->max_blk_size = 512;
		mmc->max_req_size = 512 * 1024;
		mmc->max_blk_count = mmc->max_req_size / 512;
		host->dma.adma_table = dma_alloc_coherent(host->dev,
							  MAX_BD_NUM *
							  sizeof(struct phytium_adma2_64_desc),
							  &host->dma.adma_addr, GFP_KERNEL);
		if (!host->dma.adma_table)
			return MCI_REALEASE_MEM;

		host->dma.desc_sz = ADMA2_64_DESC_SZ;
		phytium_mci_init_adma_table(host, &host->dma);
	} else {
		mmc->max_segs = MAX_BD_NUM;
		mmc->max_seg_size = 4 * 1024;
		mmc->max_blk_size = 512;
		mmc->max_req_size = 4 * 512;
		mmc->max_blk_count = mmc->max_req_size / 512;
	}

	spin_lock_init(&host->lock);

	phytium_mci_init_hw(host);
	ret = devm_request_irq(host->dev, host->irq, phytium_mci_irq,
			       host->irq_flags, "phytium-mci", host);

	if (ret)
		return ret;

	ret = mmc_add_host(mmc);

	if (ret) {
		dev_err(host->dev, "%s %d: mmc add host!\n", __func__, __LINE__);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL(phytium_mci_common_probe);

MODULE_DESCRIPTION("Phytium Multimedia Card Interface driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
