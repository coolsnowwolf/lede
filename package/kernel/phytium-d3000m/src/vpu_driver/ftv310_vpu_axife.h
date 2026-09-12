/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 axife controller hardware driver header file.
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

#ifndef _FTV310_VPU_AXIFE_H_
#define _FTV310_VPU_AXIFE_H_

#include "ftv310_vpu_priv.h"
#include "ftv310_vpu.h"

long AxifeReadRegs(struct axife_t *dev, struct core_desc *core);
long AxifeWriteRegs(struct axife_t *dev, struct core_desc *core);
int ftv310_vpu_axife_probe(dtbnode *pnode, int loop, struct axife_t *axifecore);
void ftv310_vpu_axife_cleanup(void);
int ftv310_vpu_axife_init(void);

#endif //_FTV310_VPU_DEC400_H_
