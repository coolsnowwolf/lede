/******************************************************************************
**
** FILE NAME    : ifxmips_deu_danube.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for Danube
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
  \file	ifxmips_deu_danube.c
  \ingroup IFX_DEU
  \brief board specific deu driver file for danube
*/

/*!
  \defgroup BOARD_SPECIFIC_FUNCTIONS IFX_BOARD_SPECIFIC_FUNCTIONS
  \ingroup IFX_DEU
  \brief board specific deu functions
*/

/* Project header files */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <asm/io.h> //dma_cache_inv

#include "ifxmips_deu_dma.h"
#include "ifxmips_deu_danube.h"


/* Function Declerations */
int aes_memory_allocate(int value);
int des_memory_allocate(int value);
void memory_release(u32 *addr); 
int aes_chip_init (void);
void des_chip_init (void);
int deu_dma_init (void);
u32 endian_swap(u32 input);
u32* memory_alignment(const u8 *arg, u32 *buff_alloc, int in_out, int nbytes);
void dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
void chip_version(void); 
void deu_dma_priv_init(void);
void __exit ifxdeu_fini_dma(void);

#define DES_3DES_START  IFX_DES_CON
#define AES_START       IFX_AES_CON
#define CLC_START       IFX_DEU_CLK

/* Variables definition */
int ifx_danube_pre_1_4; 
u8 *g_dma_page_ptr = NULL;
u8 *g_dma_block = NULL;
u8 *g_dma_block2 = NULL;

deu_drv_priv_t deu_dma_priv;


/*! \fn u32 endian_swap(u32 input) 
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief function is not used
 *  \param input Data input to be swapped
 *  \return input
*/

u32 endian_swap(u32 input)
{
    return input;
}

/*! \fn	u32 input_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Swap the input data if the current chip is Danube version
 *         1.4 and do nothing to the data if the current chip is 
 *         Danube version 1.3 
 *  \param input data that needs to be swapped
 *  \return input or swapped input
*/

u32 input_swap(u32 input)
{
    if (!ifx_danube_pre_1_4) {
        u8 *ptr = (u8 *)&input;
        return ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]); 
    }
    else 
        return input;
}



/*! \fn void aes_chip_init (void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 * \brief initialize AES hardware   
*/

int aes_chip_init (void)
{
    volatile struct aes_t *aes = (struct aes_t *) AES_START;

    //start crypto engine with write to ILR
    aes->controlr.SM = 1;
    aes->controlr.ARS = 1;
    return 0;
}

/*! \fn void des_chip_init (void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief initialize DES hardware
*/  
                        
void des_chip_init (void)
{
        volatile struct des_t *des = (struct des_t *) DES_3DES_START;

        // start crypto engine with write to ILR
        des->controlr.SM = 1;
        des->controlr.ARS = 1;
}

/*! \fn void chip_version (void)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief To find the version of the chip by looking at the chip ID
 *  \param ifx_danube_pre_1_4 (sets to 1 if Chip is Danube less than v1.4)
*/  
#define IFX_MPS               (KSEG1 | 0x1F107000)
#define IFX_MPS_CHIPID                          ((volatile u32*)(IFX_MPS + 0x0344))

void chip_version(void) 
{

    /* DANUBE PRE 1.4 SOFTWARE FIX */
    int chip_id = 0;
    chip_id = *IFX_MPS_CHIPID;
    chip_id >>= 28;

    if (chip_id >= 4) {
        ifx_danube_pre_1_4 = 0;
        printk("Danube Chip ver. 1.4 detected. \n");
    }
    else {
        ifx_danube_pre_1_4 = 1; 
        printk("Danube Chip ver. 1.3 or below detected. \n");
    }

    return;
}

