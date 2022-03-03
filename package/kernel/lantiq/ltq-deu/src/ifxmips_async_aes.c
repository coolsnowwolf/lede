/******************************************************************************
**
** FILE NAME    : ifxmips_async_aes.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module
**
** DATE         : October 11, 2010
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver for AES Algorithm
** COPYRIGHT    :       Copyright (c) 2010
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
** 11, Oct 2010 Mohammad Firdaus    Kernel Port incl. Async. Ablkcipher mode
** 21,March 2011 Mohammad Firdaus   Changes for Kernel 2.6.32 and IPSec integration
*******************************************************************************/
/*!
 \defgroup IFX_DEU IFX_DEU_DRIVERS
 \ingroup API
 \brief ifx DEU driver module
*/

/*!
  \file ifxmips_async_aes.c
  \ingroup IFX_DEU
  \brief AES Encryption Driver main file
*/

/*!
 \defgroup IFX_AES_FUNCTIONS IFX_AES_FUNCTIONS
 \ingroup IFX_DEU
 \brief IFX AES driver Functions
*/



#include <linux/wait.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <crypto/ctr.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>

#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/irq.h>
#include <asm/ifx/ifx_pmu.h>
#include <asm/ifx/ifx_gpio.h>
#include <asm/kmap_types.h>

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
//#define AES_START   IFX_AES_CON
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


static int disable_multiblock = 0;
module_param(disable_multiblock, int, 0);

static int disable_deudma = 1;

/* Function decleration */
int aes_chip_init(void);
u32 endian_swap(u32 input);
u32 input_swap(u32 input);
u32* memory_alignment(const u8 *arg, u32 *buff_alloc, int in_out, int nbytes);
void aes_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
int aes_memory_allocate(int value);
int des_memory_allocate(int value);
void memory_release(u32 *addr);


struct aes_ctx {
    int key_length;
    u32 buf[AES_MAX_KEY_SIZE];
    u8 nonce[CTR_RFC3686_NONCE_SIZE];

};

struct aes_container {
    u8 *iv;
    u8 *src_buf;
    u8 *dst_buf;

    int mode;
    int encdec;
    int complete;
    int flag;

    u32 bytes_processed;
    u32 nbytes;

    struct ablkcipher_request arequest;
 
};

aes_priv_t *aes_queue;
extern deu_drv_priv_t deu_dma_priv;

void hexdump(unsigned char *buf, unsigned int len)
{
        print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
                        16, 1,
                        buf, len, false);
}

