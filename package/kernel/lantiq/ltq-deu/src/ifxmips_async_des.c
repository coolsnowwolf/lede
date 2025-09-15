/******************************************************************************
**
** FILE NAME    : ifxmips_async_des.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module
**
** DATE         : October 11, 2010
** AUTHOR       : Mohammad Firdaus
** DESCRIPTION  : Data Encryption Unit Driver for DES Algorithm
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
  \file ifxmips_async_des.c
  \ingroup IFX_DEU
  \brief DES Encryption Driver main file
*/

/*!
 \defgroup IFX_DES_FUNCTIONS IFX_DES_FUNCTIONS
 \ingroup IFX_DEU
 \brief IFX DES driver Functions
*/

#include <linux/wait.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
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

/* DMA specific header and variables */

spinlock_t des_lock;
#define CRTCL_SECT_INIT        spin_lock_init(&des_lock)
#define CRTCL_SECT_START       spin_lock_irqsave(&des_lock, flag)
#define CRTCL_SECT_END         spin_unlock_irqrestore(&des_lock, flag)

/* Preprocessor declerations */
#ifdef CRYPTO_DEBUG
extern char debug_level;
#define DPRINTF(level, format, args...) if (level < debug_level) printk(KERN_INFO "[%s %s %d]: " format, __FILE__, __func__, __LINE__, ##args);
#else
#define DPRINTF(level, format, args...)
#endif
//#define DES_3DES_START  IFX_DES_CON
#define DES_KEY_SIZE            8
#define DES_EXPKEY_WORDS        32
#define DES_BLOCK_SIZE          8
#define DES3_EDE_KEY_SIZE       (3 * DES_KEY_SIZE)
#define DES3_EDE_EXPKEY_WORDS   (3 * DES_EXPKEY_WORDS)
#define DES3_EDE_BLOCK_SIZE     DES_BLOCK_SIZE

/* Function Declaration to prevent warning messages */
void des_chip_init (void);
u32 endian_swap(u32 input);
u32 input_swap(u32 input);
int aes_memory_allocate(int value);
int des_memory_allocate(int value);
void memory_release(u32 *buffer);
u32* memory_alignment(const u8 *arg, u32 *buff_alloc, int in_out, int nbytes);
void aes_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
void des_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);

static int lq_deu_des_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
             u8 *iv_arg, u32 nbytes, int encdec, int mode);

struct des_ctx {
        int controlr_M;
        int key_length;
        u8 iv[DES_BLOCK_SIZE];
        u32 expkey[DES3_EDE_EXPKEY_WORDS];
};


static int disable_multiblock = 0;
module_param(disable_multiblock, int, 0);

static int disable_deudma = 1;

struct des_container {
    u8 *iv;
    u8 *dst_buf;
    u8 *src_buf;
    int mode;
    int encdec;
    int complete;
    int flag;

    u32 bytes_processed;
    u32 nbytes;

    struct ablkcipher_request arequest;
};

des_priv_t *des_queue;
extern deu_drv_priv_t deu_dma_priv;

void hexdump1(unsigned char *buf, unsigned int len)
{
        print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
                        16, 1,
                        buf, len, false);
}


/*! \fn int lq_des_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief sets DES key
 *  \param tfm linux crypto algo transform
 *  \param key input key
 *  \param keylen key length
*/
static int lq_des_setkey(struct crypto_ablkcipher *tfm, const u8 *key,
                         unsigned int keylen)
{
        struct des_ctx *dctx = crypto_ablkcipher_ctx(tfm);

        //printk("setkey in %s\n", __FILE__);

        dctx->controlr_M = 0;   // des
        dctx->key_length = keylen;

        memcpy ((u8 *) (dctx->expkey), key, keylen);

        return 0;
}

/*! \fn int lq_des3_ede_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief sets DES key
 *  \param tfm linux crypto algo transform
 *  \param key input key
 *  \param keylen key length
*/

static int lq_des3_ede_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
                      unsigned int keylen)
{
    struct des_ctx *dctx = crypto_ablkcipher_ctx(tfm);

    //printk("setkey in %s\n", __FILE__);

    dctx->controlr_M = keylen/8 + 1;   // des
    dctx->key_length = keylen;

    memcpy ((u8 *) (dctx->expkey), in_key, keylen);

    return 0;
}

