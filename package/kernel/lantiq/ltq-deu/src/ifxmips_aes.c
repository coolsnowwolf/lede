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
#include <crypto/b128ops.h>
#include <crypto/gcm.h>
#include <crypto/gf128mul.h>
#include <crypto/scatterwalk.h>
#include <crypto/xts.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/skcipher.h>

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
#define AES_BLOCK_WORDS     4
#define CTR_RFC3686_NONCE_SIZE    4
#define CTR_RFC3686_IV_SIZE       8
#define CTR_RFC3686_MIN_KEY_SIZE  (AES_MIN_KEY_SIZE + CTR_RFC3686_NONCE_SIZE)
#define CTR_RFC3686_MAX_KEY_SIZE  (AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE)
#define AES_CBCMAC_DBN_TEMP_SIZE  128

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
    u8 buf[AES_MAX_KEY_SIZE];
    u8 tweakkey[AES_MAX_KEY_SIZE];
    u8 nonce[CTR_RFC3686_NONCE_SIZE];
    u8 lastbuffer[4 * XTS_BLOCK_SIZE];
    int use_tweak;
    u32 byte_count;
    u32 dbn;
    int started;
    u32 (*temp)[AES_BLOCK_WORDS];
    u8 block[AES_BLOCK_SIZE];
    u8 hash[AES_BLOCK_SIZE];
    struct gf128mul_4k *gf128;
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

    //printk("set_key in %s\n", __FILE__);

    //aes_chip_init();

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        return -EINVAL;
    }

    ctx->key_length = key_len;
    ctx->use_tweak = 0;
    DPRINTF(0, "ctx @%p, key_len %d, ctx->key_length %d\n", ctx, key_len, ctx->key_length);
    memcpy ((u8 *) (ctx->buf), in_key, key_len);

    return 0;
}


/*! \fn int aes_set_key_skcipher (struct crypto_skcipher *tfm, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets the AES keys for skcipher
 *  \param tfm linux crypto skcipher
 *  \param in_key input key
 *  \param key_len key lengths of 16, 24 and 32 bytes supported
 *  \return -EINVAL - bad key length, 0 - SUCCESS
*/
int aes_set_key_skcipher (struct crypto_skcipher *tfm, const u8 *in_key, unsigned int key_len)
{
    return aes_set_key(crypto_skcipher_tfm(tfm), in_key, key_len);
}


/*! \fn void aes_set_key_skcipher (void *ctx_arg)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets the AES key to the hardware, requires spinlock to be set by caller
 *  \param ctx_arg crypto algo context  
 *  \return
*/
void aes_set_key_hw (void *ctx_arg)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    volatile struct aes_t *aes = (volatile struct aes_t *) AES_START;
    struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
    u8 *in_key = ctx->buf;
    int key_len = ctx->key_length;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    if (ctx->use_tweak) in_key = ctx->tweakkey;

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
        return; //-EINVAL;
    }

    /* let HW pre-process DEcryption key in any case (even if
       ENcryption is used). Key Valid (KV) bit is then only
       checked in decryption routine! */
    aes->controlr.PNK = 1;

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
    unsigned long flag;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    int i = 0;
    int byte_cnt = nbytes; 

    CRTCL_SECT_START;

    aes_set_key_hw (ctx_arg);

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
        u8 temparea[16] = {0,};

        memcpy(temparea, ((u32 *) in_arg + (i * 4)), byte_cnt);

        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) temparea + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) temparea + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) temparea + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) temparea + 3));    /* start crypto */

        while (aes->controlr.BUS) {
        }

        *((volatile u32 *) temparea + 0) = aes->OD3R;
        *((volatile u32 *) temparea + 1) = aes->OD2R;
        *((volatile u32 *) temparea + 2) = aes->OD1R;
        *((volatile u32 *) temparea + 3) = aes->OD0R;

        memcpy(((u32 *) out_arg + (i * 4)), temparea, byte_cnt);
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

    //printk("ctr_rfc3686_aes_set_key in %s\n", __FILE__);

    memcpy(ctx->nonce, in_key + (key_len - CTR_RFC3686_NONCE_SIZE),
           CTR_RFC3686_NONCE_SIZE);

    key_len -= CTR_RFC3686_NONCE_SIZE; // remove 4 bytes of nonce

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        return -EINVAL;
    }

    ctx->key_length = key_len;
    ctx->use_tweak = 0;
    
    memcpy ((u8 *) (ctx->buf), in_key, key_len);

    return 0;
}

