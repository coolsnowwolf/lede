// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver main entrance.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

/* Table of contents*/
/* 1. Include headers*/
/* 2. External compiler flags*/
/* 3. Module defines*/

/* 1. Include headers*/

#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "vcmdswhwregisters.h"

/* NOTE:
 *  Don't use ',' in descriptions, because it is used as separator in csv
 * parsing.
 */

const regVcmdField_s asicVcmdRegisterDesc[] = {
#include "vcmdregistertable.h"
};

/* 2. External compiler flags*/

/* 3. Module defines*/

/* Define this to print debug info for every register write.*/
/* #define DEBUG_PRINT_REGS */

/**
 * Retrieve the content of a hadware register.
 *
 * \note The status register will be read after every MB so it may be needed to
 *     buffer it's content if reading the HW register is slow.
 * \param offset offset of the register in byte
 */
u32 vcmd_read_reg(const void *hwregs, u32 offset)
{
	u32 val;

	val = (u32)ioread32((void *)hwregs + offset);

	PDEBUG("%s 0x%02x --> %08x\n", __func__, offset, val);

	return val;
}

/**
 * Set the content of a hadware register
 */
void vcmd_write_reg(const void *hwregs, u32 offset, u32 val)
{
	iowrite32(val, (void *)hwregs + offset);

	PDEBUG("%s 0x%02x with value %08x\n", __func__, offset, val);
}

/**
 * Write a value into a defined register field (write will happens actually).
 */
void vcmd_write_register_value(const void *hwregs, u32 *reg_mirror,
			       regVcmdName name, u32 value)
{
	const regVcmdField_s *field;
	u32 regVal;

	field = &asicVcmdRegisterDesc[name];

#ifdef DEBUG_PRINT_REGS
	PDEBUG("%s 0x%2x  0x%08x  Value: %10d  %s\n", __func__, field->base,
	       field->mask, value, field->description);
#endif

	/* Check that value fits in field */
	PDEBUG("field->name == name=%d\n", field->name == name);
	PDEBUG("((field->mask >> field->lsb) << field->lsb == field->mask=");
	PDEBUG("%d\n",
	       ((field->mask >> field->lsb) << field->lsb) == field->mask);
	PDEBUG("(field->mask >> field->lsb) >= value=%d\n",
	       (field->mask >> field->lsb) >= value);
	PDEBUG("field->base < ASIC_VCMD_SWREG_AMOUNT*4=%d\n",
	       field->base < ASIC_VCMD_SWREG_AMOUNT * 4);

	/* Clear previous value of field in register */
	regVal = reg_mirror[field->base / 4] & ~(field->mask);

	/* Put new value of field in register */
	reg_mirror[field->base / 4] =
		regVal | ((value << field->lsb) & field->mask);

	/* write it into HW registers */
	vcmd_write_reg(hwregs, field->base, reg_mirror[field->base / 4]);
}

/**
 * Get an unsigned value from the ASIC registers
 */
u32 vcmd_get_register_value(const void *hwregs, u32 *reg_mirror,
			    regVcmdName name)
{
	const regVcmdField_s *field;
	u32 value;

	field = &asicVcmdRegisterDesc[name];

	PDEBUG("field->base < ASIC_VCMD_SWREG_AMOUNT * 4=%d\n",
	       field->base < ASIC_VCMD_SWREG_AMOUNT * 4);

	reg_mirror[field->base / 4] = vcmd_read_reg(hwregs, field->base);
	value = reg_mirror[field->base / 4];
	value = (value & field->mask) >> field->lsb;

	return value;
}
