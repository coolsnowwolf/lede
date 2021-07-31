/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */
#ifndef REG_EIP93_H
#define REG_EIP93_H

#define EIP93_REG_WIDTH			4
/*-----------------------------------------------------------------------------
 * Register Map
 */
#define DESP_BASE			0x0000000
#define EIP93_REG_PE_CTRL_STAT		((DESP_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_SOURCE_ADDR	((DESP_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_DEST_ADDR		((DESP_BASE)+(0x02 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_SA_ADDR		((DESP_BASE)+(0x03 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_ADDR		((DESP_BASE)+(0x04 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_USER_ID		((DESP_BASE)+(0x06 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_LENGTH		((DESP_BASE)+(0x07 * EIP93_REG_WIDTH))

//PACKET ENGINE RING configuartion registers
#define PE_RNG_BASE			0x0000080

#define EIP93_REG_PE_CDR_BASE		((PE_RNG_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_RDR_BASE		((PE_RNG_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_RING_CONFIG	((PE_RNG_BASE)+(0x02 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_RING_THRESH	((PE_RNG_BASE)+(0x03 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_CD_COUNT		((PE_RNG_BASE)+(0x04 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_RD_COUNT		((PE_RNG_BASE)+(0x05 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_RING_RW_PNTR	((PE_RNG_BASE)+(0x06 * EIP93_REG_WIDTH))

//PACKET ENGINE  configuartion registers
#define PE_CFG_BASE			0x0000100
#define EIP93_REG_PE_CONFIG		((PE_CFG_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_STATUS		((PE_CFG_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_BUF_THRESH		((PE_CFG_BASE)+(0x03 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_INBUF_COUNT	((PE_CFG_BASE)+(0x04 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_OUTBUF_COUNT	((PE_CFG_BASE)+(0x05 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_BUF_RW_PNTR	((PE_CFG_BASE)+(0x06 * EIP93_REG_WIDTH))

//PACKET ENGINE endian config
#define EN_CFG_BASE			0x00001CC
#define EIP93_REG_PE_ENDIAN_CONFIG	((EN_CFG_BASE)+(0x00 * EIP93_REG_WIDTH))

//EIP93 CLOCK control registers
#define CLOCK_BASE			0x01E8
#define EIP93_REG_PE_CLOCK_CTRL		((CLOCK_BASE)+(0x00 * EIP93_REG_WIDTH))

//EIP93 Device Option and Revision Register
#define REV_BASE			0x01F4
#define EIP93_REG_PE_OPTION_1		((REV_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_OPTION_0		((REV_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_PE_REVISION		((REV_BASE)+(0x02 * EIP93_REG_WIDTH))

//EIP93 Interrupt Control Register
#define INT_BASE			0x0200
#define EIP93_REG_INT_UNMASK_STAT	((INT_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_INT_MASK_STAT		((INT_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_INT_CLR		((INT_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_INT_MASK		((INT_BASE)+(0x02 * EIP93_REG_WIDTH))
#define EIP93_REG_INT_CFG		((INT_BASE)+(0x03 * EIP93_REG_WIDTH))
#define EIP93_REG_MASK_ENABLE		((INT_BASE)+(0X04 * EIP93_REG_WIDTH))
#define EIP93_REG_MASK_DISABLE		((INT_BASE)+(0X05 * EIP93_REG_WIDTH))

//EIP93 SA Record register
#define SA_BASE				0x0400
#define EIP93_REG_SA_CMD_0		((SA_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_SA_CMD_1		((SA_BASE)+(0x01 * EIP93_REG_WIDTH))

//#define EIP93_REG_SA_READY		((SA_BASE)+(31 * EIP93_REG_WIDTH))

//State save register
#define STATE_BASE			0x0500
#define EIP93_REG_STATE_IV_0		((STATE_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_STATE_IV_1		((STATE_BASE)+(0x01 * EIP93_REG_WIDTH))

#define EIP93_PE_ARC4STATE_BASEADDR_REG	0x0700

//RAM buffer start address
#define EIP93_INPUT_BUFFER		0x0800
#define EIP93_OUTPUT_BUFFER		0x0800

//EIP93 PRNG Configuration Register
#define PRNG_BASE			0x0300
#define EIP93_REG_PRNG_STAT		((PRNG_BASE)+(0x00 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_CTRL		((PRNG_BASE)+(0x01 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_SEED_0		((PRNG_BASE)+(0x02 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_SEED_1		((PRNG_BASE)+(0x03 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_SEED_2		((PRNG_BASE)+(0x04 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_SEED_3		((PRNG_BASE)+(0x05 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_KEY_0		((PRNG_BASE)+(0x06 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_KEY_1		((PRNG_BASE)+(0x07 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_KEY_2		((PRNG_BASE)+(0x08 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_KEY_3		((PRNG_BASE)+(0x09 * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_RES_0		((PRNG_BASE)+(0x0A * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_RES_1		((PRNG_BASE)+(0x0B * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_RES_2		((PRNG_BASE)+(0x0C * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_RES_3		((PRNG_BASE)+(0x0D * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_LFSR_0		((PRNG_BASE)+(0x0E * EIP93_REG_WIDTH))
#define EIP93_REG_PRNG_LFSR_1		((PRNG_BASE)+(0x0F * EIP93_REG_WIDTH))