/*! \fn void lq_deu_aes_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, 
                             size_t nbytes, int encdec, int mode)
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

static int lq_deu_aes_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
                            u8 *iv_arg, size_t nbytes, int encdec, int mode)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    volatile struct aes_t *aes = (volatile struct aes_t *) AES_START;
    struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
    u32 *in_key = ctx->buf;
    unsigned long flag;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    int key_len = ctx->key_length;

    volatile struct deu_dma_t *dma = (struct deu_dma_t *) IFX_DEU_DMA_CON;
    struct dma_device_info *dma_device = ifx_deu[0].dma_device;
    deu_drv_priv_t *deu_priv = (deu_drv_priv_t *)dma_device->priv;
    int wlen = 0;
    //u32 *outcopy = NULL;
    u32 *dword_mem_aligned_in = NULL;

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
        return -EINVAL;
    }

    /* let HW pre-process DEcryption key in any case (even if
       ENcryption is used). Key Valid (KV) bit is then only
       checked in decryption routine! */
    aes->controlr.PNK = 1;

    while (aes->controlr.BUS) {
        // this will not take long
    }
    AES_DMA_MISC_CONFIG();

    aes->controlr.E_D = !encdec;    //encryption
    aes->controlr.O = mode; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR 

    //aes->controlr.F = 128; //default; only for CFB and OFB modes; change only for customer-specific apps
    if (mode > 0) {
        aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
        aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
        aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
    };


    /* Prepare Rx buf length used in dma psuedo interrupt */
    deu_priv->deu_rx_buf = (u32 *)out_arg;
    deu_priv->deu_rx_len = nbytes;
   
    /* memory alignment issue */ 
    dword_mem_aligned_in = (u32 *) DEU_DWORD_REORDERING(in_arg, aes_buff_in, BUFFER_IN, nbytes);

    dma->controlr.ALGO = 1;   //AES
    dma->controlr.BS = 0;
    aes->controlr.DAU = 0;
    dma->controlr.EN = 1;

    while (aes->controlr.BUS) {
         // wait for AES to be ready
    };

    deu_priv->outcopy = (u32 *) DEU_DWORD_REORDERING(out_arg, aes_buff_out, BUFFER_OUT, nbytes);
    deu_priv->event_src = AES_ASYNC_EVENT;

    wlen = dma_device_write (dma_device, (u8 *)dword_mem_aligned_in, nbytes, NULL);
    if (wlen != nbytes) {
        dma->controlr.EN = 0;
        CRTCL_SECT_END;
        printk (KERN_ERR "[%s %s %d]: dma_device_write fail!\n", __FILE__, __func__, __LINE__);
        return -EINVAL;
    }

   // WAIT_AES_DMA_READY();

    CRTCL_SECT_END;

    if (mode > 0) {
        *((u32 *) iv_arg) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg));
        *((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        *((u32 *) iv_arg + 2) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
        *((u32 *) iv_arg + 3) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
    }
   
    return -EINPROGRESS;	
}

/* \fn static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Counts and return the number of scatterlists 
 * \param *sl Function pointer to the scatterlist
 * \param total_bytes The total number of bytes that needs to be encrypted/decrypted
 * \return The number of scatterlists 
*/

static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
{
        int i = 0;

        do {
                total_bytes -= sl[i].length;
                i++;

        } while (total_bytes > 0);

        return i;
}

/* \fn void lq_sg_init(struct scatterlist *src,
 *                     struct scatterlist *dst)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Maps the scatterlists into a source/destination page. 
 * \param *src Pointer to the source scatterlist
 * \param *dst Pointer to the destination scatterlist
*/

static void lq_sg_init(struct aes_container *aes_con,struct scatterlist *src,
                       struct scatterlist *dst)
{

    struct page *dst_page, *src_page;

    src_page = sg_virt(src);
    aes_con->src_buf = (char *) src_page;

    dst_page = sg_virt(dst);
    aes_con->dst_buf = (char *) dst_page;

}


/* \fn static void lq_sg_complete(struct aes_container *aes_con) 
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Free the used up memory after encryt/decrypt.
*/

static void lq_sg_complete(struct aes_container *aes_con) 
{
    unsigned long queue_flag;

    spin_lock_irqsave(&aes_queue->lock, queue_flag);
    kfree(aes_con);
    spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
}

/* \fn static inline struct aes_container *aes_container_cast (
 *                     struct scatterlist *dst)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Locate the structure aes_container in memory.
 * \param *areq Pointer to memory location where ablkcipher_request is located
 * \return *aes_cointainer The function pointer to aes_container
*/
static inline struct aes_container *aes_container_cast (
		struct ablkcipher_request *areq)
{
    return container_of(areq, struct aes_container, arequest);
}


/* \fn static int process_next_packet(struct aes_container *aes_con, struct ablkcipher_request *areq,
 * \                                  int state)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Process next packet to be encrypt/decrypt
 * \param *aes_con  AES container structure
 * \param *areq Pointer to memory location where ablkcipher_request is located 
 * \param state The state of the current packet (part of scatterlist or new packet)
 * \return -EINVAL: error, -EINPROGRESS: Crypto still running, 1: no more scatterlist 
*/

