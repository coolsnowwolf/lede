// SPDX-License-Identifier: GPL-2.0
/*
 * es8336.c -- es8336 ALSA SoC audio driver
 * Copyright Everest Semiconductor Co.,Ltd
 *           Phytium Information Technology Co.,Ltd
 *
 * Author: David Yang <yangxiaohua@everest-semi.com>
 *         Yiqun Zhang <zhangyiqun@phytium.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <linux/proc_fs.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/regmap.h>
#include "es8336.h"

static struct snd_soc_component *es8336_component;

static const struct reg_default es8336_reg_defaults[] = {
	{0x00, 0x03}, {0x01, 0x03}, {0x02, 0x00}, {0x03, 0x20},
	{0x04, 0x11}, {0x05, 0x00}, {0x06, 0x11}, {0x07, 0x00},
	{0x08, 0x00}, {0x09, 0x01}, {0x0a, 0x00}, {0x0b, 0x00},
	{0x0c, 0xf8}, {0x0d, 0x3f}, {0x0e, 0x00}, {0x0f, 0x00},
	{0x10, 0x01}, {0x11, 0xfc}, {0x12, 0x28}, {0x13, 0x00},
	{0x14, 0x00}, {0x15, 0x33}, {0x16, 0x00}, {0x17, 0x00},
	{0x18, 0x88}, {0x19, 0x06}, {0x1a, 0x22}, {0x1b, 0x03},
	{0x1c, 0x0f}, {0x1d, 0x00}, {0x1e, 0x80}, {0x1f, 0x80},
	{0x20, 0x00}, {0x21, 0x00}, {0x22, 0xc0}, {0x23, 0x00},
	{0x24, 0x01}, {0x25, 0x08}, {0x26, 0x10}, {0x27, 0xc0},
	{0x28, 0x00}, {0x29, 0x1c}, {0x2a, 0x00}, {0x2b, 0xb0},
	{0x2c, 0x32}, {0x2d, 0x03}, {0x2e, 0x00}, {0x2f, 0x11},
	{0x30, 0x10}, {0x31, 0x00}, {0x32, 0x00}, {0x33, 0xc0},
	{0x34, 0xc0}, {0x35, 0x1f}, {0x36, 0xf7}, {0x37, 0xfd},
	{0x38, 0xff}, {0x39, 0x1f}, {0x3a, 0xf7}, {0x3b, 0xfd},
	{0x3c, 0xff}, {0x3d, 0x1f}, {0x3e, 0xf7}, {0x3f, 0xfd},
	{0x40, 0xff}, {0x41, 0x1f}, {0x42, 0xf7}, {0x43, 0xfd},
	{0x44, 0xff}, {0x45, 0x1f}, {0x46, 0xf7}, {0x47, 0xfd},
	{0x48, 0xff}, {0x49, 0x1f}, {0x4a, 0xf7}, {0x4b, 0xfd},
	{0x4c, 0xff}, {0x4d, 0x00}, {0x4e, 0x00}, {0x4f, 0xff},
	{0x50, 0x00}, {0x51, 0x00}, {0x52, 0x00}, {0x53, 0x00},
};

/* codec private data */
struct es8336_priv {
	struct regmap *regmap;
	unsigned int dmic_amic;
	unsigned int sysclk;
	struct snd_pcm_hw_constraint_list *sysclk_constraints;
	struct clk *mclk;
	int debounce_time;
	struct delayed_work work;

	struct gpio_desc *spk_ctl_gpio;
	struct gpio_desc *hp_det_gpio;
	bool muted;
	bool hp_inserted;

	u8 mic_src;
	int pwr_count;
};

/*
 * es8336_reset
 * write value 0xff to reg0x00, the chip will be in reset mode
 * then, writer 0x00 to reg0x00, unreset the chip
 */
static int es8336_reset(struct snd_soc_component *component)
{
	snd_soc_component_write(component, ES8336_RESET_REG00, 0x3F);
	usleep_range(5000, 5500);
	return snd_soc_component_write(component, ES8336_RESET_REG00, 0x03);
}

static void es8336_enable_spk(struct es8336_priv *es8336, bool enable)
{
	gpiod_set_value(es8336->spk_ctl_gpio, enable);
}

static const DECLARE_TLV_DB_SCALE(dac_vol_tlv, -9600, 50, 1);
static const DECLARE_TLV_DB_SCALE(adc_vol_tlv, -9600, 50, 1);
static const DECLARE_TLV_DB_SCALE(hpmixer_gain_tlv, -1200, 150, 0);
static const DECLARE_TLV_DB_SCALE(mic_bst_tlv, 0, 1200, 0);

