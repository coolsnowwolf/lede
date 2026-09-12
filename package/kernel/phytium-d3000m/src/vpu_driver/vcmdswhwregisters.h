/* SPDX-License-Identifier: GPL-2.0*/
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

#ifndef VCMD_SWHWREGISTERS_H
#define VCMD_SWHWREGISTERS_H

/* 1. Include headers*/

#include <linux/ioctl.h>

#include <linux/kernel.h>

//#include <linux/module.h>

#undef PDEBUG /* undef it, just in case */
#ifdef REGISTER_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) pr_info("memalloc: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

/* 2. External compiler flags*/

/* 3. Module defines*/

#define ASIC_VCMD_SWREG_AMOUNT                  27
#define VCMD_REGISTER_EXE_CMDBUF_COUNT_OFFSET   0X0C
#define VCMD_REGISTER_CONTROL_OFFSET            0X40
#define VCMD_REGISTER_INT_STATUS_OFFSET         0X44
#define VCMD_REGISTER_INT_CTL_OFFSET            0X48
#define VCMD_REGISTER_EXT_INT_GATE_OFFSET       0X64
#define VCMD_REGISTER_ARBITER_CONFIG_OFFSET     0X70
#define VCMD_REGISTER_ARB_OFFSET                0X74

typedef size_t ptr_t;

/* HW Register field names */
typedef enum {
#include "vcmdregisterenum.h"
	VCMD_REGISTER_AMOUNT
} regVcmdName;

/* HW Register field descriptions */
typedef struct {
	u32 name; /* Register name and index  */
	s32 base; /* Register base address  */
	u32 mask; /* Bitmask for this field */
	s32 lsb; /* LSB for this field [31..0] */
	s32 trace; /* Enable/disable writing in swreg_params.trc */
	s32 rw; /* 1=Read-only 2=Write-only 3=Read-Write */
	char *description; /* Field description */
} regVcmdField_s;

/* Flags for read-only, write-only and read-write */
#define RO 1
#define WO 2
#define RW 3

#define REGBASE(reg) (asicVcmdRegisterDesc[reg].base)

/* Description field only needed for system model build. */
#ifdef TEST_DATA
#define VCMDREG(name, base, mask, lsb, trace, rw, desc)                        \
	{                                                                      \
		name, base, mask, lsb, trace, rw, desc                         \
	}
#else
#define VCMDREG(name, base, mask, lsb, trace, rw, desc)                        \
	{                                                                      \
		name, base, mask, lsb, trace, rw, ""                           \
	}
#endif

/* 4. Function prototypes*/

extern const regVcmdField_s asicVcmdRegisterDesc[];

/* Set a value into a defined register field */
static inline void vcmd_set_register_mirror_value(u32 *reg_mirror,
						  regVcmdName name, u32 value)
{
	const regVcmdField_s *field;
	u32 regVal;

	field = &asicVcmdRegisterDesc[name];

	pr_debug("%s 0x%2x 0x%08x Value: %10d %s\n", __func__, field->base,
		 field->mask, value, field->description);

	/* Clear previous value of field in register */
	regVal = reg_mirror[field->base / 4] & ~(field->mask);

	/* Put new value of field in register */
	reg_mirror[field->base / 4] =
		regVal | ((value << field->lsb) & field->mask);
}

static inline u32 vcmd_get_register_mirror_value(u32 *reg_mirror,
						 regVcmdName name)
{
	const regVcmdField_s *field;
	u32 regVal;

	field = &asicVcmdRegisterDesc[name];

	regVal = reg_mirror[field->base / 4];
	regVal = (regVal & field->mask) >> field->lsb;

	pr_debug("%s 0x%2x 0x%08x  Value: %10d  %s\n", __func__, field->base,
		 field->mask, regVal, field->description);

	return regVal;
}

u32 vcmd_read_reg(const void *hwregs, u32 offset);

void vcmd_write_reg(const void *hwregs, u32 offset, u32 val);

void vcmd_write_register_value(const void *hwregs, u32 *reg_mirror,
			       regVcmdName name, u32 value);

u32 vcmd_get_register_value(const void *hwregs, u32 *reg_mirror,
			    regVcmdName name);

#if defined(__LP64__) || defined(_WIN64) || defined(_WIN32)

#define vcmd_set_addr_register_value(reg_base, reg_mirror, name, value)        \
	do {                                                                   \
		typeof(reg_base)(base_) = (reg_base);                          \
		typeof(reg_mirror)(mirror_) = (reg_mirror);                    \
		typeof(value)(val_) = (value);                                 \
		vcmd_write_register_value((base_), (mirror_), (name),          \
					  (u32)(val_));                        \
		vcmd_write_register_value((base_), (mirror_), (name##_MSB),    \
					  (u32)((val_) >> 32));                \
	} while (0)

#define VCMDGetAddrRegisterValue(reg_base, reg_mirror, name)                   \
	({                                                                     \
		typeof(reg_base)(base_) = (reg_base);                          \
		u32 *mirror_ = (reg_mirror);                                   \
		(ptr_t)vcmd_get_register_value((base_), (mirror_), (name)) |  \
			(((ptr_t)vcmd_get_register_value((base_), (mirror_),   \
							 (name##_MSB))         \
			  << 32));                                             \
	})

#else

#define vcmd_set_addr_register_value(reg_base, reg_mirror, name, value)        \
	vcmd_write_register_value((reg_base), (reg_mirror), name, (u32)(value))

#define VCMDGetAddrRegisterValue(reg_base, reg_mirror, name)                   \
	((ptr_t)vcmd_get_register_value((reg_base), (reg_mirror), (name)))

#endif

#endif
