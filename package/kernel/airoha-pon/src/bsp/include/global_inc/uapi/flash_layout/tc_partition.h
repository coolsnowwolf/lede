/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef TC_PARTITION_H
#define TC_PARTITION_H

/**
* \file  tc_partition.h 
* \brief This file is tc_partition header file that redefine Marcos to API so that APP layer can get corresponding value from BSP layer.
* \author ECONET
* \date     2020-12-02
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

#include "blapi_tc_partition.h"
 #ifdef TCSUPPORT_MTD_ENCHANCEMENT
#define TC_FLASH_READ_CMD		"/userfs/bin/mtd readflash %s %lu %lu %s"
#define TC_FLASH_WRITE_CMD	"/userfs/bin/mtd writeflash %s %lu %lu %s"

#define TC_FLASH_ERASE_SECTOR_CMD "/userfs/bin/mtd erasesector %lu %s"

#define RESERVEAREA_NAME "reservearea"
#endif

#define CERM1_RA_SIZE                          get_tc_partition_value(GET_CERM1_RA_SIZE               )
#define CERM2_RA_SIZE                          get_tc_partition_value(GET_CERM2_RA_SIZE               )
#define CERM3_RA_SIZE                          get_tc_partition_value(GET_CERM3_RA_SIZE               )
#define CERM4_RA_SIZE                          get_tc_partition_value(GET_CERM4_RA_SIZE               )
#define CERM1_RA_OFFSET                        get_tc_partition_value(GET_CERM1_RA_OFFSET             )
#define CERM2_RA_OFFSET                        get_tc_partition_value(GET_CERM2_RA_OFFSET             )
#define CERM3_RA_OFFSET                        get_tc_partition_value(GET_CERM3_RA_OFFSET             )
#define CERM4_RA_OFFSET                        get_tc_partition_value(GET_CERM4_RA_OFFSET             )
#define BOB_RA_OFFSET_OLD_MODE                 get_tc_partition_value(GET_BOB_RA_OFFSET_OLD_MODE      )
#define NAND_FLASH_BLOCK_SIZE                  get_tc_partition_value(GET_NAND_FLASH_BLOCK_SIZE       )
#define EEPROM_RA_AC_SIZE                      get_tc_partition_value(GET_EEPROM_RA_AC_SIZE           )
#define EEPROM_RA_AC_RESERVE_SIZE              get_tc_partition_value(GET_EEPROM_RA_AC_RESERVE_SIZE   )
#define RESERVEAREA_ERASE_SIZE                 get_tc_partition_value(GET_RESERVEAREA_ERASE_SIZE      )
#define RESERVEAREA_TOTAL_SIZE                 get_tc_partition_value(GET_RESERVEAREA_TOTAL_SIZE      )
#define BACKUPROMFILE_RA_SIZE                  get_tc_partition_value(GET_BACKUPROMFILE_RA_SIZE       )
#define BACKUPROMFILE_RA_OFFSET                get_tc_partition_value(GET_BACKUPROMFILE_RA_OFFSET     )
#define TEMP_RA_SIZE                           get_tc_partition_value(GET_TEMP_RA_SIZE                )
#define TEMP_RA_OFFSET                         get_tc_partition_value(GET_TEMP_RA_OFFSET              )
#define USERNAMEPASSWD_RA_SIZE                 get_tc_partition_value(GET_USERNAMEPASSWD_RA_SIZE      )
#define USERNAMEPASSWD_RA_OFFSET               get_tc_partition_value(GET_USERNAMEPASSWD_RA_OFFSET    )
#define SYSLOG_RA_SIZE                         get_tc_partition_value(GET_SYSLOG_RA_SIZE              )
#define SYSLOG_RA_OFFSET                       get_tc_partition_value(GET_SYSLOG_RA_OFFSET            )
#define EEPROM_RA_SIZE                         get_tc_partition_value(GET_EEPROM_RA_SIZE              )
#define EEPROM_RA_OFFSET                       get_tc_partition_value(GET_EEPROM_RA_OFFSET            )
#define PROLINE_CWMPPARA_RA_SIZE               get_tc_partition_value(GET_PROLINE_CWMPPARA_RA_SIZE    )
#define PROLINE_CWMPPARA_RA_OFFSET             get_tc_partition_value(GET_PROLINE_CWMPPARA_RA_OFFSET  )
#define BOB_RA_SIZE                            get_tc_partition_value(GET_BOB_RA_SIZE                 )
#define BOB_RA_OFFSET                          get_tc_partition_value(GET_BOB_RA_OFFSET               )
#define IMG_BOOT_FLAG_SIZE                     get_tc_partition_value(GET_IMG_BOOT_FLAG_SIZE          )
#define IMG_BOOT_FLAG_OFFSET                   get_tc_partition_value(GET_IMG_BOOT_FLAG_OFFSET        )
#define IMG_BOOT_FLAG_RESERVE_SIZE             get_tc_partition_value(GET_IMG_BOOT_FLAG_RESERVE_SIZE  )
#define IMG_BOOT_FLAG_RESERVE_OFFSET           get_tc_partition_value(GET_IMG_BOOT_FLAG_RESERVE_OFFSET)
#define EEPROM_RA_AC_OFFSET                    get_tc_partition_value(GET_EEPROM_RA_AC_OFFSET         )
#define EEPROM_RA_AC_RESERVE_OFFSET            get_tc_partition_value(GET_EEPROM_RA_AC_RESERVE_OFFSET )
#define DEFAULTROMFILE_RA_SIZE                 get_tc_partition_value(GET_DEFAULTROMFILE_RA_SIZE      )
#define DEFAULTROMFILE_RA_OFFSET               get_tc_partition_value(GET_DEFAULTROMFILE_RA_OFFSET    )
#define EEPROM_761511N_RA_SIZE                 get_tc_partition_value(GET_EEPROM_761511N_RA_SIZE      )
#define EEPROM_761511N_RA_OFFSET               get_tc_partition_value(GET_EEPROM_761511N_RA_OFFSET    )
#define TRX_SET1_SET2_11N_RA_SIZE              get_tc_partition_value(GET_TRX_SET1_SET2_11N_RA_SIZE   )
#define TRX_SET1_SET2_11N_RA_OFFSET            get_tc_partition_value(GET_TRX_SET1_SET2_11N_RA_OFFSET )
#define EEPROM_761511AC_RA_SIZE                get_tc_partition_value(GET_EEPROM_761511AC_RA_SIZE     )
#define EEPROM_RA_761511AC_OFFSET              get_tc_partition_value(GET_EEPROM_RA_761511AC_OFFSET   )
#define TRX_SET1_SET2_11AC_RA_SIZE             get_tc_partition_value(GET_TRX_SET1_SET2_11AC_RA_SIZE  )
#define TRX_SET1_SET2_11AC_RA_OFFSET           get_tc_partition_value(GET_TRX_SET1_SET2_11AC_RA_OFFSET)
#define RESERVED_761511AC_SIZE                 get_tc_partition_value(GET_RESERVED_761511AC_SIZE      )
#define EEPROM_761311AC_RA_SIZE                get_tc_partition_value(GET_EEPROM_761311AC_RA_SIZE     )
#define EEPROM_RA_761311AC_OFFSET              get_tc_partition_value(GET_EEPROM_RA_761311AC_OFFSET   )
#define MRD_RA_SIZE                            get_tc_partition_value(GET_MRD_RA_SIZE                 )
#define MRD_RA_OFFSET                          get_tc_partition_value(GET_MRD_RA_OFFSET               )
#define ROM_T_RA_SIZE                          get_tc_partition_value(GET_ROM_T_RA_SIZE               )
#define ROM_T_RA_OFFSET                        get_tc_partition_value(GET_ROM_T_RA_OFFSET             )
#define MAX_EEPROM_BIN_FILE_SIZE               get_tc_partition_value(GET_MAX_EEPROM_BIN_FILE_SIZE    )
#define OMCI_RA_SIZE                           get_tc_partition_value(GET_OMCI_RA_SIZE                )
                         
#endif