/*!
 *  \fn int ctr_rfc3686_aes_set_key_skcipher (struct crypto_skcipher *tfm, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets RFC3686 key for skcipher
 *  \param tfm linux crypto skcipher
 *  \param in_key input key
 *  \param key_len key lengths of 20, 28 and 36 bytes supported; last 4 bytes is nonce
 *  \return 0 - SUCCESS
 *          -EINVAL - bad key length
*/
int ctr_rfc3686_aes_set_key_skcipher (struct crypto_skcipher *tfm, const uint8_t *in_key, unsigned int key_len)
{
    return ctr_rfc3686_aes_set_key(crypto_skcipher_tfm(tfm), in_key, key_len);
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
    .cra_flags      =   CRYPTO_ALG_TYPE_CIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
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

/*! \fn int ecb_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief ECB AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ecb_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = enc_bytes = walk.nbytes)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       NULL, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
                nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    return err;
}

/*! \fn int ecb_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief ECB AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ecb_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = dec_bytes = walk.nbytes)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       NULL, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_ecb_aes_alg = {
    .base.cra_name           =   "ecb(aes)",
    .base.cra_driver_name    =   "ifxdeu-ecb(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   AES_BLOCK_SIZE,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_ecb_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE,
    .max_keysize             =   AES_MAX_KEY_SIZE,
    .setkey                  =   aes_set_key_skcipher,
    .encrypt                 =   ecb_aes_encrypt,
    .decrypt                 =   ecb_aes_decrypt,
};

/*! \fn int ecb_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CBC AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int cbc_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = enc_bytes = walk.nbytes)) {
            u8 *iv = walk.iv;
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    return err;
}

/*! \fn int cbc_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CBC AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int cbc_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = dec_bytes = walk.nbytes)) {
        u8 *iv = walk.iv;
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_cbc_aes_alg = {
    .base.cra_name           =   "cbc(aes)",
    .base.cra_driver_name    =   "ifxdeu-cbc(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   AES_BLOCK_SIZE,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_cbc_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE,
    .max_keysize             =   AES_MAX_KEY_SIZE,
    .ivsize                  =   AES_BLOCK_SIZE,
    .setkey                  =   aes_set_key_skcipher,
    .encrypt                 =   cbc_aes_encrypt,
    .decrypt                 =   cbc_aes_decrypt,
};

/*! \fn void ifx_deu_aes_xts (void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, size_t nbytes, int encdec)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief main interface to AES hardware for XTS impl
 *  \param ctx_arg crypto algo context
 *  \param out_arg output bytestream
 *  \param in_arg input bytestream
 *  \param iv_arg initialization vector
 *  \param nbytes length of bytestream
 *  \param encdec 1 for encrypt; 0 for decrypt
 *
*/
void ifx_deu_aes_xts (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
        u8 *iv_arg, size_t nbytes, int encdec)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    volatile struct aes_t *aes = (volatile struct aes_t *) AES_START;
    struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
    unsigned long flag;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    u8 oldiv[16];
    int i = 0;
    int byte_cnt = nbytes; 

    CRTCL_SECT_START;

    aes_set_key_hw (ctx_arg);

    aes->controlr.E_D = !encdec;    //encryption
    aes->controlr.O = 1; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR - CBC mode for xts

    i = 0;
    while (byte_cnt >= 16) {

        if (!encdec) {
            if (((byte_cnt % 16) > 0) && (byte_cnt < (2*XTS_BLOCK_SIZE))) {
                 memcpy(oldiv, iv_arg, 16);
                 gf128mul_x_ble((le128 *)iv_arg, (le128 *)iv_arg);
            }
            u128_xor((u128 *)((u32 *) in_arg + (i * 4) + 0), (u128 *)((u32 *) in_arg + (i * 4) + 0), (u128 *)iv_arg);
        }

        aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
        aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
        aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));

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

        if (encdec) {
            u128_xor((u128 *)((volatile u32 *) out_arg + (i * 4) + 0), (u128 *)((volatile u32 *) out_arg + (i * 4) + 0), (u128 *)iv_arg);
        }
        gf128mul_x_ble((le128 *)iv_arg, (le128 *)iv_arg);
        i++;
        byte_cnt -= 16;
    }

    if (byte_cnt) {
	u8 state[XTS_BLOCK_SIZE] = {0,};

        if (!encdec) memcpy(iv_arg, oldiv, 16);

        aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
        aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
        aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));

        memcpy(state, ((u32 *) in_arg + (i * 4) + 0), byte_cnt);
        memcpy((state + byte_cnt), (out_arg + ((i - 1) * 16) + byte_cnt), (XTS_BLOCK_SIZE - byte_cnt));
        if (!encdec) {
            u128_xor((u128 *)state, (u128 *)state, (u128 *)iv_arg);
        }

        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) state + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) state + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) state + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) state + 3));    /* start crypto */

        memcpy(((u32 *) out_arg + (i * 4) + 0), ((u32 *) out_arg + ((i - 1) * 4) + 0), byte_cnt);

        while (aes->controlr.BUS) {
            // this will not take long
        }

        *((volatile u32 *) out_arg + ((i-1) * 4) + 0) = aes->OD3R;
        *((volatile u32 *) out_arg + ((i-1) * 4) + 1) = aes->OD2R;
        *((volatile u32 *) out_arg + ((i-1) * 4) + 2) = aes->OD1R;
        *((volatile u32 *) out_arg + ((i-1) * 4) + 3) = aes->OD0R;

        if (encdec) {
            u128_xor((u128 *)((volatile u32 *) out_arg + ((i-1) * 4) + 0), (u128 *)((volatile u32 *) out_arg + ((i-1) * 4) + 0), (u128 *)iv_arg);
        }
    }

    CRTCL_SECT_END;
}

