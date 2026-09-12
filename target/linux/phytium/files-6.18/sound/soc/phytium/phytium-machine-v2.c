// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (c) 2023-2024, Phytium Techonology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>
#include <linux/version.h>

#define PHYT_MACHINE_V2_VERSION "1.0.0"

/* PMDK widgets */
static const struct snd_soc_dapm_widget phyt_machine_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Front", NULL),
	SND_SOC_DAPM_HP("Rear", NULL),

	SND_SOC_DAPM_MIC("FrontIn", NULL),
	SND_SOC_DAPM_MIC("RearIn", NULL),
};

/* PMDK control */
static const struct snd_kcontrol_new phyt_machine_controls[] = {
	SOC_DAPM_PIN_SWITCH("Front"),
	SOC_DAPM_PIN_SWITCH("Rear"),
	SOC_DAPM_PIN_SWITCH("FrontIn"),
	SOC_DAPM_PIN_SWITCH("RearIn"),
};

/* PMDK connections */
static const struct snd_soc_dapm_route phyt_machine_audio_map[] = {
	{"INPUT1", NULL, "FrontIn"},
	{"INPUT2", NULL, "RearIn"},
	{"Front", NULL, "OUTPUT1"},
	{"Rear", NULL, "OUTPUT2"},
};

#define PMDK_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | \
	SND_SOC_DAIFMT_CBS_CFS)

SND_SOC_DAILINK_DEFS(phyt_machine,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-v2")),
	DAILINK_COMP_ARRAY(COMP_CODEC("PHYT1002:00", "phytium-hifi-v2")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

static struct snd_soc_dai_link phyt_machine_dai[] = {
	{
		.name = "PHYTIUM HIFI V2",
		.id = 0,
		.stream_name = "PHYTIUM HIFT V2",
		.dai_fmt = PMDK_DAI_FMT,
		SND_SOC_DAILINK_REG(phyt_machine),
	},
};

static struct snd_soc_card phyt_machine_card = {
	.name = "PHYT-MIE-V2",
	.owner = THIS_MODULE,
	.dai_link = phyt_machine_dai,
	.num_links = ARRAY_SIZE(phyt_machine_dai),

	.dapm_widgets = phyt_machine_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(phyt_machine_dapm_widgets),
	.controls = phyt_machine_controls,
	.num_controls = ARRAY_SIZE(phyt_machine_controls),
	.dapm_routes = phyt_machine_audio_map,
	.num_dapm_routes = ARRAY_SIZE(phyt_machine_audio_map),
};

static int phyt_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &phyt_machine_card;
	struct device *dev = &pdev->dev;

	card->dev = dev;

	return devm_snd_soc_register_card(&pdev->dev, card);
}

static const struct acpi_device_id phyt_machine_match[] = {
	{ "PHYT1001", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, phyt_machine_match);

static struct platform_driver phyt_machine_driver = {
	.probe = phyt_machine_probe,
	.driver = {
		.name = "phyt_machine",
		.acpi_match_table = phyt_machine_match,
#ifdef CONFIG_PM
		.pm = &snd_soc_pm_ops,
#endif
	},
};

module_platform_driver(phyt_machine_driver);

MODULE_DESCRIPTION("ALSA SoC Phytium Machine V2");
MODULE_AUTHOR("Yang Xun <yangxun@phytium.com.cn>");
MODULE_LICENSE("GPL");
MODULE_VERSION(PHYT_MACHINE_V2_VERSION);
