/******************************************************************************
**
** FILE NAME    : ifxmips_aes.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module
**
** DATE         : September 8, 2009
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver for AES Algorithm
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
 \brief ifx DEU driver module
*/

/*!
  \file	ifxmips_aes.c
  \ingroup IFX_DEU
  \brief AES Encryption Driver main file
*/

/*!
 \defgroup IFX_AES_FUNCTIONS IFX_AES_FUNCTIONS
 \ingroup IFX_DEU
 \brief IFX AES driver Functions 
*/


/* Project Header Files */
#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modeversions>
#endif

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <crypto/algapi.h>

#include "ifxmips_deu.h"

#if defined(CONFIG_DANUBE) 
#include "ifxmips_deu_danube.h"
extern int ifx_danube_pre_1_4;
#elif defined(CONFIG_AR9)
#include "ifxmips_deu_ar9.h"
#elif defined(CONFIG_VR9) || defined(CONFIG_AR10)
#include "ifxmips_deu_vr9.h"
#else
#error "Unkown platform"
#endif

/* DMA related header and variables */

spinlock_t aes_lock;
#define CRTCL_SECT_INIT        spin_lock_init(&aes_lock)
#define CRTCL_SECT_START       spin_lock_irqsave(&aes_lock, flag)
#define CRTCL_SECT_END         spin_unlock_irqrestore(&aes_lock, flag)

/* Definition of constants */
#define AES_START   IFX_AES_CON
#define AES_MIN_KEY_SIZE    16
#define AES_MAX_KEY_SIZE    32
#define AES_BLOCK_SIZE      16
#define CTR_RFC3686_NONCE_SIZE    4
#define CTR_RFC3686_IV_SIZE       8
#define CTR_RFC3686_MAX_KEY_SIZE  (AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE)

#ifdef CRYPTO_DEBUG
extern char debug_level;
#define DPRINTF(level, format, args...) if (level < debug_level) printk(KERN_INFO "[%s %s %d]: " format, __FILE__, __func__, __LINE__, ##args);
#else
#define DPRINTF(level, format, args...)
#endif /* CRYPTO_DEBUG */

/* Function decleration */
int aes_chip_init(void);
u32 endian_swap(u32 input);
u32 input_swap(u32 input);
u32* memory_alignment(const u8 *arg, u32 *buff_alloc, int in_out, int nbytes);
void aes_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
void des_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
int aes_memory_allocate(int value);
int des_memory_allocate(int value);
void memory_release(u32 *addr); 


extern void ifx_deu_aes (void *ctx_arg, uint8_t *out_arg, const uint8_t *in_arg,
        uint8_t *iv_arg, size_t nbytes, int encdec, int mode);
/* End of function decleration */

struct aes_ctx {
    int key_length;
    u32 buf[AES_MAX_KEY_SIZE];
    u8 nonce[CTR_RFC3686_NONCE_SIZE];
};

extern int disable_deudma;
extern int disable_multiblock; 

/*! \fn int aes_set_key (struct crypto_tfm *tfm, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS 
 *  \brief sets the AES keys    
 *  \param tfm linux crypto algo transform  
 *  \param in_key input key  
 *  \param key_len key lengths of 16, 24 and 32 bytes supported  
 *  \return -EINVAL - bad key length, 0 - SUCCESS
*/                                 
int aes_set_key (struct crypto_tfm *tfm, const u8 *in_key, unsigned int key_len)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
    unsigned long *flags = (unsigned long *) &tfm->crt_flags;

    //printk("set_key in %s\n", __FILE__);

    //aes_chip_init();

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        *flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    ctx->key_length = key_len;
    DPRINTF(0, "ctx @%p, key_len %d, ctx->key_length %d\n", ctx, key_len, ctx->key_length);
    memcpy ((u8 *) (ctx->buf), in_key, key_len);

    return 0;
}


/*! \fn void ifx_deu_aes (void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, size_t nbytes, int encdec, int mode)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief main interface to AES hardware
 *  \param ctx_arg crypto algo context  
 *  \param out_arg output bytestream  
 *  \param in_arg input bytestream   
 *  \param iv_arg initialization vector  
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param mode operation mode such as ebc, cbc, ctr  
 *
*/                                 
void ifx_deu_aes (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
        u8 *iv_arg, size_t nbytes, int encdec, int mode)