/*! \fn int xts_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief XTS AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int xts_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes, processed;

    err = skcipher_walk_virt(&walk, req, false);

    if (req->cryptlen < XTS_BLOCK_SIZE)
            return -EINVAL;

    ctx->use_tweak = 1;
    aes_encrypt(req->base.tfm, walk.iv, walk.iv);
    ctx->use_tweak = 0;
    processed = 0;

    while ((nbytes = walk.nbytes) && (walk.nbytes >= (XTS_BLOCK_SIZE * 2)) ) {
        u8 *iv = walk.iv;
        if (nbytes == walk.total) {
            enc_bytes = nbytes;
        } else {
            enc_bytes = nbytes & ~(XTS_BLOCK_SIZE - 1);
            if ((req->cryptlen - processed - enc_bytes) < (XTS_BLOCK_SIZE)) {
                if (enc_bytes > (2 * XTS_BLOCK_SIZE)) {
                    enc_bytes -= XTS_BLOCK_SIZE;
                } else {
                    break;
                }
            }
        }
        ifx_deu_aes_xts(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                   iv, enc_bytes, CRYPTO_DIR_ENCRYPT);
        err = skcipher_walk_done(&walk, nbytes - enc_bytes);
        processed += enc_bytes;
    }

    if ((walk.nbytes)) {
        u8 *iv = walk.iv;
        nbytes = req->cryptlen - processed;
        scatterwalk_map_and_copy(ctx->lastbuffer, req->src, (req->cryptlen - nbytes), nbytes, 0);
        ifx_deu_aes_xts(ctx, ctx->lastbuffer, ctx->lastbuffer, 
                   iv, nbytes, CRYPTO_DIR_ENCRYPT);
        scatterwalk_map_and_copy(ctx->lastbuffer, req->dst, (req->cryptlen - nbytes), nbytes, 1);
        skcipher_request_complete(req, 0);
    }

    return err;
}

/*! \fn int xts_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief XTS AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int xts_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes, processed;

    err = skcipher_walk_virt(&walk, req, false);

    if (req->cryptlen < XTS_BLOCK_SIZE)
            return -EINVAL;

    ctx->use_tweak = 1;
    aes_encrypt(req->base.tfm, walk.iv, walk.iv);
    ctx->use_tweak = 0;
    processed = 0;

    while ((nbytes = walk.nbytes) && (walk.nbytes >= (XTS_BLOCK_SIZE * 2))) {
        u8 *iv = walk.iv;
        if (nbytes == walk.total) {
            dec_bytes = nbytes;
        } else {
            dec_bytes = nbytes & ~(XTS_BLOCK_SIZE - 1);
            if ((req->cryptlen - processed - dec_bytes) < (XTS_BLOCK_SIZE)) {
                if (dec_bytes > (2 * XTS_BLOCK_SIZE)) {
                    dec_bytes -= XTS_BLOCK_SIZE;
                } else {
                    break;
                }
            }
        }
        ifx_deu_aes_xts(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                   iv, dec_bytes, CRYPTO_DIR_DECRYPT);
        err = skcipher_walk_done(&walk, nbytes - dec_bytes);
        processed += dec_bytes;
    }

    if ((walk.nbytes)) {
        u8 *iv = walk.iv;
        nbytes = req->cryptlen - processed;
        scatterwalk_map_and_copy(ctx->lastbuffer, req->src, (req->cryptlen - nbytes), nbytes, 0);
        ifx_deu_aes_xts(ctx, ctx->lastbuffer, ctx->lastbuffer, 
                   iv, nbytes, CRYPTO_DIR_DECRYPT);
        scatterwalk_map_and_copy(ctx->lastbuffer, req->dst, (req->cryptlen - nbytes), nbytes, 1);
        skcipher_request_complete(req, 0);
    }

    return err;
}

/*! \fn int xts_aes_set_key_skcipher (struct crypto_tfm *tfm, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets the AES keys for XTS
 *  \param tfm linux crypto algo transform
 *  \param in_key input key
 *  \param key_len key lengths of 16, 24 and 32 bytes supported
 *  \return -EINVAL - bad key length, 0 - SUCCESS
*/
int xts_aes_set_key_skcipher (struct crypto_skcipher *tfm, const u8 *in_key, unsigned int key_len)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(crypto_skcipher_tfm(tfm));
    unsigned int keylen = (key_len / 2);

    if (key_len % 2) return -EINVAL;

    if (keylen != 16 && keylen != 24 && keylen != 32) {
        return -EINVAL;
    }

    ctx->key_length = keylen;
    ctx->use_tweak = 0;
    DPRINTF(0, "ctx @%p, key_len %d, ctx->key_length %d\n", ctx, key_len, ctx->key_length);
    memcpy ((u8 *) (ctx->buf), in_key, keylen);
    memcpy ((u8 *) (ctx->tweakkey), in_key + keylen, keylen);

    return 0;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_xts_aes_alg = {
    .base.cra_name           =   "xts(aes)",
    .base.cra_driver_name    =   "ifxdeu-xts(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   XTS_BLOCK_SIZE,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_xts_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE * 2,
    .max_keysize             =   AES_MAX_KEY_SIZE * 2,
    .ivsize                  =   XTS_BLOCK_SIZE,
    .walksize                =   2 * XTS_BLOCK_SIZE,
    .setkey                  =   xts_aes_set_key_skcipher,
    .encrypt                 =   xts_aes_encrypt,
    .decrypt                 =   xts_aes_decrypt,
};

