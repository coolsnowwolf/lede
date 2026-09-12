/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 driver public header file.
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

#ifndef FTV310_VPU_H
#define FTV310_VPU_H

#include <linux/version.h>
#include <linux/ioctl.h>
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
#include <linux/dma-resv.h>
#else
#include <linux/reservation.h>
#endif
#include <linux/dma-mapping.h>
#include <drm/drm_vma_manager.h>
#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
#include <drm/drm_gem_dma_helper.h>
#else
#include <drm/drm_gem_cma_helper.h>
#endif
#include <drm/drm_gem.h>
#include <linux/dma-buf.h>
#include <drm/drm.h>
#include <drm/drm_auth.h>
#if KERNEL_VERSION(4, 13, 0) <= LINUX_VERSION_CODE
#include <linux/dma-fence.h>
#endif
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#else
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <drm/drm_drv.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
#include <drm/drm_pci.h>
#endif
#endif

#include "ftv310_vpu_metadata.h"

#define DRIVER_NAME "ftv310"

/*these domain definitions are identical to ftv310_vpu_bufmgr.h*/
#define FTV310_VPU_DOMAIN_NONE 0x00000
#define FTV310_VPU_CPU_DOMAIN 0x00001
#define FTV310_VPU_HEVC264_DOMAIN 0x00002
#define FTV310_VPU_JPEG_DOMAIN 0x00004
#define FTV310_VPU_DECODER0_DOMAIN 0x00008
#define FTV310_VPU_DECODER1_DOMAIN 0x00010
#define FTV310_VPU_DECODER2_DOMAIN 0x00020

#define CONFIG_HWDEC BIT(0)
#define CONFIG_HWENC BIT(1)
#define CONFIG_L2CACHE BIT(2)
#define CONFIG_DEC400 BIT(3)
#define CONFIG_FTV310_VPUMMU BIT(4)
#define CONFIG_VCMD BIT(5)
#define CONFIG_AXIFE BIT(6)

#define KCORE(id) ((u32)(id) & 0xff)
#define NODETYPE(id) (((u32)(id) >> 8) & 0xff)
#define SLICE(id) ((u32)(id) >> 16)

/* slice index definition is unchanged.
 *for dec400/cache NODE(id) refers to its parent core number based on NODETYPE
 *for dec/enc, NODE(id) refers to its core num, and NODETYPE is useless.
 */
/*node type for NODETYPE(id), apply to be expanded */
#define NODE_TYPE_DEC BIT(0)
#define NODE_TYPE_ENC BIT(1)
#define NODE_TYPE_VCMD BIT(2)

#define CORE_MAX (4)

enum {
	CORE_VCE = 0,
	CORE_VCEJ = 1,
	CORE_CUTREE = 2,
	CORE_DEC400 = 3,
	CORE_MMU = 4,
	CORE_L2CACHE = 5,
	CORE_AXIFE = 6,
	CORE_APBFT = 7,
	CORE_MMU_1 = 8,
	CORE_AXIFE_1 = 9,
	CORE_TYPE_MAX
};

typedef enum {
	VCE,
	VCD_0,
	VCD_1,
	DECODER_G1_0,
	DECODER_G1_1,
	DECODER_G2_0,
	DECODER_G2_1,
} cache_client_type;

enum CoreType {
	/* Decoder */
	HW_VCD = 0,
	HW_VCDJ,
	HW_BIGOCEAN,
	HW_VCMD,
	HW_MMU, //if set HW_MMU_WR, then HW_MMU means HW_MMU_RD
	HW_MMU_WR,
	HW_DEC400,
	HW_L2CACHE,
	HW_SHAPER,
	/* Encoder*/
	/* Auxiliary IPs */
	HW_AXIFE,
	HW_AFBC,
	HW_CORE_MAX /* max number of cores supported */
};

typedef enum { DIR_RD = 0, DIR_WR, DIR_BI } driver_cache_dir;