{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    volatile struct aes_t *aes = (volatile struct aes_t *) AES_START;
    struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
    u32 *in_key = ctx->buf;
    unsigned long flag;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    int key_len = ctx->key_length;

    int i = 0;
    int byte_cnt = nbytes; 


    CRTCL_SECT_START;
    /* 128, 192 or 256 bit key length */
    aes->controlr.K = key_len / 8 - 2;
        if (key_len == 128 / 8) {
        aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
        aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
        aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
        aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
    }
    else if (key_len == 192 / 8) {
        aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
        aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
        aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
        aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
        aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
        aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
    }
    else if (key_len == 256 / 8) {
        aes->K7R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
        aes->K6R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
        aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
        aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
        aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
        aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
        aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 6));
        aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 7));
    }
    else {
        printk (KERN_ERR "[%s %s %d]: Invalid key_len : %d\n", __FILE__, __func__, __LINE__, key_len);
        CRTCL_SECT_END;
        return;// -EINVAL;
    }

    /* let HW pre-process DEcryption key in any case (even if
       ENcryption is used). Key Valid (KV) bit is then only
       checked in decryption routine! */
    aes->controlr.PNK = 1;


    aes->controlr.E_D = !encdec;    //encryption
    aes->controlr.O = mode; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR 

    //aes->controlr.F = 128; //default; only for CFB and OFB modes; change only for customer-specific apps
    if (mode > 0) {
        aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
        aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
        aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
    };


    i = 0;
    while (byte_cnt >= 16) {

        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 3));    /* start crypto */
        
        while (aes->controlr.BUS) {
            // this will not take long
        }

        *((volatile u32 *) out_arg + (i * 4) + 0) = aes->OD3R;
        *((volatile u32 *) out_arg + (i * 4) + 1) = aes->OD2R;
        *((volatile u32 *) out_arg + (i * 4) + 2) = aes->OD1R;
        *((volatile u32 *) out_arg + (i * 4) + 3) = aes->OD0R;

        i++;
        byte_cnt -= 16;
    }

    /* To handle all non-aligned bytes (not aligned to 16B size) */
    if (byte_cnt) {
        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) in_arg + (i * 4) + 3));    /* start crypto */

        while (aes->controlr.BUS) {
        }

        *((volatile u32 *) out_arg + (i * 4) + 0) = aes->OD3R;
        *((volatile u32 *) out_arg + (i * 4) + 1) = aes->OD2R;
        *((volatile u32 *) out_arg + (i * 4) + 2) = aes->OD1R;
        *((volatile u32 *) out_arg + (i * 4) + 3) = aes->OD0R;

        /* to ensure that the extended pages are clean */
        memset (out_arg + (i * 16) + (nbytes % AES_BLOCK_SIZE), 0,
                (AES_BLOCK_SIZE - (nbytes % AES_BLOCK_SIZE)));

    }

    //tc.chen : copy iv_arg back
    if (mode > 0) {
        *((u32 *) iv_arg) = DEU_ENDIAN_SWAP(aes->IV3R);
        *((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(aes->IV2R);
        *((u32 *) iv_arg + 2) = DEU_ENDIAN_SWAP(aes->IV1R);
        *((u32 *) iv_arg + 3) = DEU_ENDIAN_SWAP(aes->IV0R);
    }

    CRTCL_SECT_END;
}

/*!
 *  \fn int ctr_rfc3686_aes_set_key (struct crypto_tfm *tfm, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets RFC3686 key   
 *  \param tfm linux crypto algo transform  
 *  \param in_key input key  
 *  \param key_len key lengths of 20, 28 and 36 bytes supported; last 4 bytes is nonce 
 *  \return 0 - SUCCESS
 *          -EINVAL - bad key length
*/                                 
int ctr_rfc3686_aes_set_key (struct crypto_tfm *tfm, const uint8_t *in_key, unsigned int key_len)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
    unsigned long *flags = (unsigned long *)&tfm->crt_flags;

    //printk("ctr_rfc3686_aes_set_key in %s\n", __FILE__);

    memcpy(ctx->nonce, in_key + (key_len - CTR_RFC3686_NONCE_SIZE),
           CTR_RFC3686_NONCE_SIZE);

    key_len -= CTR_RFC3686_NONCE_SIZE; // remove 4 bytes of nonce

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        *flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    ctx->key_length = key_len;
    
    memcpy ((u8 *) (ctx->buf), in_key, key_len);

    return 0;
}

