/******************************************************************************
**
** FILE NAME    : ifxmips_sha1_hmac.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for UEIP
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
** 21,March 2011 Mohammad Firdaus   Changes for Kernel 2.6.32 and IPSec integration
*******************************************************************************/
/*!
  \defgroup IFX_DEU IFX_DEU_DRIVERS
  \ingroup API
  \brief ifx deu driver module
*/

/*!
  \file	ifxmips_sha1_hmac.c
  \ingroup IFX_DEU
  \brief SHA1-HMAC deu driver file
*/

/*! 
  \defgroup IFX_SHA1_HMAC_FUNCTIONS IFX_SHA1_HMAC_FUNCTIONS
  \ingroup IFX_DEU
  \brief ifx sha1 hmac functions
*/


/* Project header */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <crypto/internal/hash.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <asm/byteorder.h>
#include <linux/delay.h>

#if defined(CONFIG_AR9)
#include "ifxmips_deu_ar9.h"
#elif defined(CONFIG_VR9) || defined(CONFIG_AR10)
#include "ifxmips_deu_vr9.h"
#else
#error "Plaform Unknwon!"
#endif

#define SHA1_DIGEST_SIZE    20
#define SHA1_HMAC_BLOCK_SIZE    64
#define SHA1_HMAC_DBN_TEMP_SIZE 1024 // size in dword, needed for dbn workaround 
#define HASH_START   IFX_HASH_CON

#define SHA1_HMAC_MAX_KEYLEN 64

static spinlock_t lock;
#define CRTCL_SECT_INIT        spin_lock_init(&lock)
#define CRTCL_SECT_START       spin_lock_irqsave(&lock, flag)
#define CRTCL_SECT_END         spin_unlock_irqrestore(&lock, flag)

#ifdef CRYPTO_DEBUG
extern char debug_level;
#define DPRINTF(level, format, args...) if (level < debug_level) printk(KERN_INFO "[%s %s %d]: " format, __FILE__, __func__, __LINE__, ##args);
#else
#define DPRINTF(level, format, args...)
#endif

struct sha1_hmac_ctx {
    int keylen;

    u8 buffer[SHA1_HMAC_BLOCK_SIZE];
    u8 key[SHA1_HMAC_MAX_KEYLEN];
    u32 state[5];
    u32 dbn;
    u64 count;

};

static u32 temp[SHA1_HMAC_DBN_TEMP_SIZE];  

extern int disable_deudma;

/*! \fn static void sha1_hmac_transform(struct crypto_tfm *tfm, u32 const *in)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief save input block to context   
 *  \param tfm linux crypto algo transform  
 *  \param in 64-byte block of input  
*/                                 
static int sha1_hmac_transform(struct shash_desc *desc, u32 const *in)
{
    struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);

    memcpy(&temp[sctx->dbn<<4], in, 64); //dbn workaround
    sctx->dbn += 1;
    
    if ( (sctx->dbn<<4) > SHA1_HMAC_DBN_TEMP_SIZE )
    {
        printk("SHA1_HMAC_DBN_TEMP_SIZE exceeded\n");
    }
   
    return 0;
}

/*! \fn int sha1_hmac_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief sets sha1 hmac key   
 *  \param tfm linux crypto algo transform  
 *  \param key input key  
 *  \param keylen key length greater than 64 bytes IS NOT SUPPORTED  
*/                                 
static int sha1_hmac_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen)
{
    struct sha1_hmac_ctx *sctx = crypto_shash_ctx(tfm);
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    
    if (keylen > SHA1_HMAC_MAX_KEYLEN) {
	printk("Key length exceeds maximum key length\n");
	return -EINVAL;
    }

    //printk("Setting keys of len: %d\n", keylen);
     
    hashs->KIDX |= 0x80000000; //reset keys back to 0
    memcpy(&sctx->key, key, keylen);
    sctx->keylen = keylen;

    return 0;
         
}


/*! \fn int sha1_hmac_setkey_hw(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief sets sha1 hmac key  into hw registers 
 *  \param tfm linux crypto algo transform  
 *  \param key input key  
 *  \param keylen key length greater than 64 bytes IS NOT SUPPORTED  
*/                                 
static int sha1_hmac_setkey_hw(const u8 *key, unsigned int keylen)
{
    volatile struct deu_hash_t *hash = (struct deu_hash_t *) HASH_START;
    int i, j;
    unsigned long flag;
    u32 *in_key = (u32 *)key;        

    j = 0;

    CRTCL_SECT_START;
    for (i = 0; i < keylen; i+=4)
    {
         hash->KIDX = j;
         asm("sync");
         hash->KEY = *((u32 *) in_key + j); 
         j++;
    }

    CRTCL_SECT_END;
    return 0;
}

/*! \fn void sha1_hmac_init(struct crypto_tfm *tfm)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief initialize sha1 hmac context   
 *  \param tfm linux crypto algo transform  
*/                                 
static int sha1_hmac_init(struct shash_desc *desc)
{
    struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);

    //printk("debug ln: %d, fn: %s\n", __LINE__, __func__);
    sctx->dbn = 0; //dbn workaround
    sha1_hmac_setkey_hw(sctx->key, sctx->keylen);

    return 0;
}