static unsigned int linin_pga_tlv[] = {
	TLV_DB_RANGE_HEAD(9),
	0, 0, TLV_DB_SCALE_ITEM(0, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(300, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(600, 0, 0),
	3, 3, TLV_DB_SCALE_ITEM(900, 0, 0),
	4, 4, TLV_DB_SCALE_ITEM(1200, 0, 0),
	5, 5, TLV_DB_SCALE_ITEM(1500, 0, 0),
	6, 6, TLV_DB_SCALE_ITEM(1800, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(2100, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(2400, 0, 0),
};

static unsigned int hpout_vol_tlv[] = {
	TLV_DB_RANGE_HEAD(1),
	0, 3, TLV_DB_SCALE_ITEM(-4800, 1200, 0),
};

static const char *const alc_func_txt[] = { "Off", "On" };

static const struct soc_enum alc_func =
	SOC_ENUM_SINGLE(ES8336_ADC_ALC1_REG29, 6, 2, alc_func_txt);

static const char *const ng_type_txt[] = {
	"Constant PGA Gain", "Mute ADC Output" };

static const struct soc_enum ng_type =
	SOC_ENUM_SINGLE(ES8336_ADC_ALC6_REG2E, 6, 2, ng_type_txt);

static const char *const adcpol_txt[] = { "Normal", "Invert" };

static const struct soc_enum adcpol =
	SOC_ENUM_SINGLE(ES8336_ADC_MUTE_REG26, 1, 2, adcpol_txt);

static const char *const dacpol_txt[] = {
	"Normal", "R Invert", "L Invert", "L + R Invert" };

static const struct soc_enum dacpol =
	SOC_ENUM_SINGLE(ES8336_DAC_SET1_REG30, 0, 4, dacpol_txt);

static const struct snd_kcontrol_new es8336_snd_controls[] = {
	/* HP OUT VOLUME */
	SOC_DOUBLE_TLV("HP Playback Volume", ES8336_CPHP_ICAL_VOL_REG18,
		       4, 0, 4, 1, hpout_vol_tlv),
	/* HPMIXER VOLUME Control */
	SOC_DOUBLE_TLV("HPMixer Gain", ES8336_HPMIX_VOL_REG16,
		       0, 4, 7, 0, hpmixer_gain_tlv),

	/* DAC Digital controls */
	SOC_DOUBLE_R_TLV("DAC Playback Volume", ES8336_DAC_VOLL_REG33,
			 ES8336_DAC_VOLR_REG34, 0, 0xC0, 1, dac_vol_tlv),

	SOC_SINGLE("Enable DAC Soft Ramp", ES8336_DAC_SET1_REG30, 4, 1, 1),
	SOC_SINGLE("DAC Soft Ramp Rate", ES8336_DAC_SET1_REG30, 2, 4, 0),

	SOC_ENUM("Playback Polarity", dacpol),
	SOC_SINGLE("DAC Notch Filter", ES8336_DAC_SET2_REG31, 6, 1, 0),
	SOC_SINGLE("DAC Double Fs Mode", ES8336_DAC_SET2_REG31, 7, 1, 0),
	SOC_SINGLE("DAC Volume Control-LeR", ES8336_DAC_SET2_REG31, 2, 1, 0),
	SOC_SINGLE("DAC Stereo Enhancement", ES8336_DAC_SET3_REG32, 0, 7, 0),

	/* +20dB D2SE PGA Control */
	SOC_SINGLE_TLV("MIC Boost", ES8336_ADC_D2SEPGA_REG24,
		       0, 1, 0, mic_bst_tlv),
	/* 0-+24dB Lineinput PGA Control */
	SOC_SINGLE_TLV("Input PGA", ES8336_ADC_PGAGAIN_REG23,
		       4, 8, 0, linin_pga_tlv),
};

/* Analog Input MUX */
static const char * const es8336_analog_in_txt[] = {
	"lin1-rin1",
	"lin2-rin2",
	"lin1-rin1 with 15db Boost",
	"lin2-rin2 with 15db Boost"
};

static const unsigned int es8336_analog_in_values[] = { 0, 1, 2, 3 };

static const struct soc_enum es8336_analog_input_enum =
	SOC_VALUE_ENUM_SINGLE(ES8336_ADC_PDN_LINSEL_REG22, 4, 3,
			      ARRAY_SIZE(es8336_analog_in_txt),
			      es8336_analog_in_txt,
			      es8336_analog_in_values);

static const struct snd_kcontrol_new es8336_analog_in_mux_controls =
	SOC_DAPM_ENUM("Route", es8336_analog_input_enum);

/* Dmic MUX */
static const char * const es8336_dmic_txt[] = {
	"dmic disable",
	"dmic data at high level",
	"dmic data at low level",
};

static const unsigned int es8336_dmic_values[] = { 0, 2, 3 };

static const struct soc_enum es8336_dmic_src_enum =
	SOC_VALUE_ENUM_SINGLE(ES8336_ADC_DMIC_REG25, 0, 3,
			      ARRAY_SIZE(es8336_dmic_txt),
			      es8336_dmic_txt,
			      es8336_dmic_values);

static const struct snd_kcontrol_new es8336_dmic_src_controls =
	SOC_DAPM_ENUM("Route", es8336_dmic_src_enum);

/* hp mixer mux */
static const char *const es8336_hpmux_texts[] = {
	"lin1-rin1",
	"lin2-rin2",
	"lin-rin with Boost",
	"lin-rin with Boost and PGA"
};

static const unsigned int es8336_hpmux_values[] = { 0, 1, 2, 3 };

static const struct soc_enum es8336_left_hpmux_enum =
	SOC_VALUE_ENUM_SINGLE(ES8336_HPMIX_SEL_REG13, 4, 7,
			      ARRAY_SIZE(es8336_hpmux_texts),
			      es8336_hpmux_texts,
			      es8336_hpmux_values);

static const struct snd_kcontrol_new es8336_left_hpmux_controls =
	SOC_DAPM_ENUM("Route", es8336_left_hpmux_enum);

static const struct soc_enum es8336_right_hpmux_enum =
	SOC_VALUE_ENUM_SINGLE(ES8336_HPMIX_SEL_REG13, 0, 7,
			      ARRAY_SIZE(es8336_hpmux_texts),
			      es8336_hpmux_texts,
			      es8336_hpmux_values);

static const struct snd_kcontrol_new es8336_right_hpmux_controls =
	SOC_DAPM_ENUM("Route", es8336_right_hpmux_enum);

/* headphone Output Mixer */
static const struct snd_kcontrol_new es8336_out_left_mix[] = {
	SOC_DAPM_SINGLE("LLIN Switch", ES8336_HPMIX_SWITCH_REG14,
			6, 1, 0),
	SOC_DAPM_SINGLE("Left DAC Switch", ES8336_HPMIX_SWITCH_REG14,
			7, 1, 0),
};

static const struct snd_kcontrol_new es8336_out_right_mix[] = {
	SOC_DAPM_SINGLE("RLIN Switch", ES8336_HPMIX_SWITCH_REG14,
			2, 1, 0),
	SOC_DAPM_SINGLE("Right DAC Switch", ES8336_HPMIX_SWITCH_REG14,
			3, 1, 0),
};

/* DAC data source mux */
static const char * const es8336_dacsrc_texts[] = {
	"LDATA TO LDAC, RDATA TO RDAC",
	"LDATA TO LDAC, LDATA TO RDAC",
	"RDATA TO LDAC, RDATA TO RDAC",
	"RDATA TO LDAC, LDATA TO RDAC",
};

static const unsigned int es8336_dacsrc_values[] = { 0, 1, 2, 3 };

static const struct soc_enum es8336_dacsrc_mux_enum =
	SOC_VALUE_ENUM_SINGLE(ES8336_DAC_SET1_REG30, 6, 4,
			      ARRAY_SIZE(es8336_dacsrc_texts),
			      es8336_dacsrc_texts,
			      es8336_dacsrc_values);
static const struct snd_kcontrol_new es8336_dacsrc_mux_controls =
	SOC_DAPM_ENUM("Route", es8336_dacsrc_mux_enum);

static const struct snd_soc_dapm_widget es8336_dapm_widgets[] = {
	/* Input Lines */
	SND_SOC_DAPM_INPUT("DMIC"),
	SND_SOC_DAPM_INPUT("MIC1"),
	SND_SOC_DAPM_INPUT("MIC2"),

	SND_SOC_DAPM_MICBIAS("micbias", SND_SOC_NOPM,
			     0, 0),
	/* Input MUX */
	SND_SOC_DAPM_MUX("Differential Mux", SND_SOC_NOPM, 0, 0,
			 &es8336_analog_in_mux_controls),

	SND_SOC_DAPM_PGA("Line input PGA", ES8336_ADC_PDN_LINSEL_REG22,
			 7, 1, NULL, 0),

	/* ADCs */
	SND_SOC_DAPM_ADC("Mono ADC", NULL, ES8336_ADC_PDN_LINSEL_REG22, 6, 1),

	/* Dmic MUX */
	SND_SOC_DAPM_MUX("Digital Mic Mux", SND_SOC_NOPM, 0, 0,
			 &es8336_dmic_src_controls),

	/* Digital Interface */
	SND_SOC_DAPM_AIF_OUT("I2S OUT", "I2S1 Capture",  1,
			     SND_SOC_NOPM, 0, 0),

	SND_SOC_DAPM_AIF_IN("I2S IN", "I2S1 Playback", 0,
			    SND_SOC_NOPM, 0, 0),

	/*  DACs DATA SRC MUX */
	SND_SOC_DAPM_MUX("DAC SRC Mux", SND_SOC_NOPM, 0, 0,
			 &es8336_dacsrc_mux_controls),
	/*  DACs  */
	SND_SOC_DAPM_DAC("Right DAC", NULL, ES8336_DAC_PDN_REG2F, 0, 1),
	SND_SOC_DAPM_DAC("Left DAC", NULL, ES8336_DAC_PDN_REG2F, 4, 1),

	/* Headphone Output Side */
	/* hpmux for hp mixer */
	SND_SOC_DAPM_MUX("Left Hp mux", SND_SOC_NOPM, 0, 0,
			 &es8336_left_hpmux_controls),
	SND_SOC_DAPM_MUX("Right Hp mux", SND_SOC_NOPM, 0, 0,
			 &es8336_right_hpmux_controls),
	/* Output mixer  */
	SND_SOC_DAPM_MIXER("Left Hp mixer", ES8336_HPMIX_PDN_REG15,
			   4, 1, &es8336_out_left_mix[0],
			   ARRAY_SIZE(es8336_out_left_mix)),
	SND_SOC_DAPM_MIXER("Right Hp mixer", ES8336_HPMIX_PDN_REG15,
			   0, 1, &es8336_out_right_mix[0],
			   ARRAY_SIZE(es8336_out_right_mix)),
	SND_SOC_DAPM_MIXER("Left Hp mixer2", SND_SOC_NOPM,
			   4, 1, &es8336_out_left_mix[0],
			   ARRAY_SIZE(es8336_out_left_mix)),
	SND_SOC_DAPM_MIXER("Right Hp mixer2", SND_SOC_NOPM,
			   0, 1, &es8336_out_right_mix[0],
			   ARRAY_SIZE(es8336_out_right_mix)),

	/* Output charge pump */
	SND_SOC_DAPM_PGA("HPCP L", ES8336_CPHP_OUTEN_REG17,
			 6, 0, NULL, 0),
	SND_SOC_DAPM_PGA("HPCP R", ES8336_CPHP_OUTEN_REG17,
			 2, 0, NULL, 0),

	/* Output Driver */
	SND_SOC_DAPM_PGA("HPVOL L", ES8336_CPHP_OUTEN_REG17,
			 5, 0, NULL, 0),
	SND_SOC_DAPM_PGA("HPVOL R", ES8336_CPHP_OUTEN_REG17,
			 1, 0, NULL, 0),
	/* Output Lines */
	SND_SOC_DAPM_OUTPUT("HPOL"),
	SND_SOC_DAPM_OUTPUT("HPOR"),
};

static const struct snd_soc_dapm_route es8336_dapm_routes[] = {
	/*
	 * record route map
	 */
	{"MIC1", NULL, "micbias"},
	{"MIC2", NULL, "micbias"},
	{"DMIC", NULL, "micbias"},

	{"Differential Mux", "lin1-rin1", "MIC1"},
	{"Differential Mux", "lin2-rin2", "MIC2"},
	{"Differential Mux", "lin1-rin1 with 15db Boost", "MIC1"},
	{"Differential Mux", "lin2-rin2 with 15db Boost", "MIC2"},
	{"Line input PGA", NULL, "Differential Mux"},

	{"Mono ADC", NULL, "Line input PGA"},

	{"Digital Mic Mux", "dmic disable", "Mono ADC"},
	{"Digital Mic Mux", "dmic data at high level", "DMIC"},
	{"Digital Mic Mux", "dmic data at low level", "DMIC"},

	{"I2S OUT", NULL, "Digital Mic Mux"},
	/*
	 * playback route map
	 */
	{"DAC SRC Mux", "LDATA TO LDAC, RDATA TO RDAC", "I2S IN"},
	{"DAC SRC Mux", "LDATA TO LDAC, LDATA TO RDAC", "I2S IN"},
	{"DAC SRC Mux", "RDATA TO LDAC, RDATA TO RDAC", "I2S IN"},
	{"DAC SRC Mux", "RDATA TO LDAC, LDATA TO RDAC", "I2S IN"},

	{"Left DAC", NULL, "DAC SRC Mux"},
	{"Right DAC", NULL, "DAC SRC Mux"},

	{"Left Hp mux", "lin1-rin1", "MIC1"},
	{"Left Hp mux", "lin2-rin2", "MIC2"},
	{"Left Hp mux", "lin-rin with Boost", "Differential Mux"},
	{"Left Hp mux", "lin-rin with Boost and PGA", "Line input PGA"},

	{"Right Hp mux", "lin1-rin1", "MIC1"},
	{"Right Hp mux", "lin2-rin2", "MIC2"},
	{"Right Hp mux", "lin-rin with Boost", "Differential Mux"},
	{"Right Hp mux", "lin-rin with Boost and PGA", "Line input PGA"},

	{"Left Hp mixer", "LLIN Switch", "Left Hp mux"},
	{"Left Hp mixer", "Left DAC Switch", "Left DAC"},

	{"Right Hp mixer", "RLIN Switch", "Right Hp mux"},
	{"Right Hp mixer", "Right DAC Switch", "Right DAC"},

	{"HPCP L", NULL, "Left Hp mixer"},
	{"HPCP R", NULL, "Right Hp mixer"},

	{"HPVOL L", NULL, "HPCP L"},
	{"HPVOL R", NULL, "HPCP R"},

	{"HPOL", NULL, "HPVOL L"},
	{"HPOR", NULL, "HPVOL R"},
};


/* The set of rates we can generate from the above for each SYSCLK */

static unsigned int rates_12288[] = {
	8000, 12000, 16000, 24000, 24000, 32000, 48000, 96000,
};

static struct snd_pcm_hw_constraint_list constraints_12288 = {
	.count	= ARRAY_SIZE(rates_12288),
	.list	= rates_12288,
};

static unsigned int rates_112896[] = {
	8000, 11025, 22050, 44100,
};

static struct snd_pcm_hw_constraint_list constraints_112896 = {
	.count	= ARRAY_SIZE(rates_112896),
	.list	= rates_112896,
};

static unsigned int rates_12[] = {
	8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000,
	48000, 88235, 96000,
};

static struct snd_pcm_hw_constraint_list constraints_12 = {
	.count	= ARRAY_SIZE(rates_12),
	.list	= rates_12,
};

/*
 * Note that this should be called from init rather than from hw_params.
 */
static int es8336_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				 int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = codec_dai->component;
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);

	switch (freq) {
	case 11289600:
	case 18432000:
	case 22579200:
	case 36864000:
		es8336->sysclk_constraints = &constraints_112896;
		es8336->sysclk = freq;
		return 0;
	case 12288000:
	case 19200000:
	case 16934400:
	case 24576000:
	case 33868800:
		es8336->sysclk_constraints = &constraints_12288;
		es8336->sysclk = freq;
		return 0;
	case 12000000:
	case 24000000:
		es8336->sysclk_constraints = &constraints_12;
		es8336->sysclk = freq;
		return 0;
	}

	return 0;
}

static int es8336_set_dai_fmt(struct snd_soc_dai *codec_dai,
			      unsigned int fmt)
{
	struct snd_soc_component *component = codec_dai->component;
	u8 iface = 0;
	u8 adciface = 0;
	u8 daciface = 0;

	iface    = snd_soc_component_read(component, ES8336_IFACE);
	adciface = snd_soc_component_read(component, ES8336_ADC_IFACE);
	daciface = snd_soc_component_read(component, ES8336_DAC_IFACE);

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface |= 0x80;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		iface &= 0x7F;
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		adciface &= 0xFC;
		daciface &= 0xFC;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		return -EINVAL;
	case SND_SOC_DAIFMT_LEFT_J:
		adciface &= 0xFC;
		daciface &= 0xFC;
		adciface |= 0x01;
		daciface |= 0x01;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		adciface &= 0xDC;
		daciface &= 0xDC;
		adciface |= 0x03;
		daciface |= 0x03;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		adciface &= 0xDC;
		daciface &= 0xDC;
		adciface |= 0x23;
		daciface |= 0x23;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		iface    &= 0xDF;
		adciface &= 0xDF;
		daciface &= 0xDF;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface    |= 0x20;
		adciface |= 0x20;
		daciface |= 0x20;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface    |= 0x20;
		adciface &= 0xDF;
		daciface &= 0xDF;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface    &= 0xDF;
		adciface |= 0x20;
		daciface |= 0x20;
		break;
	default:
		return -EINVAL;
	}
	snd_soc_component_write(component, ES8336_IFACE, iface);
	snd_soc_component_write(component, ES8336_ADC_IFACE, adciface);
	snd_soc_component_write(component, ES8336_DAC_IFACE, daciface);
	return 0;
}

static int es8336_pcm_startup(struct snd_pcm_substream *substream,
			      struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);
	bool playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);

	snd_soc_component_write(component, ES8336_RESET_REG00, 0xC0);
	snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x00);
	/* es8336: both playback and capture need dac mclk */
	snd_soc_component_update_bits(component, ES8336_CLKMGR_CLKSW_REG01,
			    ES8336_CLKMGR_MCLK_DIV_MASK |
			    ES8336_CLKMGR_DAC_MCLK_MASK,
			    ES8336_CLKMGR_MCLK_DIV_NML |
			    ES8336_CLKMGR_DAC_MCLK_EN);
	es8336->pwr_count++;

	if (playback) {
		snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0x3F);
		snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0x1F);
		snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x88);
		snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x00);
		snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0xBB);
		snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x10);
		snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x30);
		snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x02);
		snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x00);
		snd_soc_component_write(component, ES8336_CPHP_OUTEN_REG17, 0x66);
		snd_soc_component_update_bits(component, ES8336_CLKMGR_CLKSW_REG01,
				    ES8336_CLKMGR_DAC_MCLK_MASK |
				    ES8336_CLKMGR_DAC_ANALOG_MASK,
				    ES8336_CLKMGR_DAC_MCLK_EN |
				    ES8336_CLKMGR_DAC_ANALOG_EN);
		msleep(50);
	} else {
		snd_soc_component_update_bits(component, ES8336_ADC_PDN_LINSEL_REG22, 0xC0, 0x00);
		snd_soc_component_update_bits(component, ES8336_CLKMGR_CLKSW_REG01,
				    ES8336_CLKMGR_ADC_MCLK_MASK |
				    ES8336_CLKMGR_ADC_ANALOG_MASK,
				    ES8336_CLKMGR_ADC_MCLK_EN |
				    ES8336_CLKMGR_ADC_ANALOG_EN);
	}

	return 0;
}

