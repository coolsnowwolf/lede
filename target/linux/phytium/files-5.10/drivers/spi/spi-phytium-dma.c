// SPDX-License-Identifier: GPL-2.0-only
/*
 * Special handling for phytium DMA core
 *
 */
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/irqreturn.h>
#include <linux/jiffies.h>
#include <linux/pci.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/module.h>
#include "spi-phytium.h"

#define RX_BUSY		0
#define RX_BURST_LEVEL	16
#define TX_BUSY		1
#define TX_BURST_LEVEL	16

#define DMA_MAX_BUF_SIZE	4096

static void phytium_spi_dma_maxburst_init(struct phytium_spi *fts)
{
	struct dma_slave_caps caps;
	u32 max_burst, def_burst;
	int ret;

	def_burst = fts->fifo_len / 2;

	ret = dma_get_slave_caps(fts->rxchan, &caps);
	if (!ret && caps.max_burst)
		max_burst = caps.max_burst;
	else
		max_burst = RX_BURST_LEVEL;

	fts->rxburst = min(max_burst, def_burst);
	phytium_writel(fts, DMARDLR, 0x0);

	ret = dma_get_slave_caps(fts->txchan, &caps);
	if (!ret && caps.max_burst)
		max_burst = caps.max_burst;
	else
		max_burst = TX_BURST_LEVEL;

	/*
	 * Having a Rx DMA channel serviced with higher priority than a Tx DMA
	 * channel might not be enough to provide a well balanced DMA-based
	 * SPI transfer interface. There might still be moments when the Tx DMA
	 * channel is occasionally handled faster than the Rx DMA channel.
	 * That in its turn will eventually cause the SPI Rx FIFO overflow if
	 * SPI bus speed is high enough to fill the SPI Rx FIFO in before it's
	 * cleared by the Rx DMA channel. In order to fix the problem the Tx
	 * DMA activity is intentionally slowed down by limiting the SPI Tx
	 * FIFO depth with a value twice bigger than the Tx burst length.
	 */
	fts->txburst = min(max_burst, def_burst);
	/* set dmatdlr to 0 + 1 */
	phytium_writel(fts, DMATDLR, 0);
}

static void phytium_spi_dma_sg_burst_init(struct phytium_spi *fts)
{
	struct dma_slave_caps tx = {0}, rx = {0};

	dma_get_slave_caps(fts->txchan, &tx);
	dma_get_slave_caps(fts->rxchan, &rx);

	if (tx.max_sg_burst > 0 && rx.max_sg_burst > 0)
		fts->dma_sg_burst = min(tx.max_sg_burst, rx.max_sg_burst);
	else if (tx.max_sg_burst > 0)
		fts->dma_sg_burst = tx.max_sg_burst;
	else if (rx.max_sg_burst > 0)
		fts->dma_sg_burst = rx.max_sg_burst;
	else
		fts->dma_sg_burst = 0;
}

static int phytium_spi_dma_init(struct device *dev, struct phytium_spi *fts)
{
	fts->rxchan = dma_request_chan(dev, "rx");
	if (IS_ERR_OR_NULL(fts->rxchan))
		return -ENODEV;

	fts->txchan = dma_request_chan(dev, "tx");
	if (IS_ERR_OR_NULL(fts->txchan)) {
		dev_err(dev, "can't request chan\n");
		dma_release_channel(fts->rxchan);
		fts->rxchan = NULL;
		return -ENODEV;
	}

	fts->master->dma_rx = fts->rxchan;
	fts->master->dma_tx = fts->txchan;
	init_completion(&fts->dma_completion);

	phytium_spi_dma_maxburst_init(fts);
	phytium_spi_dma_sg_burst_init(fts);

	return 0;
}

static void phytium_spi_dma_exit(struct phytium_spi *fts)
{
	if (fts->txchan) {
		dmaengine_terminate_sync(fts->txchan);
		dma_release_channel(fts->txchan);
	}

	if (fts->rxchan) {
		dmaengine_terminate_sync(fts->rxchan);
		dma_release_channel(fts->rxchan);
	}
}

static irqreturn_t phytium_spi_dma_transfer_handler(struct phytium_spi *fts)
{
	phytium_spi_check_status(fts, false);

	complete(&fts->dma_completion);

	return IRQ_HANDLED;
}

static bool phytium_spi_can_dma(struct spi_controller *master,
			   struct spi_device *spi, struct spi_transfer *xfer)
{
	struct phytium_spi *fts = spi_controller_get_devdata(master);

	return xfer->len > fts->fifo_len;
}

