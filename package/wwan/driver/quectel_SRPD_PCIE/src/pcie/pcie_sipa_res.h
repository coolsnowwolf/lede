/*
 * Copyright (C) 2018-2019 Unisoc Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef PCIE_SIPA_RES_H
#define PCIE_SIPA_RES_H

#include "../include/sipa.h"

/*
 * pcie_sipa_res_create - create pcie res for sipa module.
 * @prod_id: which res is the producer.
 * @cons_id: which res is the consumer.
 *
 * Returns:
 *  failed, return NULL,
 *  succ, return a void * pointer.
 */
void *pcie_sipa_res_create(u8 dst, enum sipa_rm_res_id prod_id,
			   enum sipa_rm_res_id cons_id);

/*
 * pcie_sipa_res_destroy -detroy pcie res for sipa module
 * @res_id: the return point of call function pcie_sipa_res_create.
 */
void pcie_sipa_res_destroy(void *res);
#endif

