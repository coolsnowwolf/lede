/******************************************************************************
**
** FILE NAME    : ifxmips_md5.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for UEIP
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
  \defgroup    IFX_DEU IFX_DEU_DRIVERS
  \ingroup API
  \brief ifx deu driver module
*/

/*!
  \file		ifxmips_md5.c
  \ingroup 	IFX_DEU
  \brief 	MD5 encryption deu driver file 
*/

/*!
  \defgroup IFX_MD5_FUNCTIONS IFX_MD5_FUNCTIONS
  \ingroup IFX_DEU
  \brief ifx deu MD5 functions
*/

/*Project header files */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <crypto/internal/hash.h>
#include <asm/byteorder.h>

/* Project header */
#if defined(CONFIG_DANUBE)
#include "ifxmips_deu_danube.h"
#elif defined(CONFIG_AR9)
#include "ifxmips_deu_ar9.h"
#elif defined(CONFIG_VR9) || defined(CONFIG_AR10)
#include "ifxmips_deu_vr9.h"
#else
#error "Plaform Unknwon!"
#endif

#define MD5_DIGEST_SIZE     16
#define MD5_HMAC_BLOCK_SIZE 64
#define MD5_BLOCK_WORDS     16
#define MD5_HASH_WORDS      4
#define HASH_START   IFX_HASH_CON

static spinlock_t lock;
#define CRTCL_SECT_INIT        spin_lock_init(&lock)
#define CRTCL_SECT_START       spin_lock_irqsave(&lock, flag)
#define CRTCL_SECT_END         spin_unlock_irqrestore(&lock, flag)

//#define CRYPTO_DEBUG
#ifdef CRYPTO_DEBUG
extern char debug_level;
#define DPRINTF(level, format, args...) if (level < debug_level) printk(KERN_INFO "[%s %s %d]: " format, __FILE__, __func__, __LINE__, ##args);
#else
#define DPRINTF(level, format, args...)
#endif

struct md5_ctx {
    int started;
    u32 hash[MD5_HASH_WORDS];
    u32 block[MD5_BLOCK_WORDS];
    u64 byte_count;
};

extern int disable_deudma;

/*! \fn static u32 endian_swap(u32 input)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief perform dword level endian swap   
 *  \param input value of dword that requires to be swapped  
*/ 
static u32 endian_swap(u32 input)
{
    u8 *ptr = (u8 *)&input;
    
    return ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);     
}

/*! \fn static void md5_transform(u32 *hash, u32 const *in)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief main interface to md5 hardware   
 *  \param hash current hash value  
 *  \param in 64-byte block of input  
*/                                 
static void md5_transform(struct md5_ctx *mctx, u32 *hash, u32 const *in)
{
    int i;
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    unsigned long flag;

    CRTCL_SECT_START;

    if (mctx->started) { 
        hashs->D1R = endian_swap(*((u32 *) hash + 0));
    	hashs->D2R = endian_swap(*((u32 *) hash + 1));
        hashs->D3R = endian_swap(*((u32 *) hash + 2));
        hashs->D4R = endian_swap(*((u32 *) hash + 3));
    }

    for (i = 0; i < 16; i++) {
        hashs->MR = endian_swap(in[i]);
//	printk("in[%d]: %08x\n", i, endian_swap(in[i]));
    };

    //wait for processing
    while (hashs->controlr.BSY) {
        // this will not take long
    }

    *((u32 *) hash + 0) = endian_swap (hashs->D1R);
    *((u32 *) hash + 1) = endian_swap (hashs->D2R);
    *((u32 *) hash + 2) = endian_swap (hashs->D3R);
    *((u32 *) hash + 3) = endian_swap (hashs->D4R);

    mctx->started = 1; 

    CRTCL_SECT_END;
}

/*! \fn static inline void md5_transform_helper(struct md5_ctx *ctx)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief interfacing function for md5_transform()   
 *  \param ctx crypto context  
*/                                 
static inline void md5_transform_helper(struct md5_ctx *ctx)
{
    //le32_to_cpu_array(ctx->block, sizeof(ctx->block) / sizeof(u32));
    md5_transform(ctx, ctx->hash, ctx->block);
}

/*! \fn static void md5_init(struct crypto_tfm *tfm)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief initialize md5 hardware   
 *  \param tfm linux crypto algo transform  
*/                                 
static int md5_init(struct shash_desc *desc)
{
    struct md5_ctx *mctx = shash_desc_ctx(desc);
    volatile struct deu_hash_t *hash = (struct deu_hash_t *) HASH_START;

    hash->controlr.ENDI = 0;
    hash->controlr.SM = 1;
    hash->controlr.ALGO = 1;    // 1 = md5  0 = sha1
    hash->controlr.INIT = 1;    // Initialize the hash operation by writing a '1' to the INIT bit.

    mctx->byte_count = 0;
    mctx->started = 0;
    return 0;
}

