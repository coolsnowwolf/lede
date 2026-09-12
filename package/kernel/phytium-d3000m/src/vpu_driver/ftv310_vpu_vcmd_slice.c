// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 vcmd driver.
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

#include <linux/module.h>
/* Our header */
#include <linux/of_reserved_mem.h>
#include <linux/of_irq.h>

#include "ftv310_vpu.h"
#include "ftv310_vpu_vcmd.h"
#include "ftv310_vpu_device.h"
#include "ftv310_vpu_priv.h"

static vcmd_slice_str * slice_list;
static int vcmd_slice_num;

vcmd_slice_str *get_vcmd_slice_head(void)
{
	return slice_list;
}

void set_vcmd_slice_listnull(void)
{
	slice_list = NULL;
}

vcmd_core_str *get_dev_of_core(vcmd_dev_str *vcmd_dev, u32 id)
{
	vcmd_core_str *vcmd_core;

	vcmd_core = vcmd_dev->vcmd_core;
	while (vcmd_core) {
		if (vcmd_core->core_id == id)
			break;
		vcmd_core = vcmd_core->core_next;
	}
	if (!vcmd_core)
		pr_err("ERROR %s,%d\n", __func__, __LINE__);
	return vcmd_core;
}

vcmd_slice_str *get_slice_by_sliceidx(u32 sliceidx)
{
	vcmd_slice_str *slice_node;

	slice_node = slice_list;
	while (slice_node) {
		if (slice_node->sliceidx == sliceidx)
			break;
		slice_node = slice_node->slice_next;
	}
	if (!slice_node)
		pr_err("ERROR %s,%d\n", __func__, __LINE__);

	return slice_node;
}

vcmd_dev_str *get_dev_by_sliceidx(u32 sliceidx, u32 group_codec)
{
	vcmd_slice_str *slice_node;
	vcmd_dev_str *codec_dev;

	slice_node = get_slice_by_sliceidx(sliceidx);
	if (group_codec == FTV310_VPU_DECODER)
		codec_dev = &slice_node->dec_vcmd;
	else
		codec_dev = &slice_node->enc_vcmd;

	return codec_dev;
}

u32 get_vcmd_slice_config(u32 sliceidx)
{
	vcmd_slice_str *slice_node;

	slice_node = get_slice_by_sliceidx(sliceidx);
	return slice_node->config;
}

void add_vcmd_slice_config(u32 sliceidx, u32 config)
{
	vcmd_slice_str *slice_node;

	slice_node = get_slice_by_sliceidx(sliceidx);
	slice_node->config |= config;
}

void set_vcmd_slice_config(vcmd_slice_str *slice, struct vcmd_config *vcmd_cfg_p)
{
	if (vcmd_cfg_p->sub_module_type == VCMD_TYPE_DECODER) {
		slice->config |= SET_DEC_CFG_BIT(CONFIG_HWDEC);
		slice->config |= SET_DEC_CFG_BIT(CONFIG_VCMD);
		if (vcmd_cfg_p->submodule_dec400_addr != 0xffff)
			slice->config |= SET_DEC_CFG_BIT(CONFIG_DEC400);
		if (vcmd_cfg_p->submodule_L2Cache_addr != 0xffff)
			slice->config |= SET_DEC_CFG_BIT(CONFIG_L2CACHE);
		if (vcmd_cfg_p->submodule_axife_addr[0] != 0xffff)
			slice->config |= SET_DEC_CFG_BIT(CONFIG_AXIFE);
		if (vcmd_cfg_p->submodule_MMU_addr != 0xffff)
			slice->config |= SET_DEC_CFG_BIT(CONFIG_FTV310_VPUMMU);
	}

	if (vcmd_cfg_p->sub_module_type == VCMD_TYPE_ENCODER) {
		slice->config |= SET_ENC_CFG_BIT(CONFIG_HWENC);
		slice->config |= SET_ENC_CFG_BIT(CONFIG_VCMD);
		if (vcmd_cfg_p->submodule_dec400_addr != 0xffff)
			slice->config |= SET_ENC_CFG_BIT(CONFIG_DEC400);
		if (vcmd_cfg_p->submodule_L2Cache_addr != 0xffff)
			slice->config |= SET_ENC_CFG_BIT(CONFIG_L2CACHE);
		if (vcmd_cfg_p->submodule_axife_addr[0] != 0xffff)
			slice->config |= SET_ENC_CFG_BIT(CONFIG_AXIFE);
		if (vcmd_cfg_p->submodule_MMU_addr != 0xffff)
			slice->config |= SET_ENC_CFG_BIT(CONFIG_FTV310_VPUMMU);
	}

}

