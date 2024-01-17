// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>

static struct snd_soc_jack hs_jack;

/* Headset jack detection DAPM pins */
static struct snd_soc_jack_pin hs_jack_pins[] = {
	{
		.pin	= "FrontIn",
		.mask	= SND_JACK_MICROPHONE,
	},
	{
		.pin	= "RearIn",
		.mask	= SND_JACK_MICROPHONE,
		.invert = 1
	},
	{
		.pin	= "Front",
		.mask	= SND_JACK_HEADPHONE,
	},
	{
		.pin    = "Rear",
		.mask   = SND_JACK_HEADPHONE,
		.invert = 1
	},
};

/* Headset jack detection gpios */
static struct snd_soc_jack_gpio hs_jack_gpios[] = {
	{
		.name		= "det",
		.report		= SND_JACK_HEADSET,
		.debounce_time	= 200,
		.invert		= 1,
	},
};

/* PMDK widgets */
static const struct snd_soc_dapm_widget pmdk_es8388_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Front", NULL),
	SND_SOC_DAPM_HP("Rear", NULL),

	SND_SOC_DAPM_MIC("FrontIn", NULL),
	SND_SOC_DAPM_MIC("RearIn", NULL),
};

/* PMDK control */
static const struct snd_kcontrol_new pmdk_controls[] = {
	SOC_DAPM_PIN_SWITCH("Front"),
	SOC_DAPM_PIN_SWITCH("Rear"),
	SOC_DAPM_PIN_SWITCH("FrontIn"),
	SOC_DAPM_PIN_SWITCH("RearIn"),
};

/* PMDK connections */
static const struct snd_soc_dapm_route pmdk_es8388_audio_map[] = {
	{"LINPUT1", NULL, "FrontIn"},
	{"RINPUT1", NULL, "FrontIn"},

	{"LINPUT2", NULL, "RearIn"},
	{"RINPUT2", NULL, "RearIn"},

	{"Front", NULL, "LOUT1"},
	{"Front", NULL, "ROUT1"},

	{"Rear", NULL, "LOUT2"},
	{"Rear", NULL, "ROUT2"},
};

static int pmdk_es8388_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret;

	/* Jack detection API stuff */
	ret = snd_soc_card_jack_new(rtd->card, "Headset Jack", SND_JACK_HEADSET,
			    &hs_jack, hs_jack_pins,
			    ARRAY_SIZE(hs_jack_pins));
	if (ret)
		goto err;

	ret = snd_soc_jack_add_gpios(&hs_jack, ARRAY_SIZE(hs_jack_gpios),
				hs_jack_gpios);
	if (ret)
		goto err;

	return 0;

err:
	return ret;
}

#define PMDK_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | \
	SND_SOC_DAIFMT_CBS_CFS)

SND_SOC_DAILINK_DEFS(pmdk_es8388,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-lsd")),
	DAILINK_COMP_ARRAY(COMP_CODEC("i2c-ESSX8388:00", "es8388-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

static struct snd_soc_dai_link pmdk_dai[] = {
	{
		.name = "ES8388 HIFI",
		.stream_name = "ES8388 HIFI",
		.dai_fmt = PMDK_DAI_FMT,
		.init = pmdk_es8388_init,
		SND_SOC_DAILINK_REG(pmdk_es8388),
	},
};

static struct snd_soc_card pmdk = {
	.name = "PMDK-I2S",
	.owner = THIS_MODULE,
	.dai_link = pmdk_dai,
	.num_links = ARRAY_SIZE(pmdk_dai),

	.dapm_widgets = pmdk_es8388_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(pmdk_es8388_dapm_widgets),
	.controls = pmdk_controls,
	.num_controls = ARRAY_SIZE(pmdk_controls),
	.dapm_routes = pmdk_es8388_audio_map,
	.num_dapm_routes = ARRAY_SIZE(pmdk_es8388_audio_map),
};

static int pmdk_sound_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &pmdk;
	struct device *dev = &pdev->dev;
	int n;

	card->dev = dev;
	hs_jack_gpios[0].gpiod_dev = dev;
	n = gpiod_count(dev, "det");

	if (n < 0)
		pmdk_dai[0].init = NULL;

	return devm_snd_soc_register_card(&pdev->dev, card);
}

static const struct acpi_device_id pmdk_sound_acpi_match[] = {
	{ "PHYT8004", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, pmdk_sound_acpi_match);

static struct platform_driver pmdk_sound_driver = {
	.probe = pmdk_sound_probe,
	.driver = {
		.name = "pmdk_es8388",
		.acpi_match_table = pmdk_sound_acpi_match,
#ifdef CONFIG_PM
		.pm = &snd_soc_pm_ops,
#endif
	},
};

module_platform_driver(pmdk_sound_driver);

MODULE_AUTHOR("Zhang Yiqun<zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("ALSA SoC PMDK ES8388");
MODULE_LICENSE("GPL");
