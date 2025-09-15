/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_common.h
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver header file (common definitions)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 17 JUN 2009  Xu Liang        Init Version
*******************************************************************************/

#ifndef IFXMIPS_PTM_COMMON_H
#define IFXMIPS_PTM_COMMON_H



/*
 * ####################################
 *              Version No.
 * ####################################
 */

#define IFX_PTM_VER_MAJOR               1
#define IFX_PTM_VER_MID                 0
#define IFX_PTM_VER_MINOR               27



/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Compile Options
 */

#define ENABLE_DEBUG                    1

#define ENABLE_ASSERT                   1

#define INLINE

#define DEBUG_DUMP_SKB                  1

#define DEBUG_QOS                       1

#define ENABLE_DBG_PROC                 0

#define ENABLE_FW_PROC                  1

#if defined(CONFIG_DSL_MEI_CPE_DRV) && !defined(CONFIG_IFXMIPS_DSL_CPE_MEI)
  #define CONFIG_IFXMIPS_DSL_CPE_MEI    1
#endif

/*
 *  Debug/Assert/Error Message
 */

#define DBG_ENABLE_MASK_ERR             (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT     (1 << 1)
#define DBG_ENABLE_MASK_ASSERT          (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX     (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX     (1 << 9)
#define DBG_ENABLE_MASK_DUMP_QOS        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT       (1 << 11)
#define DBG_ENABLE_MASK_MAC_SWAP        (1 << 12)
#define DBG_ENABLE_MASK_ALL             (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX | DBG_ENABLE_MASK_DUMP_QOS | DBG_ENABLE_MASK_DUMP_INIT | DBG_ENABLE_MASK_MAC_SWAP)

#define err(format, arg...)             do { if ( (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_ERR) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
  #undef  dbg
  #define dbg(format, arg...)           do { if ( (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT) ) printk(KERN_WARNING __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #if !defined(dbg)
    #define dbg(format, arg...)
  #endif
#endif

#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #define ASSERT(cond, format, arg...)  do { if ( (ifx_ptm_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif



#endif  //  IFXMIPS_PTM_COMMON_H