static int process_next_packet(struct aes_container *aes_con, struct ablkcipher_request *areq,
                               int state)
{
    u8 *iv;
    int mode, dir, err = -EINVAL;
    unsigned long queue_flag;
    u32 inc, nbytes, remain, chunk_size;
    struct scatterlist *src = NULL;
    struct scatterlist *dst = NULL;
    struct crypto_ablkcipher *cipher;
    struct aes_ctx *ctx;

    spin_lock_irqsave(&aes_queue->lock, queue_flag);

    dir = aes_con->encdec;
    mode = aes_con->mode;
    iv = aes_con->iv;
 
    if (state & PROCESS_SCATTER) {
        src = scatterwalk_sg_next(areq->src);
        dst = scatterwalk_sg_next(areq->dst);
 
        if (!src || !dst) {
            spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
            return 1;
        }
    }
    else if (state & PROCESS_NEW_PACKET) { 
        src = areq->src;
        dst = areq->dst;
    }

    remain = aes_con->bytes_processed;
    chunk_size = src->length;

    if (remain > DEU_MAX_PACKET_SIZE)
       inc = DEU_MAX_PACKET_SIZE;
    else if (remain > chunk_size)
       inc = chunk_size;
    else
       inc = remain;

    remain -= inc;
    aes_con->nbytes = inc;
 
    if (state & PROCESS_SCATTER) {
        aes_con->src_buf += aes_con->nbytes;
        aes_con->dst_buf += aes_con->nbytes;
    }

    lq_sg_init(aes_con, src, dst);

    nbytes = aes_con->nbytes;

    //printk("debug - Line: %d, func: %s, reqsize: %d, scattersize: %d\n",
    //          __LINE__, __func__, nbytes, chunk_size);

    cipher = crypto_ablkcipher_reqtfm(areq);
    ctx = crypto_ablkcipher_ctx(cipher);


    if (aes_queue->hw_status == AES_IDLE)
        aes_queue->hw_status = AES_STARTED;

    aes_con->bytes_processed -= aes_con->nbytes;
    err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);
    if (err == -EBUSY) {
        spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
        printk("Failed to enqueue request, ln: %d, err: %d\n",
                __LINE__, err);
        return -EINVAL;
    }

    spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

    err = lq_deu_aes_core(ctx, aes_con->dst_buf, aes_con->src_buf, iv, nbytes, dir, mode);
    return err;

}

/* \fn static void process_queue (unsigned long data)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief tasklet to signal the dequeuing of the next packet to be processed
 * \param unsigned long data Not used
 * \return void 
*/

static void process_queue(unsigned long data)
{

    DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait, AES_ASYNC_EVENT,
                deu_dma_priv.aes_event_flags);
}


