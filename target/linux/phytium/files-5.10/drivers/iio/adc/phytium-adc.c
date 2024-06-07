// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium ADC device driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#include <linux/iio/iio.h>
#include <linux/iio/events.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/sysfs.h>

/* ADC register */
#define ADC_CTRL_REG				0x00
#define   ADC_CTRL_REG_PD_EN			BIT(31)
#define   ADC_CTRL_REG_CH_ONLY_S(x)		((x & 0x7) << 16)
#define   ADC_CTRL_REG_CLK_DIV(x)		((x) << 12)
#define   ADC_CTRL_REG_CHANNEL_EN(x)		BIT((x) + 4)
#define   ADC_CTRL_REG_CH_ONLY_EN		BIT(3)
#define   ADC_CTRL_REG_SINGLE_EN		BIT(2)
#define   ADC_CTRL_REG_SINGLE_SEL		BIT(1)
#define   ADC_CTRL_REG_SOC_EN			BIT(0)
#define ADC_INTER_REG				0x04
#define ADC_STATE_REG				0x08
#define   ADC_STATE_REG_B_STA(x)		((x) << 8)
#define   ADC_STATE_REG_EOC_STA			BIT(7)
#define   ADC_STATE_REG_S_STA(x)		((x) << 4)
#define   ADC_STATE_REG_SOC_STA			BIT(3)
#define   ADC_STATE_REG_ERR_STA			BIT(2)
#define   ADC_STATE_REG_COV_FINISH_STA		BIT(1)
#define   ADC_STATE_REG_ADCCTL_BUSY_STA		BIT(0)
#define ADC_ERRCLR_REG				0x0c
#define ADC_LEVEL_REG(x)			(0x10 + ((x) << 2))
#define   ADC_LEVEL_REG_HIGH_LEVEL(x)		((x) << 16)
#define   ADC_LEVEL_REG_LOW_LEVEL(x)		(x)
#define ADC_INTRMASK_REG			0x30
#define   ADC_INTRMASK_REG_ERR_INTR_MASK	BIT(24)
#define   ADC_INTRMASK_REG_ULIMIT_OFF(x)	BIT(9 + ((x) << 1))
#define   ADC_INTRMASK_REG_DLIMIT_MASK(x)	BIT(8 + ((x) << 1))
#define   ADC_INTRMASK_REG_COVFIN_MASK(x)	BIT((x))
#define ADC_INTR_REG				0x34
#define   ADC_INTR_REG_ERR			BIT(24)
#define   ADC_INTR_REG_ULIMIT(x)		BIT(9 + ((x) << 1))
#define   ADC_INTR_REG_DLIMIT(x)		BIT(8 + ((x) << 1))
#define   ADC_INTR_REG_LIMIT_MASK		GENMASK(23, 8)
#define   ADC_INTR_REG_COVFIN(x)		BIT((x))
#define   ADC_INTR_REG_COVFIN_MASK		GENMASK(7, 0)
#define ADC_COV_RESULT_REG(x)			(0x38 + ((x) << 2))
#define   ADC_COV_RESULT_REG_MASK		GENMASK(9, 0)
#define ADC_FINISH_CNT_REG(x)			(0x58 + ((x) << 2))
#define ADC_HIS_LIMIT_REG(x)			(0x78 + ((x) << 2))

#define PHYTIUM_MAX_CHANNELS	8
#define PHYTIUM_ADC_TIMEOUT	usecs_to_jiffies(1000 * 1000)

static const struct iio_event_spec phytium_adc_event[] = {
	{
		.type = IIO_EV_TYPE_THRESH,
		.dir = IIO_EV_DIR_RISING,
		.mask_separate = BIT(IIO_EV_INFO_VALUE),
	}, {
		.type = IIO_EV_TYPE_THRESH,
		.dir = IIO_EV_DIR_FALLING,
		.mask_separate = BIT(IIO_EV_INFO_VALUE),
	},
};

struct phytium_adc_data {
	const struct iio_chan_spec *channels;
	u8 num_channels;
};

struct phytium_adc {
	struct device *dev;
	void __iomem *regs;
	struct clk *adc_clk;

	u32 interval;
	u16 thresh_high[PHYTIUM_MAX_CHANNELS];
	u16 thresh_low[PHYTIUM_MAX_CHANNELS];
	u16 last_val[PHYTIUM_MAX_CHANNELS];
	const struct phytium_adc_data *data;
	u16 *scan_data;

