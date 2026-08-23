/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __UAPI_SHEADER_H__
#define __UAPI_SHEADER_H__

/**
* \file  sHeader.h 
* \brief This file is secure header file used for secure boot.
* \author ECONET
* \date     2020-12-02
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#define I2C_MAGIC   (0x12345678)
#define FLASH_MAGIC (0x23456789)

typedef enum {
	AES_MODE_DISABLE = 0,
	AES_MODE_ECB,
	AES_MODE_MAX
} AES_MODE_T;

typedef struct {
	unsigned char iv[16];
	unsigned char hmac_sha256[32];
} AES_INFO;

typedef struct {
	unsigned int magic;
	unsigned int version;
	unsigned int header_len;
	unsigned int image_len;
	unsigned char sig[512];
	unsigned char rsa_pub[512];
	AES_MODE_T aes_mode;
	unsigned char aes_img_key[256];
	AES_INFO aes_img_key_info;
	union {
		AES_INFO aes_efuse_info;
		AES_INFO aes_bootram_info;
		AES_INFO aes_kernel_info;
	};
	AES_INFO aes_spram_info;
	AES_INFO aes_lzma_info;
	AES_INFO aes_verify_info;
    unsigned int i2c_flash_magic;
	unsigned char resv3[500];
	unsigned int crc;
} SECURE_HEADER_V2;

typedef struct {
	unsigned int magic;
	unsigned int version;
	unsigned char sig[256];
	unsigned int image_len;
	unsigned int resv2;
	unsigned int crc;
} SECURE_HEADER_V1;

typedef struct {
	union {
		SECURE_HEADER_V1 v1;
		SECURE_HEADER_V2 v2;
	};
} SECURE_HEADER;

typedef enum {
	SIGNATURE_VERIFY_STATUS_CORRECT = 0,
	SIGNATURE_VERIFY_INCORRECT
} SIGNATURE_VERIFY_STATUS_T;

typedef struct secure_img{
	SECURE_HEADER *sHeader;
	unsigned char *start;
	unsigned int len;
	unsigned int crc;
} SECURE_IMG_T;

#endif /* __SHEADER_H__ */
