/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_gpio.c
*/

#include "rt_config.h"

static VOID GPIOMuxSelect(RTMP_ADAPTER *pAd, UINT32 GPIO)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;
	UINT32 Value;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	switch (GPIO) {
	case GPIO0:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO0_SEL_MASK;
		Value |= GPIO0_SEL(GPIO0_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO1:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO1_SEL_MASK;
		Value |= GPIO1_SEL(GPIO1_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO2:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO2_SEL_MASK;
		Value |= GPIO2_SEL(GPIO2_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO3:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO3_SEL_MASK;
		Value |= GPIO3_SEL(GPIO3_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO4:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO4_SEL_MASK;
		Value |= GPIO4_SEL(GPIO4_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO5:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO5_SEL_MASK;
		Value |= GPIO5_SEL(GPIO5_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO6:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO6_SEL_MASK;
		Value |= GPIO6_SEL(GPIO6_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO7:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL1) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL1);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO7_SEL_MASK;
		Value |= GPIO7_SEL(GPIO7_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO8:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO8_SEL_MASK;
		Value |= GPIO8_SEL(GPIO8_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO9:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO9_SEL_MASK;
		Value |= GPIO9_SEL(GPIO9_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO10:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO10_SEL_MASK;
		Value |= GPIO10_SEL(GPIO10_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO11:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO11_SEL_MASK;
		Value |= GPIO11_SEL(GPIO11_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO12:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO12_SEL_MASK;
		Value |= GPIO12_SEL(GPIO12_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO13:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO13_SEL_MASK;
		Value |= GPIO13_SEL(GPIO13_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO14:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO14_SEL_MASK;
		Value |= GPIO14_SEL(GPIO14_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO15:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL2) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL2);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO15_SEL_MASK;
		Value |= GPIO15_SEL(GPIO15_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	case GPIO16:
		RemapBase = GET_REMAP_2_BASE(PINMUX_SEL3) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(PINMUX_SEL3);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~GPIO16_SEL_MASK;
		Value |= GPIO16_SEL(GPIO16_SEL_VALUE);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("GPIO (%d) not support\n", GPIO));
		break;
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
}


INT32 GPIODirectionInput(RTMP_ADAPTER *pAd, UINT32 GPIO)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	GPIOMuxSelect(pAd, GPIO);
	RemapBase = GET_REMAP_2_BASE(GPIO_OE1_RESET) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(GPIO_OE1_RESET);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	return 0;
}


INT32 GPIODirectionOuput(RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	GPIOMuxSelect(pAd, GPIO);
	RemapBase = GET_REMAP_2_BASE(GPIO_OE1_SET) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(GPIO_OE1_SET);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));

	if (Value == OUTPUT_HIGH) {
		RemapBase = GET_REMAP_2_BASE(GPIO_DOUT1_SET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_DOUT1_SET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	} else {
		RemapBase = GET_REMAP_2_BASE(GPIO_DOUT1_RESET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_DOUT1_RESET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	return 0;
}


UINT32 GPIOGetValue(RTMP_ADAPTER *pAd, UINT32 GPIO)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;
	UINT32 Value;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	RemapBase = GET_REMAP_2_BASE(GPIO_DIN1) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(GPIO_DIN1);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);

	if (((Value & (1 << GPIO)) >> GPIO))
		return INPUT_HIGH;
	else
		return INPUT_LOW;
}


VOID GPIOSetValue(RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	if (Value == OUTPUT_HIGH) {
		RemapBase = GET_REMAP_2_BASE(GPIO_DOUT1_SET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_DOUT1_SET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	} else {
		RemapBase = GET_REMAP_2_BASE(GPIO_DOUT1_RESET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_DOUT1_RESET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
}


UINT32 GPIOGetMode(RTMP_ADAPTER *pAd, UINT32 GPIO)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;
	UINT32 Value;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	RemapBase = GET_REMAP_2_BASE(GPIO_OE1) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(GPIO_OE1);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);

	if (((Value & (1 << GPIO)) >> GPIO))
		return GPIO_OUTPUT;
	else
		return GPIO_INPUT;
}


INT32 GPIOPullUp(RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	if (Value == PULL_UP) {
		RemapBase = GET_REMAP_2_BASE(GPIO_PU1_SET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_PU1_SET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	} else {
		RemapBase = GET_REMAP_2_BASE(GPIO_PU1_RESET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_PU1_RESET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	return 0;
}


INT32 GPIOPullDown(RTMP_ADAPTER *pAd, UINT32 GPIO, UINT8 Value)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	if (Value == PULL_DOWN) {
		RemapBase = GET_REMAP_2_BASE(GPIO_PD1_SET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_PD1_SET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	} else {
		RemapBase = GET_REMAP_2_BASE(GPIO_PD1_RESET) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(GPIO_PD1_RESET);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, (1 << GPIO));
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	return 0;
}
