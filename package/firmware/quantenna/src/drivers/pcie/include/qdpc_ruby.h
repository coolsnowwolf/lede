/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#ifndef __QDPC_RUBY_H__
#define __QDPC_RUBY_H__

#define QDPC_SYSCTL_INTREG        0x2c
#define QDPC_SYSCTL_INTMASK       0x30

/* DMA Base Address */
#define QDPC_DMA_BASE_ADDR        0xEA000000

/* SRAM/DDR2 Base Address */
#define QDPC_SRAM_BASE_ADDR       0x80000000
#define QDPC_DDR2_BASE_ADDR       0x88000000

#define QDPC_PCIE_BASE_ADDR       0xc2000000

#define QDPC_SYSCTL_REG_BASE      0xe0000000
#define QDPC_SYSCTL_REG_INTSTS    0x2c
#define QDPC_SYSCTL_REG_INTMSK    0x30

/* ATU register offsets in the configuration space */

/* View port register */
#define QDPC_ATU_VIEWPORT_REG     0x900

/* CTRL1 register */
#define QDPC_ATU_CTRL1_REG        0x904

/* CTRL2 register */
#define QDPC_ATU_CTRL2_REG        0x908

/* LBAR register */
#define QDPC_ATU_LBAR_REG         0x90c

/* UBAR register */
#define QDPC_ATU_UBAR_REG         0x910

/* LAR register */
#define QDPC_ATU_LAR_REG          0x914

/* LTAR register */
#define QDPC_ATU_LTAR_REG         0x918

/* UTAR register */
#define QDPC_ATU_UTAR_REG         0x91c

#define QDPC_ATU_OB_REGION        0x00000000
#define QDPC_ATU_IB_REGION        0x80000001
#define QDPC_ATU_EN_OB_REGION     0x80000000

#endif
