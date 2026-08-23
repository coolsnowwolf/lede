 /***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

/*======================================================================================
 * MODULE NAME: NAND
 * FILE NAME: nand_flash_otp.h
 * VERSION: 1.00
 * PURPOSE: To Provide NAND OTP Access interface.
 * NOTES:
 *
 * FUNCTIONS  
 *
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 * ** This is the first versoin for creating to support the functions of
 *    current module.
 *
 *======================================================================================
 */

#ifndef __NAND_FLASH_OTP_H__
#define __NAND_FLASH_OTP_H__

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */

/* MACRO DECLARATIONS ---------------------------------------------------------------- */

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef struct {
	unsigned int otp_locked		: 1;
	unsigned int otp_enabled	: 1;
	unsigned int resve			: 30;
} NAND_FLASH_OTP_STS_T;

typedef enum {
	NAND_FLASH_OTP_DISABLE = 0,
	NAND_FLASH_OTP_ENABLE,
} NAND_FLASH_OTP_ENABLE_T;

typedef enum {
	NAND_FLASH_OTP_HAS_NO_RSA_PUBKEY = 0,
	NAND_FLASH_OTP_HAS_RSA_PUBKEY,
} NAND_FLASH_OTP_RSA_PUBKEY_T;

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
#if defined(TCSUPPORT_SECURE_BOOT_FLASH_OTP)
int nand_otp_read_rsa_pub_key(u8 *data, u32 len);
NAND_FLASH_OTP_RSA_PUBKEY_T nand_otp_is_rsa_pub_key(u32 len);
int nand_otp_write_rsa_pub_key(u8 *data, u32 len);
#endif

#endif /* ifndef __NAND_FLASH_OTP_H__ */