	struct completion completion;
	struct mutex lock;
};

static ssize_t phytium_adc_show_conv_interval(struct iio_dev *indio_dev,
					      uintptr_t priv,
					      struct iio_chan_spec const *ch,
					      char *buf)
{
	struct phytium_adc *adc = iio_priv(indio_dev);

	return sprintf(buf, "%u\n", adc->interval);
}

static ssize_t phytium_adc_store_conv_interval(struct iio_dev *indio_dev,
					       uintptr_t priv,
					       struct iio_chan_spec const *ch,
					       const char *buf, size_t len)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	u32 interval;
	int ret;

	ret = kstrtou32(buf, 0, &interval);
	if (ret < 0)
		return ret;

	mutex_lock(&adc->lock);
	adc->interval = interval;
	mutex_unlock(&adc->lock);

	return len;
}

static const struct iio_chan_spec_ext_info phytium_adc_ext_info[] = {
	{
		.name  = "conv_interval",
		.read  = phytium_adc_show_conv_interval,
		.write = phytium_adc_store_conv_interval,
	},
	{ /* sentinel */ }
};

static int phytium_adc_parse_properties(struct platform_device *pdev, struct phytium_adc *adc)
{
	struct iio_chan_spec *chan_array;
	struct fwnode_handle *fwnode;
	struct phytium_adc_data *data;
	unsigned int channel;
	int num_channels;
	int ret, i = 0;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	num_channels = device_get_child_node_count(&pdev->dev);
	if (!num_channels) {
		dev_err(&pdev->dev, "no channel children\n");
		return -ENODEV;
	}

	if (num_channels > PHYTIUM_MAX_CHANNELS) {
		dev_err(&pdev->dev, "num of channel children out of range\n");
		return -EINVAL;
	}

	chan_array = devm_kcalloc(&pdev->dev, num_channels, sizeof(*chan_array),
				  GFP_KERNEL);
	if (!chan_array)
		return -ENOMEM;

	device_for_each_child_node(&pdev->dev, fwnode) {
		ret = fwnode_property_read_u32(fwnode, "reg", &channel);
		if (ret)
			return ret;

		if (channel >= PHYTIUM_MAX_CHANNELS)
			return -EINVAL;

		chan_array[i].type = IIO_VOLTAGE;
		chan_array[i].indexed = 1;
		chan_array[i].channel = channel;
		chan_array[i].info_mask_separate = BIT(IIO_CHAN_INFO_RAW);
		chan_array[i].event_spec = phytium_adc_event;
		chan_array[i].num_event_specs = ARRAY_SIZE(phytium_adc_event);
		chan_array[i].scan_index = channel;
		chan_array[i].scan_type.sign = 'u';
		chan_array[i].scan_type.realbits = 10;
		chan_array[i].scan_type.storagebits = 16;
		chan_array[i].scan_type.endianness = IIO_LE;
		chan_array[i].ext_info = phytium_adc_ext_info;
		i++;
	}

	data->num_channels = num_channels;
	data->channels = chan_array;
	adc->data = data;

	return 0;
}

static void phytium_adc_start_stop(struct phytium_adc *adc, bool start)
{
	u32 ctrl;

	ctrl = readl(adc->regs + ADC_CTRL_REG);
	if (start)
		ctrl |= ADC_CTRL_REG_SOC_EN | ADC_CTRL_REG_SINGLE_EN;
	else
		ctrl &= ~ADC_CTRL_REG_SOC_EN;
	/* Start conversion */
	writel(ctrl, adc->regs + ADC_CTRL_REG);
}

static void phytium_adc_power_setup(struct phytium_adc *adc, bool on)
{
	u32 reg;

	reg = readl(adc->regs + ADC_CTRL_REG);
	if (on)
		reg &= ~ADC_CTRL_REG_PD_EN;
	else
		reg |= ADC_CTRL_REG_PD_EN;
	writel(reg, adc->regs + ADC_CTRL_REG);
}

