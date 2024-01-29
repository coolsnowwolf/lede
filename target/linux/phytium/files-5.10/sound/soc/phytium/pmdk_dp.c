// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>

#define DAI_CNT(pmdk_dai_) sizeof(pmdk_dai_)/sizeof(struct snd_soc_dai_link)

struct pmdk_dp_private {
	struct snd_soc_jack jack0;
	struct snd_soc_jack jack1;
	struct snd_soc_jack jack2;
};

/* PMDK widgets */
static const struct snd_soc_dapm_widget pmdk_dp_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("DP", NULL),
};

/* PMDK control */
static const struct snd_kcontrol_new pmdk_controls[] = {
	SOC_DAPM_PIN_SWITCH("DP"),
};

/* PMDK connections */
static const struct snd_soc_dapm_route pmdk_dp_audio_map[] = {
	{"DP", NULL, "TX"},
};

static struct snd_soc_jack_pin dp0_pins[] = {
	{
		.pin	= "HDMI/DP,pcm=0",
		.mask	= SND_JACK_LINEOUT,
	},
};

static struct snd_soc_jack_pin dp1_pins[] = {
	{
		.pin	= "HDMI/DP,pcm=1",
		.mask	= SND_JACK_LINEOUT,
	},
};

static struct snd_soc_jack_pin dp2_pins[] = {
	{
		.pin	= "HDMI/DP,pcm=2",
		.mask	= SND_JACK_LINEOUT,
	},
};

#define SMDK_DAI_FMT (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | \
	SND_SOC_DAIFMT_CBS_CFS)

static int pmdk_dp0_init(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_soc_card *card = runtime->card;
	struct pmdk_dp_private *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_component *component = asoc_rtd_to_codec(runtime, 0)->component;
	int ret;

	ret = snd_soc_card_jack_new(card, "HDMI/DP,pcm=0",
				    SND_JACK_LINEOUT,
				    &priv->jack0, dp0_pins,
				    ARRAY_SIZE(dp0_pins));
	if (ret) {
		dev_err(card->dev, "Jack creation failed %d\n", ret);
		return ret;
	}
	snd_soc_component_set_jack(component, &priv->jack0, NULL);
	return ret;
}

static int pmdk_dp1_init(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_soc_card *card = runtime->card;
	struct pmdk_dp_private *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_component *component = asoc_rtd_to_codec(runtime, 0)->component;
	int ret;

	ret = snd_soc_card_jack_new(card, "HDMI/DP,pcm=1",
				    SND_JACK_LINEOUT,
				    &priv->jack1, dp1_pins,
				    ARRAY_SIZE(dp1_pins));

	if (ret) {
		dev_err(card->dev, "Jack creation failed %d\n", ret);
		return ret;
	}
	snd_soc_component_set_jack(component, &priv->jack1, NULL);
	return ret;
}

static int pmdk_dp2_init(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_soc_card *card = runtime->card;
	struct pmdk_dp_private *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_component *component = asoc_rtd_to_codec(runtime, 0)->component;
	int ret;

	ret = snd_soc_card_jack_new(card, "HDMI/DP,pcm=2",
				    SND_JACK_LINEOUT,
				    &priv->jack2, dp2_pins,
				    ARRAY_SIZE(dp2_pins));
	if (ret) {
		dev_err(card->dev, "Jack creation failed %d\n", ret);
		return ret;
	}
	snd_soc_component_set_jack(component, &priv->jack2, NULL);
	return ret;
}

SND_SOC_DAILINK_DEFS(pmdk_dp0_dai,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-dp0")),
	DAILINK_COMP_ARRAY(COMP_CODEC("hdmi-audio-codec.1346918656", "i2s-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(pmdk_dp1_dai,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-dp1")),
	DAILINK_COMP_ARRAY(COMP_CODEC("hdmi-audio-codec.1346918657", "i2s-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

SND_SOC_DAILINK_DEFS(pmdk_dp2_dai,
	DAILINK_COMP_ARRAY(COMP_CPU("phytium-i2s-dp2")),
	DAILINK_COMP_ARRAY(COMP_CODEC("hdmi-audio-codec.1346918658", "i2s-hifi")),
	DAILINK_COMP_ARRAY(COMP_PLATFORM("snd-soc-dummy")));

static struct snd_soc_dai_link pmdk_dai_local[] = {
{
	.name = "Phytium dp0-audio",
	.stream_name = "Playback",
	.dai_fmt = SMDK_DAI_FMT,
	.init = pmdk_dp0_init,
	SND_SOC_DAILINK_REG(pmdk_dp0_dai),
},{
	.name = "Phytium dp1-audio",
	.stream_name = "Playback",
	.dai_fmt = SMDK_DAI_FMT,
	.init = pmdk_dp1_init,
	SND_SOC_DAILINK_REG(pmdk_dp1_dai),
},
{
	.name = "Phytium dp2-audio",
	.stream_name = "Playback",
	.dai_fmt = SMDK_DAI_FMT,
	.init = pmdk_dp2_init,
	SND_SOC_DAILINK_REG(pmdk_dp2_dai),
},
};

static struct snd_soc_card pmdk = {
	.name = "PMDK-I2S",
	.owner = THIS_MODULE,

	.dapm_widgets = pmdk_dp_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(pmdk_dp_dapm_widgets),
	.controls = pmdk_controls,
	.num_controls = ARRAY_SIZE(pmdk_controls),
	.dapm_routes = pmdk_dp_audio_map,
	.num_dapm_routes = ARRAY_SIZE(pmdk_dp_audio_map),
};

static int pmdk_sound_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &pmdk;
	struct pmdk_dp_private *priv;
	struct snd_soc_dai_link *pmdk_dai;
	int num_dp = 2;
	char dp_mask = 0x7;
	int i,j = 0;
	card->dev = &pdev->dev;

	device_property_read_u32(&pdev->dev, "num-dp", &num_dp);
	device_property_read_u8(&pdev->dev, "dp-mask", &dp_mask);
	pmdk_dai = devm_kzalloc(&pdev->dev, num_dp * sizeof(*pmdk_dai), GFP_KERNEL);
	if (!pmdk_dai)
		return -ENOMEM;

	if (!num_dp || num_dp > DAI_CNT(pmdk_dai_local))
		return -EINVAL;

	for(i = 0; i < num_dp; i++) {
		for (; j < DAI_CNT(pmdk_dai_local); j++) {
			if (BIT(j) & dp_mask) {
				pmdk_dai[i] = pmdk_dai_local[j];
				j++;
				break;
			}
		}
	}

	card->dai_link = pmdk_dai;
	card->num_links = num_dp;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	snd_soc_card_set_drvdata(card, priv);

	return devm_snd_soc_register_card(&pdev->dev, card);
}

static const struct of_device_id pmdk_sound_of_match[] = {
	{ .compatible = "phytium,pmdk-dp",},
	{ }
};
MODULE_DEVICE_TABLE(of, pmdk_sound_of_match);

static const struct acpi_device_id pmdk_sound_acpi_match[] = {
	{ "PHYT8006", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, pmdk_sound_acpi_match);

static struct platform_driver pmdk_sound_driver = {
	.probe = pmdk_sound_probe,
	.driver = {
		.name = "pmdk_dp",
		.acpi_match_table = pmdk_sound_acpi_match,
		.of_match_table = pmdk_sound_of_match,
#ifdef CONFIG_PM
		.pm = &snd_soc_pm_ops,
#endif
	},
};

module_platform_driver(pmdk_sound_driver);

MODULE_AUTHOR("Zhang Yiqun<zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("ALSA SoC PMDK DP");
MODULE_LICENSE("GPL");