/*! \fn static void md5_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief on-the-fly md5 computation   
 *  \param tfm linux crypto algo transform  
 *  \param data input data  
 *  \param len size of input data  
*/                                 
static int md5_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    struct md5_ctx *mctx = shash_desc_ctx(desc);
    const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x3f);

    mctx->byte_count += len;

    if (avail > len) {
        memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
               data, len);
        return 0;
    }

    memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
           data, avail);

    md5_transform_helper(mctx);
    data += avail;
    len -= avail;

    while (len >= sizeof(mctx->block)) {
        memcpy(mctx->block, data, sizeof(mctx->block));
        md5_transform_helper(mctx);
        data += sizeof(mctx->block);
        len -= sizeof(mctx->block);
    }

    memcpy(mctx->block, data, len);
    return 0;
}

/*! \fn static void md5_final(struct crypto_tfm *tfm, u8 *out)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief compute final md5 value   
 *  \param tfm linux crypto algo transform  
 *  \param out final md5 output value  
*/                                 
static int md5_final(struct shash_desc *desc, u8 *out)
{
    struct md5_ctx *mctx = shash_desc_ctx(desc);
    const unsigned int offset = mctx->byte_count & 0x3f;
    char *p = (char *)mctx->block + offset;
    int padding = 56 - (offset + 1);
    volatile struct deu_hash_t *hashs = (struct deu_hash_t *) HASH_START;
    unsigned long flag;

    *p++ = 0x80;
    if (padding < 0) {
        memset(p, 0x00, padding + sizeof (u64));
        md5_transform_helper(mctx);
        p = (char *)mctx->block;
        padding = 56;
    }

    memset(p, 0, padding);
    mctx->block[14] = endian_swap(mctx->byte_count << 3);
    mctx->block[15] = endian_swap(mctx->byte_count >> 29);

#if 0
    le32_to_cpu_array(mctx->block, (sizeof(mctx->block) -
                      sizeof(u64)) / sizeof(u32));
#endif

    md5_transform(mctx, mctx->hash, mctx->block);                                                 

    CRTCL_SECT_START;

    *((u32 *) out + 0) = endian_swap (hashs->D1R);
    *((u32 *) out + 1) = endian_swap (hashs->D2R);
    *((u32 *) out + 2) = endian_swap (hashs->D3R);
    *((u32 *) out + 3) = endian_swap (hashs->D4R);

    CRTCL_SECT_END;

    // Wipe context
    memset(mctx, 0, sizeof(*mctx));

    return 0;
}

/*
 * \brief MD5 function mappings
*/
static struct shash_alg ifxdeu_md5_alg = {
    .digestsize         =       MD5_DIGEST_SIZE,
    .init               =       md5_init,
    .update             =       md5_update,
    .final              =       md5_final,
    .descsize           =       sizeof(struct md5_ctx),
    .base               =       {
                .cra_name       =       "md5",
                .cra_driver_name=       "ifxdeu-md5",
                .cra_priority   =       300,
                .cra_flags      =       CRYPTO_ALG_TYPE_DIGEST,
                .cra_blocksize  =       MD5_HMAC_BLOCK_SIZE,
                .cra_module     =       THIS_MODULE,
    }
};

/*! \fn int __init ifxdeu_init_md5 (void)
 *  \ingroup IFX_MD5_FUNCTIONS
 *  \brief initialize md5 driver   
*/                                 
int __init ifxdeu_init_md5 (void)
{
    int ret = -ENOSYS;


    if ((ret = crypto_register_shash(&ifxdeu_md5_alg)))
        goto md5_err;

    CRTCL_SECT_INIT;

    printk (KERN_NOTICE "IFX DEU MD5 initialized%s.\n", disable_deudma ? "" : " (DMA)");
    return ret;

md5_err:
    printk(KERN_ERR "IFX DEU MD5 initialization failed!\n");
    return ret;
}

/*! \fn void __exit ifxdeu_fini_md5 (void)
  * \ingroup IFX_MD5_FUNCTIONS
  * \brief unregister md5 driver   
*/                  
               
void __exit ifxdeu_fini_md5 (void)
{
    crypto_unregister_shash(&ifxdeu_md5_alg);

}