static void es8336_pcm_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);
	bool playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);

	if (playback) {
		snd_soc_component_write(component, ES8336_CPHP_OUTEN_REG17, 0x00);
		snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x11);
		snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x03);
		snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x22);
		snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x06);
		snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x00);
		snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x33);
		snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0x00);
		snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x00);
		snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0xFF);
		snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0xFF);
		snd_soc_component_update_bits(component, ES8336_CLKMGR_CLKSW_REG01,
				    ES8336_CLKMGR_DAC_ANALOG_MASK,
				    ES8336_CLKMGR_DAC_ANALOG_DIS);
	} else {
		snd_soc_component_update_bits(component, ES8336_ADC_PDN_LINSEL_REG22, 0xC0, 0xc0);
		snd_soc_component_update_bits(component, ES8336_CLKMGR_CLKSW_REG01,
				    ES8336_CLKMGR_ADC_MCLK_MASK |
				    ES8336_CLKMGR_ADC_ANALOG_MASK,
				    ES8336_CLKMGR_ADC_MCLK_DIS |
				    ES8336_CLKMGR_ADC_ANALOG_DIS);
	}

	if (--es8336->pwr_count == 0) {
		if (!es8336->hp_inserted)
			snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x3F);
		snd_soc_component_write(component, ES8336_CLKMGR_CLKSW_REG01, 0xF3);
	}
}