/*! \fn void ifx_deu_aes (void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, u32 nbytes, int encdec, int mode)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief main interface with deu hardware in DMA mode
 *  \param ctx_arg crypto algo context 
 *  \param out_arg output bytestream   
 *  \param in_arg input bytestream   
 *  \param iv_arg initialization vector  
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param mode operation mode such as ebc, cbc, ctr  
*/


//definitions from linux/include/crypto.h:
//#define CRYPTO_TFM_MODE_ECB       0x00000001
//#define CRYPTO_TFM_MODE_CBC       0x00000002
//#define CRYPTO_TFM_MODE_CFB       0x00000004
//#define CRYPTO_TFM_MODE_CTR       0x00000008
//#define CRYPTO_TFM_MODE_OFB       0x00000010 // not even defined
//but hardware definition: 0 ECB 1 CBC 2 OFB 3 CFB 4 CTR

/*! \fn void ifx_deu_aes_ecb (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets AES hardware to ECB mode   
 *  \param ctx crypto algo context  
 *  \param dst output bytestream  
 *  \param src input bytestream  
 *  \param iv initialization vector   
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param inplace not used  
*/                                 
void ifx_deu_aes_ecb (void *ctx, uint8_t *dst, const uint8_t *src,
        uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
     ifx_deu_aes (ctx, dst, src, NULL, nbytes, encdec, 0);
}

/*! \fn void ifx_deu_aes_cbc (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets AES hardware to CBC mode   
 *  \param ctx crypto algo context  
 *  \param dst output bytestream  
 *  \param src input bytestream  
 *  \param iv initialization vector   
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param inplace not used  
*/                                 
void ifx_deu_aes_cbc (void *ctx, uint8_t *dst, const uint8_t *src,
        uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
     ifx_deu_aes (ctx, dst, src, iv, nbytes, encdec, 1);
}

/*! \fn void ifx_deu_aes_ofb (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets AES hardware to OFB mode   
 *  \param ctx crypto algo context  
 *  \param dst output bytestream  
 *  \param src input bytestream  
 *  \param iv initialization vector   
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param inplace not used  
*/                                 
void ifx_deu_aes_ofb (void *ctx, uint8_t *dst, const uint8_t *src,
        uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
     ifx_deu_aes (ctx, dst, src, iv, nbytes, encdec, 2);
}

/*! \fn void ifx_deu_aes_cfb (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets AES hardware to CFB mode   
 *  \param ctx crypto algo context  
 *  \param dst output bytestream  
 *  \param src input bytestream  
 *  \param iv initialization vector   
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param inplace not used  
*/                                 
void ifx_deu_aes_cfb (void *ctx, uint8_t *dst, const uint8_t *src,
        uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
     ifx_deu_aes (ctx, dst, src, iv, nbytes, encdec, 3);
}

/*! \fn void ifx_deu_aes_ctr (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets AES hardware to CTR mode   
 *  \param ctx crypto algo context  
 *  \param dst output bytestream  
 *  \param src input bytestream  
 *  \param iv initialization vector   
 *  \param nbytes length of bytestream  
 *  \param encdec 1 for encrypt; 0 for decrypt  
 *  \param inplace not used  
*/                                 
void ifx_deu_aes_ctr (void *ctx, uint8_t *dst, const uint8_t *src,
        uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
     ifx_deu_aes (ctx, dst, src, iv, nbytes, encdec, 4);
}

/*! \fn void aes_encrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief encrypt AES_BLOCK_SIZE of data   
 *  \param tfm linux crypto algo transform  
 *  \param out output bytestream  
 *  \param in input bytestream  
*/                                 
void aes_encrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
    ifx_deu_aes (ctx, out, in, NULL, AES_BLOCK_SIZE,
            CRYPTO_DIR_ENCRYPT, 0);
}

