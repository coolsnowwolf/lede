// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

/* PMDK widgets */
static const struct snd_soc_dapm_widget pmdk_es8336_dapm_widgets[] = {
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_MIC("Int Mic", NULL),
	SND_SOC_DAPM_MIC("Mic In", NULL),
};

/* PMDK control */
static const struct snd_kcontrol_new pmdk_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("Int Mic"),
	SOC_DAPM_PIN_SWITCH("Mic In"),
};

/* PMDK connections */
static const struct snd_soc_dapm_route pmdk_es8336_audio_map[] = {
	{"DMIC", NULL, "Int Mic"},
	{"MIC1", NULL, "Mic In"},
	{"MIC2", NULL, "Mic In"},

	{"HP", NULL, "HPOL"},
	{"HP", NULL, "HPOR"},
};

#define PMDK_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | \
	SND_SOC_DAIFMT_CBS_CFS)

SND_SOC_DAILINK_DEFS(pmdk_es8366,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-lsd")),
	DAILINK_COMP_ARRAY(COMP_CODEC("i2c-ESSX8336:00", "es8336-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

static struct snd_soc_dai_link pmdk_dai[] = {
	{
		.name = "ES8336 HIFI",
		.stream_name = "ES8336 HIFI",
		.dai_fmt = PMDK_DAI_FMT,
		SND_SOC_DAILINK_REG(pmdk_es8366),
	},
};

static struct snd_soc_card pmdk = {
	.name = "PMDK-I2S",
	.owner = THIS_MODULE,
	.dai_link = pmdk_dai,
	.num_links = ARRAY_SIZE(pmdk_dai),

	.dapm_widgets = pmdk_es8336_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(pmdk_es8336_dapm_widgets),
	.controls = pmdk_controls,
	.num_controls = ARRAY_SIZE(pmdk_controls),
	.dapm_routes = pmdk_es8336_audio_map,
	.num_dapm_routes = ARRAY_SIZE(pmdk_es8336_audio_map),
};

static int pmdk_sound_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &pmdk;
	struct device *dev = &pdev->dev;

	card->dev = dev;

	return devm_snd_soc_register_card(&pdev->dev, card);
}

static const struct acpi_device_id pmdk_sound_acpi_match[] = {
	{ "PHYT8005", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, pmdk_sound_acpi_match);

static struct platform_driver pmdk_sound_driver = {
	.probe = pmdk_sound_probe,
	.driver = {
		.name = "pmdk_es8336",
		.acpi_match_table = pmdk_sound_acpi_match,
#ifdef CONFIG_PM
		.pm = &snd_soc_pm_ops,
#endif
	},
};

module_platform_driver(pmdk_sound_driver);
MODULE_AUTHOR("Zhang Yiqun <zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("ALSA SoC PMDK ES8336");
MODULE_LICENSE("GPL");