static int phytium_adc_hw_init(struct phytium_adc *adc)
{
	int ret;
	u32 reg;

	ret = clk_prepare_enable(adc->adc_clk);
	if (ret)
		return ret;

	/*
	 * Setup ctrl register:
	 * - Power up conversion module
	 * - Set the division by 4 as default
	 */
	reg = ADC_CTRL_REG_CLK_DIV(4);
	writel(reg, adc->regs + ADC_CTRL_REG);

	/* Set all the interrupt mask, unmask them when necessary. */
	writel(0x1ffffff, adc->regs + ADC_INTRMASK_REG);

	/* Set default conversion interval */
	adc->interval = (clk_get_rate(adc->adc_clk) * 1000) / NSEC_PER_SEC;

	phytium_adc_power_setup(adc, true);

	return 0;
}

static void phytium_adc_intrmask_setup(struct phytium_adc *adc, unsigned long chan_mask, bool on)
{
	u32 reg;
	u16 limit_mask = 0;
	int ch;

	for_each_set_bit(ch, &chan_mask, PHYTIUM_MAX_CHANNELS)
		limit_mask |= BIT(ch << 1) | BIT((ch << 1) + 1);

	reg = readl(adc->regs + ADC_INTRMASK_REG);
	if (on)
		reg &= ~(ADC_INTRMASK_REG_ERR_INTR_MASK |
			 (limit_mask << 8) | chan_mask);
	else
		reg |= (ADC_INTRMASK_REG_ERR_INTR_MASK |
			(limit_mask << 8) | chan_mask);
	writel(reg, adc->regs + ADC_INTRMASK_REG);
}

static void phytium_adc_single_conv_setup(struct phytium_adc *adc, u8 ch)
{
	u32 reg;

	/*
	 * Setup control register:
	 * - Single conversion mode selection
	 * - Single conversion enable
	 * - Fixed channel conversion
	 * - Target channel
	 */
	reg = readl(adc->regs + ADC_CTRL_REG);

	/* Clean ch_only_s bits */
	reg &= ~ADC_CTRL_REG_CH_ONLY_S(7);

	/* Clean channel_en bit */
	reg &= 0xFFF00F;

	reg |=  ADC_CTRL_REG_SINGLE_SEL | ADC_CTRL_REG_SINGLE_EN |
		ADC_CTRL_REG_CH_ONLY_EN | ADC_CTRL_REG_CH_ONLY_S(ch) | ADC_CTRL_REG_CHANNEL_EN(ch);
	writel(reg, adc->regs + ADC_CTRL_REG);
}

static int phytium_adc_single_conv(struct iio_dev *indio_dev, u8 ch)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	int ret;

	ret = iio_device_claim_direct_mode(indio_dev);
	if (ret)
		return ret;
	mutex_lock(&adc->lock);

	phytium_adc_intrmask_setup(adc, BIT(ch), true);
	reinit_completion(&adc->completion);
	phytium_adc_single_conv_setup(adc, ch);
	phytium_adc_start_stop(adc, true);

	if (!wait_for_completion_timeout(&adc->completion, PHYTIUM_ADC_TIMEOUT))
		ret = -ETIMEDOUT;

	phytium_adc_start_stop(adc, false);
	phytium_adc_intrmask_setup(adc, BIT(ch), false);

	mutex_unlock(&adc->lock);
	iio_device_release_direct_mode(indio_dev);

	return ret;
}

static int phytium_adc_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val, int *val2, long mask)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->type != IIO_VOLTAGE)
			return -EINVAL;

		ret = phytium_adc_single_conv(indio_dev, chan->channel);
		if (ret)
			return ret;
		*val = adc->last_val[chan->channel];

		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int phytium_read_thresh(struct iio_dev *indio_dev,
			       const struct iio_chan_spec *chan,
			       enum iio_event_type type,
			       enum iio_event_direction dir,
			       enum iio_event_info iinfo, int *val, int *val2)
{
	struct phytium_adc *adc = iio_priv(indio_dev);

	if (dir == IIO_EV_DIR_FALLING)
		*val = adc->thresh_low[chan->channel];
	else
		*val = adc->thresh_high[chan->channel];

	return IIO_VAL_INT;
}