/*! \fn void aes_decrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief decrypt AES_BLOCK_SIZE of data   
 *  \param tfm linux crypto algo transform  
 *  \param out output bytestream  
 *  \param in input bytestream  
*/                                 
void aes_decrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
    ifx_deu_aes (ctx, out, in, NULL, AES_BLOCK_SIZE,
            CRYPTO_DIR_DECRYPT, 0);
}

/* 
 * \brief AES function mappings 
*/
struct crypto_alg ifxdeu_aes_alg = {
    .cra_name       =   "aes",
    .cra_driver_name    =   "ifxdeu-aes",
    .cra_priority   =   300,
    .cra_flags      =   CRYPTO_ALG_TYPE_CIPHER,
    .cra_blocksize      =   AES_BLOCK_SIZE,
    .cra_ctxsize        =   sizeof(struct aes_ctx),
    .cra_module     =   THIS_MODULE,
    .cra_list       =   LIST_HEAD_INIT(ifxdeu_aes_alg.cra_list),
    .cra_u          =   {
        .cipher = {
            .cia_min_keysize    =   AES_MIN_KEY_SIZE,
            .cia_max_keysize    =   AES_MAX_KEY_SIZE,
            .cia_setkey     =   aes_set_key,
            .cia_encrypt        =   aes_encrypt,
            .cia_decrypt        =   aes_decrypt,
        }
    }
};

/*! \fn int ecb_aes_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief ECB AES encrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ecb_aes_encrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int enc_bytes;
    
    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = enc_bytes = walk.nbytes)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       NULL, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
                nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/*! \fn int ecb_aes_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief ECB AES decrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ecb_aes_decrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int dec_bytes;

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = dec_bytes = walk.nbytes)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       NULL, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/* 
 * \brief AES function mappings
*/
struct crypto_alg ifxdeu_ecb_aes_alg = {
    .cra_name       =   "ecb(aes)",
    .cra_driver_name    =   "ifxdeu-ecb(aes)",
    .cra_priority   =   400,
    .cra_flags      =   CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize      =   AES_BLOCK_SIZE,
    .cra_ctxsize        =   sizeof(struct aes_ctx),
    .cra_type       =   &crypto_blkcipher_type,
    .cra_module     =   THIS_MODULE,
    .cra_list       =   LIST_HEAD_INIT(ifxdeu_ecb_aes_alg.cra_list),
    .cra_u          =   {
        .blkcipher = {
            .min_keysize        =   AES_MIN_KEY_SIZE,
            .max_keysize        =   AES_MAX_KEY_SIZE,
            .setkey         =   aes_set_key,
            .encrypt        =   ecb_aes_encrypt,
            .decrypt        =   ecb_aes_decrypt,
        }
    }
};