/*! \fn int ofb_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief OFB AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ofb_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = enc_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ofb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ofb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			walk.iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*! \fn int ofb_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief OFB AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ofb_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = dec_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ofb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ofb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			walk.iv, walk.nbytes, CRYPTO_DIR_DECRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_ofb_aes_alg = {
    .base.cra_name           =   "ofb(aes)",
    .base.cra_driver_name    =   "ifxdeu-ofb(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   1,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_ofb_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE,
    .max_keysize             =   AES_MAX_KEY_SIZE,
    .ivsize                  =   AES_BLOCK_SIZE,
    .chunksize               =   AES_BLOCK_SIZE,
    .walksize                =   AES_BLOCK_SIZE,
    .setkey                  =   aes_set_key_skcipher,
    .encrypt                 =   ofb_aes_encrypt,
    .decrypt                 =   ofb_aes_decrypt,
};

/*! \fn int cfb_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CFB AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int cfb_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = enc_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cfb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_cfb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			walk.iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*! \fn int cfb_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief CFB AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int cfb_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = dec_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_cfb(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_cfb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			walk.iv, walk.nbytes, CRYPTO_DIR_DECRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_cfb_aes_alg = {
    .base.cra_name           =   "cfb(aes)",
    .base.cra_driver_name    =   "ifxdeu-cfb(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   1,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_cfb_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE,
    .max_keysize             =   AES_MAX_KEY_SIZE,
    .ivsize                  =   AES_BLOCK_SIZE,
    .chunksize               =   AES_BLOCK_SIZE,
    .walksize                =   AES_BLOCK_SIZE,
    .setkey                  =   aes_set_key_skcipher,
    .encrypt                 =   cfb_aes_encrypt,
    .decrypt                 =   cfb_aes_decrypt,
};

/*! \fn int ctr_basic_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ctr_basic_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int enc_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = enc_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       walk.iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
        err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*! \fn int ctr_basic_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ctr_basic_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    int err;
    unsigned int dec_bytes, nbytes;

    err = skcipher_walk_virt(&walk, req, false);

    while ((nbytes = dec_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       walk.iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       walk.iv, walk.nbytes, CRYPTO_DIR_DECRYPT, 0);
        err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_ctr_basic_aes_alg = {
    .base.cra_name           =   "ctr(aes)",
    .base.cra_driver_name    =   "ifxdeu-ctr(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   1,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_ctr_basic_aes_alg.base.cra_list),
    .min_keysize             =   AES_MIN_KEY_SIZE,
    .max_keysize             =   AES_MAX_KEY_SIZE,
    .ivsize                  =   AES_BLOCK_SIZE,
    .walksize                =   AES_BLOCK_SIZE,
    .setkey                  =   aes_set_key_skcipher,
    .encrypt                 =   ctr_basic_aes_encrypt,
    .decrypt                 =   ctr_basic_aes_decrypt,
};

/*! \fn int ctr_rfc3686_aes_encrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES (rfc3686) encrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ctr_rfc3686_aes_encrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    unsigned int nbytes, enc_bytes;
    int err;
    u8 rfc3686_iv[16];

    err = skcipher_walk_virt(&walk, req, false);
    nbytes = walk.nbytes;

    /* set up counter block */
    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE); 
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    while ((nbytes = enc_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       rfc3686_iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*! \fn int ctr_rfc3686_aes_decrypt(struct skcipher_req *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief Counter mode AES (rfc3686) decrypt using linux crypto skcipher
 *  \param req skcipher request
 *  \return err
*/
int ctr_rfc3686_aes_decrypt(struct skcipher_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    unsigned int nbytes, dec_bytes;
    int err;
    u8 rfc3686_iv[16];

    err = skcipher_walk_virt(&walk, req, false);
    nbytes = walk.nbytes;

    /* set up counter block */
    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE); 
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    while ((nbytes = dec_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr, 
                       rfc3686_iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* to handle remaining bytes < AES_BLOCK_SIZE */
    if (walk.nbytes) {
	ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
			rfc3686_iv, walk.nbytes, CRYPTO_DIR_DECRYPT, 0);
	err = skcipher_walk_done(&walk, 0);
    }

    return err;
}

/*
 * \brief AES function mappings
*/
struct skcipher_alg ifxdeu_ctr_rfc3686_aes_alg = {
    .base.cra_name           =   "rfc3686(ctr(aes))",
    .base.cra_driver_name    =   "ifxdeu-ctr-rfc3686(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   1,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_ctr_rfc3686_aes_alg.base.cra_list),
    .min_keysize             =   CTR_RFC3686_MIN_KEY_SIZE,
    .max_keysize             =   CTR_RFC3686_MAX_KEY_SIZE,
    .ivsize                  =   CTR_RFC3686_IV_SIZE,
    .walksize                =   AES_BLOCK_SIZE,
    .setkey                  =   ctr_rfc3686_aes_set_key_skcipher,
    .encrypt                 =   ctr_rfc3686_aes_encrypt,
    .decrypt                 =   ctr_rfc3686_aes_decrypt,
};

static int aes_cbcmac_final_impl(struct shash_desc *desc, u8 *out, bool hash_final);

/*! \fn static void aes_cbcmac_transform(struct shash_desc *desc, u8 const *in)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief save input block to context
 *  \param desc linux crypto shash descriptor
 *  \param in 16-byte block of input
*/
static void aes_cbcmac_transform(struct shash_desc *desc, u8 const *in)
{
    struct aes_ctx *mctx = crypto_shash_ctx(desc->tfm);

    if ( ((mctx->dbn)+1) > AES_CBCMAC_DBN_TEMP_SIZE )
    {
        //printk("aes_cbcmac_DBN_TEMP_SIZE exceeded\n");
        aes_cbcmac_final_impl(desc, (u8 *)mctx->hash, false);
    }

    memcpy(&mctx->temp[mctx->dbn], in, 16); //dbn workaround
    mctx->dbn += 1;
}

/*! \fn int aes_cbcmac_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief sets cbcmac aes key
 *  \param tfm linux crypto shash transform
 *  \param key input key
 *  \param keylen key
*/
static int aes_cbcmac_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen)
{
    return aes_set_key(crypto_shash_tfm(tfm), key, keylen);

    return 0;
}

/*! \fn void aes_cbcmac_init(struct shash_desc *desc)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief initialize md5 hmac context
 *  \param desc linux crypto shash descriptor
*/
static int aes_cbcmac_init(struct shash_desc *desc)
{

    struct aes_ctx *mctx = crypto_shash_ctx(desc->tfm);

    mctx->dbn = 0; //dbn workaround
    mctx->started = 0;
    mctx->byte_count = 0;
    memset(mctx->hash, 0, AES_BLOCK_SIZE);

    return 0;
}

/*! \fn void aes_cbcmac_update(struct shash_desc *desc, const u8 *data, unsigned int len)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief on-the-fly cbcmac aes computation
 *  \param desc linux crypto shash descriptor
 *  \param data input data
 *  \param len size of input data
*/
static int aes_cbcmac_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    struct aes_ctx *mctx = crypto_shash_ctx(desc->tfm);
    const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x0f);

    mctx->byte_count += len;

    if (avail > len) {
        memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
               data, len);
        return 0;
    }

    memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
           data, avail);

    aes_cbcmac_transform(desc, mctx->block);
    data += avail;
    len -= avail;

    while (len >= sizeof(mctx->block)) {
        memcpy(mctx->block, data, sizeof(mctx->block));
        aes_cbcmac_transform(desc, mctx->block);
        data += sizeof(mctx->block);
        len -= sizeof(mctx->block);
    }

    memcpy(mctx->block, data, len);
    return 0;
}