/* \fn static int aes_crypto_thread (void *data)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief AES thread that handles crypto requests from upper layer & DMA
 * \param *data Not used
 * \return -EINVAL: DEU failure, -EBUSY: DEU HW busy, 0: exit thread
*/
static int aes_crypto_thread (void *data)
{
    struct aes_container *aes_con = NULL;
    struct ablkcipher_request *areq = NULL;
    int err;
    unsigned long queue_flag;
    
    daemonize("lq_aes_thread");
    printk("AES Queue Manager Starting\n");

    while (1)
    {
        DEU_WAIT_EVENT(deu_dma_priv.deu_thread_wait, AES_ASYNC_EVENT, 
                       deu_dma_priv.aes_event_flags);

        spin_lock_irqsave(&aes_queue->lock, queue_flag);
   
        /* wait to prevent starting a crypto session before
        * exiting the dma interrupt thread.
        */
        if (aes_queue->hw_status == AES_STARTED) {
            areq = ablkcipher_dequeue_request(&aes_queue->list);
            aes_con = aes_container_cast(areq);
            aes_queue->hw_status = AES_BUSY;
        }
        else if (aes_queue->hw_status == AES_IDLE) {
            areq = ablkcipher_dequeue_request(&aes_queue->list);
            aes_con = aes_container_cast(areq);
            aes_queue->hw_status = AES_STARTED;
        }
        else if (aes_queue->hw_status == AES_BUSY) {
            areq = ablkcipher_dequeue_request(&aes_queue->list);
            aes_con = aes_container_cast(areq);
	}
        else if (aes_queue->hw_status == AES_COMPLETED) {
            lq_sg_complete(aes_con);
            aes_queue->hw_status = AES_IDLE;
            areq->base.complete(&areq->base, 0);
            spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
	    return 0;
	}
        //printk("debug ln: %d, bytes proc: %d\n", __LINE__, aes_con->bytes_processed);
        spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

        if (!aes_con) {
           printk("AES_CON return null\n");
           goto aes_done;
	}

        if (aes_con->bytes_processed == 0) {
            goto aes_done;
        }
       
        /* Process new packet or the next packet in a scatterlist */
        if (aes_con->flag & PROCESS_NEW_PACKET) {
           aes_con->flag = PROCESS_SCATTER;
           err = process_next_packet(aes_con, areq, PROCESS_NEW_PACKET);
        }
        else 
            err = process_next_packet(aes_con, areq, PROCESS_SCATTER);
 
        if (err == -EINVAL) {
            areq->base.complete(&areq->base, err);
            lq_sg_complete(aes_con);
            printk("src/dst returned -EINVAL in func: %s\n", __func__);
        }
        else if (err > 0) {
            printk("src/dst returned zero in func: %s\n", __func__);
            goto aes_done; 
        }
        
	continue;

aes_done:
        //printk("debug line - %d, func: %s, qlen: %d\n", __LINE__, __func__, aes_queue->list.qlen);
        areq->base.complete(&areq->base, 0);    
        lq_sg_complete(aes_con);

        spin_lock_irqsave(&aes_queue->lock, queue_flag);
        if (aes_queue->list.qlen > 0) {
            spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
            tasklet_schedule(&aes_queue->aes_task); 
        }
        else {
            aes_queue->hw_status = AES_IDLE;
            spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
        }
    } //while(1)
    
    return 0;
} 

/* \fn static int lq_aes_queue_mgr(struct aes_ctx *ctx, struct ablkcipher_request *areq, 
                            u8 *iv, int dir, int mode)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief starts the process of queuing DEU requests
 * \param *ctx crypto algo contax
 * \param *areq Pointer to the balkcipher requests
 * \param *iv Pointer to intput vector location
 * \param dir Encrypt/Decrypt
 * \mode The mode AES algo is running
 * \return 0 if success
*/

