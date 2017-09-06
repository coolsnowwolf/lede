/******************************************************************************
**
** FILE NAME    : ifxmips_deu.h
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
  \ingroup  API
  \brief ifx deu driver module
*/

/*!
  \file	ifxmips_deu.h
  \brief main deu driver header file
*/

/*!
  \defgroup IFX_DEU_DEFINITIONS IFX_DEU_DEFINITIONS
  \ingroup  IFX_DEU
  \brief ifx deu definitions
*/


#ifndef IFXMIPS_DEU_H
#define IFXMIPS_DEU_H

#include <crypto/algapi.h>
#include <linux/interrupt.h>

#define IFXDEU_ALIGNMENT 16

#define IFX_DEU_BASE_ADDR                       (KSEG1 | 0x1E103100)
#define IFX_DEU_CLK                             ((volatile u32 *)(IFX_DEU_BASE_ADDR + 0x0000))
#define IFX_DES_CON                             ((volatile u32 *)(IFX_DEU_BASE_ADDR + 0x0010))
#define IFX_AES_CON                             ((volatile u32 *)(IFX_DEU_BASE_ADDR + 0x0050))
#define IFX_HASH_CON                            ((volatile u32 *)(IFX_DEU_BASE_ADDR + 0x00B0))
#define IFX_ARC4_CON                            ((volatile u32 *)(IFX_DEU_BASE_ADDR + 0x0100))

#define PFX	"ifxdeu: "
#define CLC_START IFX_DEU_CLK
#define IFXDEU_CRA_PRIORITY	300
#define IFXDEU_COMPOSITE_PRIORITY 400
//#define KSEG1                         0xA0000000
#define IFX_PMU_ENABLE 1
#define IFX_PMU_DISABLE 0

#define CRYPTO_DIR_ENCRYPT 1
#define CRYPTO_DIR_DECRYPT 0

#define AES_IDLE 0
#define AES_BUSY 1
#define AES_STARTED 2
#define AES_COMPLETED 3
#define DES_IDLE 0
#define DES_BUSY 1
#define DES_STARTED 2
#define DES_COMPLETED 3

#define PROCESS_SCATTER 1
#define PROCESS_NEW_PACKET 2

#define PMU_DEU BIT(20)
#define START_DEU_POWER        \
    do {                       \
        volatile struct clc_controlr_t *clc = (struct clc_controlr_t *) CLC_START;  \
        ltq_pmu_enable(PMU_DEU); \
        clc->FSOE = 0;           \
        clc->SBWE = 0;           \
        clc->SPEN = 0;           \
        clc->SBWE = 0;           \
        clc->DISS = 0;           \
        clc->DISR = 0;           \
    } while(0)

#define STOP_DEU_POWER		\
    do {			\
        volatile struct clc_controlr_t *clc = (struct clc_controlr_t *) CLC_START; \
	ltq_pmu_disable(PMU_DEU); \
        clc->FSOE = 1;		\
        clc->SBWE = 1;           \
        clc->SPEN = 1;           \
        clc->SBWE = 1;           \
        clc->DISS = 1;           \
        clc->DISR = 1;           \
    } while (0)

/* 
 * Not used anymore in UEIP (use IFX_DES_CON, IFX_AES_CON, etc instead) 
 * #define DEU_BASE   (KSEG1+0x1E103100)
 * #define DES_CON		(DEU_BASE+0x10)
 * #define AES_CON		(DEU_BASE+0x50)
 * #define HASH_CON	(DEU_BASE+0xB0)
 * #define DMA_CON		(DEU_BASE+0xEC)
 * #define INT_CON		(DEU_BASE+0xF4)
 * #define ARC4_CON	(DEU_BASE+0x100)
 */


int __init ifxdeu_init_des (void);
int __init ifxdeu_init_aes (void);
int __init ifxdeu_init_arc4 (void);
int __init ifxdeu_init_sha1 (void);
int __init ifxdeu_init_md5 (void);
int __init ifxdeu_init_sha1_hmac (void);
int __init ifxdeu_init_md5_hmac (void);
int __init lqdeu_async_aes_init(void);
int __init lqdeu_async_des_init(void);

void __exit ifxdeu_fini_des (void);
void __exit ifxdeu_fini_aes (void);
void __exit ifxdeu_fini_arc4 (void);
void __exit ifxdeu_fini_sha1 (void);
void __exit ifxdeu_fini_md5 (void);
void __exit ifxdeu_fini_sha1_hmac (void);
void __exit ifxdeu_fini_md5_hmac (void);
void __exit ifxdeu_fini_dma(void);
void __exit lqdeu_fini_async_aes(void);
void __exit lqdeu_fini_async_des(void);
void __exit deu_fini (void);
int deu_dma_init (void);



#define DEU_WAKELIST_INIT(queue) \
    init_waitqueue_head(&queue)

#define DEU_WAIT_EVENT_TIMEOUT(queue, event, flags, timeout)     \
    do {                                                         \
        wait_event_interruptible_timeout((queue),                \
            test_bit((event), &(flags)), (timeout));            \
        clear_bit((event), &(flags));                            \
    }while (0)


#define DEU_WAKEUP_EVENT(queue, event, flags)         \
    do {                                              \
        set_bit((event), &(flags));                   \
        wake_up_interruptible(&(queue));              \
    }while (0)
    
#define DEU_WAIT_EVENT(queue, event, flags)           \
    do {                                              \
        wait_event_interruptible(queue,               \
            test_bit((event), &(flags)));             \
        clear_bit((event), &(flags));                 \
    }while (0)

typedef struct deu_drv_priv {
    wait_queue_head_t  deu_thread_wait;
#define DEU_EVENT       1
#define DES_ASYNC_EVENT 2
#define AES_ASYNC_EVENT 3
    volatile long      des_event_flags;
    volatile long      aes_event_flags;
    volatile long      deu_event_flags;
    int                event_src;
    u32                *deu_rx_buf;
    u32                *outcopy;
    u32                deu_rx_len;

    struct aes_priv    *aes_dataptr;
    struct des_priv    *des_dataptr;
}deu_drv_priv_t;


/**
 *	struct aes_priv_t - ASYNC AES
 *	@lock: spinlock lock
 *	@lock_flag: flag for spinlock activities
 *	@list: crypto queue API list
 *	@hw_status: DEU hw status flag 
 *	@aes_wait_flag: flag for sleep queue
 *	@aes_wait_queue: queue attributes for aes
 *	@bytes_processed: number of bytes to process by DEU
 *	@aes_pid: pid number for AES thread
 *	@aes_sync: atomic wait sync for AES
 *
*/

typedef struct {
    spinlock_t lock;
    struct crypto_queue list;
    unsigned int hw_status;
    volatile long aes_wait_flag;
    wait_queue_head_t aes_wait_queue;

    pid_t aes_pid;

    struct tasklet_struct aes_task;

} aes_priv_t;

/**
 *      struct des_priv_t - ASYNC DES
 *      @lock: spinlock lock
 *      @list: crypto queue API list
 *      @hw_status: DEU hw status flag
 *      @des_wait_flag: flag for sleep queue
 *      @des_wait_queue: queue attributes for des
 *      @des_pid: pid number for DES thread
 *      @des_sync: atomic wait sync for DES
 *
*/

typedef struct {
    spinlock_t lock;
    struct crypto_queue list;
    unsigned int hw_status;
    volatile long des_wait_flag;
    wait_queue_head_t des_wait_queue;

    pid_t des_pid;

    struct tasklet_struct des_task;

} des_priv_t;
    
#endif	/* IFXMIPS_DEU_H */


