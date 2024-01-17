/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef _PHYTIUM_JPEG_REG_H
#define _PHYTIUM_JPEG_REG_H

#include <linux/bits.h>
/* define the all kinds of control registers in a JPEG soc */

/* The register is used to set the information of the video comes from main memory */
#define SRC_DDR_INFO_REG	0x00000800

/* The register is used to get the information of the video comes from external VGA */
#define SRC_VGA_INFO_REG	0x00000894

#define SRC_FORMAT		BIT(0)		 /* 0:RGB888, 1:RGB565 */
#define SRC_DE_POLARITY		BIT(1)		 /* 0:low level is effect, other */
#define SRC_HS_POLARITY		BIT(2)		 /* 0:low level is effect, other */
#define SRC_VS_POLARITY		BIT(3)		 /* 0:low level is effect, other */
#define SRC_HOR_PIXELS		GENMASK(15, 4)	 /* the number of the horizontal pixels */
#define SRC_WIDTH_SHIFT		4		 /* shift right to get width */
#define SRC_VER_PIXELS		GENMASK(26, 16)	 /* the number of the vertical pixels */
#define SRC_HEIGHT_SHIFT	16		 /* shift right to get height */
/* The below bit fields is only used by image comes from main memory */
#define SRC_COMP_DDR_IMG_EN	BIT(27)		 /* 0: disable to JPEG compression, others */

/* marks which ocm buffer is occupied to store an image */
#define SRC_DDR_IMG2OCM_VALID	GENMASK(29, 28)

/* The register controls starting work of the JPEG engine */
#define TRANSFORM_INFO_REG	0x00000804
#define TRANSINFO_ENABLE_ENGINE	BIT(0)	 /* 1: enable the JPEG engine */
/* 1: video comes from external VGA, 0: video comes from DDR */
#define TRANSINFO_SRC_SELECT	BIT(1)
/* 0: video comes from external VGA is cached to OCM, 1: DDR */
#define TRANSINFO_IMAGE_STORE	BIT(2)
#define TRANSINFO_FRAME_RATE	GENMASK(9, 4)    /* the value notes frame rate */
#define TRANSINFO_BLOCK_SIZE	BIT(12)          /* 0: 8x8, 1: 16x16 */
#define TRANSINFO_ENABLE_YUV422	BIT(13)          /* 1: JPEG block is populated YUV422 */
/* support burst with the values such as 1, 2, 4, 8, 16, 32, 64. using default value 0xf */
#define TRANSINFO_AXI_LEN	GENMASK(22, 16)
#define TRANS_AXI_LEN_SHIFT	16

/* The interrupt and status register */
#define INT_STATUS_CTRL_REG		0x00000808
#define INT_FIFO_OVERFLOW		BIT(0)	/* video fifo overflow, write 1 to clear */
#define INT_OCM_BUF_OVERFLOW		BIT(1)	/* ocm buffer overflow, write 1 to clear */
/* JPEG engine complete compression, write 1 to clear */
#define INT_JPEG_ENCODE_COMPLETE	BIT(2)
/* in VGA mode, video's format is changed */
#define INT_VIDEO_FORMAT_CHANGE		BIT(3)
/* enable the interrupt of th video fifo overflow and source resolution */
#define DETECT_RESOLUTION_CHANGE_EN	BIT(4)
/* enable the interrupt of the ocm buffer overflow */
#define STS_VE_OCM_BUF_OVERFLOW_EN	BIT(5)
/* enable the interrupt of the JPEG complete compression */
#define STS_VE_JPEG_CODE_COMP_EN	BIT(6)
/* in VGA mode, the bit notes ocm buff is busy */
#define STS_VE_OCM_BUF_BUSY		BIT(7)
/* in VGA mode, the bit notes sequence number of the frame */
#define STS_VE_CUR_FRAME_NUMBER		GENMASK(9, 8)
/* in VGA mode, the bit notes sequence number of the cached frame */
#define STS_VE_BUF_CACHE_NUMBER		GENMASK(11, 10)
/* in VGA mode, the buffer number in buffer list */
#define STS_JPEG_COMP_BUF_NO		GENMASK(15, 12)
#define INT_JPEG_COMP_BUF_LIST_NO	GENMASK(31, 16)	/* the interrupt number of the buffer */

#define OCM_BUF0_ADDR	0x0000080C
#define OCM_BUF1_ADDR	0x00000810
#define OCM_BUF_SHIFT	8

#define BUF_LIST_BASE_ADDR	0x00000814

#define PHYTIUM_BUF_LIST_ACTRL_AND_STS_BASE_ADDR_REG	0x00000818
#define STS_JPEG_BUF_HIGH_LEVEL_VALID		BIT(0)		/* Hight levle is validity */
#define JPEG_BUF_CAPACITY_SIZE			GENMASK(29, 8)	/* the capacity of the  buffer */
#define JPEG_BUF_CAPACITY_SIZE_SHIFT		8

/* There are 16 buffers in the buffer list, the width between each other' address is 8 bytes */
#define BUF_LIST_ADDR_OFFSET		0x8
#define BUF_LIST_CTRL_AND_STS_OFFSET	0x8

/* Get the address of the specific index buffer */
#define BUF_LIST_INDEX_ADDR(index)  \
	(BUF_LIST_BASE_ADDR + (index) * BUF_LIST_ADDR_OFFSET)

#define JPEG_DST_ADDR_SHIFT    8

#define BUF_LIST_INDEX_CTRL_STS_ADDR(index)  \
	(PHYTIUM_BUF_LIST_ACTRL_AND_STS_BASE_ADDR_REG + (index) * BUF_LIST_CTRL_AND_STS_OFFSET)

#define FRAME_SAMPLE_CTRL	0x00000898
#define FRAME_SAMPLE_CTRL_EN	BIT(31)
#define FRAME_SAMPLE_INTERVAL	GENMASK(30, 0)

/* The below registers are all related to quantilize */
#define HUFF_MODE_REG		0x300
#define SAMPLE_MODE_REG		0x304

#define Y_QUANT_BASE_ADDR_REG	0x400
#define C_QUANT_BASE_ADDR_REG	0x500

#define QUANT_REG_NUM		64

#define Y_QUANT_INDEX_ADDR_REG(index)   \
	(Y_QUANT_BASE_ADDR_REG + 4 * (index))

#define C_QUANT_INDEX_ADDR_REG(index)   \
	(C_QUANT_BASE_ADDR_REG + 4 * (index))

#endif  /* _PHYTIUM_JPEG_REG_H */