static int es8336_pcm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	int val = 0;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		val = ES8336_DACWL_16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		val = ES8336_DACWL_20;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		val = ES8336_DACWL_24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		val = ES8336_DACWL_32;
		break;
	default:
		val = ES8336_DACWL_16;
		break;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		snd_soc_component_update_bits(component, ES8336_SDP_DACFMT_REG0B,
				    ES8336_DACWL_MASK, val);
	else
		snd_soc_component_update_bits(component, ES8336_SDP_ADCFMT_REG0A,
				    ES8336_ADCWL_MASK, val);

	return 0;
}

static int es8336_mute(struct snd_soc_dai *dai, int mute, int direction)
{
	struct snd_soc_component *component = dai->component;
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);

	es8336->muted = mute;

	if (mute) {
		es8336_enable_spk(es8336, false);
		msleep(100);
		snd_soc_component_write(component, ES8336_DAC_SET1_REG30, 0x20);
	}

	snd_soc_component_write(component, ES8336_DAC_SET1_REG30, 0x00);
	msleep(130);

	if (!es8336->hp_inserted)
		es8336_enable_spk(es8336, true);

	return 0;
}

static int es8336_set_bias_level(struct snd_soc_component *component,
				 enum snd_soc_bias_level level)
{
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);

	switch (level) {
	case SND_SOC_BIAS_ON:
		break;

	case SND_SOC_BIAS_PREPARE:
		break;

	case SND_SOC_BIAS_STANDBY:
		break;

	case SND_SOC_BIAS_OFF:
		snd_soc_component_write(component, ES8336_CPHP_OUTEN_REG17, 0x00);
		snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x11);
		snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x03);
		snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x22);
		snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x06);
		snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x00);
		snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x33);
		snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0x00);
		snd_soc_component_update_bits(component, ES8336_ADC_PDN_LINSEL_REG22, 0xC0, 0xC0);
		if (!es8336->hp_inserted)
			snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x3F);
		snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0x3F);
		snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0x1F);
		snd_soc_component_write(component, ES8336_RESET_REG00, 0x00);
		break;
	}

	return 0;
}

