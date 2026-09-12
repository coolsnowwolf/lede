// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021-2024 Phytium Technology Co.,Ltd.
 */

#include <linux/module.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>

/* PMDK widgets */
static const struct snd_soc_dapm_widget pmdk_ma1026_dapm_widgets[] = {
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_MIC("Int Mic", NULL),
	SND_SOC_DAPM_MIC("Mic In", NULL),
};

/* PMDK control */
static const struct snd_kcontrol_new pmdk_ma1026_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("Int Mic"),
	SOC_DAPM_PIN_SWITCH("Mic In"),
};

/* PMDK connections */
static const struct snd_soc_dapm_route pmdk_ma1026_audio_map[] = {
	{"DMIC", NULL, "Int Mic"},
	{"MIC1", NULL, "Mic In"},
	{"MIC2", NULL, "Mic In"},

	{"HP", NULL, "HPOL"},
	{"HP", NULL, "HPOR"},
};

#define PMDK_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | \
	SND_SOC_DAIFMT_CBS_CFS)

SND_SOC_DAILINK_DEFS(pmdk_ma1026,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-lsd")),
	DAILINK_COMP_ARRAY(COMP_CODEC("i2c-MAMA1026:00", "ma1026-asp")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

static struct snd_soc_dai_link pmdk_dai[] = {
	{
		.name = "MA1026 ASP",
		.stream_name = "MA1026 ASP",
		.dai_fmt = PMDK_DAI_FMT,
		SND_SOC_DAILINK_REG(pmdk_ma1026),
	},
};

static struct snd_soc_card pmdk = {
	.name = "PMDK-I2S",
	.owner = THIS_MODULE,
	.dai_link = pmdk_dai,
	.num_links = ARRAY_SIZE(pmdk_dai),
};

static int pmdk_sound_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &pmdk;
	struct device *dev = &pdev->dev;

	card->dev = dev;
	return devm_snd_soc_register_card(&pdev->dev, card);
}

static const struct acpi_device_id pmdk_sound_acpi_match[] = {
	{ "PHYT8013", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, pmdk_sound_acpi_match);

static const struct of_device_id phytium_ma1026_of_match[] = {
	{ .compatible = "phytium,audio-ma1026",},
	{},
};

static struct platform_driver pmdk_sound_driver = {
	.probe = pmdk_sound_probe,
	.driver = {
		.name = "pmdk_ma1026",
		.acpi_match_table = pmdk_sound_acpi_match,
	    .of_match_table = phytium_ma1026_of_match,
#ifdef CONFIG_PM
		.pm = &snd_soc_pm_ops,
#endif
	},
};

module_platform_driver(pmdk_sound_driver);

MODULE_AUTHOR("Shi Guangyuan <shiguangyuan1029@phytium.com.cn>");
MODULE_DESCRIPTION("ALSA SoC PMDK MA1026");
MODULE_LICENSE("GPL");