/*! \fn int cbc_aes_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CBC AES encrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int cbc_aes_encrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int enc_bytes;

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = enc_bytes = walk.nbytes)) {
            u8 *iv = walk.iv;
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/*! \fn int cbc_aes_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CBC AES decrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int cbc_aes_decrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int dec_bytes;

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = dec_bytes = walk.nbytes)) {
        u8 *iv = walk.iv;
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct crypto_alg ifxdeu_cbc_aes_alg = {
    .cra_name       =   "cbc(aes)",
    .cra_driver_name    =   "ifxdeu-cbc(aes)",
    .cra_priority   =   400,
    .cra_flags      =   CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize      =   AES_BLOCK_SIZE,
    .cra_ctxsize        =   sizeof(struct aes_ctx),
    .cra_type       =   &crypto_blkcipher_type,
    .cra_module     =   THIS_MODULE,
    .cra_list       =   LIST_HEAD_INIT(ifxdeu_cbc_aes_alg.cra_list),
    .cra_u          =   {
        .blkcipher = {
            .min_keysize        =   AES_MIN_KEY_SIZE,
            .max_keysize        =   AES_MAX_KEY_SIZE,
            .ivsize         =   AES_BLOCK_SIZE,
            .setkey         =   aes_set_key,
            .encrypt        =   cbc_aes_encrypt,
            .decrypt        =   cbc_aes_decrypt,
        }
    }
};


/*! \fn int ctr_basic_aes_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES encrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ctr_basic_aes_encrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int enc_bytes;

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = enc_bytes = walk.nbytes)) {
            u8 *iv = walk.iv;
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/*! \fn  int ctr_basic_aes_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES decrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ctr_basic_aes_decrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;
    unsigned int dec_bytes;

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    while ((nbytes = dec_bytes = walk.nbytes)) {
        u8 *iv = walk.iv;
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    return err;
}

/* 
 * \brief AES function mappings
*/
struct crypto_alg ifxdeu_ctr_basic_aes_alg = {
    .cra_name       =   "ctr(aes)",
    .cra_driver_name    =   "ifxdeu-ctr(aes)",
    .cra_priority   =   400,
    .cra_flags      =   CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize      =   AES_BLOCK_SIZE,
    .cra_ctxsize        =   sizeof(struct aes_ctx),
    .cra_type       =   &crypto_blkcipher_type,
    .cra_module     =   THIS_MODULE,
    .cra_list       =   LIST_HEAD_INIT(ifxdeu_ctr_basic_aes_alg.cra_list),
    .cra_u          =   {
        .blkcipher = {
            .min_keysize        =   AES_MIN_KEY_SIZE,
            .max_keysize        =   AES_MAX_KEY_SIZE,
            .ivsize         =   AES_BLOCK_SIZE,
            .setkey         =   aes_set_key,
            .encrypt        =   ctr_basic_aes_encrypt,
            .decrypt        =   ctr_basic_aes_decrypt,
        }
    }
};


/*! \fn  int ctr_rfc3686_aes_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES (rfc3686) encrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ctr_rfc3686_aes_encrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err, bsize = nbytes;
    u8 rfc3686_iv[16];

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);
    
    /* set up counter block */
    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE); 
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    /* scatterlist source is the same size as request size, just process once */
    if (nbytes == walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);
	nbytes -= walk.nbytes;
	err = blkcipher_walk_done(desc, &walk, nbytes);
	return err;
    }

    while ((nbytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);

	nbytes -= walk.nbytes;
	bsize -= walk.nbytes;
	err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
	err = blkcipher_walk_done(desc, &walk, 0);
    }
   
    return err;
}

/*! \fn int ctr_rfc3686_aes_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES (rfc3686) decrypt using linux crypto blkcipher    
 *  \param desc blkcipher descriptor  
 *  \param dst output scatterlist  
 *  \param src input scatterlist  
 *  \param nbytes data size in bytes  
 *  \return err
*/                                 
int ctr_rfc3686_aes_decrypt(struct blkcipher_desc *desc,
               struct scatterlist *dst, struct scatterlist *src,
               unsigned int nbytes)
{
    struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err, bsize = nbytes;
    u8 rfc3686_iv[16];

    blkcipher_walk_init(&walk, dst, src, nbytes);
    err = blkcipher_walk_virt(desc, &walk);

    /* set up counter block */
    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE); 
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    /* scatterlist source is the same size as request size, just process once */
    if (nbytes == walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);
	nbytes -= walk.nbytes;
	err = blkcipher_walk_done(desc, &walk, nbytes);
	return err;
    }

    while ((nbytes = walk.nbytes) % (walk.nbytes >= AES_BLOCK_SIZE)) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, nbytes, CRYPTO_DIR_DECRYPT, 0);

	nbytes -= walk.nbytes;
	bsize -= walk.nbytes;
	err = blkcipher_walk_done(desc, &walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
	err = blkcipher_walk_done(desc, &walk, 0);
    }

    return err;
}