#define es8336_RATES SNDRV_PCM_RATE_8000_96000

#define es8336_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
	SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops es8336_ops = {
	.startup = es8336_pcm_startup,
	.hw_params = es8336_pcm_hw_params,
	.set_fmt = es8336_set_dai_fmt,
	.set_sysclk = es8336_set_dai_sysclk,
	.mute_stream = es8336_mute,
	.shutdown = es8336_pcm_shutdown,
};

static struct snd_soc_dai_driver es8336_dai = {
	.name = "es8336-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = es8336_RATES,
		.formats = es8336_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = es8336_RATES,
		.formats = es8336_FORMATS,
	},
	.ops = &es8336_ops,
	.symmetric_rates = 1,
};

static int es8336_init_regs(struct snd_soc_component *component)
{
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);
	snd_soc_component_write(component, ES8336_RESET_REG00, 0x3f);
	usleep_range(5000, 5500);
	snd_soc_component_write(component, ES8336_RESET_REG00, 0x00);
	snd_soc_component_write(component, ES8336_SYS_VMIDSEL_REG0C, 0xFF);
	msleep(30);
	snd_soc_component_write(component, ES8336_CLKMGR_CLKSEL_REG02, 0x08);
	snd_soc_component_write(component, ES8336_CLKMGR_ADCOSR_REG03, 0x20);
	snd_soc_component_write(component, ES8336_CLKMGR_ADCDIV1_REG04, 0x11);
	snd_soc_component_write(component, ES8336_CLKMGR_ADCDIV2_REG05, 0x00);
	snd_soc_component_write(component, ES8336_CLKMGR_DACDIV1_REG06, 0x11);
	snd_soc_component_write(component, ES8336_CLKMGR_DACDIV2_REG07, 0x00);
	snd_soc_component_write(component, ES8336_CLKMGR_CPDIV_REG08, 0x00);
	snd_soc_component_write(component, ES8336_SDP_MS_BCKDIV_REG09, 0x04);
	snd_soc_component_write(component, ES8336_CLKMGR_CLKSW_REG01, 0x7F);
	snd_soc_component_write(component, ES8336_CAL_TYPE_REG1C, 0x0F);
	snd_soc_component_write(component, ES8336_CAL_HPLIV_REG1E, 0x90);
	snd_soc_component_write(component, ES8336_CAL_HPRIV_REG1F, 0x90);
	snd_soc_component_write(component, ES8336_ADC_VOLUME_REG27, 0x00);
	snd_soc_component_write(component, ES8336_ADC_PDN_LINSEL_REG22, es8336->mic_src);
	snd_soc_component_write(component, ES8336_ADC_D2SEPGA_REG24, 0x00);
	snd_soc_component_write(component, ES8336_ADC_DMIC_REG25, 0x08);
	snd_soc_component_write(component, ES8336_DAC_SET2_REG31, 0x20);
	snd_soc_component_write(component, ES8336_DAC_SET3_REG32, 0x00);
	snd_soc_component_write(component, ES8336_DAC_VOLL_REG33, 0x00);
	snd_soc_component_write(component, ES8336_DAC_VOLR_REG34, 0x00);
	snd_soc_component_write(component, ES8336_SDP_ADCFMT_REG0A, 0x00);
	snd_soc_component_write(component, ES8336_SDP_DACFMT_REG0B, 0x00);
	snd_soc_component_write(component, ES8336_SYS_VMIDLOW_REG10, 0x11);
	snd_soc_component_write(component, ES8336_SYS_VSEL_REG11, 0xFC);
	snd_soc_component_write(component, ES8336_SYS_REF_REG12, 0x28);
	snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0x04);
	snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0x0C);
	snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x11);
	snd_soc_component_write(component, ES8336_HPMIX_SEL_REG13, 0x00);
	snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x88);
	snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x00);
	snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0xBB);
	snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x10);
	snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x30);
	snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x02);
	snd_soc_component_write(component, ES8336_CPHP_ICAL_VOL_REG18, 0x00);
	snd_soc_component_write(component, ES8336_GPIO_SEL_REG4D, 0x02);
	snd_soc_component_write(component, ES8336_GPIO_DEBUNCE_INT_REG4E, 0x02);
	snd_soc_component_write(component, ES8336_TESTMODE_REG50, 0xA0);
	snd_soc_component_write(component, ES8336_TEST1_REG51, 0x00);
	snd_soc_component_write(component, ES8336_TEST2_REG52, 0x00);
	snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x00);
	snd_soc_component_write(component, ES8336_RESET_REG00, 0xC0);
	msleep(50);
	snd_soc_component_write(component, ES8336_ADC_PGAGAIN_REG23, 0x60);
	snd_soc_component_write(component, ES8336_ADC_D2SEPGA_REG24, 0x01);
	/* adc ds mode, HPF enable */
	snd_soc_component_write(component, ES8336_ADC_DMIC_REG25, 0x08);
	snd_soc_component_write(component, ES8336_ADC_ALC1_REG29, 0xcd);
	snd_soc_component_write(component, ES8336_ADC_ALC2_REG2A, 0x08);
	snd_soc_component_write(component, ES8336_ADC_ALC3_REG2B, 0xa0);
	snd_soc_component_write(component, ES8336_ADC_ALC4_REG2C, 0x05);
	snd_soc_component_write(component, ES8336_ADC_ALC5_REG2D, 0x06);
	snd_soc_component_write(component, ES8336_ADC_ALC6_REG2E, 0x61);
	return 0;
}