struct ftv310_vpu_drm_fb {
	struct drm_framebuffer fb;
	struct drm_gem_object *obj[4];
};

/**
 * The memta data location information exchange IOCTRL parameters.
 */
struct ftv310_vpu_metadata_params {
	int handle; /* the handle of current bo */
	struct viv_vidmem_metadata meta_data; /* the meta data */
};

struct dmapriv {
	struct viv_vidmem_metadata meta_data;
	void *self; //ptr of parent cma
};

struct vram_record {
        struct list_head node;
        unsigned long size;
        void *vaddr;
};

struct drm_gem_ftv310_vpu_object {
	/* base of gem object */
	struct drm_gem_object base;

	struct dmapriv dmapriv;

	/* following is private data for ftv310_vpu object */

	dma_addr_t paddr;
	dma_addr_t mem_base;
	struct page **pages;
	unsigned int pages_pin_count;
	struct mutex pages_lock;
	struct sg_table *sgt;

	/* For objects with DMA memory allocated by GEM CMA */
	void *vaddr;
	struct page *pageaddr;
	unsigned long num_pages;
	/*fence ref*/
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	struct dma_resv kresv;
#else
	struct reservation_object kresv;
#endif
	unsigned int ctxno;
	int handle;
	int sliceidx;
	int flag;

	struct vram_record vram_node;

	/* mmu */
	struct mmu_t *mmu;
	void *uptr; /* mmap addr */
};

struct ftv310_vpu_fencecheck {
	unsigned int handle;
	int ready;
};

struct ftv310_vpu_domainset {
	unsigned int handle;
	unsigned int writedomain;
	unsigned int readdomain;
};

struct ftv310_vpu_addrmap {
	unsigned int handle;
	unsigned long long vm_addr;
	unsigned long long phy_addr;
	unsigned long long mem_base;
};

struct ftv310_vpu_regtransfer {
	unsigned long coreid;
	unsigned long offset;
	unsigned long size;
	const void *data;
	int benc; /*encoder core or decoder core*/
	int direction; /*0=read, 1=write*/
};

struct ftv310_vpu_corenum {
	unsigned int deccore;
	unsigned int enccore;
};

#define FTV310_VPU_FENCE_WRITE 1
struct ftv310_vpu_acquirebuf {
	unsigned long handle;
	unsigned long flags;
	unsigned long timeout;
	unsigned long fence_handle;
};

struct ftv310_vpu_releasebuf {
	unsigned long fence_handle;
};

struct core_desc {
	__u32 id; /* id of the core */
	__u32 type; /* type of core to be written */
	__u32 *regs; /* pointer to user registers */
	__u32 size; /* size of register space */
	__u32 reg_id;
};

struct nor64_parameter {
	unsigned long data;
	u32 id;
};

struct nor32_parameter {
	u32 data;
	u32 id;
};

typedef struct {
	unsigned int type_info;
	/*indicate which IP is contained
	 *in this subsystem and each uses one bit of this variable
	 */
	unsigned long offset[CORE_TYPE_MAX];
	unsigned long regSize[CORE_TYPE_MAX];
	int irq[CORE_TYPE_MAX];
	unsigned int id;
} SUBSYS_CORE_INFO;

typedef struct CoreWaitOut {
	u32 job_id[CORE_MAX];
	u32 irq_status[CORE_MAX];
	u32 irq_num;
	u32 id;
} CORE_WAIT_OUT;

struct axife_cfg {
	u8 axi_rd_chn_num;
	u8 axi_wr_chn_num;
	u8 axi_rd_burst_length;
	u8 axi_wr_burst_length;
	u8 fe_mode;
	u32 id;
};

/*********define the parameters to transfer to iocontrol**********/
typedef enum {
	CORE_FLAG = 0,
	VCMD_FLAG = 0x10,
	VCMD_BUF_FLAG = 0x11,
	DEC400_FLAG = 0x20,
	CACHE_FLAG = 0x30,
	MMU0_FLAG = 0x40,
	MMU1_FLAG = 0x41,
} SUB_NODE;

