/*
 * (C) Copyright 2010
 * Michael Kurz <michi.kurz@googlemail.com>.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <net.h>
#include <netdev.h>
#include <miiphy.h>
#include MII_GPIOINCLUDE

#include "rtl8366.h"

#ifdef DEBUG_RTL8366
	#define DBG(fmt,args...)	printf (fmt ,##args)
#else
	#define DBG(fmt,args...)
#endif


//-------------------------------------------------------------------
// Soft SMI functions
//-------------------------------------------------------------------

#define DELAY 2

static void smi_init(void)
{
    MII_SDAINPUT;
    MII_SCKINPUT;

	MII_SETSDA(1);
	MII_SETSCK(1);

    udelay(20);
}

static void smi_start(void)
{
/*
 * rtl8366 chip needs a extra clock with
 * SDA high before start condition
 */

    /* set gpio pins output */
    MII_SDAOUTPUT;
    MII_SCKOUTPUT;
    udelay(DELAY);

    /* set initial state: SCK:0, SDA:1 */
    MII_SETSCK(0);
    MII_SETSDA(1);
    udelay(DELAY);

    /* toggle clock */
    MII_SETSCK(1);
    udelay(DELAY);
    MII_SETSCK(0);
    udelay(DELAY);

    /* start condition */
    MII_SETSCK(1);
    udelay(DELAY);
    MII_SETSDA(0);
    udelay(DELAY);
    MII_SETSCK(0);
    udelay(DELAY);
    MII_SETSDA(1);
}

static void smi_stop(void)
{
/*
 * rtl8366 chip needs a extra clock with
 * SDA high after stop condition
 */

    /* stop condition */
	udelay(DELAY);
    MII_SETSDA(0);
    MII_SETSCK(1);
    udelay(DELAY);
    MII_SETSDA(1);
    udelay(DELAY);
    MII_SETSCK(1);
    udelay(DELAY);
    MII_SETSCK(0);
    udelay(DELAY);

    /* toggle clock */
    MII_SETSCK(1);
    udelay(DELAY);
    MII_SETSCK(0);
    udelay(DELAY);
    MII_SETSCK(1);

    /* set gpio pins input */
    MII_SDAINPUT;
    MII_SCKINPUT;
}

static void smi_writeBits(uint32_t data, uint8_t length)
{
    uint8_t test;

    for( ; length > 0; length--) {
        udelay(DELAY);

        /* output data */
        test = (((data & (1 << (length - 1))) != 0) ? 1 : 0);
        MII_SETSDA(test);
        udelay(DELAY);

        /* toogle clock */
        MII_SETSCK(1);
        udelay(DELAY);
        MII_SETSCK(0);
    }
}

static uint32_t smi_readBits(uint8_t length)
{
    uint32_t ret;

    MII_SDAINPUT;

    for(ret = 0 ; length > 0; length--) {
        udelay(DELAY);

        ret <<= 1;

        /* toogle clock */
        MII_SETSCK(1);
        udelay(DELAY);
        ret |= MII_GETSDA;
        MII_SETSCK(0);
    }

    MII_SDAOUTPUT;

    return ret;
}

static int smi_waitAck(void)
{
    uint32_t retry = 0;

	while (smi_readBits(1)) {
		if (retry++ == 5)
			return -1;
	}

	return 0;

}