static int phytium_write_thresh(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan,
				     enum iio_event_type type,
				     enum iio_event_direction dir,
				     enum iio_event_info einfo, int val, int val2)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	u32 thresh;

	switch (dir) {
	case IIO_EV_DIR_FALLING:
		adc->thresh_low[chan->channel] = val;
		thresh = readl(adc->regs + ADC_LEVEL_REG(chan->channel)) & 0x3ff0000;
		thresh |= ADC_LEVEL_REG_LOW_LEVEL(val);
		writel(thresh, adc->regs + ADC_LEVEL_REG(chan->channel));
		break;
	case IIO_EV_DIR_RISING:
		adc->thresh_high[chan->channel] = val;
		thresh = readl(adc->regs + ADC_LEVEL_REG(chan->channel)) & 0xffff;
		thresh |= ADC_LEVEL_REG_HIGH_LEVEL(val);
		writel(thresh, adc->regs + ADC_LEVEL_REG(chan->channel));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int phytium_update_scan_mode(struct iio_dev *indio_dev, const unsigned long *mask)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	unsigned int n;

	n = bitmap_weight(mask, indio_dev->masklength);

	kfree(adc->scan_data);
	adc->scan_data = kcalloc(n, sizeof(*adc->scan_data), GFP_KERNEL);
	if (!adc->scan_data)
		return -ENOMEM;

	return 0;
}

static const u64 phytium_adc_event_codes[] = {
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 0,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 0,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 1,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 1,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 2,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 2,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 3,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 3,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 4,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 4,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 5,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 5,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 6,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 6,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 7,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	IIO_UNMOD_EVENT_CODE(IIO_VOLTAGE, 7,
			     IIO_EV_TYPE_THRESH, IIO_EV_DIR_FALLING),
};

static irqreturn_t phytium_adc_threaded_irq(int irq, void *data)
{
	struct iio_dev *indio_dev = data;
	struct phytium_adc *adc = iio_priv(indio_dev);
	s64 timestamp = iio_get_time_ns(indio_dev);
	unsigned long status;
	int ch;
	u32 intr;

	intr = readl(adc->regs + ADC_INTR_REG);

	if (intr & ADC_INTR_REG_ERR) {
		dev_err(adc->dev, "conversion error: ADC_INTR_REG(0x%x)\n", intr);
		writel(ADC_INTR_REG_ERR, adc->regs + ADC_INTR_REG);
		return IRQ_HANDLED;
	}

	status = (intr & ADC_INTR_REG_LIMIT_MASK) >> 8;
	if (status) {
		for_each_set_bit(ch, &status, PHYTIUM_MAX_CHANNELS * 2)
			iio_push_event(indio_dev, phytium_adc_event_codes[ch], timestamp);
	}

	status = intr & ADC_INTR_REG_COVFIN_MASK;
	if (status) {
		for_each_set_bit(ch, &status, PHYTIUM_MAX_CHANNELS)
			adc->last_val[ch] = readl(adc->regs + ADC_COV_RESULT_REG(ch)) &
					    ADC_COV_RESULT_REG_MASK;

		if (iio_buffer_enabled(indio_dev))
			iio_trigger_poll(indio_dev->trig);
		else
			complete(&adc->completion);
	}

	/* Clear all the interrupts */
	writel(status, adc->regs + ADC_INTR_REG);

	return IRQ_HANDLED;
}

static void phytium_adc_cont_conv_setup(struct phytium_adc *adc,
					unsigned long chan_mask,
					u32 interval)
{
	u32 reg;

	/*
	 * Setup control register:
	 * - Continuous conversion mode
	 * - Multi-channel rotation mode
	 * - Channel enablement
	 */
	reg = readl(adc->regs + ADC_CTRL_REG);
	reg &= ~(ADC_CTRL_REG_SINGLE_SEL | ADC_CTRL_REG_SINGLE_EN |
		 ADC_CTRL_REG_CH_ONLY_EN);
	reg |= chan_mask << 4;
	writel(reg, adc->regs + ADC_CTRL_REG);

	/* Setup interval between two conversions */
	writel(interval, adc->regs + ADC_INTER_REG);
}

static int phytium_adc_preenable(struct iio_dev *indio_dev)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	unsigned long scan_mask = *indio_dev->active_scan_mask;

	phytium_adc_cont_conv_setup(adc, scan_mask & 0xff, adc->interval);
	phytium_adc_intrmask_setup(adc, scan_mask & 0xff, true);

	return 0;
}

static int phytium_adc_postenable(struct iio_dev *indio_dev)
{
	struct phytium_adc *adc = iio_priv(indio_dev);

	phytium_adc_start_stop(adc, true);

	return 0;
}