static int es8336_suspend(struct snd_soc_component *component)
{
	return 0;
}

static int es8336_resume(struct snd_soc_component *component)
{
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);
	int ret;

	es8336_reset(component); /* UPDATED BY DAVID,15-3-5 */
	ret = snd_soc_component_read(component, ES8336_CLKMGR_ADCDIV2_REG05);
	if (!ret) {
		es8336_init_regs(component);
		snd_soc_component_write(component, ES8336_GPIO_SEL_REG4D, 0x02);
		/* max debance time, enable interrupt, low active */
		snd_soc_component_write(component, ES8336_GPIO_DEBUNCE_INT_REG4E, 0xf3);
		/* es8336_set_bias_level(component, SND_SOC_BIAS_OFF); */
		snd_soc_component_write(component, ES8336_CPHP_OUTEN_REG17, 0x00);
		snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x11);
		snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x03);
		snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x22);
		snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x06);
		snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x00);
		snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x33);
		snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0x00);
		if (!es8336->hp_inserted)
			snd_soc_component_write(component, ES8336_SYS_PDN_REG0D, 0x3F);
		snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0xFF);
		snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0xFF);
		snd_soc_component_write(component, ES8336_CLKMGR_CLKSW_REG01, 0xF3);
		snd_soc_component_update_bits(component, ES8336_ADC_PDN_LINSEL_REG22, 0xC0, 0xC0);
	}
	return 0;
}