/*! \fn void ifx_deu_des_core(void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, u32 nbytes, int encdec, int mode)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief main interface to DES hardware
 *  \param ctx_arg crypto algo context
 *  \param out_arg output bytestream
 *  \param in_arg input bytestream
 *  \param iv_arg initialization vector
 *  \param nbytes length of bytestream
 *  \param encdec 1 for encrypt; 0 for decrypt
 *  \param mode operation mode such as ebc, cbc
*/

static int lq_deu_des_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
             u8 *iv_arg, u32 nbytes, int encdec, int mode)
{
        volatile struct des_t *des = (struct des_t *) DES_3DES_START;
        struct des_ctx *dctx = ctx_arg;
        u32 *key = dctx->expkey;
        unsigned long flag;

        int i = 0;
        int nblocks = 0;

        CRTCL_SECT_START;

        des->controlr.M = dctx->controlr_M;
        if (dctx->controlr_M == 0)      // des
        {
                des->K1HR = DEU_ENDIAN_SWAP(*((u32 *) key + 0));
                des->K1LR = DEU_ENDIAN_SWAP(*((u32 *) key + 1));

        }
        else {
                /* Hardware Section */
                switch (dctx->key_length) {
                case 24:
                        des->K3HR = DEU_ENDIAN_SWAP(*((u32 *) key + 4));
                        des->K3LR = DEU_ENDIAN_SWAP(*((u32 *) key + 5));
                        /* no break; */

                case 16:
                        des->K2HR = DEU_ENDIAN_SWAP(*((u32 *) key + 2));
                        des->K2LR = DEU_ENDIAN_SWAP(*((u32 *) key + 3));

                        /* no break; */
                case 8:
                        des->K1HR = DEU_ENDIAN_SWAP(*((u32 *) key + 0));
                        des->K1LR = DEU_ENDIAN_SWAP(*((u32 *) key + 1));
                        break;

                default:
                        CRTCL_SECT_END;
                        return -EINVAL;
                }
        }

        des->controlr.E_D = !encdec;    //encryption
        des->controlr.O = mode; //0 ECB 1 CBC 2 OFB 3 CFB 4 CTR hexdump(prin,sizeof(*des));

        if (mode > 0) {
                des->IVHR = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
                des->IVLR = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
        };

    /* memory alignment issue */
    dword_mem_aligned_in = (u32 *) DEU_DWORD_REORDERING(in_arg, des_buff_in, BUFFER_IN, nbytes);
    
    deu_priv->deu_rx_buf = (u32 *) out_arg;
    deu_priv->deu_rx_len = nbytes;

    dma->controlr.ALGO = 0;       //DES
    des->controlr.DAU = 0;
    dma->controlr.BS = 0;
    dma->controlr.EN = 1;

    while (des->controlr.BUS) {
    };

    wlen = dma_device_write (dma_device, (u8 *) dword_mem_aligned_in, nbytes, NULL);
    if (wlen != nbytes) {
        dma->controlr.EN = 0;
        CRTCL_SECT_END;
        printk (KERN_ERR "[%s %s %d]: dma_device_write fail!\n", __FILE__, __func__, __LINE__);
        return -EINVAL;
    }


    /* Prepare Rx buf length used in dma psuedo interrupt */
    outcopy = (u32 *) DEU_DWORD_REORDERING(out_arg, des_buff_out, BUFFER_OUT, nbytes);
    deu_priv->outcopy = outcopy;
    deu_priv->event_src = DES_ASYNC_EVENT;
     
    if (mode > 0) {
        *(u32 *) iv_arg = DEU_ENDIAN_SWAP(des->IVHR);
        *((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(des->IVLR);
    };

    CRTCL_SECT_END; 

    return -EINPROGRESS;

}

static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
{
        int i = 0;

        do {
                total_bytes -= sl[i].length;
                i++;

        } while (total_bytes > 0);

        return i;
}

/* \fn static inline struct des_container *des_container_cast (
 *                     struct scatterlist *dst)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Locate the structure des_container in memory.
 * \param *areq Pointer to memory location where ablkcipher_request is located
 * \return *des_cointainer The function pointer to des_container
*/

static inline struct des_container *des_container_cast(
                        struct ablkcipher_request *areq)
{
    return container_of(areq, struct des_container, arequest);
}

/* \fn static void lq_sg_complete(struct des_container *des_con)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Free the used up memory after encryt/decrypt.
*/

static void lq_sg_complete(struct des_container *des_con)
{
    unsigned long queue_flag;
  
    spin_lock_irqsave(&des_queue->lock, queue_flag);
    kfree(des_con); 
    spin_unlock_irqrestore(&des_queue->lock, queue_flag);
}

/* \fn void lq_sg_init(struct scatterlist *src,
 *                     struct scatterlist *dst)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Maps the scatterlists into a source/destination page.
 * \param *src Pointer to the source scatterlist
 * \param *dst Pointer to the destination scatterlist
*/

static void lq_sg_init(struct des_container *des_con, struct scatterlist *src,
                       struct scatterlist *dst)
{
    struct page *dst_page, *src_page;

    src_page = sg_virt(src);
    des_con->src_buf = (char *) src_page;

    dst_page = sg_virt(dst);
    des_con->dst_buf = (char *) dst_page;
}

/* \fn static int process_next_packet(struct des_container *des_con,  struct ablkcipher_request *areq,
 *                                     int state)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Process the next packet after dequeuing the packet from crypto queue
 * \param *des_con  Pointer to DES container structure
 * \param *areq     Pointer to ablkcipher_request container
 * \param state     State of the packet (scattered packet or new packet to be processed)
 * \return -EINVAL: DEU failure, -EINPROGRESS: DEU encrypt/decrypt in progress, 1: no scatterlist left
*/

static int process_next_packet(struct des_container *des_con,  struct ablkcipher_request *areq,
                               int state) 
{
    u8 *iv;
    int mode, encdec, err = -EINVAL;
    u32 remain, inc, chunk_size, nbytes;
    struct scatterlist *src = NULL;
    struct scatterlist *dst = NULL;
    struct crypto_ablkcipher *cipher;
    struct des_ctx *ctx;
    unsigned long queue_flag;

    spin_lock_irqsave(&des_queue->lock, queue_flag);

    mode = des_con->mode;
    encdec = des_con->encdec;
    iv = des_con->iv;

    if (state & PROCESS_SCATTER) {
        src = scatterwalk_sg_next(areq->src);
        dst = scatterwalk_sg_next(areq->dst);

        if (!src || !dst) {
            spin_unlock_irqrestore(&des_queue->lock, queue_flag);
      	    return 1;
        }
    }
    else if (state & PROCESS_NEW_PACKET) {
        src = areq->src;
        dst = areq->dst;
    }

    remain = des_con->bytes_processed;
    chunk_size = src->length;

    //printk("debug ln: %d, func: %s, reqsize: %d, scattersize: %d\n", 
//		__LINE__, __func__, areq->nbytes, chunk_size);

    if (remain > DEU_MAX_PACKET_SIZE)
        inc = DEU_MAX_PACKET_SIZE;
    else if(remain > chunk_size)
        inc = chunk_size;
    else
        inc = remain;
 
    remain -= inc;
    des_con->nbytes = inc;
    
    if (state & PROCESS_SCATTER) {
        des_con->src_buf += des_con->nbytes;
        des_con->dst_buf += des_con->nbytes;
    } 

    lq_sg_init(des_con, src, dst);

    nbytes = des_con->nbytes;

    cipher = crypto_ablkcipher_reqtfm(areq);
    ctx = crypto_ablkcipher_ctx(cipher);

    if (des_queue->hw_status == DES_IDLE) {
        des_queue->hw_status = DES_STARTED;
    }
    
    des_con->bytes_processed -= des_con->nbytes;
    err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
    if (err == -EBUSY) {
        printk("Failed to enqueue request, ln: %d, err: %d\n",
               __LINE__, err);
        spin_unlock_irqrestore(&des_queue->lock, queue_flag);
        return -EINVAL;
    }

    spin_unlock_irqrestore(&des_queue->lock, queue_flag);
    err = lq_deu_des_core(ctx, des_con->dst_buf, des_con->src_buf, iv, nbytes, encdec, mode);
 
    return err;
}

/* \fn static void process_queue(unsigned long data)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Process next packet in queue
 * \param data not used
 * \return 
*/

static void process_queue(unsigned long data)
{
      DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait, DES_ASYNC_EVENT,
                deu_dma_priv.des_event_flags);

}

/* \fn static int des_crypto_thread (void *data)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief DES thread that handles crypto requests from upper layer & DMA
 * \param *data Not used
 * \return -EINVAL: DEU failure, -EBUSY: DEU HW busy, 0: exit thread
*/

static int des_crypto_thread(void *data)
{
    struct des_container *des_con = NULL;
    struct ablkcipher_request *areq = NULL;
    int err;
    unsigned long queue_flag;

    daemonize("lq_des_thread");
   
    while (1)
    {  
       DEU_WAIT_EVENT(deu_dma_priv.deu_thread_wait, DES_ASYNC_EVENT, 
                       deu_dma_priv.des_event_flags);
       spin_lock_irqsave(&des_queue->lock, queue_flag);

       /* wait to prevent starting a crypto session before 
        * exiting the dma interrupt thread.
        */
       
       if (des_queue->hw_status == DES_STARTED) {
            areq = ablkcipher_dequeue_request(&des_queue->list);
            des_con = des_container_cast(areq);
            des_queue->hw_status = DES_BUSY;
       }
       else if (des_queue->hw_status == DES_IDLE) {
            areq = ablkcipher_dequeue_request(&des_queue->list);
            des_con = des_container_cast(areq);
            des_queue->hw_status = DES_STARTED;
       }
       else if (des_queue->hw_status == DES_BUSY) {
            areq = ablkcipher_dequeue_request(&des_queue->list);
            des_con = des_container_cast(areq);
       }
       else if (des_queue->hw_status == DES_COMPLETED) {
            areq->base.complete(&areq->base, 0);
            lq_sg_complete(des_con);
            des_queue->hw_status = DES_IDLE;
            spin_unlock_irqrestore(&des_queue->lock, queue_flag);
            return 0;
       }
       spin_unlock_irqrestore(&des_queue->lock, queue_flag);
            
       if ((des_con->bytes_processed == 0)) {
            goto des_done;
       }

       if (!des_con) {
           goto des_done;
       }

       if (des_con->flag & PROCESS_NEW_PACKET) { 
           des_con->flag = PROCESS_SCATTER;
           err = process_next_packet(des_con, areq, PROCESS_NEW_PACKET);  
       }
       else
           err = process_next_packet(des_con, areq, PROCESS_SCATTER);  
       
       if (err == -EINVAL) {
           areq->base.complete(&areq->base, err);
           lq_sg_complete(des_con);
           printk("src/dst returned -EINVAL in func: %s\n", __func__);
       }
       else if (err > 0) { 
           printk("src/dst returned zero in func: %s\n", __func__);
           goto des_done;
       }

       continue;
   
des_done:
       //printk("debug line - %d, func: %s, qlen: %d\n", __LINE__, __func__, des_queue->list.qlen);
       areq->base.complete(&areq->base, 0);
       lq_sg_complete(des_con);

       spin_lock_irqsave(&des_queue->lock, queue_flag);
       if (des_queue->list.qlen > 0) {
           spin_unlock_irqrestore(&des_queue->lock, queue_flag);
           tasklet_schedule(&des_queue->des_task);
       } 
       else {
           des_queue->hw_status = DES_IDLE;
           spin_unlock_irqrestore(&des_queue->lock, queue_flag);
       }
    } // while(1)
    
    return 0;

}

/* \fn static int lq_des_queue_mgr(struct des_ctx *ctx, struct ablkcipher_request *areq,
                            u8 *iv, int encdec, int mode)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief starts the process of queuing DEU requests
 * \param *ctx crypto algo contax
 * \param *areq Pointer to the balkcipher requests
 * \param *iv Pointer to intput vector location
 * \param dir Encrypt/Decrypt
 * \mode The mode DES algo is running
 * \return 0 if success
*/

static int lq_queue_mgr(struct des_ctx *ctx, struct ablkcipher_request *areq, 
                        u8 *iv, int encdec, int mode)
{
    int err = -EINVAL;
    unsigned long queue_flag;
    struct scatterlist *src = areq->src;
    struct scatterlist *dst = areq->dst;
    struct des_container *des_con = NULL;
    u32 remain, inc, nbytes = areq->nbytes;
    u32 chunk_bytes = src->length;
   
    des_con = (struct des_container *)kmalloc(sizeof(struct des_container), 
                                       GFP_KERNEL);

    if (!(des_con)) {
        printk("Cannot allocate memory for AES container, fn %s, ln %d\n",
                __func__, __LINE__);
        return -ENOMEM;
    }
  
    /* DES encrypt/decrypt mode  */
    if (mode == 5) {
        nbytes = DES_BLOCK_SIZE;
        chunk_bytes = DES_BLOCK_SIZE;
        mode = 0;
    }

    des_con->bytes_processed = nbytes;
    des_con->arequest = (*areq);
    remain = nbytes;

    //printk("debug - Line: %d, func: %s, reqsize: %d, scattersize: %d\n", 
	//	__LINE__, __func__, nbytes, chunk_bytes);

    if (remain > DEU_MAX_PACKET_SIZE)  
        inc = DEU_MAX_PACKET_SIZE;
    else if(remain > chunk_bytes)
        inc = chunk_bytes;
    else 
        inc = remain;
    
    remain -= inc;
    lq_sg_init(des_con, src, dst);
     
    if (remain <= 0 ) { 
        des_con->complete = 1;
    }
    else 
        des_con->complete = 0;
        
    des_con->nbytes = inc; 
    des_con->iv = iv;
    des_con->mode = mode;
    des_con->encdec = encdec;

    spin_lock_irqsave(&des_queue->lock, queue_flag);

    if (des_queue->hw_status == DES_STARTED || des_queue->hw_status == DES_BUSY ||
        des_queue->list.qlen > 0) {

        des_con->flag = PROCESS_NEW_PACKET;
        err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
        if (err == -EBUSY) {
            spin_unlock_irqrestore(&des_queue->lock, queue_flag); 
            printk("Fail to enqueue ablkcipher request ln: %d, err: %d\n",
                   __LINE__, err);
            return err;
        }

        spin_unlock_irqrestore(&des_queue->lock, queue_flag); 
        return -EINPROGRESS;
              
    }
    else if (des_queue->hw_status == DES_IDLE) {
        des_queue->hw_status = DES_STARTED;            
    }
   
    des_con->flag = PROCESS_SCATTER;
    des_con->bytes_processed -= des_con->nbytes;

    err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
    if (err == -EBUSY) {
        printk("Fail to enqueue ablkcipher request ln: %d, err: %d\n",
	       __LINE__, err);

        spin_unlock_irqrestore(&des_queue->lock, queue_flag);
        return err;
     }
                  
     spin_unlock_irqrestore(&des_queue->lock, queue_flag); 
     return lq_deu_des_core(ctx, des_con->dst_buf, des_con->src_buf, iv, inc, encdec, mode);

}

/* \fn static int lq_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
	
static int lq_des_encrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_ENCRYPT, 5);

}

/* \fn static int lq_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_des_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_DECRYPT, 5);
}

/* \fn static int lq_ecb_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_ecb_des_encrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 0);
}

/* \fn static int lq_ecb_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
static int lq_ecb_des_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 0);

}

/* \fn static int lq_cbc_ecb_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_cbc_des_encrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 1);
}
/* \fn static int lq_cbc_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup IFX_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_cbc_des_decrypt(struct ablkcipher_request *areq)
{
    struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
    struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

    return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 1);
}

struct lq_des_alg {
    struct crypto_alg alg;
};

/* DES Supported algo array */
static struct lq_des_alg des_drivers_alg [] = {
    {
        .alg = {
            .cra_name        = "des",
            .cra_driver_name = "lqdeu-des",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 300,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des_setkey,
                                .encrypt = lq_des_encrypt,
                                .decrypt = lq_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES_KEY_SIZE,
                                .max_keysize = DES_KEY_SIZE,
                                .ivsize = DES_BLOCK_SIZE,
            }
        }

    },{
        .alg = {
            .cra_name        = "ecb(des)",
            .cra_driver_name = "lqdeu-ecb(des)",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 400,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des_setkey,
                                .encrypt = lq_ecb_des_encrypt,
                                .decrypt = lq_ecb_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES_KEY_SIZE,
                                .max_keysize = DES_KEY_SIZE,
                                .ivsize = DES_BLOCK_SIZE,
            }
         }
    },{
        .alg = {
            .cra_name        = "cbc(des)",
            .cra_driver_name = "lqdeu-cbc(des)",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 400,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des_setkey,
                                .encrypt = lq_cbc_des_encrypt,
                                .decrypt = lq_cbc_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES3_EDE_KEY_SIZE,
                                .max_keysize = DES3_EDE_KEY_SIZE,
                                .ivsize = DES3_EDE_BLOCK_SIZE,
            }
         }
    },{
        .alg = {
            .cra_name        = "des3_ede",
            .cra_driver_name = "lqdeu-des3_ede",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 300,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des3_ede_setkey,
                                .encrypt = lq_des_encrypt,
                                .decrypt = lq_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES_KEY_SIZE,
                                .max_keysize = DES_KEY_SIZE,
                                .ivsize = DES_BLOCK_SIZE,
            }
         }
    },{
        .alg = {
            .cra_name        = "ecb(des3_ede)",
            .cra_driver_name = "lqdeu-ecb(des3_ede)",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 400,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des3_ede_setkey,
                                .encrypt = lq_ecb_des_encrypt,
                                .decrypt = lq_ecb_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES3_EDE_KEY_SIZE,
                                .max_keysize = DES3_EDE_KEY_SIZE,
                                .ivsize = DES3_EDE_BLOCK_SIZE,
            }
         } 
    },{
        .alg = {
            .cra_name        = "cbc(des3_ede)",
            .cra_driver_name = "lqdeu-cbc(des3_ede)",
            .cra_flags       = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC, 
            .cra_blocksize   = DES_BLOCK_SIZE,
            .cra_ctxsize     = sizeof(struct des_ctx),
            .cra_type        = &crypto_ablkcipher_type,
            .cra_priority    = 400,
            .cra_module      = THIS_MODULE,
            .cra_ablkcipher  = {
                                .setkey = lq_des3_ede_setkey,
                                .encrypt = lq_cbc_des_encrypt,
                                .decrypt = lq_cbc_des_decrypt,
                                .geniv = "eseqiv",
                                .min_keysize = DES3_EDE_KEY_SIZE,
                                .max_keysize = DES3_EDE_KEY_SIZE,
                                .ivsize = DES3_EDE_BLOCK_SIZE,
            }
         }
    } 
};