/*! \fn static int aes_cbcmac_final_impl(struct shash_desc *desc, u8 *out, bool hash_final)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief compute final or intermediate md5 hmac value
 *  \param desc linux crypto shash descriptor
 *  \param out final cbcmac aes output value
 *  \param in finalize or intermediate processing
*/
static int aes_cbcmac_final_impl(struct shash_desc *desc, u8 *out, bool hash_final)
{
    struct aes_ctx *mctx = crypto_shash_ctx(desc->tfm);
    const unsigned int offset = mctx->byte_count & 0x0f;
    char *p = (char *)mctx->block + offset;
    volatile struct aes_t *aes = (volatile struct aes_t *) AES_START;
    unsigned long flag;
    int i = 0;
    int dbn;
    u32 *in = mctx->temp[0];

    CRTCL_SECT_START;

    aes_set_key_hw (mctx);

    aes->controlr.E_D = !CRYPTO_DIR_ENCRYPT;    //encryption
    aes->controlr.O = 1; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR 

    //aes->controlr.F = 128; //default; only for CFB and OFB modes; change only for customer-specific apps

    //printk("\ndbn = %d\n", mctx->dbn);

    if (mctx->started) {
        aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) mctx->hash);
        aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) mctx->hash + 1));
        aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) mctx->hash + 2));
        aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) mctx->hash + 3));
    } else {
        mctx->started = 1;
        aes->IV3R = 0;
        aes->IV2R = 0;
        aes->IV1R = 0;
        aes->IV0R = 0;
    }

    i = 0;
    for (dbn = 0; dbn < mctx->dbn; dbn++)
    {
        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) in + (i * 4) + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) in + (i * 4) + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) in + (i * 4) + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) in + (i * 4) + 3));    /* start crypto */

        while (aes->controlr.BUS) {
            // this will not take long
        }

        in += 4;
    }

    *((u32 *) mctx->hash) = DEU_ENDIAN_SWAP(aes->IV3R);
    *((u32 *) mctx->hash + 1) = DEU_ENDIAN_SWAP(aes->IV2R);
    *((u32 *) mctx->hash + 2) = DEU_ENDIAN_SWAP(aes->IV1R);
    *((u32 *) mctx->hash + 3) = DEU_ENDIAN_SWAP(aes->IV0R);

    if (hash_final && offset) {
        aes->controlr.O = 0; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR 
        crypto_xor(mctx->block, mctx->hash, offset);

        memcpy(p, mctx->hash + offset, (AES_BLOCK_SIZE - offset));

        aes->ID3R = INPUT_ENDIAN_SWAP(*((u32 *) mctx->block + 0));
        aes->ID2R = INPUT_ENDIAN_SWAP(*((u32 *) mctx->block + 1));
        aes->ID1R = INPUT_ENDIAN_SWAP(*((u32 *) mctx->block + 2));
        aes->ID0R = INPUT_ENDIAN_SWAP(*((u32 *) mctx->block + 3));    /* start crypto */

        while (aes->controlr.BUS) {
            // this will not take long
        }

        *((u32 *) mctx->hash) = DEU_ENDIAN_SWAP(aes->OD3R);
        *((u32 *) mctx->hash + 1) = DEU_ENDIAN_SWAP(aes->OD2R);
        *((u32 *) mctx->hash + 2) = DEU_ENDIAN_SWAP(aes->OD1R);
        *((u32 *) mctx->hash + 3) = DEU_ENDIAN_SWAP(aes->OD0R);
    }

    CRTCL_SECT_END;

    if (hash_final) {
        memcpy(out, mctx->hash, AES_BLOCK_SIZE);
        /* reset the context after we finish with the hash */
        aes_cbcmac_init(desc);
    } else {
        mctx->dbn = 0;
    }
    return 0;
}

/*! \fn static int aes_cbcmac_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief call aes_cbcmac_final_impl with hash_final true
 *  \param tfm linux crypto algo transform
 *  \param out final md5 hmac output value
*/
static int aes_cbcmac_final(struct shash_desc *desc, u8 *out)
{
    return aes_cbcmac_final_impl(desc, out, true);
}

/*! \fn void aes_cbcmac_init_tfm(struct crypto_tfm *tfm)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief initialize pointers in aes_ctx
 *  \param tfm linux crypto shash transform
*/
static int aes_cbcmac_init_tfm(struct crypto_tfm *tfm)
{
    struct aes_ctx *mctx = crypto_tfm_ctx(tfm);
    mctx->temp = kzalloc(AES_BLOCK_SIZE * AES_CBCMAC_DBN_TEMP_SIZE, GFP_KERNEL);
    if (IS_ERR(mctx->temp)) return PTR_ERR(mctx->temp);

    return 0;
}

/*! \fn void aes_cbcmac_exit_tfm(struct crypto_tfm *tfm)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief free pointers in aes_ctx
 *  \param tfm linux crypto shash transform
*/
static void aes_cbcmac_exit_tfm(struct crypto_tfm *tfm)
{
    struct aes_ctx *mctx = crypto_tfm_ctx(tfm);
    kfree(mctx->temp);
}