typedef enum {
	CODEC_DEC_FLGA = 0,
	CODEC_ENC_FLGA = 1,
} CODEC_GROP;

//use for mmap's offset parameter
//low pageshift bits are skipped since it can't be seen in driver.
//so standard DRM mmap's offset will starts from 1ul << (32 + pageshift)
//0 .. (1ul << (32 + pageshift) - 1) is used by our own mmap, such as core reg, vcmd buf.
//offset parameter is used as ioctl_id's ID_PAR.
#define MAPOFF_2COREID(a)		(((offset_t)(a)) >> PAGE_SHIFT)
// this is the least address value used for normal DRM map offset
#define PHY_MMAP_ADDRES_CEIL	(1ul << (32 + PAGE_SHIFT))
#define PHY_MMAP_ADDRES_CEIL_MMAP	(1ul << 32)

typedef union ioctl_id {
	unsigned int data;
	struct id_par {
		unsigned int sub_mod_idx : 8;
		unsigned int codec_idx : 8;
		unsigned int group_idx : 8;
		unsigned int node_idx : 8;
	} ID_PAR;
} ftv310_vpu_ioctl_id;
/*********define the parameters to transfer to iocontrol**********/

/************* some cache related defines   ************/
//#define PCI_BUS
/*Define Cache&Shaper Offset from common base*/
#define SHAPER_OFFSET (0x8 << 2)
#define CACHE_ONLY_OFFSET (0x8 << 2)
#define CACHE_WITH_SHAPER_OFFSET (0x80 << 2)
/************* some cache related defines  end ************/

/* Ioctl definitions */
#define FTV310_VPU_DRM_IOCTL_START (DRM_COMMAND_BASE)
#define DRM_IOCTL_FTV310_VPU_TESTCMD DRM_IOWR(FTV310_VPU_DRM_IOCTL_START, unsigned int)
#define DRM_IOCTL_FTV310_VPU_GETPADDR                                              \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 1, struct ftv310_vpu_addrmap)
#define DRM_IOCTL_FTV310_VPU_HWCFG DRM_IO(FTV310_VPU_DRM_IOCTL_START + 2)
#define DRM_IOCTL_FTV310_VPU_TESTREADY                                             \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 3, struct ftv310_vpu_fencecheck)
#define DRM_IOCTL_FTV310_VPU_SETDOMAIN                                             \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 4, struct ftv310_vpu_domainset)
#define DRM_IOCTL_FTV310_VPU_ACQUIREBUF                                            \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 6, struct ftv310_vpu_acquirebuf)
#define DRM_IOCTL_FTV310_VPU_RELEASEBUF                                            \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 7, struct ftv310_vpu_releasebuf)
#define DRM_IOCTL_FTV310_VPU_GETPRIMEADDR                                          \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 8, unsigned long *)
#define DRM_IOCTL_FTV310_VPU_PTR_PHYADDR                                           \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 9, unsigned long *)
//#define DRM_IOCTL_FTV310_VPU_QUERY_METADATA
//	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 10, struct ftv310_vpu_metadata_params)
#define DRM_IOCTL_FTV310_VPU_UPDATE_METADATA                                       \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 11, struct ftv310_vpu_metadata_params)
#define DRM_IOCTL_FTV310_VPU_GET_SLICENUM DRM_IO(FTV310_VPU_DRM_IOCTL_START + 12)
#define DRM_IOCTL_FTV310_VPU_GET_VCMDSUP                                           \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 13, unsigned int *)

#define DRM_IOCTL_FTV310_VPU_GET_IRQINFO											\
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 14, unsigned int *)
#define DRM_IOCTL_FTV310_VPU_GET_PMSUP                                           \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 15, unsigned int *)

#define DRM_IOCTL_FTV310_VPU_QUERY_METADATA                                        \
	DRM_IOWR(FTV310_VPU_DRM_IOCTL_START + 10, struct ftv310_vpu_metadata_params *)