static irqreturn_t es8336_irq_handler(int irq, void *data)
{
	struct es8336_priv *es8336 = data;

	queue_delayed_work(system_power_efficient_wq, &es8336->work,
			   msecs_to_jiffies(es8336->debounce_time));

	return IRQ_HANDLED;
}

static void hp_work(struct work_struct *work)
{
	struct es8336_priv *es8336;

	es8336 = container_of(work, struct es8336_priv, work.work);

	es8336->hp_inserted = gpiod_get_value(es8336->hp_det_gpio);
	if (!es8336->muted) {
		if (es8336->hp_inserted)
			es8336_enable_spk(es8336, false);
		else
			es8336_enable_spk(es8336, true);
	}
}

static int es8336_probe(struct snd_soc_component *component)
{
	struct es8336_priv *es8336 = snd_soc_component_get_drvdata(component);
	int ret = 0;

	es8336_component = component;
	ret = snd_soc_component_read(component, ES8336_CLKMGR_ADCDIV2_REG05);
	if (!ret) {
		es8336_reset(component); /* UPDATED BY DAVID,15-3-5 */
		ret = snd_soc_component_read(component, ES8336_CLKMGR_ADCDIV2_REG05);
		if (!ret) {
			es8336_init_regs(component);
			snd_soc_component_write(component, ES8336_GPIO_SEL_REG4D, 0x02);
			/* max debance time, enable interrupt, low active */
			snd_soc_component_write(component,
				      ES8336_GPIO_DEBUNCE_INT_REG4E, 0xf3);

			/* es8336_set_bias_level(codec, SND_SOC_BIAS_OFF); */
			snd_soc_component_write(component, ES8336_CPHP_OUTEN_REG17, 0x00);
			snd_soc_component_write(component, ES8336_DAC_PDN_REG2F, 0x11);
			snd_soc_component_write(component, ES8336_CPHP_LDOCTL_REG1B, 0x03);
			snd_soc_component_write(component, ES8336_CPHP_PDN2_REG1A, 0x22);
			snd_soc_component_write(component, ES8336_CPHP_PDN1_REG19, 0x06);
			snd_soc_component_write(component, ES8336_HPMIX_SWITCH_REG14, 0x00);
			snd_soc_component_write(component, ES8336_HPMIX_PDN_REG15, 0x33);
			snd_soc_component_write(component, ES8336_HPMIX_VOL_REG16, 0x00);
			if (!es8336->hp_inserted)
				snd_soc_component_write(component, ES8336_SYS_PDN_REG0D,
					      0x3F);
			snd_soc_component_write(component, ES8336_SYS_LP1_REG0E, 0xFF);
			snd_soc_component_write(component, ES8336_SYS_LP2_REG0F, 0xFF);
			snd_soc_component_write(component, ES8336_CLKMGR_CLKSW_REG01, 0xF3);
			snd_soc_component_update_bits(component, ES8336_ADC_PDN_LINSEL_REG22, 0xC0, 0xC0);
		}
	}

	return ret;
}