/*! \fn int __init lqdeu_async_des_init (void)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief initialize des driver
*/
int __init lqdeu_async_des_init (void)
{
    int i, j, ret = -EINVAL;

     for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++) {
         ret = crypto_register_alg(&des_drivers_alg[i].alg);
	 //printk("driver: %s\n", des_drivers_alg[i].alg.cra_name);
         if (ret)
             goto des_err;
     }
            
     des_chip_init();
     CRTCL_SECT_INIT;


    printk (KERN_NOTICE "IFX DEU DES initialized%s%s.\n", disable_multiblock ? "" : " (multiblock)", disable_deudma ? "" : " (DMA)");
    return ret;

des_err:
     for (j = 0; j < i; j++) 
        crypto_unregister_alg(&des_drivers_alg[i].alg);

     printk(KERN_ERR "Lantiq %s driver initialization failed!\n", (char *)&des_drivers_alg[i].alg.cra_driver_name);
     return ret;

cbc_des3_ede_err:
     for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++) {
         if (!strcmp((char *)&des_drivers_alg[i].alg.cra_name, "cbc(des3_ede)"))
             crypto_unregister_alg(&des_drivers_alg[i].alg);
     }     

     printk(KERN_ERR "Lantiq %s driver initialization failed!\n", (char *)&des_drivers_alg[i].alg.cra_driver_name);
     return ret;
}

/*! \fn void __exit lqdeu_fini_async_des (void)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief unregister des driver
*/
void __exit lqdeu_fini_async_des (void)
{
    int i;
    
    for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++)
        crypto_unregister_alg(&des_drivers_alg[i].alg);

    des_queue->hw_status = DES_COMPLETED;
    DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait, DES_ASYNC_EVENT,
                                 deu_dma_priv.des_event_flags); 
   
    kfree(des_queue);
}