int get_vcmd_slice_num(void)
{
	return vcmd_slice_num;
}

void add_vcmd_slice(struct device *dev, vcmd_slice_str *slice_node)
{
	u32 num = 0;
	vcmd_slice_str *head;

	head = slice_list;

	slice_node->dec_vcmd.dev = dev;
	slice_node->enc_vcmd.dev = dev;
	slice_node->dec_vcmd.type = FTV310_VPU_DECODER;
	slice_node->enc_vcmd.type = FTV310_VPU_ENCODER;
	vcmd_slice_num++;

	if (!head) {
		slice_node->sliceidx = 0;
		slice_list = slice_node;
		return;
	}
	while (head->slice_next) {
		head = head->slice_next;
		num++;
	}
	slice_node->sliceidx = num + 1;
	head->slice_next = slice_node;
}

void add_vcmd_core(vcmd_slice_str *slice_node, vcmd_core_str *vcmd_core)
{
	vcmd_core_str **head, *temp_head;

	if (vcmd_core->type == FTV310_VPU_CORE_DEC ||
	    vcmd_core->type == FTV310_VPU_CORE_DECJPG) {
		head = &slice_node->dec_vcmd.vcmd_core;
		vcmd_core->core_id = slice_node->dec_vcmd.subsys_num;
		vcmd_core->parent_dev = &slice_node->dec_vcmd;
		if (!slice_node->dec_vcmd.subsys_num)
			sema_init(&slice_node->dec_vcmd.vcmd_reserve_cmdbuf_sem[vcmd_core->vcmd_core_cfg.sub_module_type], 1);
		slice_node->dec_vcmd.subsys_num++;
	} else if (vcmd_core->type == FTV310_VPU_CORE_ENC ||
	    vcmd_core->type == FTV310_VPU_CORE_IM ||
	    vcmd_core->type == FTV310_VPU_CORE_ENCJPG) {
		head = &slice_node->enc_vcmd.vcmd_core;
		vcmd_core->core_id = slice_node->enc_vcmd.subsys_num;
		vcmd_core->parent_dev = &slice_node->enc_vcmd;
		if (!slice_node->enc_vcmd.subsys_num)
			sema_init(&slice_node->enc_vcmd.vcmd_reserve_cmdbuf_sem[vcmd_core->vcmd_core_cfg.sub_module_type], 1);
		slice_node->enc_vcmd.subsys_num++;
	} else {
		pr_err("unkonwn vcmd_core->type %d\n", vcmd_core->type);
		return;
	}

	if (!(*head)) {
		*head = vcmd_core;
		return;
	}
	temp_head = *head;

	while (temp_head->core_next)
		temp_head = temp_head->core_next;
	temp_head->core_next = vcmd_core;
}

#ifdef USE_DTB_PROBE