#define FTV310_VPU_IOC_MAGIC 'h'
#define FTV310_VPU_IOCTL_START FTV310_VPU_IOC_MAGIC

/* ftv310_vpu enc related */
#define FTV310_VPUENC_IOC_START _IO(FTV310_VPU_IOCTL_START, 17)
#define FTV310_VPUENC_IOCGHWOFFSET                                                 \
	_IOR(FTV310_VPU_IOCTL_START, 17, unsigned long long *)
#define FTV310_VPUENC_IOCGHWIOSIZE _IOWR(FTV310_VPU_IOCTL_START, 18, unsigned long *)
#define FTV310_VPUENC_IOC_CLI _IO(FTV310_VPU_IOCTL_START, 19)
#define FTV310_VPUENC_IOC_STI _IO(FTV310_VPU_IOCTL_START, 20)
#define FTV310_VPUENC_IOCHARDRESET _IO(FTV310_VPU_IOCTL_START, 21) /* for debugging */
#define FTV310_VPUENC_IOCGSRAMOFFSET                                               \
	_IOR(FTV310_VPU_IOCTL_START, 22, unsigned long long *)
#define FTV310_VPUENC_IOCGSRAMEIOSIZE _IOR(FTV310_VPU_IOCTL_START, 23, unsigned int *)
#define FTV310_VPUENC_IOCH_ENC_RESERVE                                             \
	_IOWR(FTV310_VPU_IOCTL_START, 24, struct nor32_parameter *)
#define FTV310_VPUENC_IOCH_ENC_RELEASE                                             \
	_IOW(FTV310_VPU_IOCTL_START, 25, struct nor32_parameter *)
#define FTV310_VPUENC_IOCG_CORE_NUM _IO(FTV310_VPU_IOCTL_START, 26)
#define FTV310_VPUENC_IOCG_CORE_WAIT                                               \
	_IOWR(FTV310_VPU_IOCTL_START, 27, struct nor32_parameter *)
#define FTV310_VPUENC_IOCG_CORE_INFO                                               \
	_IOR(FTV310_VPU_IOCTL_START, 28, SUBSYS_CORE_INFO *)
#define FTV310_VPUENC_IOCG_ANYCORE_WAIT                                            \
	_IOR(FTV310_VPU_IOCTL_START, 29, CORE_WAIT_OUT *)
#define FTV310_VPU_IOCG_ENABLE_CORE                                               \
	_IOR(FTV310_VPU_IOCTL_START, 32, unsigned int *)
#define FTV310_VPUENC_IOC_END _IO(FTV310_VPU_IOCTL_START, 33)

/* ftv310_vpu dec related */
#define FTV310_VPUDEC_IOC_START _IO(FTV310_VPU_IOCTL_START, 41)
#define FTV310_VPUDEC_PP_INSTANCE _IO(FTV310_VPU_IOCTL_START, 41)
#define FTV310_VPUDEC_HW_PERFORMANCE _IO(FTV310_VPU_IOCTL_START, 42)
#define FTV310_VPUDEC_IOCGHWOFFSET                                                 \
	_IOWR(FTV310_VPU_IOCTL_START, 43, unsigned long long *)
#define FTV310_VPUDEC_IOCGHWIOSIZE _IOWR(FTV310_VPU_IOCTL_START, 44, unsigned int *)
#define FTV310_VPUDEC_IOC_CLI _IO(FTV310_VPU_IOCTL_START, 45)
#define FTV310_VPUDEC_IOC_STI _IO(FTV310_VPU_IOCTL_START, 46)
#define FTV310_VPUDEC_IOC_MC_OFFSETS                                               \
	_IOWR(FTV310_VPU_IOCTL_START, 47, unsigned long long *)
#define FTV310_VPUDEC_IOC_MC_CORES _IO(FTV310_VPU_IOCTL_START, 48)
#define FTV310_VPUDEC_IOCS_DEC_PUSH_REG                                            \
	_IOW(FTV310_VPU_IOCTL_START, 49, struct core_desc *)