static void es8336_remove(struct snd_soc_component *component)
{
	es8336_set_bias_level(component, SND_SOC_BIAS_OFF);
}

const struct regmap_config es8336_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
	.max_register	= ES8336_TEST3_REG53,
	.cache_type	= REGCACHE_RBTREE,
	.reg_defaults = es8336_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(es8336_reg_defaults),
};

static const struct snd_soc_component_driver soc_component_dev_es8336 = {
	.probe =	es8336_probe,
	.remove =	es8336_remove,
	.suspend =	es8336_suspend,
	.resume =	es8336_resume,
	.set_bias_level = es8336_set_bias_level,

	.controls = es8336_snd_controls,
	.num_controls = ARRAY_SIZE(es8336_snd_controls),
	.dapm_widgets = es8336_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(es8336_dapm_widgets),
	.dapm_routes = es8336_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(es8336_dapm_routes),
};

static int es8336_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct es8336_priv *es8336;
	int ret = -1;
	int hp_irq;

	es8336 = devm_kzalloc(&i2c->dev, sizeof(*es8336), GFP_KERNEL);
	if (!es8336)
		return -ENOMEM;

	es8336->debounce_time = 200;
	es8336->pwr_count = 0;
	es8336->hp_inserted = false;
	es8336->muted = true;

	es8336->regmap = devm_regmap_init_i2c(i2c, &es8336_regmap_config);
	if (IS_ERR(es8336->regmap)) {
		ret = PTR_ERR(es8336->regmap);
		dev_err(&i2c->dev, "Failed to init regmap: %d\n", ret);
		return ret;
	}

	i2c_set_clientdata(i2c, es8336);

	es8336->spk_ctl_gpio = devm_gpiod_get_index_optional(&i2c->dev, "sel", 0,
							GPIOD_OUT_HIGH);
	ret = of_property_read_u8(i2c->dev.of_node, "mic-src", &es8336->mic_src);
	if (ret != 0) {
		dev_dbg(&i2c->dev, "mic1-src return %d", ret);
		es8336->mic_src = 0x20;
	}
	dev_dbg(&i2c->dev, "mic1-src %x", es8336->mic_src);

	if (IS_ERR_OR_NULL(es8336->spk_ctl_gpio))
		dev_info(&i2c->dev, "Can not get spk_ctl_gpio\n");
	else
		es8336_enable_spk(es8336, false);

	es8336->hp_det_gpio = devm_gpiod_get_index_optional(&i2c->dev, "det", 0,
							GPIOD_IN);

	if (IS_ERR_OR_NULL(es8336->hp_det_gpio)) {
		dev_info(&i2c->dev, "Can not get hp_det_gpio\n");
	} else {
		INIT_DELAYED_WORK(&es8336->work, hp_work);
		hp_irq = gpiod_to_irq(es8336->hp_det_gpio);
		ret = devm_request_threaded_irq(&i2c->dev, hp_irq, NULL,
						es8336_irq_handler,
						IRQF_TRIGGER_FALLING |
						IRQF_TRIGGER_RISING |
						IRQF_ONESHOT,
						"es8336_interrupt", es8336);
		if (ret < 0) {
			dev_err(&i2c->dev, "request_irq failed: %d\n", ret);
			return ret;
		}

		schedule_delayed_work(&es8336->work,
				      msecs_to_jiffies(es8336->debounce_time));
	}

	ret = snd_soc_register_component(&i2c->dev,
				     &soc_component_dev_es8336,
				     &es8336_dai, 1);

	return ret;
}

static  int es8336_i2c_remove(struct i2c_client *client)
{
	kfree(i2c_get_clientdata(client));
	return 0;
}

static void es8336_i2c_shutdown(struct i2c_client *client)
{
	struct es8336_priv *es8336 = i2c_get_clientdata(client);

	if (es8336_component != NULL) {
		es8336_enable_spk(es8336, false);
		msleep(20);
		es8336_set_bias_level(es8336_component, SND_SOC_BIAS_OFF);
	}
}

static const struct i2c_device_id es8336_i2c_id[] = {
	{"es8336", 0},
	{"10ES8336:00", 0},
	{"10ES8336", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, es8336_i2c_id);

static const struct of_device_id es8336_of_match[] = {
	{ .compatible = "everest,es8336", },
	{ }
};
MODULE_DEVICE_TABLE(of, es8336_of_match);

static const struct acpi_device_id es8336_acpi_match[] = {
	{ "ESSX8336", 0},
	{ }
};
MODULE_DEVICE_TABLE(acpi, es8336_acpi_match);

static struct i2c_driver es8336_i2c_driver = {
	.driver = {
		.name		= "es8336",
		.of_match_table = es8336_of_match,
		.acpi_match_table = es8336_acpi_match,
	},
	.probe    = es8336_i2c_probe,
	.remove	= es8336_i2c_remove,
	.shutdown = es8336_i2c_shutdown,
	.id_table = es8336_i2c_id,
};

module_i2c_driver(es8336_i2c_driver);
MODULE_DESCRIPTION("ASoC es8336 driver");
MODULE_LICENSE("GPL");