static void get_node_info(struct device_node *ofnode, u64 *addr, u64 *size,
			  u32 *irq)
{
	int i, na, ns;
	struct fwnode_handle *fwnode;
	u32 reg_u32[4];
	u64 ioaddress, iosize;
	struct resource r;
	int endian = of_device_is_big_endian(ofnode);

	fwnode = &ofnode->fwnode;
	na = of_n_addr_cells(ofnode);
	ns = of_n_size_cells(ofnode);
	if (na > 2 || ns > 2) {
		pr_err("cell size too big");
		return;
	}

	fwnode_property_read_u32_array(fwnode, "reg", reg_u32, na + ns);
	if (na == 2) {
		if (!endian) {
			ioaddress = reg_u32[0];
			ioaddress <<= 32;
			ioaddress |= reg_u32[1];
		} else {
			ioaddress = reg_u32[1];
			ioaddress <<= 32;
			ioaddress |= reg_u32[0];
		}
	} else {
		ioaddress = reg_u32[0];
	}
	if (ns == 2) {
		if (!endian) {
			iosize = reg_u32[na];
			iosize <<= 32;
			iosize |= reg_u32[na + 1];
		} else {
			iosize = reg_u32[na + 1];
			iosize <<= 32;
			iosize |= reg_u32[na];
		}
	} else {
		iosize = reg_u32[na];
	}
	*addr = ioaddress;
	*size = iosize;
	pr_info("node regio =%llx:%llx", ioaddress, iosize);

	for (i = 0; i < 4; i++) {
		*(irq + i) = -1;
		if (of_irq_to_resource(ofnode, i, &r) > 0) {
			int irq_val = of_irq_get(ofnode, i);

			pr_info("irq %d:%s = %lld:%d", i, r.name, r.start,
				irq_val);
			if (irq_val > 0)
				*(irq + i) = irq_val;
		}
	}
}

static void get_submodule(struct vcmd_config *vcmd_cfg_p,
			  struct subsys_addr *temp_subsys_addr,
			  struct device_node *ofnode)
{
	int type;
	int irq[4];
	u64 ioaddress, iosize;

	get_node_info(ofnode, &ioaddress, &iosize, &irq[0]);
	type = getnodetype(ofnode->name);

	switch (type) {
	case FTV310_VPU_CORE_VCMD: {
		vcmd_cfg_p->vcmd_base_addr = ioaddress;
		vcmd_cfg_p->vcmd_iosize = iosize;
		vcmd_cfg_p->vcmd_irq = irq[0];
		break;
	}
	case FTV310_VPU_CORE_DEC400: {
		temp_subsys_addr->submodule_dec400_addr = ioaddress;
		break;
	}
	case FTV310_VPU_CORE_AXIFE: {
		if (temp_subsys_addr->submodule_axife_addr[0] == 0xFFFF)
			temp_subsys_addr->submodule_axife_addr[0] = ioaddress;
		else
			temp_subsys_addr->submodule_axife_addr[1] = ioaddress;
		break;
	}
	case FTV310_VPU_CORE_CACHE: {
		temp_subsys_addr->submodule_L2Cache_addr = ioaddress;
		break;
	}
	case FTV310_VPU_CORE_MMU: {
		temp_subsys_addr->submodule_MMU_addr = ioaddress;
		break;
	}
	default:
		break;
	}
}

vcmd_slice_str *ftv310_vpu_vcmd_analyze_subnode(struct platform_device *pdev, int useirq,
					    struct device_node *slice)
{
	struct device *dev = &pdev->dev;
	struct device_node *child_level_0, *child_level_1;
	vcmd_slice_str *cur_slice;
	vcmd_core_str *vcmd_core;
	struct vcmd_config *vcmd_cfg_p;
	struct subsys_addr temp_subsys_addr;
	int irq[4];
	u64 ioaddress, iosize;

	cur_slice = kzalloc(sizeof(vcmd_slice_str), GFP_KERNEL);
	add_vcmd_slice(dev, cur_slice);