/*
 * \brief aes_cbcmac function mappings
*/
static struct shash_alg ifxdeu_cbcmac_aes_alg = {
    .digestsize         =       AES_BLOCK_SIZE,
    .init               =       aes_cbcmac_init,
    .update             =       aes_cbcmac_update,
    .final              =       aes_cbcmac_final,
    .setkey             =       aes_cbcmac_setkey,
    .descsize           =       sizeof(struct aes_ctx),
    .base               =       {
        .cra_name       =       "cbcmac(aes)",
        .cra_driver_name=       "ifxdeu-cbcmac(aes)",
        .cra_priority   =       400,
        .cra_ctxsize    =       sizeof(struct aes_ctx),
        .cra_flags      =       CRYPTO_ALG_TYPE_HASH | CRYPTO_ALG_KERN_DRIVER_ONLY,
        .cra_blocksize  =       1,
        .cra_module     =       THIS_MODULE,
        .cra_init       =       aes_cbcmac_init_tfm,
        .cra_exit       =       aes_cbcmac_exit_tfm,
        }
};

/*! \fn int aes_set_key_aead (struct crypto_aead *aead, const uint8_t *in_key, unsigned int key_len)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets the AES keys for aead gcm
 *  \param aead linux crypto aead
 *  \param in_key input key
 *  \param key_len key lengths of 16, 24 and 32 bytes supported
 *  \return -EINVAL - bad key length, 0 - SUCCESS
*/
int aes_set_key_aead (struct crypto_aead *aead, const u8 *in_key, unsigned int key_len)
{
    struct aes_ctx *ctx = crypto_aead_ctx(aead);
    int err;

    err = aes_set_key(&aead->base, in_key, key_len);
    if (err) return err;

    memset(ctx->block, 0, sizeof(ctx->block));
    memset(ctx->lastbuffer, 0, AES_BLOCK_SIZE);
    ifx_deu_aes_ctr(ctx, ctx->block, ctx->block,
                       ctx->lastbuffer, AES_BLOCK_SIZE, CRYPTO_DIR_ENCRYPT, 0);
    if (ctx->gf128) gf128mul_free_4k(ctx->gf128);
    ctx->gf128 = gf128mul_init_4k_lle((be128 *)ctx->block);

    return err;
}

/*! \fn int gcm_aes_setauthsize (struct crypto_aead *aead, unsigned int authsize)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief sets the AES keys for aead gcm
 *  \param aead linux crypto aead
 *  \param in_key input authsize
 *  \return -EINVAL - bad authsize length, 0 - SUCCESS
*/
int gcm_aes_setauthsize (struct crypto_aead *aead, unsigned int authsize)
{
    return crypto_gcm_check_authsize(authsize);
}

/*! \fn int gcm_aes_encrypt(struct aead_request *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief GCM AES encrypt using linux crypto aead
 *  \param req aead request
 *  \return err
*/
int gcm_aes_encrypt(struct aead_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    struct skcipher_request request;
    int err;
    unsigned int enc_bytes, nbytes;
    be128 lengths;
    u8 iv[AES_BLOCK_SIZE];

    lengths.a = cpu_to_be64(req->assoclen * 8);
    lengths.b = cpu_to_be64(req->cryptlen * 8);

    memset(ctx->hash, 0, sizeof(ctx->hash));
    memset(ctx->block, 0, sizeof(ctx->block));
    memcpy(iv, req->iv, GCM_AES_IV_SIZE);
    *(__be32 *)((void *)iv + GCM_AES_IV_SIZE) = cpu_to_be32(1);
    ifx_deu_aes_ctr(ctx, ctx->block, ctx->block,
                       iv, 16, CRYPTO_DIR_ENCRYPT, 0);

    request.cryptlen = req->cryptlen + req->assoclen;
    request.src = req->src;
    request.dst = req->dst;
    request.base = req->base;

    crypto_skcipher_alg(crypto_skcipher_reqtfm(&request))->walksize = AES_BLOCK_SIZE;

    if (req->assoclen && (req->assoclen < AES_BLOCK_SIZE))
        crypto_skcipher_alg(crypto_skcipher_reqtfm(&request))->walksize = req->assoclen;

    err = skcipher_walk_virt(&walk, &request, false);

    //process assoc data if available
    if (req->assoclen > 0) {
        unsigned int assoc_remain, ghashlen;

        assoc_remain = req->assoclen;
        ghashlen = min(req->assoclen, walk.nbytes);
        while ((nbytes = enc_bytes = ghashlen) && (ghashlen >= AES_BLOCK_SIZE)) {
            u8 *temp;
            if (nbytes > req->assoclen) nbytes = enc_bytes = req->assoclen;
            enc_bytes -= (nbytes % AES_BLOCK_SIZE);
            memcpy(walk.dst.virt.addr, walk.src.virt.addr, enc_bytes);
            assoc_remain -= enc_bytes;
            temp = walk.dst.virt.addr;
            while (enc_bytes > 0) {
                u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)temp);
                gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
                enc_bytes -= AES_BLOCK_SIZE;
                temp += 16;
            }
            if (assoc_remain < AES_BLOCK_SIZE) walk.stride = assoc_remain;
            if (assoc_remain == 0) walk.stride = AES_BLOCK_SIZE;
            enc_bytes = nbytes - (nbytes % AES_BLOCK_SIZE);
            err = skcipher_walk_done(&walk, (walk.nbytes - enc_bytes));
            ghashlen = min(assoc_remain, walk.nbytes);
        }

        if ((enc_bytes = ghashlen)) {
            memcpy(ctx->lastbuffer, walk.src.virt.addr, enc_bytes);
            memset(ctx->lastbuffer + enc_bytes, 0, (AES_BLOCK_SIZE - enc_bytes));
            memcpy(walk.dst.virt.addr, walk.src.virt.addr, ghashlen);
            u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->lastbuffer);
            gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
            walk.stride = AES_BLOCK_SIZE;
            err = skcipher_walk_done(&walk, (walk.nbytes - ghashlen));
        }
    }

    //crypt and hash
    while ((nbytes = enc_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
        u8 *temp;
        enc_bytes -= (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        temp = walk.dst.virt.addr;
        while (enc_bytes) {
            u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)temp);
            gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
            enc_bytes -= AES_BLOCK_SIZE;
            temp += 16;
        }
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* crypt and hash remaining bytes < AES_BLOCK_SIZE */
    if ((enc_bytes = walk.nbytes)) {
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
        memcpy(ctx->lastbuffer, walk.dst.virt.addr, enc_bytes);
        memset(ctx->lastbuffer + enc_bytes, 0, (AES_BLOCK_SIZE - enc_bytes));
        u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->lastbuffer);
        gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
        err = skcipher_walk_done(&walk, 0);
    }

    //finalize and copy hash
    u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)&lengths);
    gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
    u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->block);
    scatterwalk_map_and_copy(ctx->hash, req->dst, req->cryptlen + req->assoclen, crypto_aead_authsize(crypto_aead_reqtfm(req)), 1);

    aead_request_complete(req, 0);

    return err;
}