static int lq_aes_queue_mgr(struct aes_ctx *ctx, struct ablkcipher_request *areq, 
                            u8 *iv, int dir, int mode)
{
    int err = -EINVAL; 
    unsigned long queue_flag;
    struct scatterlist *src = areq->src;
    struct scatterlist *dst = areq->dst;
    struct aes_container *aes_con = NULL;
    u32 remain, inc, nbytes = areq->nbytes;
    u32 chunk_bytes = src->length;
    
 
    aes_con = (struct aes_container *)kmalloc(sizeof(struct aes_container),
    	                                       GFP_KERNEL);

    if (!(aes_con)) {
        printk("Cannot allocate memory for AES container, fn %s, ln %d\n",
		__func__, __LINE__);
	return -ENOMEM;
    }

    /* AES encrypt/decrypt mode */
    if (mode == 5) {
        nbytes = AES_BLOCK_SIZE;
        chunk_bytes = AES_BLOCK_SIZE;
        mode = 0;
    }

    aes_con->bytes_processed = nbytes;
    aes_con->arequest = *(areq);
    remain = nbytes;

    //printk("debug - Line: %d, func: %s, reqsize: %d, scattersize: %d\n",
    //        __LINE__, __func__, nbytes, chunk_bytes);

    if (remain > DEU_MAX_PACKET_SIZE) 
       inc = DEU_MAX_PACKET_SIZE;
    else if (remain > chunk_bytes)
       inc = chunk_bytes; 
    else
       inc = remain;
         
    remain -= inc;
    lq_sg_init(aes_con, src, dst);  

    if (remain <= 0)
        aes_con->complete = 1;
    else
        aes_con->complete = 0;

    aes_con->nbytes = inc;
    aes_con->iv = iv;
    aes_con->mode = mode;
    aes_con->encdec = dir;
 
    spin_lock_irqsave(&aes_queue->lock, queue_flag);

    if (aes_queue->hw_status == AES_STARTED || aes_queue->hw_status == AES_BUSY ||
             aes_queue->list.qlen > 0) {

        aes_con->flag = PROCESS_NEW_PACKET;
        err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);

         /* max queue length reached */
        if (err == -EBUSY) {
            spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
            printk("Unable to enqueue request ln: %d, err: %d\n", __LINE__, err);
             return err;
         }

        spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
        return -EINPROGRESS;
    }
    else if (aes_queue->hw_status == AES_IDLE) 
        aes_queue->hw_status = AES_STARTED;

    aes_con->flag = PROCESS_SCATTER;
    aes_con->bytes_processed -= aes_con->nbytes;
    /* or enqueue the whole structure so as to get back the info 
     * at the moment that it's queued. nbytes might be different */
    err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);

    if (err == -EBUSY) {
        spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
        printk("Unable to enqueue request ln: %d, err: %d\n", __LINE__, err);
        return err;
    }

    spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
    return lq_deu_aes_core(ctx, aes_con->dst_buf, aes_con->src_buf, iv, inc, dir, mode);

}

/* \fn static int aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *                     unsigned int keylen)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *in_key Pointer to input keys
 * \param key_len Length of the AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
                      unsigned int keylen)
{
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(tfm); 
    unsigned long *flags = (unsigned long *) &tfm->base.crt_flags;

    DPRINTF(2, "set_key in %s\n", __FILE__);

    if (keylen != 16 && keylen != 24 && keylen != 32) {
        *flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    ctx->key_length = keylen;
    DPRINTF(0, "ctx @%p, keylen %d, ctx->key_length %d\n", ctx, keylen, ctx->key_length);
    memcpy ((u8 *) (ctx->buf), in_key, keylen);

    return 0;

}

/* \fn static int aes_generic_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *                     unsigned int keylen)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *key Pointer to input keys
 * \param keylen Length of AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int aes_generic_setkey(struct crypto_ablkcipher *tfm, const u8 *key,
                              unsigned int keylen)
{
   return aes_setkey(tfm, key, keylen);
}

/* \fn static int rfc3686_aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *                     unsigned int keylen)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *in_key Pointer to input keys
 * \param key_len Length of the AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int rfc3686_aes_setkey(struct crypto_ablkcipher *tfm,
                             const u8 *in_key, unsigned int keylen)
{
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(tfm);
    unsigned long *flags = (unsigned long *)&tfm->base.crt_flags;

    DPRINTF(2, "ctr_rfc3686_aes_set_key in %s\n", __FILE__);

    memcpy(ctx->nonce, in_key + (keylen - CTR_RFC3686_NONCE_SIZE),
           CTR_RFC3686_NONCE_SIZE);

    keylen -= CTR_RFC3686_NONCE_SIZE; // remove 4 bytes of nonce

    if (keylen != 16 && keylen != 24 && keylen != 32) {
        *flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    ctx->key_length = keylen;

    memcpy ((u8 *) (ctx->buf), in_key, keylen);

    return 0;
}

/* \fn static int aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_ENCRYPT, 5);

}

/* \fn static int aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
static int aes_decrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_DECRYPT, 5);
}

/* \fn static int ecb_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ecb_aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 0);

}
/* \fn static int ecb_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/ 
static int ecb_aes_decrypt(struct ablkcipher_request *areq)

{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 0);
}

/* \fn static int cbc_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int cbc_aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 1);

}

/* \fn static int cbc_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int cbc_aes_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 1);
}
#if 0
static int ofb_aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 2);

}

static int ofb_aes_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 2);
}

static int cfb_aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 3);

}

static int cfb_aes_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 3);
}
#endif	

/* \fn static int ctr_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ctr_aes_encrypt (struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);
   
    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 4);

}

/* \fn static int ctr_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ctr_aes_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 4);
}

/* \fn static int rfc3686_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int rfc3686_aes_encrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);
    int ret;
    u8 *info = areq->info;
    u8 rfc3686_iv[16];

    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, info, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    areq->info = rfc3686_iv;
    ret = lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 4);
    areq->info = info;
    return ret;
}

/* \fn static int rfc3686_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int rfc3686_aes_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);
    int ret;
    u8 *info = areq->info;
    u8 rfc3686_iv[16];

    /* set up counter block */
    memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
    memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, info, CTR_RFC3686_IV_SIZE);

    /* initialize counter portion of counter block */
    *(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
        cpu_to_be32(1);

    areq->info = rfc3686_iv;
    ret = lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 4);
    areq->info = info;
    return ret;
}