#define FTV310_VPUDEC_IOCS_PP_PUSH_REG                                             \
	_IOW(FTV310_VPU_IOCTL_START, 50, struct core_desc *)
#define FTV310_VPUDEC_IOCH_DEC_RESERVE                                             \
	_IOW(FTV310_VPU_IOCTL_START, 51, struct nor32_parameter *)
#define FTV310_VPUDEC_IOCT_DEC_RELEASE _IO(FTV310_VPU_IOCTL_START, 52)
#define FTV310_VPUDEC_IOCQ_PP_RESERVE _IO(FTV310_VPU_IOCTL_START, 53)
#define FTV310_VPUDEC_IOCT_PP_RELEASE _IO(FTV310_VPU_IOCTL_START, 54)
#define FTV310_VPUDEC_IOCX_DEC_WAIT _IOW(FTV310_VPU_IOCTL_START, 55, struct core_desc *)
#define FTV310_VPUDEC_IOCX_PP_WAIT _IOWR(FTV310_VPU_IOCTL_START, 56, struct core_desc *)
#define FTV310_VPUDEC_IOCS_DEC_PULL_REG                                            \
	_IOWR(FTV310_VPU_IOCTL_START, 57, struct core_desc *)
#define FTV310_VPUDEC_IOCS_PP_PULL_REG                                             \
	_IOWR(FTV310_VPU_IOCTL_START, 58, struct core_desc *)
#define FTV310_VPUDEC_IOCG_CORE_WAIT _IO(FTV310_VPU_IOCTL_START, 59)
#define FTV310_VPUDEC_IOX_ASIC_ID _IO(FTV310_VPU_IOCTL_START, 60)
#define FTV310_VPUDEC_IOCG_CORE_ID                                                 \
	_IOW(FTV310_VPU_IOCTL_START, 61, struct nor32_parameter *)
#define FTV310_VPUDEC_IOCS_DEC_WRITE_REG                                           \
	_IOW(FTV310_VPU_IOCTL_START, 62, struct core_desc *)
#define FTV310_VPUDEC_IOCS_DEC_READ_REG                                            \
	_IOWR(FTV310_VPU_IOCTL_START, 63, struct core_desc *)
#define FTV310_VPUDEC_DEBUG_STATUS _IO(FTV310_VPU_IOCTL_START, 64)
#define FTV310_VPUDEC_IOX_ASIC_BUILD_ID                                            \
	_IOWR(FTV310_VPU_IOCTL_START, 65, unsigned int *)
#define FTV310_VPUDEC_IOX_IOCX_POLL _IOWR(FTV310_VPU_IOCTL_START, 66, unsigned int *)

#define FTV310_VPUDEC_IOC_END _IO(FTV310_VPU_IOCTL_START, 79)

/* ftv310_vpu cache related */
#define FTV310_VPUCACHE_IOC_START _IO(FTV310_VPU_IOCTL_START, 80)
#define CACHE_IOCGHWOFFSET _IOR(FTV310_VPU_IOCTL_START, 80, unsigned long long *)
#define CACHE_IOCGHWIOSIZE _IO(FTV310_VPU_IOCTL_START, 81)
#define CACHE_IOCHARDRESET _IO(FTV310_VPU_IOCTL_START, 82) /* debugging tool */
#define CACHE_IOCH_HW_RESERVE _IOW(FTV310_VPU_IOCTL_START, 83, unsigned long long *)
#define CACHE_IOCH_HW_RELEASE _IO(FTV310_VPU_IOCTL_START, 84)
#define CACHE_IOCG_CORE_NUM _IO(FTV310_VPU_IOCTL_START, 85)
#define CACHE_IOCG_ABORT_WAIT _IO(FTV310_VPU_IOCTL_START, 86)
//#define CACHE_IOCGBUFBUSADDRESS _IOR(CACHE_IOC_MAGIC,  87, unsigned long *)
//#define CACHE_IOCGBUFSIZE       _IOR(CACHE_IOC_MAGIC,  88, unsigned int *)
#define FTV310_VPUCACHE_IOC_END _IO(FTV310_VPU_IOCTL_START, 89)