/*! \fn int gcm_aes_decrypt(struct aead_request *req)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief GCM AES decrypt using linux crypto aead
 *  \param req aead request
 *  \return err
*/
int gcm_aes_decrypt(struct aead_request *req)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
    struct skcipher_walk walk;
    struct skcipher_request request;
    int err;
    unsigned int dec_bytes, nbytes, authsize;
    be128 lengths;
    u8 iv[AES_BLOCK_SIZE];

    authsize = crypto_aead_authsize(crypto_aead_reqtfm(req));

    lengths.a = cpu_to_be64(req->assoclen * 8);
    lengths.b = cpu_to_be64((req->cryptlen - authsize) * 8);

    memset(ctx->hash, 0, sizeof(ctx->hash));
    memset(ctx->block, 0, sizeof(ctx->block));
    memcpy(iv, req->iv, GCM_AES_IV_SIZE);
    *(__be32 *)((void *)iv + GCM_AES_IV_SIZE) = cpu_to_be32(1);
    ifx_deu_aes_ctr(ctx, ctx->block, ctx->block,
                       iv, 16, CRYPTO_DIR_ENCRYPT, 0);

    request.cryptlen = req->cryptlen + req->assoclen - authsize;
    request.src = req->src;
    request.dst = req->dst;
    request.base = req->base;
    crypto_skcipher_alg(crypto_skcipher_reqtfm(&request))->walksize = AES_BLOCK_SIZE;

    if (req->assoclen && (req->assoclen < AES_BLOCK_SIZE))
        crypto_skcipher_alg(crypto_skcipher_reqtfm(&request))->walksize = req->assoclen;

    err = skcipher_walk_virt(&walk, &request, false);

    //process assoc data if available
    if (req->assoclen > 0) {
        unsigned int assoc_remain, ghashlen;

        assoc_remain = req->assoclen;
        ghashlen = min(req->assoclen, walk.nbytes);
        while ((nbytes = dec_bytes = ghashlen) && (ghashlen >= AES_BLOCK_SIZE)) {
            u8 *temp;
            if (nbytes > req->assoclen) nbytes = dec_bytes = req->assoclen;
            dec_bytes -= (nbytes % AES_BLOCK_SIZE);
            memcpy(walk.dst.virt.addr, walk.src.virt.addr, dec_bytes);
            assoc_remain -= dec_bytes;
            temp = walk.dst.virt.addr;
            while (dec_bytes > 0) {
                u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)temp);
                gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
                dec_bytes -= AES_BLOCK_SIZE;
                temp += 16;
            }
            if (assoc_remain < AES_BLOCK_SIZE) walk.stride = assoc_remain;
            if (assoc_remain == 0) walk.stride = AES_BLOCK_SIZE;
            dec_bytes = nbytes - (nbytes % AES_BLOCK_SIZE);
            err = skcipher_walk_done(&walk, (walk.nbytes - dec_bytes));
            ghashlen = min(assoc_remain, walk.nbytes);
        }

        if ((dec_bytes = ghashlen)) {
            memcpy(ctx->lastbuffer, walk.src.virt.addr, dec_bytes);
            memset(ctx->lastbuffer + dec_bytes, 0, (AES_BLOCK_SIZE - dec_bytes));
            memcpy(walk.dst.virt.addr, walk.src.virt.addr, ghashlen);
            u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->lastbuffer);
            gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
            walk.stride = AES_BLOCK_SIZE;
            err = skcipher_walk_done(&walk, (walk.nbytes - ghashlen));
        }
    }

    //crypt and hash
    while ((nbytes = dec_bytes = walk.nbytes) && (walk.nbytes >= AES_BLOCK_SIZE)) {
        u8 *temp;
        dec_bytes -= (nbytes % AES_BLOCK_SIZE);
        temp = walk.src.virt.addr;
        while (dec_bytes) {
            u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)temp);
            gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
            dec_bytes -= AES_BLOCK_SIZE;
            temp += 16;
        }
        dec_bytes = nbytes - (nbytes % AES_BLOCK_SIZE);
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
        nbytes &= AES_BLOCK_SIZE - 1;
        err = skcipher_walk_done(&walk, nbytes);
    }

    /* crypt and hash remaining bytes < AES_BLOCK_SIZE */
    if ((dec_bytes = walk.nbytes)) {
        memcpy(ctx->lastbuffer, walk.src.virt.addr, dec_bytes);
        memset(ctx->lastbuffer + dec_bytes, 0, (AES_BLOCK_SIZE - dec_bytes));
        u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->lastbuffer);
        gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
        ifx_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
                       iv, walk.nbytes, CRYPTO_DIR_DECRYPT, 0);
        err = skcipher_walk_done(&walk, 0);
    }

    //finalize and copy hash
    u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)&lengths);
    gf128mul_4k_lle((be128 *)ctx->hash, ctx->gf128);
    u128_xor((u128 *)ctx->hash, (u128 *)ctx->hash, (u128 *)ctx->block);

    scatterwalk_map_and_copy(ctx->lastbuffer, req->src, req->cryptlen + req->assoclen - authsize, authsize, 0);
    err = crypto_memneq(ctx->lastbuffer, ctx->hash, authsize) ? -EBADMSG : 0;

    aead_request_complete(req, 0);

    return err;
}

