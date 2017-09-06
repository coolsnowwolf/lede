/******************************************************************************
**
** FILE NAME    : ifxmips_deu_ar9.h
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for AR9
**
** DATE         : September 8, 2009
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver
** COPYRIGHT    :       Copyright (c) 2009
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author             $Comment
** 08,Sept 2009 Mohammad Firdaus    Initial UEIP release
*******************************************************************************/
/*!
  \defgroup IFX_DEU IFX_DEU_DRIVERS
  \ingroup API
  \brief deu driver module
*/

/*!
  \defgroup IFX_DEU_DEFINITIONS IFX_DEU_DEFINITIONS
  \ingroup IFX_DEU
  \brief ifx deu definitions
*/

/*!
  \file		ifxmips_deu_ar9.h
  \brief 	deu driver header file
*/


#ifndef IFXMIPS_DEU_AR9_H
#define IFXMIPS_DEU_AR9_H

/* Project Header Files */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <crypto/algapi.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "ifxmips_deu.h"


/* SHA CONSTANTS */
#define HASH_CON_VALUE    0x0700002C

#define INPUT_ENDIAN_SWAP(input)    input_swap(input)
#define DEU_ENDIAN_SWAP(input)    endian_swap(input)
#define DELAY_PERIOD    10 
#define FIND_DEU_CHIP_VERSION    chip_version()
#define CLC_START IFX_DEU_CLK 

#define AES_INIT 0
#define DES_INIT 1
#define ARC4_INIT 2
#define SHA1_INIT 3
#define MD5_INIT 4
#define SHA1_HMAC_INIT 5
#define MD5_HMAC_INIT 6

#define AES_START IFX_AES_CON
#define DES_3DES_START  IFX_DES_CON
				      
#define WAIT_AES_DMA_READY()          \
    do { 			      \
        int i;			      \
        volatile struct deu_dma_t *dma = (struct deu_dma_t *) IFX_DEU_DMA_CON; \
        volatile struct aes_t *aes = (volatile struct aes_t *) AES_START; \
        for (i = 0; i < 10; i++)      \
	    udelay(DELAY_PERIOD);     \
        while (dma->controlr.BSY) {}; \
        while (aes->controlr.BUS) {}; \
    } while (0)

#define WAIT_DES_DMA_READY()          \
    do { 			      \
        int i;			      \
        volatile struct deu_dma_t *dma = (struct deu_dma_t *) IFX_DEU_DMA_CON; \
        volatile struct des_t *des = (struct des_t *) DES_3DES_START; \
        for (i = 0; i < 10; i++)      \
            udelay(DELAY_PERIOD);     \
        while (dma->controlr.BSY) {}; \
        while (des->controlr.BUS) {}; \
    } while (0)

#define AES_DMA_MISC_CONFIG()        \
    do {                             \
        volatile struct aes_t *aes = (volatile struct aes_t *) AES_START; \
        aes->controlr.KRE = 1;        \
        aes->controlr.GO = 1;         \
    } while(0)

#define SHA_HASH_INIT                  \
    do {                               \
        volatile struct deu_hash_t *hash = (struct deu_hash_t *) HASH_START; \
        hash->controlr.SM = 1;    \
        hash->controlr.ALGO = 0;  \
        hash->controlr.INIT = 1;  \
    } while(0)

/* DEU Common Structures for AR9*/
 
struct clc_controlr_t {
	u32 Res:26;
	u32 FSOE:1;
	u32 SBWE:1;
	u32 EDIS:1;
	u32 SPEN:1;
	u32 DISS:1;
	u32 DISR:1;

};

struct des_t {
	struct des_controlr {	//10h
		u32 KRE:1;
		u32 reserved1:5;
		u32 GO:1;
		u32 STP:1;
		u32 Res2:6;
                u32 NDC:1;
                u32 ENDI:1;
                u32 Res3:2;
		u32 F:3;
		u32 O:3;
		u32 BUS:1;
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 M:3;

	} controlr;
	u32 IHR;		//14h
	u32 ILR;		//18h
	u32 K1HR;		//1c
	u32 K1LR;		//
	u32 K2HR;
	u32 K2LR;
	u32 K3HR;
	u32 K3LR;		//30h
	u32 IVHR;		//34h
	u32 IVLR;		//38
	u32 OHR;		//3c
	u32 OLR;		//40
};

struct aes_t {
	struct aes_controlr {

		u32 KRE:1;
		u32 reserved1:4;
		u32 PNK:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
                u32 reserved3:2;
		u32 F:3;	//fbs
		u32 O:3;	//om
		u32 BUS:1;	//bsy
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 KV:1;
		u32 K:2;	//KL

	} controlr;
	u32 ID3R;		//80h
	u32 ID2R;		//84h
	u32 ID1R;		//88h
	u32 ID0R;		//8Ch
	u32 K7R;		//90h
	u32 K6R;		//94h
	u32 K5R;		//98h
	u32 K4R;		//9Ch
	u32 K3R;		//A0h
	u32 K2R;		//A4h
	u32 K1R;		//A8h
	u32 K0R;		//ACh
	u32 IV3R;		//B0h
	u32 IV2R;		//B4h
	u32 IV1R;		//B8h
	u32 IV0R;		//BCh
	u32 OD3R;		//D4h
	u32 OD2R;		//D8h
	u32 OD1R;		//DCh
	u32 OD0R;		//E0h
};

struct arc4_t {
	struct arc4_controlr {

		u32 KRE:1;
		u32 KLEN:4;
		u32 KSAE:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved1:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved2:8;
		u32 BUS:1;	//bsy
		u32 reserved3:1;
		u32 ARS:1;
		u32 SM:1;
		u32 reserved4:4;						

	} controlr;
	u32 K3R;		//104h
	u32 K2R;		//108h
	u32 K1R;		//10Ch
	u32 K0R;		//110h

        u32 IDLEN;		//114h

	u32 ID3R;		//118h
	u32 ID2R;		//11Ch
	u32 ID1R;		//120h
	u32 ID0R;		//124h
	
	u32 OD3R;		//128h
	u32 OD2R;		//12Ch
	u32 OD1R;		//130h
	u32 OD0R;		//134h
};

struct deu_hash_t {
	struct hash_controlr {
		u32 reserved1:5;
		u32 KHS:1;		
		u32 GO:1;
		u32 INIT:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved3:7;
		u32 DGRY:1;		
		u32 BSY:1;
		u32 reserved4:1;
		u32 IRCL:1;
		u32 SM:1;
		u32 KYUE:1;
                u32 HMEN:1;
		u32 SSEN:1;
		u32 ALGO:1;

	} controlr;
	u32 MR;			//B4h
	u32 D1R;		//B8h
	u32 D2R;		//BCh
	u32 D3R;		//C0h
	u32 D4R;		//C4h
	u32 D5R;		//C8h

	u32 dummy;		//CCh

	u32 KIDX;		//D0h
	u32 KEY;		//D4h
	u32 DBN;		//D8h
};


struct deu_dma_t {
	struct dma_controlr {
		u32 reserved1:22;
		u32 BS:2;
		u32 BSY:1;
		u32 reserved2:1;
		u32 ALGO:2;
		u32 RXCLS:2;
		u32 reserved3:1;
		u32 EN:1;

	} controlr;
};

#endif /* IFXMIPS_DEU_AR9_H */