struct lq_aes_alg {
    struct crypto_alg alg;
};

/* AES supported algo array */
static struct lq_aes_alg aes_drivers_alg[] = {
     {
         .alg = {
           .cra_name 	    = "aes",
           .cra_driver_name = "ifxdeu-aes",
           .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
           .cra_blocksize   = AES_BLOCK_SIZE,
           .cra_ctxsize     = sizeof(struct aes_ctx),
           .cra_type        = &crypto_ablkcipher_type,
           .cra_priority    = 300,
           .cra_module      = THIS_MODULE,
           .cra_ablkcipher = {
                                .setkey = aes_setkey,
                                .encrypt = aes_encrypt,
                                .decrypt = aes_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = AES_MIN_KEY_SIZE,
                                .max_keysize = AES_MAX_KEY_SIZE,
                                .ivsize = AES_BLOCK_SIZE,
             }
          }
     },{
     .alg = {
           .cra_name        = "ecb(aes)",
           .cra_driver_name = "ifxdeu-ecb(aes)",
           .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
           .cra_blocksize   = AES_BLOCK_SIZE,
           .cra_ctxsize     = sizeof(struct aes_ctx),
           .cra_type        = &crypto_ablkcipher_type,
           .cra_priority    = 400,
           .cra_module      = THIS_MODULE,
           .cra_ablkcipher = {
                                .setkey = aes_generic_setkey,
                                .encrypt = ecb_aes_encrypt,
                                .decrypt = ecb_aes_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = AES_MIN_KEY_SIZE,
                                .max_keysize = AES_MAX_KEY_SIZE,
                                .ivsize = AES_BLOCK_SIZE,
             }      
          }
     },{
         .alg = {
           .cra_name        = "cbc(aes)",
           .cra_driver_name = "ifxdeu-cbc(aes)",
           .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
           .cra_blocksize   = AES_BLOCK_SIZE,
           .cra_ctxsize     = sizeof(struct aes_ctx),
           .cra_type        = &crypto_ablkcipher_type,
           .cra_priority    = 400,
           .cra_module      = THIS_MODULE,
           .cra_ablkcipher = {
                                .setkey = aes_generic_setkey,
                                .encrypt = cbc_aes_encrypt,
                                .decrypt = cbc_aes_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = AES_MIN_KEY_SIZE,
                                .max_keysize = AES_MAX_KEY_SIZE,
                                .ivsize = AES_BLOCK_SIZE,
                }
          }
     },{
         .alg = {
           .cra_name        = "ctr(aes)",
           .cra_driver_name = "ifxdeu-ctr(aes)",
           .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
           .cra_blocksize   = AES_BLOCK_SIZE,
           .cra_ctxsize     = sizeof(struct aes_ctx),
           .cra_type        = &crypto_ablkcipher_type,
           .cra_priority    = 400,
           .cra_module      = THIS_MODULE,
           .cra_ablkcipher = {
                                .setkey = aes_generic_setkey,
                                .encrypt = ctr_aes_encrypt,
                                .decrypt = ctr_aes_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = AES_MIN_KEY_SIZE,
                                .max_keysize = AES_MAX_KEY_SIZE,
                                .ivsize = AES_BLOCK_SIZE,
                }
         }
     },{
     .alg = {
           .cra_name        = "rfc3686(ctr(aes))",
           .cra_driver_name = "ifxdeu-rfc3686(ctr(aes))",
           .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_KERN_DRIVER_ONLY | CRYPTO_ALG_ASYNC,
           .cra_blocksize   = AES_BLOCK_SIZE,
           .cra_ctxsize     = sizeof(struct aes_ctx),
           .cra_type        = &crypto_ablkcipher_type,
           .cra_priority    = 400,
           .cra_module      = THIS_MODULE,
           .cra_ablkcipher = {
                                .setkey = rfc3686_aes_setkey,
                                .encrypt = rfc3686_aes_encrypt,
                                .decrypt = rfc3686_aes_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = AES_MIN_KEY_SIZE,
                                .max_keysize = CTR_RFC3686_MAX_KEY_SIZE,
				//.max_keysize = AES_MAX_KEY_SIZE,
                                //.ivsize = CTR_RFC3686_IV_SIZE,
				.ivsize = AES_BLOCK_SIZE,  // else cannot reg
               }
         }
      }
};