/*! \fn void aes_gcm_exit_tfm(struct crypto_tfm *tfm)
 *  \ingroup IFX_aes_cbcmac_FUNCTIONS
 *  \brief free pointers in aes_ctx
 *  \param tfm linux crypto shash transform
*/
static void aes_gcm_exit_tfm(struct crypto_tfm *tfm)
{
    struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
    if (ctx->gf128) gf128mul_free_4k(ctx->gf128);
}

/*
 * \brief AES function mappings
*/
struct aead_alg ifxdeu_gcm_aes_alg = {
    .base.cra_name           =   "gcm(aes)",
    .base.cra_driver_name    =   "ifxdeu-gcm(aes)",
    .base.cra_priority       =   400,
    .base.cra_flags          =   CRYPTO_ALG_TYPE_SKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY,
    .base.cra_blocksize      =   1,
    .base.cra_ctxsize        =   sizeof(struct aes_ctx),
    .base.cra_module         =   THIS_MODULE,
    .base.cra_list           =   LIST_HEAD_INIT(ifxdeu_gcm_aes_alg.base.cra_list),
    .base.cra_exit           =   aes_gcm_exit_tfm,
    .ivsize                  =   GCM_AES_IV_SIZE,
    .maxauthsize             =   AES_BLOCK_SIZE,
    .chunksize               =   AES_BLOCK_SIZE,
    .setkey                  =   aes_set_key_aead,
    .encrypt                 =   gcm_aes_encrypt,
    .decrypt                 =   gcm_aes_decrypt,
    .setauthsize             =   gcm_aes_setauthsize,
};

/*! \fn int ifxdeu_init_aes (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief function to initialize AES driver
 *  \return ret
*/
int ifxdeu_init_aes (void)
{
    int ret = -ENOSYS;

    aes_chip_init();

    if ((ret = crypto_register_alg(&ifxdeu_aes_alg)))
        goto aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_ecb_aes_alg)))
        goto ecb_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_cbc_aes_alg)))
        goto cbc_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_xts_aes_alg)))
        goto xts_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_ofb_aes_alg)))
        goto ofb_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_cfb_aes_alg)))
        goto cfb_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_ctr_basic_aes_alg)))
        goto ctr_basic_aes_err;

    if ((ret = crypto_register_skcipher(&ifxdeu_ctr_rfc3686_aes_alg)))
        goto ctr_rfc3686_aes_err;

    if ((ret = crypto_register_shash(&ifxdeu_cbcmac_aes_alg)))
        goto cbcmac_aes_err;

    if ((ret = crypto_register_aead(&ifxdeu_gcm_aes_alg)))
        goto gcm_aes_err;

    CRTCL_SECT_INIT;


    printk (KERN_NOTICE "IFX DEU AES initialized%s%s.\n", disable_multiblock ? "" : " (multiblock)", disable_deudma ? "" : " (DMA)");
    return ret;

gcm_aes_err:
    crypto_unregister_aead(&ifxdeu_gcm_aes_alg);
    printk (KERN_ERR "IFX gcm_aes initialization failed!\n");
    return ret;
cbcmac_aes_err:
    crypto_unregister_shash(&ifxdeu_cbcmac_aes_alg);
    printk (KERN_ERR "IFX cbcmac_aes initialization failed!\n");
    return ret;
ctr_rfc3686_aes_err:
    crypto_unregister_skcipher(&ifxdeu_ctr_rfc3686_aes_alg);
    printk (KERN_ERR "IFX ctr_rfc3686_aes initialization failed!\n");
    return ret;
ctr_basic_aes_err:
    crypto_unregister_skcipher(&ifxdeu_ctr_basic_aes_alg);
    printk (KERN_ERR "IFX ctr_basic_aes initialization failed!\n");
    return ret;
cfb_aes_err:
    crypto_unregister_skcipher(&ifxdeu_cfb_aes_alg);
    printk (KERN_ERR "IFX cfb_aes initialization failed!\n");
    return ret;
ofb_aes_err:
    crypto_unregister_skcipher(&ifxdeu_ofb_aes_alg);
    printk (KERN_ERR "IFX ofb_aes initialization failed!\n");
    return ret;
xts_aes_err:
    crypto_unregister_skcipher(&ifxdeu_xts_aes_alg);
    printk (KERN_ERR "IFX xts_aes initialization failed!\n");
    return ret;
cbc_aes_err:
    crypto_unregister_skcipher(&ifxdeu_cbc_aes_alg);
    printk (KERN_ERR "IFX cbc_aes initialization failed!\n");
    return ret;
ecb_aes_err:
    crypto_unregister_skcipher(&ifxdeu_ecb_aes_alg);
    printk (KERN_ERR "IFX aes initialization failed!\n");
    return ret;
aes_err:
    printk(KERN_ERR "IFX DEU AES initialization failed!\n");

    return ret;
}

/*! \fn void ifxdeu_fini_aes (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief unregister aes driver
*/
void ifxdeu_fini_aes (void)
{
    crypto_unregister_alg (&ifxdeu_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_ecb_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_cbc_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_xts_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_ofb_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_cfb_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_ctr_basic_aes_alg);
    crypto_unregister_skcipher (&ifxdeu_ctr_rfc3686_aes_alg);
    crypto_unregister_shash (&ifxdeu_cbcmac_aes_alg);
    crypto_unregister_aead (&ifxdeu_gcm_aes_alg);
}