/* ftv310_vpu dec400 related */
#define FTV310_VPUDEC400_IOC_START _IO(FTV310_VPU_IOCTL_START, 90)
#define DEC400_IOCGHWIOSIZE _IO(FTV310_VPU_IOCTL_START, 90)
#define DEC400_IOCS_DEC_WRITE_REG                                              \
	_IOW(FTV310_VPU_IOCTL_START, 91, struct core_desc *)
#define DEC400_IOCS_DEC_READ_REG                                               \
	_IOWR(FTV310_VPU_IOCTL_START, 92, struct core_desc *)
#define DEC400_IOCS_DEC_PUSH_REG                                               \
	_IOW(FTV310_VPU_IOCTL_START, 93, struct core_desc *)
#define DEC400_IOCGHWOFFSET _IOWR(FTV310_VPU_IOCTL_START, 94, unsigned long long *)
#define FTV310_VPUDEC400_IOC_END _IO(FTV310_VPU_IOCTL_START, 99)

/* ftv310_vpu mmu related */
#define FTV310_VPUMMU_IOC_START _IO(FTV310_VPU_IOCTL_START, 100)
#define FTV310_VPU_IOCS_MMU_MEM_MAP                                                \
	_IOWR(FTV310_VPU_IOCTL_START, 101, struct addr_desc *)
#define FTV310_VPU_IOCS_MMU_MEM_UNMAP                                              \
	_IOWR(FTV310_VPU_IOCTL_START, 102, struct addr_desc *)
#define FTV310_VPU_IOCS_MMU_FLUSH _IOW(FTV310_VPU_IOCTL_START, 103, struct addr_desc *)
#define FTV310_VPUMMU_IOC_END _IO(FTV310_VPU_IOCTL_START, 109)

#define VCMD_IOC_START _IO(FTV310_VPU_IOCTL_START, 110)
#define VCMD_IOCH_GET_CMDBUF_PARAMETER                                         \
	_IOWR(FTV310_VPU_IOCTL_START, 111, struct cmdbuf_mem_parameter *)
#define VCMD_IOCH_GET_CMDBUF_POOL_SIZE                                         \
	_IOWR(FTV310_VPU_IOCTL_START, 112, unsigned long)
#define VCMD_IOCH_SET_CMDBUF_POOL_BASE                                         \
	_IOWR(FTV310_VPU_IOCTL_START, 113, unsigned long)
#define VCMD_IOCH_GET_VCMD_PARAMETER                                           \
	_IOWR(FTV310_VPU_IOCTL_START, 114, struct vcmd_cfg_par *)
#define VCMD_IOCH_RESERVE_CMDBUF                                               \
	_IOWR(FTV310_VPU_IOCTL_START, 115, struct exchange_parameter *)
#define VCMD_IOCH_LINK_RUN_CMDBUF                                              \
	_IOR(FTV310_VPU_IOCTL_START, 116, struct exchange_parameter *)
#define VCMD_IOCH_WAIT_CMDBUF                                                  \
	_IOR(FTV310_VPU_IOCTL_START, 117, struct cmdbuf_id_parameter *)
#define VCMD_IOCH_RELEASE_CMDBUF                                               \
	_IOR(FTV310_VPU_IOCTL_START, 118, struct cmdbuf_id_parameter *)
#define VCMD_IOCH_POLLING_CMDBUF _IOR(FTV310_VPU_IOCTL_START, 119, unsigned int *)
#define VCMD_IOC_END _IO(FTV310_VPU_IOCTL_START, 124)

#define FTV310_VPU_IOC_AXIFE_CONFIG                                                \
	_IOR(FTV310_VPU_IOCTL_START, 125, struct axife_cfg *)

#endif /* FTV310_VPU_H */