/* 
 * \brief AES function mappings
*/
struct crypto_alg ifxdeu_ctr_rfc3686_aes_alg = {
    .cra_name       	=   "rfc3686(ctr(aes))",
    .cra_driver_name    =   "ifxdeu-ctr-rfc3686(aes)",
    .cra_priority       =   400,
    .cra_flags      	=   CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize      =   AES_BLOCK_SIZE,
    .cra_ctxsize        =   sizeof(struct aes_ctx),
    .cra_type       	=   &crypto_blkcipher_type,
    .cra_module     	=   THIS_MODULE,
    .cra_list       	=   LIST_HEAD_INIT(ifxdeu_ctr_rfc3686_aes_alg.cra_list),
    .cra_u          =   {
        .blkcipher = {
            .min_keysize        =   AES_MIN_KEY_SIZE,
            .max_keysize        =   CTR_RFC3686_MAX_KEY_SIZE,
            .ivsize         =   CTR_RFC3686_IV_SIZE,
            .setkey         =   ctr_rfc3686_aes_set_key,
            .encrypt        =   ctr_rfc3686_aes_encrypt,
            .decrypt        =   ctr_rfc3686_aes_decrypt,
        }
    }
};


/*! \fn int __init ifxdeu_init_aes (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief function to initialize AES driver   
 *  \return ret 
*/                                 
int __init ifxdeu_init_aes (void)
{
    int ret = -ENOSYS;


 
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
    if (!disable_multiblock) {
        ifxdeu_aes_alg.cra_u.cipher.cia_max_nbytes = AES_BLOCK_SIZE;    //(size_t)-1;
        ifxdeu_aes_alg.cra_u.cipher.cia_req_align = 16;
        ifxdeu_aes_alg.cra_u.cipher.cia_ecb = ifx_deu_aes_ecb;
        ifxdeu_aes_alg.cra_u.cipher.cia_cbc = ifx_deu_aes_cbc;
        ifxdeu_aes_alg.cra_u.cipher.cia_cfb = ifx_deu_aes_cfb;
        ifxdeu_aes_alg.cra_u.cipher.cia_ofb = ifx_deu_aes_ofb;
    }
#endif

    if ((ret = crypto_register_alg(&ifxdeu_aes_alg)))
        goto aes_err;

    if ((ret = crypto_register_alg(&ifxdeu_ecb_aes_alg)))
        goto ecb_aes_err;

    if ((ret = crypto_register_alg(&ifxdeu_cbc_aes_alg)))
        goto cbc_aes_err;

    if ((ret = crypto_register_alg(&ifxdeu_ctr_basic_aes_alg)))
        goto ctr_basic_aes_err;

    if ((ret = crypto_register_alg(&ifxdeu_ctr_rfc3686_aes_alg)))
        goto ctr_rfc3686_aes_err;

    aes_chip_init ();

    CRTCL_SECT_INIT;


    printk (KERN_NOTICE "IFX DEU AES initialized%s%s.\n", disable_multiblock ? "" : " (multiblock)", disable_deudma ? "" : " (DMA)");
    return ret;

ctr_rfc3686_aes_err:
    crypto_unregister_alg(&ifxdeu_ctr_rfc3686_aes_alg);
    printk (KERN_ERR "IFX ctr_rfc3686_aes initialization failed!\n");
    return ret;
ctr_basic_aes_err:
    crypto_unregister_alg(&ifxdeu_ctr_basic_aes_alg);
    printk (KERN_ERR "IFX ctr_basic_aes initialization failed!\n");
    return ret;
cbc_aes_err:
    crypto_unregister_alg(&ifxdeu_cbc_aes_alg);
    printk (KERN_ERR "IFX cbc_aes initialization failed!\n");
    return ret;
ecb_aes_err:
    crypto_unregister_alg(&ifxdeu_ecb_aes_alg);
    printk (KERN_ERR "IFX aes initialization failed!\n");
    return ret;
aes_err:
    printk(KERN_ERR "IFX DEU AES initialization failed!\n");

    return ret;
}

/*! \fn void __exit ifxdeu_fini_aes (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief unregister aes driver   
*/                                 
void __exit ifxdeu_fini_aes (void)
{
    crypto_unregister_alg (&ifxdeu_aes_alg);
    crypto_unregister_alg (&ifxdeu_ecb_aes_alg);
    crypto_unregister_alg (&ifxdeu_cbc_aes_alg);
    crypto_unregister_alg (&ifxdeu_ctr_basic_aes_alg);
    crypto_unregister_alg (&ifxdeu_ctr_rfc3686_aes_alg);

}