static int smi_read(uint32_t reg, uint32_t *data)
{
    uint32_t rawData;

    /* send start condition */
    smi_start();
    /* send CTRL1 code: 0b1010*/
    smi_writeBits(0x0a, 4);
    /* send CTRL2 code: 0b100 */
    smi_writeBits(0x04, 3);
    /* send READ command */
    smi_writeBits(0x01, 1);

    /* wait for ACK */
    if (smi_waitAck())
        return -1;

    /* send address low */
    smi_writeBits(reg & 0xFF, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;
    /* send address high */
    smi_writeBits((reg & 0xFF00) >> 8, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;

    /* read data low */
    rawData = (smi_readBits(8) & 0xFF);
    /* send ACK */
    smi_writeBits(0, 1);
    /* read data high */
    rawData |= (smi_readBits(8) & 0xFF) << 8;
    /* send NACK */
    smi_writeBits(1, 1);

    /* send stop condition */
    smi_stop();

    if (data)
        *data = rawData;

    return 0;
}

static int smi_write(uint32_t reg, uint32_t data)
{
    /* send start condition */
    smi_start();
    /* send CTRL1 code: 0b1010*/
    smi_writeBits(0x0a, 4);
    /* send CTRL2 code: 0b100 */
    smi_writeBits(0x04, 3);
    /* send WRITE command */
    smi_writeBits(0x00, 1);

    /* wait for ACK */
    if (smi_waitAck())
        return -1;

    /* send address low */
    smi_writeBits(reg & 0xFF, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;
    /* send address high */
    smi_writeBits((reg & 0xFF00) >> 8, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;

    /* send data low */
    smi_writeBits(data & 0xFF, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;
    /* send data high */
    smi_writeBits((data & 0xFF00) >> 8, 8);
    /* wait for ACK */
    if (smi_waitAck())
        return -1;

    /* send stop condition */
    smi_stop();

    return 0;
}


//-------------------------------------------------------------------
// Switch register read / write functions
//-------------------------------------------------------------------
static int rtl8366_readRegister(uint32_t reg, uint16_t *data)
{
    uint32_t regData;

    DBG("rtl8366: read register=%#04x, data=", reg);

    if (smi_read(reg, &regData)) {
        printf("\nrtl8366 smi read failed!\n");
        return -1;
    }

    if (data)
        *data = regData;

    DBG("%#04x\n", regData);

    return 0;
}

static int rtl8366_writeRegister(uint32_t reg, uint16_t data)
{
    DBG("rtl8366: write register=%#04x, data=%#04x\n", reg, data);

    if (smi_write(reg, data)) {
        printf("rtl8366 smi write failed!\n");
        return -1;
    }

    return 0;
}

static int rtl8366_setRegisterBit(uint32_t reg, uint32_t bitNum, uint32_t value)
{
    uint16_t regData;

    if (bitNum >= 16)
        return -1;

    if (rtl8366_readRegister(reg, &regData))
        return -1;

    if (value)
        regData |= (1 << bitNum);
    else
        regData &= ~(1 << bitNum);

    if (rtl8366_writeRegister(reg, regData))
        return -1;

    return 0;
}

//-------------------------------------------------------------------
// MII PHY read / write functions
//-------------------------------------------------------------------
static int rtl8366_getPhyReg(uint32_t phyNum, uint32_t reg, uint16_t *data)
{
    uint16_t phyAddr, regData;

    if (phyNum > RTL8366S_PHY_NO_MAX) {
		printf("rtl8366s: invalid phy number!\n");
		return -1;
	}

    if (phyNum > RTL8366S_PHY_ADDR_MAX) {
		printf("rtl8366s: invalid phy register number!\n");
		return -1;
	}

	if (rtl8366_writeRegister(RTL8366S_PHY_ACCESS_CTRL_REG,
                           RTL8366S_PHY_CTRL_READ))
        return -1;

    phyAddr = 0x8000 | (1 << (phyNum + RTL8366S_PHY_NO_OFFSET))
                     | (reg & RTL8366S_PHY_REG_MASK);
    if (rtl8366_writeRegister(phyAddr, 0))
        return -1;

    if (rtl8366_readRegister(RTL8366S_PHY_ACCESS_DATA_REG, &regData))
        return -1;

    if (data)
        *data = regData;

    return 0;
}

static int rtl8366_setPhyReg(uint32_t phyNum, uint32_t reg, uint16_t data)
{
    uint16_t phyAddr;

    if (phyNum > RTL8366S_PHY_NO_MAX) {
		printf("rtl8366s: invalid phy number!\n");
		return -1;
	}

    if (phyNum > RTL8366S_PHY_ADDR_MAX) {
		printf("rtl8366s: invalid phy register number!\n");
		return -1;
	}

	if (rtl8366_writeRegister(RTL8366S_PHY_ACCESS_CTRL_REG,
                           RTL8366S_PHY_CTRL_WRITE))
        return -1;

    phyAddr = 0x8000 | (1 << (phyNum + RTL8366S_PHY_NO_OFFSET))
                     | (reg & RTL8366S_PHY_REG_MASK);
    if (rtl8366_writeRegister(phyAddr, data))
        return -1;

    return 0;
}

static int rtl8366_miiread(char *devname, uchar phy_adr, uchar reg, ushort *data)
{
    uint16_t regData;

    DBG("rtl8366_miiread: devname=%s, addr=%#02x, reg=%#02x\n",
          devname, phy_adr, reg);

    if (strcmp(devname, RTL8366_DEVNAME) != 0)
        return -1;

    if (rtl8366_getPhyReg(phy_adr, reg, &regData)) {
        printf("rtl8366_miiread: write failed!\n");
        return -1;
    }

    if (data)
        *data = regData;

    return 0;
}

static int rtl8366_miiwrite(char *devname, uchar phy_adr, uchar reg, ushort data)
{
    DBG("rtl8366_miiwrite: devname=%s, addr=%#02x, reg=%#02x, data=%#04x\n",
          devname, phy_adr, reg, data);

    if (strcmp(devname, RTL8366_DEVNAME) != 0)
        return -1;

    if (rtl8366_setPhyReg(phy_adr, reg, data)) {
        printf("rtl8366_miiwrite: write failed!\n");
        return -1;
    }

    return 0;
}

int rtl8366_mii_register(bd_t *bis)
{
    miiphy_register(strdup(RTL8366_DEVNAME), rtl8366_miiread,
			rtl8366_miiwrite);

    return 0;
}


//-------------------------------------------------------------------
// Switch management functions
//-------------------------------------------------------------------

int rtl8366s_setGreenFeature(uint32_t tx, uint32_t rx)
{
    if (rtl8366_setRegisterBit(RTL8366S_GREEN_FEATURE_REG,
                               RTL8366S_GREEN_FEATURE_TX_BIT, tx))
        return -1;

    if (rtl8366_setRegisterBit(RTL8366S_GREEN_FEATURE_REG,
                               RTL8366S_GREEN_FEATURE_RX_BIT, rx))
        return -1;

    return 0;
}

int rtl8366s_setPowerSaving(uint32_t phyNum, uint32_t enabled)
{
    uint16_t regData;

    if (phyNum > RTL8366S_PHY_NO_MAX)
        return -1;

    if (rtl8366_getPhyReg(phyNum, 12, &regData))
        return -1;

    if (enabled)
        regData |= (1 << 12);
    else
        regData &= ~(1 << 12);

    if (rtl8366_setPhyReg(phyNum, 12, regData))
        return -1;

    return 0;
}

int rtl8366s_setGreenEthernet(uint32_t greenFeature, uint32_t powerSaving)
{
    uint32_t phyNum, i;
    uint16_t regData;

	const uint16_t greenSettings[][2] =
	{
		{0xBE5B,0x3500},
		{0xBE5C,0xB975},
		{0xBE5D,0xB9B9},
		{0xBE77,0xA500},
		{0xBE78,0x5A78},
		{0xBE79,0x6478}
	};

    if (rtl8366_readRegister(RTL8366S_MODEL_ID_REG, &regData))
        return -1;

	switch (regData)
	{
		case 0x0000:
			for (i = 0; i < 6; i++) {
				if (rtl8366_writeRegister(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
					return -1;
				if (rtl8366_writeRegister(greenSettings[i][0], greenSettings[i][1]))
					return -1;
			}
			break;

		case RTL8366S_MODEL_8366SR:
			if (rtl8366_writeRegister(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
				return -1;
			if (rtl8366_writeRegister(greenSettings[0][0], greenSettings[0][1]))
				return -1;
			break;

		default:
			printf("rtl8366s_initChip: unsupported chip found!\n");
			return -1;
	}

    if (rtl8366s_setGreenFeature(greenFeature, powerSaving))
        return -1;

    for (phyNum = 0; phyNum <= RTL8366S_PHY_NO_MAX; phyNum++) {
        if (rtl8366s_setPowerSaving(phyNum, powerSaving))
            return -1;
    }

    return 0;
}

int rtl8366s_setCPUPortMask(uint8_t port, uint32_t enabled)
{
	if(port >= 6){
		printf("rtl8366s_setCPUPortMask: invalid port number\n");
		return -1;
	}

	return rtl8366_setRegisterBit(RTL8366S_CPU_CTRL_REG, port, enabled);
}

int rtl8366s_setCPUDisableInsTag(uint32_t enable)
{
	return rtl8366_setRegisterBit(RTL8366S_CPU_CTRL_REG,
		RTL8366S_CPU_INSTAG_BIT, enable);
}

int rtl8366s_setCPUDropUnda(uint32_t enable)
{
	return rtl8366_setRegisterBit(RTL8366S_CPU_CTRL_REG,
		RTL8366S_CPU_DRP_BIT, enable);
}

int rtl8366s_setCPUPort(uint8_t port, uint32_t noTag, uint32_t dropUnda)
{
	uint32_t i;

	if(port >= 6){
		printf("rtl8366s_setCPUPort: invalid port number\n");
		return -1;
	}

	/* reset register */
	for(i = 0; i < 6; i++)
	{
		if(rtl8366s_setCPUPortMask(i, 0)){
			printf("rtl8366s_setCPUPort: rtl8366s_setCPUPortMask failed\n");
			return -1;
		}
	}

	if(rtl8366s_setCPUPortMask(port, 1)){
		printf("rtl8366s_setCPUPort: rtl8366s_setCPUPortMask failed\n");
		return -1;
	}

	if(rtl8366s_setCPUDisableInsTag(noTag)){
		printf("rtl8366s_setCPUPort: rtl8366s_setCPUDisableInsTag fail\n");
		return -1;
	}

	if(rtl8366s_setCPUDropUnda(dropUnda)){
		printf("rtl8366s_setCPUPort: rtl8366s_setCPUDropUnda fail\n");
		return -1;
	}

	return 0;
}

int rtl8366s_setLedConfig(uint32_t ledNum, uint8_t config)
{
    uint16_t regData;

	if(ledNum >= RTL8366S_LED_GROUP_MAX) {
		DBG("rtl8366s_setLedConfig: invalid led group\n");
		return -1;
	}

    if(config > RTL8366S_LEDCONF_LEDFORCE) {
		DBG("rtl8366s_setLedConfig: invalid led config\n");
		return -1;
	}

	if (rtl8366_readRegister(RTL8366S_LED_INDICATED_CONF_REG, &regData)) {
        printf("rtl8366s_setLedConfig: failed to get led register!\n");
        return -1;
	}

	regData &= ~(0xF << (ledNum * 4));
	regData |= config << (ledNum * 4);

	if (rtl8366_writeRegister(RTL8366S_LED_INDICATED_CONF_REG, regData)) {
        printf("rtl8366s_setLedConfig: failed to set led register!\n");
        return -1;
	}

	return 0;
}

int rtl8366s_getLedConfig(uint32_t ledNum, uint8_t *config)
{
    uint16_t regData;

	if(ledNum >= RTL8366S_LED_GROUP_MAX) {
		DBG("rtl8366s_getLedConfig: invalid led group\n");
		return -1;
	}

    if (rtl8366_readRegister(RTL8366S_LED_INDICATED_CONF_REG, &regData)) {
        printf("rtl8366s_getLedConfig: failed to get led register!\n");
        return -1;
	}

	if (config)
        *config = (regData >> (ledNum * 4)) & 0xF;

    return 0;
}

int rtl8366s_setLedForceValue(uint32_t group0, uint32_t group1,
                              uint32_t group2, uint32_t group3)
{
    uint16_t regData;

    regData = (group0 & 0x3F) | ((group1 & 0x3F) << 6);
	if (rtl8366_writeRegister(RTL8366S_LED_0_1_FORCE_REG, regData)) {
        printf("rtl8366s_setLedForceValue: failed to set led register!\n");
        return -1;
	}

    regData = (group2 & 0x3F) | ((group3 & 0x3F) << 6);
	if (rtl8366_writeRegister(RTL8366S_LED_2_3_FORCE_REG, regData)) {
        printf("rtl8366s_setLedForceValue: failed to set led register!\n");
        return -1;
	}

	return 0;
}

int rtl8366s_initChip(void)
{
    uint32_t ledGroup, i = 0;
    uint16_t regData;
    uint8_t ledData[RTL8366S_LED_GROUP_MAX];
	const uint16_t (*chipData)[2];

	const uint16_t chipB[][2] =
	{
		{0x0000,	0x0038},{0x8100,	0x1B37},{0xBE2E,	0x7B9F},{0xBE2B,	0xA4C8},
		{0xBE74,	0xAD14},{0xBE2C,	0xDC00},{0xBE69,	0xD20F},{0xBE3B,	0xB414},
		{0xBE24,	0x0000},{0xBE23,	0x00A1},{0xBE22,	0x0008},{0xBE21,	0x0120},
		{0xBE20,	0x1000},{0xBE24,	0x0800},{0xBE24,	0x0000},{0xBE24,	0xF000},
		{0xBE23,	0xDF01},{0xBE22,	0xDF20},{0xBE21,	0x101A},{0xBE20,	0xA0FF},
		{0xBE24,	0xF800},{0xBE24,	0xF000},{0x0242,	0x02BF},{0x0245,	0x02BF},
		{0x0248,	0x02BF},{0x024B,	0x02BF},{0x024E,	0x02BF},{0x0251,	0x02BF},
		{0x0230,	0x0A32},{0x0233,	0x0A32},{0x0236,	0x0A32},{0x0239,	0x0A32},
		{0x023C,	0x0A32},{0x023F,	0x0A32},{0x0254,	0x0A3F},{0x0255,	0x0064},
		{0x0256,	0x0A3F},{0x0257,	0x0064},{0x0258,	0x0A3F},{0x0259,	0x0064},
		{0x025A,	0x0A3F},{0x025B,	0x0064},{0x025C,	0x0A3F},{0x025D,	0x0064},
		{0x025E,	0x0A3F},{0x025F,	0x0064},{0x0260,	0x0178},{0x0261,	0x01F4},
		{0x0262,	0x0320},{0x0263,	0x0014},{0x021D,	0x9249},{0x021E,	0x0000},
		{0x0100,	0x0004},{0xBE4A,	0xA0B4},{0xBE40,	0x9C00},{0xBE41,	0x501D},
		{0xBE48,	0x3602},{0xBE47,	0x8051},{0xBE4C,	0x6465},{0x8000,	0x1F00},
		{0x8001,	0x000C},{0x8008,	0x0000},{0x8007,	0x0000},{0x800C,	0x00A5},
		{0x8101,	0x02BC},{0xBE53,	0x0005},{0x8E45,	0xAFE8},{0x8013,	0x0005},
		{0xBE4B,	0x6700},{0x800B,	0x7000},{0xBE09,	0x0E00},
		{0xFFFF, 0xABCD}
	};

    const uint16_t chipDefault[][2] =
    {
        {0x0242, 0x02BF},{0x0245, 0x02BF},{0x0248, 0x02BF},{0x024B, 0x02BF},
		{0x024E, 0x02BF},{0x0251, 0x02BF},
		{0x0254, 0x0A3F},{0x0256, 0x0A3F},{0x0258, 0x0A3F},{0x025A, 0x0A3F},
		{0x025C, 0x0A3F},{0x025E, 0x0A3F},
		{0x0263, 0x007C},{0x0100, 0x0004},
		{0xBE5B, 0x3500},{0x800E, 0x200F},{0xBE1D, 0x0F00},{0x8001, 0x5011},
		{0x800A, 0xA2F4},{0x800B, 0x17A3},{0xBE4B, 0x17A3},{0xBE41, 0x5011},
		{0xBE17, 0x2100},{0x8000, 0x8304},{0xBE40, 0x8304},{0xBE4A, 0xA2F4},
		{0x800C, 0xA8D5},{0x8014, 0x5500},{0x8015, 0x0004},{0xBE4C, 0xA8D5},
		{0xBE59, 0x0008},{0xBE09, 0x0E00},{0xBE36, 0x1036},{0xBE37, 0x1036},
		{0x800D, 0x00FF},{0xBE4D, 0x00FF},
		{0xFFFF, 0xABCD}
    };

	DBG("rtl8366s_initChip\n");

    /* save current led config and set to led force */
    for (ledGroup = 0; ledGroup < RTL8366S_LED_GROUP_MAX; ledGroup++) {
        if (rtl8366s_getLedConfig(ledGroup, &ledData[ledGroup]))
            return -1;

        if (rtl8366s_setLedConfig(ledGroup, RTL8366S_LEDCONF_LEDFORCE))
            return -1;
    }

    if (rtl8366s_setLedForceValue(0,0,0,0))
        return -1;

    if (rtl8366_readRegister(RTL8366S_MODEL_ID_REG, &regData))
        return -1;

	switch (regData)
	{
		case 0x0000:
			chipData = chipB;
			break;

		case RTL8366S_MODEL_8366SR:
			chipData = chipDefault;
			break;

		default:
			printf("rtl8366s_initChip: unsupported chip found!\n");
			return -1;
	}

    DBG("rtl8366s_initChip: found %x chip\n", regData);

    while ((chipData[i][0] != 0xFFFF) && (chipData[i][1] != 0xABCD)) {

        /* phy settings*/
        if ((chipData[i][0] & 0xBE00) == 0xBE00) {
            if (rtl8366_writeRegister(RTL8366S_PHY_ACCESS_CTRL_REG,
                                      RTL8366S_PHY_CTRL_WRITE))
                return -1;
        }

        if (rtl8366_writeRegister(chipData[i][0], chipData[i][1]))
            return -1;

        i++;
    }

    /* chip needs some time */
    udelay(100 * 1000);

    /* restore led config */
    for (ledGroup = 0; ledGroup < RTL8366S_LED_GROUP_MAX; ledGroup++) {
        if (rtl8366s_setLedConfig(ledGroup, ledData[ledGroup]))
            return -1;
    }

    return 0;
}

int rtl8366s_initialize(void)
{
	uint16_t regData;

    DBG("rtl8366s_initialize: start setup\n");

    smi_init();

	rtl8366_readRegister(RTL8366S_CHIP_ID_REG, &regData);
	DBG("Realtek 8366SR switch ID %#04x\n", regData);

	if (regData != 0x8366) {
		printf("rtl8366s_initialize: found unsupported switch\n");
		return -1;
	}

    if (rtl8366s_initChip()) {
        printf("rtl8366s_initialize: init chip failed\n");
        return -1;
    }

	if (rtl8366s_setGreenEthernet(1, 1)) {
       printf("rtl8366s_initialize: set green ethernet failed\n");
       return -1;
   }

   	/* Set port 5 noTag and don't dropUnda */
	if (rtl8366s_setCPUPort(5, 1, 0)) {
		printf("rtl8366s_initialize: set CPU port failed\n");
		return -1;
	}

    return 0;
}