	for_each_child_of_node(slice, child_level_0) {
		vcmd_core = kzalloc(sizeof(vcmd_dev_str), GFP_KERNEL);
		vcmd_cfg_p = &vcmd_core->vcmd_core_cfg;
		vcmd_core->useirq = useirq;
		vcmd_core->type = getnodetype(child_level_0->name);
		switch (vcmd_core->type) {
		case FTV310_VPU_CORE_DEC:
			vcmd_cfg_p->sub_module_type = VCMD_TYPE_DECODER;
			break;
		case FTV310_VPU_CORE_ENC:
			vcmd_cfg_p->sub_module_type = VCMD_TYPE_ENCODER;
			break;
		case FTV310_VPU_CORE_IM:
			vcmd_cfg_p->sub_module_type = VCMD_TYPE_CUTREE;
			break;
		case FTV310_VPU_CORE_DECJPG:
			vcmd_cfg_p->sub_module_type = VCMD_TYPE_JPEG_DECODER;
			break;
		case FTV310_VPU_CORE_ENCJPG:
			vcmd_cfg_p->sub_module_type = VCMD_TYPE_JPEG_ENCODER;
			break;
		default:
			pr_err("Error, DTB codec name is not recognized\n");
			break;
		}
		vcmd_cfg_p->submodule_main_addr = 0xffff;
		vcmd_cfg_p->submodule_L2Cache_addr = 0xffff;
		vcmd_cfg_p->submodule_dec400_addr = 0xffff;
		vcmd_cfg_p->submodule_axife_addr[0] = 0xffff;
		vcmd_cfg_p->submodule_axife_addr[1] = 0xffff;
		vcmd_cfg_p->submodule_MMU_addr = 0xffff;

		get_node_info(child_level_0, &ioaddress, &iosize, &irq[0]);
		temp_subsys_addr.submodule_main_addr = ioaddress;
		add_vcmd_core(cur_slice, vcmd_core);
		for_each_child_of_node(child_level_0, child_level_1) {
			get_submodule(vcmd_cfg_p, &temp_subsys_addr,
				      child_level_1);
		}
		if (temp_subsys_addr.submodule_main_addr != 0xffff) {
			if (temp_subsys_addr.submodule_main_addr >
			    vcmd_cfg_p->vcmd_base_addr) {
				vcmd_cfg_p->submodule_main_addr =
					temp_subsys_addr.submodule_main_addr -
					vcmd_cfg_p->vcmd_base_addr;
			} else {
				pr_err("Error, DTB submodule_main_addr is not correct\n");
			}
		}

		if (temp_subsys_addr.submodule_dec400_addr != 0xffff) {
			if (temp_subsys_addr.submodule_dec400_addr >
			    vcmd_cfg_p->vcmd_base_addr) {
				vcmd_cfg_p->submodule_dec400_addr =
					temp_subsys_addr.submodule_dec400_addr -
					vcmd_cfg_p->vcmd_base_addr;
				cur_slice->config |=
					(vcmd_cfg_p->sub_module_type == VCMD_TYPE_ENCODER)
						? SET_ENC_CFG_BIT(CONFIG_DEC400)
						: SET_DEC_CFG_BIT(CONFIG_DEC400);
			} else {
				pr_err("Error, DTB submodule_dec400_addr is not correct\n");
			}
		}

		if (temp_subsys_addr.submodule_L2Cache_addr != 0xffff) {
			if (temp_subsys_addr.submodule_L2Cache_addr >
			    vcmd_cfg_p->vcmd_base_addr) {
				vcmd_cfg_p->submodule_L2Cache_addr =
					temp_subsys_addr.submodule_L2Cache_addr -
					vcmd_cfg_p->vcmd_base_addr;
				cur_slice->config |=
					(vcmd_cfg_p->sub_module_type == VCMD_TYPE_ENCODER)
						? SET_ENC_CFG_BIT(CONFIG_L2CACHE)
						: SET_DEC_CFG_BIT(CONFIG_L2CACHE);
			} else {
				pr_err("Error, DTB submodule_L2Cache_addr is not correct\n");
			}
		}

		if (temp_subsys_addr.submodule_MMU_addr != 0xffff) {
			if (temp_subsys_addr.submodule_MMU_addr >
			    vcmd_cfg_p->vcmd_base_addr) {
				vcmd_cfg_p->submodule_MMU_addr =
					temp_subsys_addr.submodule_MMU_addr -
					vcmd_cfg_p->vcmd_base_addr;
				cur_slice->config |=
					(vcmd_cfg_p->sub_module_type == VCMD_TYPE_ENCODER)
						? SET_ENC_CFG_BIT(CONFIG_FTV310_VPUMMU)
						: SET_DEC_CFG_BIT(CONFIG_FTV310_VPUMMU);
			} else {
				pr_err("Error, DTB submodule_L2Cache_addr is not correct\n");
			}
		}
	}
	return cur_slice;
}
#endif