/*! \fn static void sha1_hmac_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief on-the-fly sha1 hmac computation   
 *  \param tfm linux crypto algo transform  
 *  \param data input data  
 *  \param len size of input data 
*/                                 
static int sha1_hmac_update(struct shash_desc *desc, const u8 *data,
            unsigned int len)
{
    struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);
    unsigned int i, j;

    j = (sctx->count >> 3) & 0x3f;
    sctx->count += len << 3;
   // printk("sctx->count = %d\n", sctx->count);

    if ((j + len) > 63) {
        memcpy (&sctx->buffer[j], data, (i = 64 - j));
        sha1_hmac_transform (desc, (const u32 *)sctx->buffer);
        for (; i + 63 < len; i += 64) {
            sha1_hmac_transform (desc, (const u32 *)&data[i]);
        }

        j = 0;
    }
    else
        i = 0;

    memcpy (&sctx->buffer[j], &data[i], len - i);
    return 0;
}

/*! \fn static void sha1_hmac_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief ompute final sha1 hmac value   
 *  \param tfm linux crypto algo transform  
 *  \param out final sha1 hmac output value  
*/                                 
static int sha1_hmac_final(struct shash_desc *desc, u8 *out)
{
    //struct sha1_hmac_ctx *sctx = shash_desc_ctx(desc);
    struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);
    u32 index, padlen;
    u64 t;
    u8 bits[8] = { 0, };
    static const u8 padding[64] = { 0x80, };
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    unsigned long flag;
    int i = 0;
    int dbn;
    u32 *in = &temp[0];
        
    t = sctx->count + 512; // need to add 512 bit of the IPAD operation
    bits[7] = 0xff & t;
    t >>= 8;
    bits[6] = 0xff & t;
    t >>= 8;
    bits[5] = 0xff & t;
    t >>= 8;
    bits[4] = 0xff & t;
    t >>= 8;
    bits[3] = 0xff & t;
    t >>= 8;
    bits[2] = 0xff & t;
    t >>= 8;
    bits[1] = 0xff & t;
    t >>= 8;
    bits[0] = 0xff & t;

    /* Pad out to 56 mod 64 */
    index = (sctx->count >> 3) & 0x3f;
    padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
    sha1_hmac_update (desc, padding, padlen);

    /* Append length */
    sha1_hmac_update (desc, bits, sizeof bits);

    CRTCL_SECT_START;
    
    hashs->DBN = sctx->dbn;
    
    //for vr9 change, ENDI = 1
    *IFX_HASH_CON = HASH_CON_VALUE; 

    //wait for processing
    while (hashs->controlr.BSY) {
        // this will not take long
    }

    for (dbn = 0; dbn < sctx->dbn; dbn++)
    {
    for (i = 0; i < 16; i++) {
        hashs->MR = in[i];
    };

    hashs->controlr.GO = 1;
    asm("sync");

    //wait for processing
    while (hashs->controlr.BSY) {
            // this will not take long
    }
    
    in += 16;
}


#if 1
    //wait for digest ready
    while (! hashs->controlr.DGRY) {
        // this will not take long
    }
#endif

    *((u32 *) out + 0) = hashs->D1R;
    *((u32 *) out + 1) = hashs->D2R;
    *((u32 *) out + 2) = hashs->D3R;
    *((u32 *) out + 3) = hashs->D4R;
    *((u32 *) out + 4) = hashs->D5R;

    memset(&sctx->buffer[0], 0, SHA1_HMAC_BLOCK_SIZE);
    sctx->count = 0; 
 
    //printk("debug ln: %d, fn: %s\n", __LINE__, __func__);
    CRTCL_SECT_END;


    return 0;

}

/*
 * \brief SHA1-HMAC function mappings
*/
static struct shash_alg ifxdeu_sha1_hmac_alg = {
        .digestsize     =       SHA1_DIGEST_SIZE,
        .init           =       sha1_hmac_init,
        .update         =       sha1_hmac_update,
        .final          =       sha1_hmac_final,
        .setkey         =       sha1_hmac_setkey,
        .descsize       =       sizeof(struct sha1_hmac_ctx),
        .base           =       {
                .cra_name       =       "hmac(sha1)",
                .cra_driver_name=       "ifxdeu-sha1_hmac",
                .cra_priority   =       400,
		.cra_ctxsize    =	sizeof(struct sha1_hmac_ctx),
                .cra_flags      =       CRYPTO_ALG_TYPE_DIGEST,
                .cra_blocksize  =       SHA1_HMAC_BLOCK_SIZE,
                .cra_module     =       THIS_MODULE,
        }

};


/*! \fn int __init ifxdeu_init_sha1_hmac (void)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief initialize sha1 hmac driver    
*/                                 
int __init ifxdeu_init_sha1_hmac (void)
{
    int ret = -ENOSYS;



    if ((ret = crypto_register_shash(&ifxdeu_sha1_hmac_alg)))
        goto sha1_err;

    CRTCL_SECT_INIT;

    printk (KERN_NOTICE "IFX DEU SHA1_HMAC initialized%s.\n", disable_deudma ? "" : " (DMA)");
    return ret;

sha1_err:
    printk(KERN_ERR "IFX DEU SHA1_HMAC initialization failed!\n");
    return ret;
}

/*! \fn void __exit ifxdeu_fini_sha1_hmac (void)
 *  \ingroup IFX_SHA1_HMAC_FUNCTIONS
 *  \brief unregister sha1 hmac driver    
*/                                 
void __exit ifxdeu_fini_sha1_hmac (void)
{

    crypto_unregister_shash(&ifxdeu_sha1_hmac_alg);


}