static enum dma_slave_buswidth phytium_spi_dma_convert_width(u8 n_bytes)
{
	if (n_bytes == 1)
		return DMA_SLAVE_BUSWIDTH_1_BYTE;
	else if (n_bytes == 2)
		return DMA_SLAVE_BUSWIDTH_2_BYTES;

	return DMA_SLAVE_BUSWIDTH_UNDEFINED;
}

static int phytium_spi_dma_wait(struct phytium_spi *fts, unsigned int len,
					u32 speed)
{
	unsigned long long ms;

	ms = len * MSEC_PER_SEC * BITS_PER_BYTE;
	do_div(ms, speed);
	ms += ms + 200;

	if (ms > UINT_MAX)
		ms = UINT_MAX;

	ms = wait_for_completion_timeout(&fts->dma_completion,
					 msecs_to_jiffies(ms));

	if (ms == 0) {
		dev_err(&fts->master->cur_msg->spi->dev,
			"DMA transaction timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static inline bool phytium_spi_dma_tx_busy(struct phytium_spi *fts)
{
	return !(phytium_readl(fts, SR) & SR_TF_EMPT);
}

static int phytium_spi_dma_wait_tx_done(struct phytium_spi *fts,
				   struct spi_transfer *xfer)
{
	int retry = SPI_WAIT_RETRIES;
	struct spi_delay delay;
	u32 nents;

	nents = phytium_readl(fts, TXFLR);
	delay.unit = SPI_DELAY_UNIT_SCK;
	delay.value = nents * fts->n_bytes * BITS_PER_BYTE;

	while (phytium_spi_dma_tx_busy(fts) && retry--)
		spi_delay_exec(&delay, xfer);

	if (retry < 0) {
		dev_err(&fts->master->dev, "Tx hanged up\n");
		return -EIO;
	}

	return 0;
}

/*
 * fts->dma_chan_busy is set before the dma transfer starts, callback for tx
 * channel will clear a corresponding bit.
 */
static void phytium_spi_dma_tx_done(void *arg)
{
	struct phytium_spi *fts = arg;

	clear_bit(TX_BUSY, &fts->dma_chan_busy);
	if (test_bit(RX_BUSY, &fts->dma_chan_busy))
		return;

	complete(&fts->dma_completion);
}

static int phytium_spi_dma_config_tx(struct phytium_spi *fts)
{
	struct dma_slave_config txconf;

	memset(&txconf, 0, sizeof(txconf));
	txconf.direction = DMA_MEM_TO_DEV;
	txconf.dst_addr = fts->dma_addr;
	txconf.dst_maxburst = fts->txburst;
	txconf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	txconf.dst_addr_width = phytium_spi_dma_convert_width(fts->n_bytes);
	txconf.device_fc = false;

	return dmaengine_slave_config(fts->txchan, &txconf);
}

static int phytium_spi_dma_submit_tx(struct phytium_spi *fts, struct scatterlist *sgl,
				unsigned int nents)
{
	struct dma_async_tx_descriptor *txdesc;
	dma_cookie_t cookie;
	int ret;

	txdesc = dmaengine_prep_slave_sg(fts->txchan, sgl, nents,
					 DMA_MEM_TO_DEV,
					 DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!txdesc)
		return -ENOMEM;

	txdesc->callback = phytium_spi_dma_tx_done;
	txdesc->callback_param = fts;

	cookie = dmaengine_submit(txdesc);
	ret = dma_submit_error(cookie);
	if (ret) {
		dmaengine_terminate_sync(fts->txchan);
		return ret;
	}

	set_bit(TX_BUSY, &fts->dma_chan_busy);

	return 0;
}

static inline bool phytium_spi_dma_rx_busy(struct phytium_spi *fts)
{
	return !!(phytium_readl(fts, SR) & SR_RF_NOT_EMPT);
}

static int phytium_spi_dma_wait_rx_done(struct phytium_spi *fts)
{
	int retry = SPI_WAIT_RETRIES;
	struct spi_delay delay;
	unsigned long ns, us;
	u32 nents;

	/*
	 * It's unlikely that DMA engine is still doing the data fetching, but
	 * if it's let's give it some reasonable time. The timeout calculation
	 * is based on the synchronous APB/SSI reference clock rate, on a
	 * number of data entries left in the Rx FIFO, times a number of clock
	 * periods normally needed for a single APB read/write transaction
	 * without PREADY signal utilized (which is true for the phytium APB SSI
	 * controller).
	 */
	nents = phytium_readl(fts, RXFLR);
	ns = 4U * NSEC_PER_SEC / fts->max_freq * nents;
	if (ns <= NSEC_PER_USEC) {
		delay.unit = SPI_DELAY_UNIT_NSECS;
		delay.value = ns;
	} else {
		us = DIV_ROUND_UP(ns, NSEC_PER_USEC);
		delay.unit = SPI_DELAY_UNIT_USECS;
		delay.value = clamp_val(us, 0, USHRT_MAX);
	}

	while (phytium_spi_dma_rx_busy(fts) && retry--)
		spi_delay_exec(&delay, NULL);

	if (retry < 0) {
		dev_err(&fts->master->dev, "Rx hanged up, nents = %d\n", nents);
		return -EIO;
	}

	return 0;
}

/*
 * fts->dma_chan_busy is set before the dma transfer starts, callback for rx
 * channel will clear a corresponding bit.
 */
static void phytium_spi_dma_rx_done(void *arg)
{
	struct phytium_spi *fts = arg;

	clear_bit(RX_BUSY, &fts->dma_chan_busy);
	if (test_bit(TX_BUSY, &fts->dma_chan_busy))
		return;

	complete(&fts->dma_completion);
}

static int phytium_spi_dma_config_rx(struct phytium_spi *fts)
{
	struct dma_slave_config rxconf;

	memset(&rxconf, 0, sizeof(rxconf));
	rxconf.direction = DMA_DEV_TO_MEM;
	rxconf.src_addr = fts->dma_addr;
	rxconf.src_maxburst = fts->rxburst;
	rxconf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	rxconf.src_addr_width = phytium_spi_dma_convert_width(fts->n_bytes);
	rxconf.device_fc = false;

	return dmaengine_slave_config(fts->rxchan, &rxconf);
}

static int phytium_spi_dma_submit_rx(struct phytium_spi *fts, struct scatterlist *sgl,
				unsigned int nents)
{
	struct dma_async_tx_descriptor *rxdesc;
	dma_cookie_t cookie;
	int ret;

	rxdesc = dmaengine_prep_slave_sg(fts->rxchan, sgl, nents,
					 DMA_DEV_TO_MEM,
					 DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!rxdesc)
		return -ENOMEM;

	rxdesc->callback = phytium_spi_dma_rx_done;
	rxdesc->callback_param = fts;

	cookie = dmaengine_submit(rxdesc);
	ret = dma_submit_error(cookie);
	if (ret) {
		dmaengine_terminate_sync(fts->rxchan);
		return ret;
	}

	set_bit(RX_BUSY, &fts->dma_chan_busy);

	return 0;
}

static int phytium_spi_dma_setup(struct phytium_spi *fts, struct spi_transfer *xfer)
{
	u16 imr, dma_ctrl;
	int ret;

	if (!xfer->tx_buf)
		return -EINVAL;

	/* Setup DMA channels */
	ret = phytium_spi_dma_config_tx(fts);
	if (ret)
		return ret;

	if (xfer->rx_buf) {
		ret = phytium_spi_dma_config_rx(fts);
		if (ret)
			return ret;
	}

	/* Set the DMA handshaking interface */
	dma_ctrl = SPI_DMA_TDMAE;
	if (xfer->rx_buf)
		dma_ctrl |= SPI_DMA_RDMAE;
	phytium_writel(fts, DMACR, dma_ctrl);

	/* Set the interrupt mask */
	imr = INT_TXOI;
	if (xfer->rx_buf)
		imr |= INT_RXUI | INT_RXOI;

	spi_umask_intr(fts, imr);

	reinit_completion(&fts->dma_completion);

	fts->transfer_handler = phytium_spi_dma_transfer_handler;

	return 0;
}

static int phytium_spi_dma_transfer_all(struct phytium_spi *fts,
				   struct spi_transfer *xfer)
{
	int ret;

	/* Submit the DMA Tx transfer */
	ret = phytium_spi_dma_submit_tx(fts, xfer->tx_sg.sgl, xfer->tx_sg.nents);
	if (ret)
		goto err_clear_dmac;

	/* Submit the DMA Rx transfer if required */
	if (xfer->rx_buf) {
		ret = phytium_spi_dma_submit_rx(fts, xfer->rx_sg.sgl,
					   xfer->rx_sg.nents);
		if (ret)
			goto err_clear_dmac;

		/* rx must be started before tx due to spi instinct */
		dma_async_issue_pending(fts->rxchan);
	}

	dma_async_issue_pending(fts->txchan);

	ret = phytium_spi_dma_wait(fts, xfer->len, xfer->effective_speed_hz);

err_clear_dmac:
	phytium_writel(fts, DMACR, 0);

	return ret;
}

static int phytium_spi_dma_transfer_one(struct phytium_spi *fts,
				   struct spi_transfer *xfer)
{
	struct scatterlist *tx_sg = NULL, *rx_sg = NULL, tx_tmp, rx_tmp;
	unsigned int tx_len = 0, rx_len = 0;
	unsigned int base, len;
	int ret;

	sg_init_table(&tx_tmp, 1);
	sg_init_table(&rx_tmp, 1);

	for (base = 0, len = 0; base < xfer->len; base += len) {
		/* Fetch next Tx DMA data chunk */
		if (!tx_len) {
			tx_sg = !tx_sg ? &xfer->tx_sg.sgl[0] : sg_next(tx_sg);
			sg_dma_address(&tx_tmp) = sg_dma_address(tx_sg);
			tx_len = sg_dma_len(tx_sg);
		}

		/* Fetch next Rx DMA data chunk */
		if (!rx_len) {
			rx_sg = !rx_sg ? &xfer->rx_sg.sgl[0] : sg_next(rx_sg);
			sg_dma_address(&rx_tmp) = sg_dma_address(rx_sg);
			rx_len = sg_dma_len(rx_sg);
		}

		if ((base + DMA_MAX_BUF_SIZE) > xfer->len)
			len = xfer->len - base;
		else
			len = DMA_MAX_BUF_SIZE;

		len = min3(len, tx_len, rx_len);

		sg_dma_len(&tx_tmp) = len;
		sg_dma_len(&rx_tmp) = len;

		/* Submit DMA Tx transfer */
		ret = phytium_spi_dma_submit_tx(fts, &tx_tmp, 1);
		if (ret)
			break;

		/* Submit DMA Rx transfer */
		ret = phytium_spi_dma_submit_rx(fts, &rx_tmp, 1);
		if (ret)
			break;

		/* Rx must be started before Tx due to SPI instinct */
		dma_async_issue_pending(fts->rxchan);

		dma_async_issue_pending(fts->txchan);

		/*
		 * Here we only need to wait for the DMA transfer to be
		 * finished since SPI controller is kept enabled during the
		 * procedure this loop implements and there is no risk to lose
		 * data left in the Tx/Rx FIFOs.
		 */
		ret = phytium_spi_dma_wait(fts, len, xfer->effective_speed_hz);
		if (ret)
			break;

		reinit_completion(&fts->dma_completion);

		sg_dma_address(&tx_tmp) += len;
		sg_dma_address(&rx_tmp) += len;
		tx_len -= len;
		rx_len -= len;
	}

	phytium_writel(fts, DMACR, 0);

	return ret;
}

static int phytium_spi_dma_transfer(struct phytium_spi *fts, struct spi_transfer *xfer)
{
	unsigned int nents;
	int ret;

	nents = max(xfer->tx_sg.nents, xfer->rx_sg.nents);

	/*
	 * large transfer length caused spi RX FIFO full event
	 * transfer 4096 bytes each time
	 */
	if (xfer->len <= DMA_MAX_BUF_SIZE)
		ret = phytium_spi_dma_transfer_all(fts, xfer);
	else
		ret = phytium_spi_dma_transfer_one(fts, xfer);
	if (ret)
		return ret;

	if (fts->master->cur_msg->status == -EINPROGRESS) {
		ret = phytium_spi_dma_wait_tx_done(fts, xfer);
		if (ret)
			return ret;
	}

	if (xfer->rx_buf && fts->master->cur_msg->status == -EINPROGRESS)
		ret = phytium_spi_dma_wait_rx_done(fts);

	return ret;
}

static void phytium_spi_dma_stop(struct phytium_spi *fts)
{
	if (test_bit(TX_BUSY, &fts->dma_chan_busy)) {
		dmaengine_terminate_sync(fts->txchan);
		clear_bit(TX_BUSY, &fts->dma_chan_busy);
	}
	if (test_bit(RX_BUSY, &fts->dma_chan_busy)) {
		dmaengine_terminate_sync(fts->rxchan);
		clear_bit(RX_BUSY, &fts->dma_chan_busy);
	}
}

static const struct phytium_spi_dma_ops phytium_spi_dma_generic_ops = {
	.dma_init	= phytium_spi_dma_init,
	.dma_exit	= phytium_spi_dma_exit,
	.dma_setup	= phytium_spi_dma_setup,
	.can_dma	= phytium_spi_can_dma,
	.dma_transfer	= phytium_spi_dma_transfer,
	.dma_stop	= phytium_spi_dma_stop,
};

void phytium_spi_dmaops_set(struct phytium_spi *fts)
{
	fts->dma_ops = &phytium_spi_dma_generic_ops;
}
EXPORT_SYMBOL_GPL(phytium_spi_dmaops_set);

MODULE_LICENSE("GPL v2");