/*-----------------------------------------------------------------------------
 * Constants & masks
 */

#define EIP93_SUPPORTED_INTERRUPTS_MASK	0xffff7f00
#define EIP93_PRNG_DT_TEXT_LOWERHALF	0xDEAD
#define EIP93_PRNG_DT_TEXT_UPPERHALF	0xC0DE
#define EIP93_10BITS_MASK		0X3FF
#define EIP93_12BITS_MASK		0XFFF
#define EIP93_4BITS_MASK		0X04
#define EIP93_20BITS_MASK		0xFFFFF

#define EIP93_MIN_DESC_DONE_COUNT	0
#define EIP93_MAX_DESC_DONE_COUNT	15

#define EIP93_MIN_DESC_PENDING_COUNT	0
#define EIP93_MAX_DESC_PENDING_COUNT	1023

#define EIP93_MIN_TIMEOUT_COUNT		0
#define EIP93_MAX_TIMEOUT_COUNT		15

#define EIP93_MIN_PE_INPUT_THRESHOLD	1
#define EIP93_MAX_PE_INPUT_THRESHOLD	511

#define EIP93_MIN_PE_OUTPUT_THRESHOLD	1
#define EIP93_MAX_PE_OUTPUT_THRESHOLD	432

#define EIP93_MIN_PE_RING_SIZE		1
#define EIP93_MAX_PE_RING_SIZE		1023

#define EIP93_MIN_PE_DESCRIPTOR_SIZE	7
#define EIP93_MAX_PE_DESCRIPTOR_SIZE	15

//3DES keys,seed,known data and its result
#define EIP93_KEY_0			0x133b3454
#define EIP93_KEY_1			0x5e5b890b
#define EIP93_KEY_2			0x5eb30757
#define EIP93_KEY_3			0x93ab15f7
#define EIP93_SEED_0			0x62c4bf5e
#define EIP93_SEED_1			0x972667c8
#define EIP93_SEED_2			0x6345bf67
#define EIP93_SEED_3			0xcb3482bf
#define EIP93_LFSR_0			0xDEADC0DE
#define EIP93_LFSR_1			0xBEEFF00D

/*-----------------------------------------------------------------------------
 * EIP93 device initialization specifics
 */

/*----------------------------------------------------------------------------
 * Byte Order Reversal Mechanisms Supported in EIP93
 * EIP93_BO_REVERSE_HALF_WORD : reverse the byte order within a half-word
 * EIP93_BO_REVERSE_WORD :  reverse the byte order within a word
 * EIP93_BO_REVERSE_DUAL_WORD : reverse the byte order within a dual-word
 * EIP93_BO_REVERSE_QUAD_WORD : reverse the byte order within a quad-word
 */
typedef enum
{
    EIP93_BO_REVERSE_HALF_WORD = 1,
    EIP93_BO_REVERSE_WORD = 2,
    EIP93_BO_REVERSE_DUAL_WORD = 4,
    EIP93_BO_REVERSE_QUAD_WORD = 8,
} EIP93_Byte_Order_Value_t;

/*----------------------------------------------------------------------------
 * Byte Order Reversal Mechanisms Supported in EIP93 for Target Data
 * EIP93_BO_REVERSE_HALF_WORD : reverse the byte order within a half-word
 * EIP93_BO_REVERSE_WORD :  reverse the byte order within a word
 */
typedef enum
{
    EIP93_BO_REVERSE_HALF_WORD_TD = 1,
    EIP93_BO_REVERSE_WORD_TD = 2,
} EIP93_Byte_Order_Value_TD_t;


// BYTE_ORDER_CFG register values
#define EIP93_BYTE_ORDER_PD		EIP93_BO_REVERSE_WORD
#define EIP93_BYTE_ORDER_SA		EIP93_BO_REVERSE_WORD
#define EIP93_BYTE_ORDER_DATA		EIP93_BO_REVERSE_WORD
#define EIP93_BYTE_ORDER_TD		EIP93_BO_REVERSE_WORD_TD

// INT_CFG register values
#define EIP93_INT_HOST_OUTPUT_TYPE	0	// 0 = Level
#define EIP93_INT_PULSE_CLEAR		0	// 0 = Manual clear

#endif
