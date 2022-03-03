/******************************************************************************
**
** FILE NAME    : ifxmips_deu_vr9.c
** PROJECT      : IFX UEIP
** MODULES      : DEU Module for VR9
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
  \defgroup  IFX_DEU IFX_DEU_DRIVERS
  \ingroup  API
  \brief deu driver module
*/

/*!
  \file		ifxmips_deu_vr9.c
  \ingroup 	IFX_DEU
  \brief 	board specific deu driver file for vr9
*/

/*!
  \defgroup   BOARD_SPECIFIC_FUNCTIONS IFX_BOARD_SPECIFIC_FUNCTIONS
  \ingroup IFX_DEU
  \brief board specific deu driver functions
*/

/* Project header files */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <asm/io.h> //dma_cache_inv

#include "ifxmips_deu_dma.h"
#include "ifxmips_deu_vr9.h"

/* Function decleration */
void aes_chip_init (void);
void des_chip_init (void);
int deu_dma_init (void);
void deu_dma_priv_init(void);
u32 endian_swap(u32 input);
u32* memory_alignment(const u8 *arg, u32 *buff_alloc, int in_out, int nbytes);
void aes_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
void des_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
void __exit ifxdeu_fini_dma(void);

#define DES_3DES_START  IFX_DES_CON
#define AES_START       IFX_AES_CON

/* Variables */

u8 *g_dma_page_ptr = NULL;
u8 *g_dma_block = NULL;
u8 *g_dma_block2 = NULL;

deu_drv_priv_t deu_dma_priv;


/*! \fn u32 endian_swap(u32 input) 
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Swap data given to the function 
 *  \param input Data input to be swapped
 *  \return either the swapped data or the input data depending on whether it is in DMA mode or FPI mode
*/


u32 endian_swap(u32 input)
{
    return input;
}

/*! \fn u32 input_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Not used  
 *  \return input
*/

u32 input_swap(u32 input)
{
    return input;
}

/*! \fn void aes_chip_init (void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief initialize AES hardware   
*/

void aes_chip_init (void)
{
    volatile struct aes_t *aes = (struct aes_t *) AES_START;

    // start crypto engine with write to ILR
    aes->controlr.SM = 1;
    aes->controlr.NDC = 1;
    asm("sync");
    aes->controlr.ENDI = 1;
    asm("sync");
    aes->controlr.ARS = 0;
	
}

/*! \fn void des_chip_init (void)
 *  \ingroup IFX_AES_FUNCTIONS
 *  \brief initialize DES hardware
*/         
                        
void des_chip_init (void)
{
    volatile struct des_t *des = (struct des_t *) DES_3DES_START;

    // start crypto engine with write to ILR
    des->controlr.SM = 1;
    des->controlr.NDC = 1;
    asm("sync");
    des->controlr.ENDI = 1;
    asm("sync");    
    des->controlr.ARS = 0;

}
/*! \fn void chip_version(void)
 *  \ingroup IFX_DES_FUNCTIONS
 *  \brief function not used in VR9
*/
void chip_version(void) 
{
    return;
}