/* \fn int __init lqdeu_async_aes_init (void)
 * \ingroup IFX_AES_FUNCTIONS
 * \brief Initializes the Async. AES driver
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

int __init lqdeu_async_aes_init (void)
{
    int i, j, ret = -EINVAL; 

#define IFX_DEU_DRV_VERSION  "2.0.0"
    printk(KERN_INFO "Lantiq Technologies DEU Driver version %s\n", IFX_DEU_DRV_VERSION);

    for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++) {
        ret = crypto_register_alg(&aes_drivers_alg[i].alg);
	printk("driver: %s\n", aes_drivers_alg[i].alg.cra_name);
        if (ret)
            goto aes_err;
    }

    aes_chip_init();

    CRTCL_SECT_INIT;


    printk (KERN_NOTICE "Lantiq DEU AES initialized %s %s.\n", 
           disable_multiblock ? "" : " (multiblock)", disable_deudma ? "" : " (DMA)");
    
    return ret;

aes_err:
    
    for (j = 0; j < i; j++) 
        crypto_unregister_alg(&aes_drivers_alg[j].alg);
    
    printk(KERN_ERR "Lantiq %s driver initialization failed!\n", (char *)&aes_drivers_alg[i].alg.cra_driver_name);
    return ret;

ctr_rfc3686_aes_err:
    for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++) {
        if (!strcmp((char *)&aes_drivers_alg[i].alg.cra_name, "rfc3686(ctr(aes))"))
            crypto_unregister_alg(&aes_drivers_alg[j].alg);
    }
    printk (KERN_ERR "Lantiq ctr_rfc3686_aes initialization failed!\n");
    return ret;
}

/*! \fn void __exit ifxdeu_fini_aes (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief unregister aes driver
*/
void __exit lqdeu_fini_async_aes (void)
{
    int i;
  
    for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++)
        crypto_unregister_alg(&aes_drivers_alg[i].alg);

    aes_queue->hw_status = AES_COMPLETED;

    DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait, AES_ASYNC_EVENT,
                                 deu_dma_priv.aes_event_flags);   

    kfree(aes_queue); 

}