void add_norslice(struct device *dev, int sliceidx)
{
	struct slice_info *cur_nor_slice;

	cur_nor_slice = getslicenode_ininit(sliceidx);
	if (!cur_nor_slice && sliceidx == get_slicenumber()) {
		sliceidx = addslice(dev, -1, 0);
		if (sliceidx < 0)
			return;
		cur_nor_slice = getslicenode_ininit(sliceidx);
	}
}

void transfer_vcmdslice_to_norslice(int sliceidx)
{
	vcmd_slice_str *cur_vcmd_slice;
	struct device *dev;
	struct slice_info *cur_nor_slice;
	struct vcmd_core *vcmd_core_tmp;
	struct ftv310_vpu_dec_t *dec_pcore;
	struct ftv310_vpu_enc_t *enc_pcore;

	cur_vcmd_slice = get_slice_by_sliceidx(sliceidx);


	if (cur_vcmd_slice->dec_vcmd.subsys_num != 0) {
		dev = cur_vcmd_slice->dec_vcmd.dev;
	} else if (cur_vcmd_slice->enc_vcmd.subsys_num != 0) {
		dev = cur_vcmd_slice->enc_vcmd.dev;
	} else {
		pr_debug("error %s vcmd slice is null\n", __func__);
		return;
	}
	cur_nor_slice = getslicenode_ininit(sliceidx);
	if (!cur_nor_slice && sliceidx == get_slicenumber()) {
		sliceidx = addslice(dev, -1, 0);
		if (sliceidx < 0)
			return;
		cur_nor_slice = getslicenode_ininit(sliceidx);
	}

	cur_nor_slice->config = get_vcmd_slice_config(sliceidx);

	if (cur_vcmd_slice->dec_vcmd.subsys_num != 0) {
		vcmd_core_tmp = cur_vcmd_slice->dec_vcmd.vcmd_core;

		while (vcmd_core_tmp) {
			dec_pcore = vmalloc(sizeof(*dec_pcore));
			if (!dec_pcore)
				break;

			memset(dec_pcore, 0, sizeof(struct ftv310_vpu_dec_t));
			dec_pcore->core_id = vcmd_core_tmp->core_id;
			dec_pcore->sliceidx = sliceidx;
			add_decnode(sliceidx, dec_pcore);
			vcmd_core_tmp = vcmd_core_tmp->core_next;
		}
	}

	if (cur_vcmd_slice->enc_vcmd.subsys_num != 0) {
		vcmd_core_tmp = cur_vcmd_slice->enc_vcmd.vcmd_core;

		while (vcmd_core_tmp) {
			enc_pcore = vmalloc(sizeof(*enc_pcore));
			if (!enc_pcore)
				break;
			memset(enc_pcore, 0, sizeof(struct ftv310_vpu_enc_t));
			enc_pcore->core_id = vcmd_core_tmp->core_id;
			enc_pcore->core_cfg.sliceidx = sliceidx;
			add_encnode(sliceidx, enc_pcore);
			vcmd_core_tmp = vcmd_core_tmp->core_next;
		}
	}
	slice_init_finish();
}