static int phytium_adc_postdisable(struct iio_dev *indio_dev)
{
	struct phytium_adc *adc = iio_priv(indio_dev);
	unsigned long scan_mask = *indio_dev->active_scan_mask;

	phytium_adc_start_stop(adc, false);
	phytium_adc_intrmask_setup(adc, scan_mask & 0xff, false);

	return 0;
}

static const struct iio_buffer_setup_ops phytium_buffer_setup_ops = {
	.preenable = &phytium_adc_preenable,
	.postenable = &phytium_adc_postenable,
	.postdisable = &phytium_adc_postdisable,
};

static irqreturn_t phytium_adc_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct phytium_adc *adc = iio_priv(indio_dev);
	int i, j = 0;

	if (!adc->scan_data)
		goto out;

	for_each_set_bit(i, indio_dev->active_scan_mask, indio_dev->masklength)
		adc->scan_data[j++] = adc->last_val[i];

	iio_push_to_buffers(indio_dev, adc->scan_data);

out:
	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

static const struct iio_info phytium_adc_iio_info = {
	.read_raw = &phytium_adc_read_raw,
	.read_event_value = &phytium_read_thresh,
	.write_event_value = &phytium_write_thresh,
	.update_scan_mode = &phytium_update_scan_mode,
};

static int phytium_adc_probe(struct platform_device *pdev)
{
	struct phytium_adc *adc;
	struct iio_dev *indio_dev;
	struct device *dev = &pdev->dev;
	struct resource *res;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*adc));
	if (!indio_dev)
		return -ENOMEM;

	adc = iio_priv(indio_dev);
	adc->dev = dev;

	ret = phytium_adc_parse_properties(pdev, adc);
	if (ret)
		return ret;

	mutex_init(&adc->lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	adc->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(adc->regs))
		return PTR_ERR(adc->regs);

	adc->adc_clk = devm_clk_get(dev, NULL);
	if (IS_ERR(adc->adc_clk))
		return PTR_ERR(adc->adc_clk);

	init_completion(&adc->completion);

	indio_dev->name = dev_name(dev);
	indio_dev->info = &phytium_adc_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = adc->data->channels;
	indio_dev->num_channels = adc->data->num_channels;

	platform_set_drvdata(pdev, indio_dev);

	ret = devm_request_threaded_irq(adc->dev, platform_get_irq(pdev, 0),
					NULL, phytium_adc_threaded_irq, IRQF_ONESHOT,
					dev_name(dev), indio_dev);
	if (ret)
		return ret;

	ret = devm_iio_triggered_buffer_setup(dev, indio_dev,
					      &iio_pollfunc_store_time,
					      phytium_adc_trigger_handler,
					      &phytium_buffer_setup_ops);
	if (ret)
		return ret;

	ret = phytium_adc_hw_init(adc);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize Phytium ADC, %d\n", ret);
		return ret;
	}

	return iio_device_register(indio_dev);
}

static int phytium_adc_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct phytium_adc *adc = iio_priv(indio_dev);

	phytium_adc_power_setup(adc, false);
	iio_device_unregister(indio_dev);
	kfree(adc->scan_data);

	return 0;
}

static const struct of_device_id phytium_of_match[] = {
	{ .compatible = "phytium,adc", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, phytium_of_match);

#ifdef CONFIG_PM
static int phytium_adc_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct phytium_adc *adc = iio_priv(indio_dev);

	phytium_adc_power_setup(adc, false);
	clk_disable_unprepare(adc->adc_clk);

	return 0;
}

static int phytium_adc_resume(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct phytium_adc *adc = iio_priv(indio_dev);

	clk_prepare_enable(adc->adc_clk);
	phytium_adc_power_setup(adc, true);

	return 0;
}
#endif

SIMPLE_DEV_PM_OPS(phytium_adc_pm_ops, phytium_adc_suspend, phytium_adc_resume);

static struct platform_driver phytium_adc_driver = {
	.driver		= {
		.name		= "phytium_adc",
		.of_match_table	= phytium_of_match,
		.pm		= &phytium_adc_pm_ops,
	},
	.probe		= phytium_adc_probe,
	.remove		= phytium_adc_remove,
};
module_platform_driver(phytium_adc_driver);

MODULE_AUTHOR("Yang Liu <yangliu2021@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium ADC driver");
MODULE_LICENSE("GPL v2");
