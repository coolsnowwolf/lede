/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt6352.c

	Abstract:
	Specific funcitons and variables for RT6352

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT6352

#include	"rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif // RTMP_RF_RW_SUPPORT //

UCHAR	RT6352_EeBuffer[EEPROM_SIZE] = {
	0x20, 0x76, 0x04, 0x01, 0x00, 0x0c, 0x43, 0x76, 0x20, 0x58, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0c, 0x43, 0x76, 0x20, 0x77, 0x00, 0x0c, 
	0x43, 0x76, 0x20, 0x66, 0x22, 0x0c, 0x20, 0x00, 0xff, 0xff, 0x3f, 0x01, 0x55, 0x77, 0xa8, 0xaa, 
	0x8c, 0x88, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0xff, 0xff, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x80, 0xff, 
	0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x02, 
	0x02, 0x02, 0x04, 0x00, 0x02, 0x02, 0x04, 0x00, 0x02, 0x02, 0x04, 0x00, 0x02, 0x02, 0x04, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	} ;

RT635x_FREQUENCY_ITEM FreqItems6352[] =
{
	/**************************************************/
	/* ISM : 2.4 to 2.483 GHz                         */
	/**************************************************/
	/*-CH--Rdiv----N---- -K----D-----Ksd------*/
	{1,      3,   0x50,    0,   0,   0x19999},
	{2,      3,   0x50,	   0,   0,   0x24444},
	{3,      3,   0x50,	   0,   0,   0x2EEEE},
	{4,      3,   0x50,	   0,   0,   0x39999},
	{5,      3,   0x51,	   0,   0,   0x04444},
	{6,      3,   0x51,	   0,   0,   0x0EEEE},
	{7,      3,   0x51,	   0,   0,   0x19999},
	{8,      3,   0x51,	   0,   0,   0x24444},
	{9,      3,   0x51,	   0,   0,   0x2EEEE},
	{10,     3,   0x51,	   0,   0,   0x39999},
	{11,     3,   0x52,	   0,   0,   0x04444},
	{12,     3,   0x52,	   0,   0,   0x0EEEE},
	{13,     3,   0x52,	   0,   0,   0x19999},
	{14,     3,   0x52,	   0,   0,   0x33333},
};
UCHAR NUM_OF_6352_CHNL = (sizeof(FreqItems6352) / sizeof(RT635x_FREQUENCY_ITEM));


REG_PAIR RT6352_RFCentralRegTable[] = {
	{RF_R00, 0x02},
	{RF_R01, 0x03},
	{RF_R02, 0x33},
	{RF_R03, 0xFF},
	//{RF_R04, 0x0E},
	{RF_R04, 0x0C},
	{RF_R05, 0x40}, /* Read only */
	{RF_R06, 0x00},
	{RF_R07, 0x00},
	{RF_R08, 0x00},
	{RF_R09, 0x00},
	{RF_R10, 0x00},
	{RF_R11, 0x00},
	//{RF_R12, 0x43}, /* EEPROM */
	{RF_R13, 0x00},
	{RF_R14, 0x40},
	{RF_R15, 0x22},
	{RF_R16, 0x4C},
	{RF_R17, 0x00},
	{RF_R18, 0x00},
	{RF_R19, 0x00},
	{RF_R20, 0xA0},
	{RF_R21, 0x12},
	{RF_R22, 0x07},
	{RF_R23, 0x13},
	{RF_R24, 0xFE},
	{RF_R25, 0x24},
	{RF_R26, 0x7A},
	{RF_R27, 0x00},
	{RF_R28, 0x00},
	{RF_R29, 0x05},
	{RF_R30, 0x00},
	{RF_R31, 0x00},
	{RF_R32, 0x00},
	{RF_R33, 0x00},
	{RF_R34, 0x00},
	{RF_R35, 0x00},
	{RF_R36, 0x00},
	{RF_R37, 0x00},
	{RF_R38, 0x00},
	{RF_R39, 0x00},
	{RF_R40, 0x00},
	{RF_R41, 0xD0},
	{RF_R42, 0x5B},
	{RF_R43, 0x00},
};

REG_PAIR RT6352_RFChannelRegTable[] = {
	{RF_R00, 0x03},
	{RF_R01, 0x00},
	{RF_R02, 0x00},
	{RF_R03, 0x00},
	{RF_R04, 0x00},
	{RF_R05, 0x08},
	{RF_R06, 0x00},
	{RF_R07, 0x51},
	{RF_R08, 0x53},
	{RF_R09, 0x16},
	{RF_R10, 0x61},
	{RF_R11, 0x53},
	{RF_R12, 0x22},
	{RF_R13, 0x3D},
	{RF_R14, 0x06},
	{RF_R15, 0x13},
	{RF_R16, 0x22},
	{RF_R17, 0x27},
	{RF_R18, 0x02},
	{RF_R19, 0xA7},
	{RF_R20, 0x01},
	{RF_R21, 0x52},
	{RF_R22, 0x80},
	{RF_R23, 0xB3},
	{RF_R24, 0x00},
	{RF_R25, 0x00},
	{RF_R26, 0x00},
	{RF_R27, 0x00},
	{RF_R28, 0x5C},
	{RF_R29, 0x6B},
	{RF_R30, 0x6B},
	{RF_R31, 0x31},
	{RF_R32, 0x5D},
	{RF_R33, 0x00},
	{RF_R34, 0xE6},
	{RF_R35, 0x55},
	{RF_R36, 0x00},
	{RF_R37, 0xBB},
	{RF_R38, 0xB3},
	{RF_R39, 0xB3},
	{RF_R40, 0x03},
	{RF_R41, 0x00},
	{RF_R42, 0x00},
	{RF_R43, 0xB3},
	{RF_R44, 0xD3}, 
	{RF_R45, 0xD5},
	{RF_R46, 0x07},
	{RF_R47, 0x68},
	{RF_R48, 0xEF},
	{RF_R49, 0x1C},
	{RF_R54, 0x07},
	{RF_R55, 0xA8},
	{RF_R56, 0x85},
	{RF_R57, 0x10},
	{RF_R58, 0x07},
	{RF_R59, 0x6A},
	{RF_R60, 0x85},
	{RF_R61, 0x10},
	{RF_R62, 0x1C},
	{RF_R63, 0x00},
};

REG_PAIR RT6352_RFChannelRegE2Table[] = {
	{RF_R09, 0x47},
	{RF_R10, 0x71},
	{RF_R11, 0x33},
	{RF_R14, 0x0E},
	{RF_R17, 0x23},
	{RF_R19, 0xA4},
	{RF_R20, 0x02},
	{RF_R21, 0x12},
	{RF_R28, 0x1C},
	{RF_R29, 0xEB},
	{RF_R32, 0x7D},
	{RF_R34, 0xD6},
	{RF_R36, 0x08},
	{RF_R38, 0xB4},
	{RF_R43, 0xD3},
	{RF_R44, 0xB3}, 
	{RF_R45, 0xD5},
	{RF_R46, 0x27},
	{RF_R47, 0x67},
	{RF_R48, 0xFF},
	{RF_R54, 0x27},
	{RF_R55, 0x66},
	{RF_R56, 0xFF},
	{RF_R57, 0x1C},
	{RF_R58, 0x20},
	{RF_R59, 0x6B},
	{RF_R60, 0xF7},
	{RF_R61, 0x09},
};

REG_PAIR RT6352_RFDCCalRegTable[] = {
	{RF_R00, 0x47},
	{RF_R01, 0x00},
	{RF_R02, 0x00},
	{RF_R03, 0x00},
	{RF_R04, 0x00},
	{RF_R05, 0x00},
	{RF_R06, 0x10},
	{RF_R07, 0x10},
	{RF_R08, 0x04},
	{RF_R09, 0x00},
	{RF_R10, 0x07},
	{RF_R11, 0x01},
	{RF_R12, 0x07},
	{RF_R13, 0x07},
	{RF_R14, 0x07},
	{RF_R15, 0x20},
	{RF_R16, 0x22},
	{RF_R17, 0x00},
	{RF_R18, 0x00},
	{RF_R19, 0x00},
	{RF_R20, 0x00},
	{RF_R21, 0xF1},
	{RF_R22, 0x11},
	{RF_R23, 0x02},
	{RF_R24, 0x41},
	{RF_R25, 0x20},
	{RF_R26, 0x00},
	{RF_R27, 0xD7},
	{RF_R28, 0xA2},
	{RF_R29, 0x20},
	{RF_R30, 0x49},
	{RF_R31, 0x20},
	{RF_R32, 0x04},
	{RF_R33, 0xF1},
	{RF_R34, 0xA1},
	{RF_R35, 0x01},
	{RF_R41, 0x00},
	{RF_R42, 0x00},
	{RF_R43, 0x00},
	{RF_R44, 0x00}, 
	{RF_R45, 0x00},
	{RF_R46, 0x00},
	{RF_R47, 0x3E},
	{RF_R48, 0x3D},
	{RF_R49, 0x3E},
	{RF_R50, 0x3D},
	{RF_R51, 0x3E},
	{RF_R52, 0x3D},
	{RF_R53, 0x00},
	{RF_R54, 0x00},
	{RF_R55, 0x00},
	{RF_R56, 0x00},
	{RF_R57, 0x00},
	{RF_R58, 0x10},
	{RF_R59, 0x10},
	{RF_R60, 0x0A},
	{RF_R61, 0x00},
	{RF_R62, 0x00},
	{RF_R63, 0x00},
};

/* RF Channel Register for DRQFN */
REG_PAIR RT6352_RF_CHANNEL_REG_DRQFN[] =
{
	{RF_R43, 0xD3},
	{RF_R44, 0xE3},
	{RF_R45, 0xE5},
	{RF_R47, 0x28},
	{RF_R55, 0x68},
	{RF_R56, 0xF7},
	{RF_R58, 0x02},
	{RF_R60, 0xC7},
};

UCHAR RT6352_NUM_RF_CENTRAL_REG_PARMS = (sizeof(RT6352_RFCentralRegTable) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_RF_CHANNEL_REG_PARMS = (sizeof(RT6352_RFChannelRegTable) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_RF_CHANNEL_E2_REG_PARMS = (sizeof(RT6352_RFChannelRegE2Table) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_RF_CHANNEL_REG_DRQFN_PARMS = (sizeof(RT6352_RF_CHANNEL_REG_DRQFN) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_RF_DCCAL_REG_PARMS = (sizeof(RT6352_RFDCCalRegTable) / sizeof(REG_PAIR));

REG_PAIR RT6352_BBPRegTable[] = {
	//{BBP_R1, 0x44},
	{BBP_R3, 0x08},
	{BBP_R4, 0x00},
	{BBP_R6, 0x08},
	{BBP_R14, 0x09},
	{BBP_R15, 0xFF},
	{BBP_R16, 0x01},
	{BBP_R20, 0x06},
	{BBP_R21, 0x00},
	{BBP_R22, 0x00},
	{BBP_R27, 0x00},
	{BBP_R28, 0x00},
	{BBP_R30, 0x00},
	{BBP_R31, 0x48},
	{BBP_R47, 0x40},
	{BBP_R62, 0x00},
	{BBP_R63, 0x00},
	{BBP_R64, 0x00},
	{BBP_R65, 0x2C},
	{BBP_R66, 0x1C},
	{BBP_R67, 0x20},
	{BBP_R68, 0xDD},
	{BBP_R69, 0x10},
	{BBP_R70, 0x05},
	{BBP_R73, 0x18},
	{BBP_R74, 0x0F},
	{BBP_R75, 0x60},
	{BBP_R76, 0x44},
	{BBP_R77, 0x59},
	{BBP_R78, 0x1E},
	{BBP_R79, 0x1C},
	{BBP_R80, 0x0C},
	{BBP_R81, 0x3A},
	{BBP_R82, 0xB6},
	{BBP_R83, 0x9A},
	{BBP_R84, 0x9A},
	{BBP_R86, 0x38},
	{BBP_R88, 0x90},
	{BBP_R91, 0x04},
	{BBP_R92, 0x02},
	{BBP_R95, 0x9A},
	{BBP_R96, 0x00},
	{BBP_R103, 0xC0},
	{BBP_R104, 0x92},
	{BBP_R105, 0x3C},
	{BBP_R106, 0x12},
	{BBP_R109, 0x00},
	{BBP_R134, 0x10},
	{BBP_R135, 0xA6},
	{BBP_R137, 0x04},
	{BBP_R142, 0x30},
	{BBP_R143, 0xF7},
	{BBP_R160, 0xEC},
	{BBP_R161, 0xC4},
	{BBP_R162, 0x77},
	{BBP_R163, 0xF9},
	{BBP_R164, 0x00},
	{BBP_R165, 0x00},
	{BBP_R186, 0x00},
	{BBP_R187, 0x00},
	{BBP_R188, 0x00},
	{BBP_R186, 0x00},
	{BBP_R187, 0x01},
	{BBP_R188, 0x00},
	{BBP_R189, 0x00},
};

REG_PAIR RT6352_BBP_DCOC[] = 
{
	{BBP_R140, 0x0C},
	{BBP_R141, 0x00},
	{BBP_R142, 0x10},
	{BBP_R143, 0x10},
	{BBP_R144, 0x10},
	{BBP_R145, 0x10},
	{BBP_R146, 0x08},
	{BBP_R147, 0x40},
	{BBP_R148, 0x04},
	{BBP_R149, 0x04},
	{BBP_R150, 0x08},
	{BBP_R151, 0x08},
	{BBP_R152, 0x03},
	{BBP_R153, 0x03},
	{BBP_R154, 0x03},
	{BBP_R155, 0x02},
	{BBP_R156, 0x40},
	{BBP_R157, 0x40},
	{BBP_R158, 0x64},
	{BBP_R159, 0x64},
};

/* BBP for G band GLRT function(BBP_128 ~ BBP_221) */
REG_PAIR RT6352_BBP_GLRT[] = 
{
	{BBP_R0, 0x00},
	{BBP_R1, 0x14},
	{BBP_R2, 0x20},
	{BBP_R3, 0x0A},
	{BBP_R10, 0x16},
	{BBP_R11, 0x06},
	{BBP_R12, 0x02},
	{BBP_R13, 0x07},
	{BBP_R14, 0x05},
	{BBP_R15, 0x09},
	{BBP_R16, 0x20},
	{BBP_R17, 0x08},
	{BBP_R18, 0x4A},
	{BBP_R19, 0x00},
	{BBP_R20, 0x00},
	{BBP_R128, 0xE0},
	{BBP_R129, 0x1F},
	{BBP_R130, 0x4F},
	{BBP_R131, 0x32},
	{BBP_R132, 0x08},
	{BBP_R133, 0x28},
	{BBP_R134, 0x19},
	{BBP_R135, 0x0A},
	{BBP_R138, 0x16},
	{BBP_R139, 0x10},
	{BBP_R140, 0x10},
	{BBP_R141, 0x1A},
	{BBP_R142, 0x36},
	{BBP_R143, 0x2C},
	{BBP_R144, 0x26},
	{BBP_R145, 0x24},
	{BBP_R146, 0x42},
	{BBP_R147, 0x40},
	{BBP_R148, 0x30},
	{BBP_R149, 0x29},
	{BBP_R150, 0x4C},
	{BBP_R151, 0x46},
	{BBP_R152, 0x3D},
	{BBP_R153, 0x40},
	{BBP_R154, 0x3E},
	{BBP_R155, 0x38},
	{BBP_R156, 0x3D},
	{BBP_R157, 0x2F},
	{BBP_R158, 0x3C},
	{BBP_R159, 0x34},
	{BBP_R160, 0x2C},
	{BBP_R161, 0x2F},
	{BBP_R162, 0x3C},
	{BBP_R163, 0x35},
	{BBP_R164, 0x2E},
	{BBP_R165, 0x2F},
	{BBP_R166, 0x49},
	{BBP_R167, 0x41},
	{BBP_R168, 0x36},
	{BBP_R169, 0x39},
	{BBP_R170, 0x30},
	{BBP_R171, 0x30},
	{BBP_R172, 0x0E},
	{BBP_R173, 0x0D},
	{BBP_R174, 0x28},
	{BBP_R175, 0x21},
	{BBP_R176, 0x1C},
	{BBP_R177, 0x16},
	{BBP_R178, 0x50},
	{BBP_R179, 0x4A},
	{BBP_R180, 0x43},
	{BBP_R181, 0x50},
	{BBP_R182, 0x10},
	{BBP_R183, 0x10},
	{BBP_R184, 0x10},
	{BBP_R185, 0x10},
	{BBP_R200, 0x7D},
	{BBP_R201, 0x14},
	{BBP_R202, 0x32},
	{BBP_R203, 0x2C},
	{BBP_R204, 0x36},
	{BBP_R205, 0x4C},
	{BBP_R206, 0x43},
	{BBP_R207, 0x2C},
	{BBP_R208, 0x2E},
	{BBP_R209, 0x36},
	{BBP_R210, 0x30},
	{BBP_R211, 0x6E},
};

/* BBP for G band BW */
REG_PAIR_BW RT6352_BBP_GLRT_BW[] =
{
	{BBP_R141, BW_20, 0x1A},
	{BBP_R141, BW_40, 0x10},
	{BBP_R157, BW_20, 0x40},
	{BBP_R157, BW_40, 0x2F},
};

/* RF for G band BW */
REG_PAIR_BW RT6352_RFDCCal_BW[] = {
	{RF_R06, BW_20, 0x20},
	{RF_R06, BW_40, 0x10},
	{RF_R07, BW_20, 0x20},
	{RF_R07, BW_40, 0x10},
	{RF_R08, BW_20, 0x00},
	{RF_R08, BW_40, 0x04},
	{RF_R58, BW_20, 0x20},
	{RF_R58, BW_40, 0x10},
	{RF_R59, BW_20, 0x20},
	{RF_R59, BW_40, 0x10},

};

UCHAR RT6352_NUM_BBP_REG_PARMS = (sizeof(RT6352_BBPRegTable) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_BBP_GLRT = (sizeof(RT6352_BBP_GLRT) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_BBP_GLRT_BW = (sizeof(RT6352_BBP_GLRT_BW) / sizeof(REG_PAIR_BW));
UCHAR RT6352_NUM_BBP_DCOC = (sizeof(RT6352_BBP_DCOC) / sizeof(REG_PAIR));
UCHAR RT6352_NUM_RF_DCCAL_BW = (sizeof(RT6352_RFDCCal_BW) / sizeof(REG_PAIR_BW));

RTMP_REG_PAIR	RT6352_MACRegTable[] =	{
	{TX_SW_CFG0,		0x0401},   // Jason,2012-08-27
	{TX_SW_CFG1,		0x000C0001},   // Jason,2012-09-13, 2015-10-08
	{TX_SW_CFG2,		0x00},   // Jason,2012-08-27
	{MIMO_PS_CFG,		0x02},   // Jason,2012-09-13

	/* enable HW to autofallback to legacy rate to prevent ping fail in long range */
	{HT_FBK_TO_LEGACY,	0x1818},	// HT_FBK_TO_LEGACY = OFDM_6
};

UCHAR RT6352_NUM_MAC_REG_PARMS = (sizeof(RT6352_MACRegTable) / sizeof(RTMP_REG_PAIR));


#ifdef RTMP_INTERNAL_TX_ALC
TX_POWER_TUNING_ENTRY_STRUCT RT6352_TxPowerTuningTable[] =
{
/*	idxTxPowerTable		Tx power control over RF			Tx power control over MAC */
/*  	(zero-based array)   	{ RT3350: RF_R12[4:0]: Tx0 ALC},  	{MAC 0x1314~0x1320} */
/*                      			{ RT3352: RF_R47[4:0]: Tx0 ALC} */
/*                      			{ RT3352: RF_R48[4:0]: Tx1 ALC} */
/*  0   */                         	{0x00,                                   		-15},
/*  1   */                         	{0x01,                                    		-15},
/*  2   */                         	{0x00,                                    		-14},
/*  3   */                         	{0x01,                                    		-14},
/*  4   */                         	{0x00,                                    		-13},
/*  5   */                         	{0x01,                                    		-13},
/*  6   */                         	{0x00,                                    		-12},
/*  7   */                         	{0x01,                                    		-12},
/*  8   */                         	{0x00,                                    		-11},
/*  9   */                         	{0x01,                                    		-11},
/*  10  */                         	{0x00,                                    		-10},
/*  11  */                         	{0x01,                                    		-10},
/*  12  */                         	{0x00,                                     		-9},
/*  13  */                         	{0x01,                                     		-9},
/*  14  */                         	{0x00,                                     		-8},
/*  15  */                         	{0x01,                                     		-8},
/*  16  */                         	{0x00,                                     		-7},
/*  17  */                         	{0x01,                                     		-7},
/*  18  */                         	{0x00,                                     		-6},
/*  19  */                         	{0x01,                                     		-6},
/*  20  */                         	{0x00,                                     		-5},
/*  21  */                         	{0x01,                                     		-5},
/*  22  */                         	{0x00,                                     		-4},
/*  23  */                         	{0x01,                                     		-4},
/*  24  */                         	{0x00,                                     		-3},
/*  25  */                         	{0x01,                                     		-3},
/*  26  */                         	{0x00,                                     		-2},
/*  27  */                         	{0x01,                                     		-2},
/*  28  */                         	{0x00,                                     		-1},
/*  29  */                          {0x01,                                     		-1},
/*  30  */                          {0x00,                                      	0},
/*  31  */                          {0x01,                                      	0},
/*  32  */                          {0x02,                                      	0},
/*  33  */                          {0x03,                                      	0},
/*  34  */                          {0x04,                                      	0},
/*  35  */                          {0x05,                                      	0},
/*  36  */                          {0x06,                                      	0},
/*  37  */                          {0x07,                                      	0},
/*  38  */                          {0x08,                                      	0},
/*  39  */                          {0x09,                                      	0},
/*  40  */                          {0x0A,                                      	0},
/*  41  */                          {0x0B,                                      	0},
/*  42  */                          {0x0C,                                      	0},
/*  43  */                          {0x0D,                                      	0},
/*  44  */                          {0x0E,                                      	0},
/*  45  */                          {0x0F,                                      	0},
/*  46  */                          {0x10,                                      	0},
/*  47  */                          {0x11,                                      	0},
/*  48  */                          {0x12,                                      	0},
/*  49  */                          {0x13,                                      	0},
/*  50  */                          {0x14,                                      	0},
/*  51  */                          {0x15,                                      	0},
/*  52  */                          {0x16,                                      	0},
/*  53  */                          {0x17,                                      	0},
/*  54  */                          {0x18,                                      	0},
/*  55  */                          {0x19,                                      	0},
/*  56  */                          {0x1A,                                      	0},
/*  57  */                          {0x1B,                                      	0},
/*  58  */                          {0x1C,                                      	0},
/*  59  */                          {0x1D,                                      	0},
/*  60  */                          {0x1E,                                      	0},
/*  61  */                          {0x1F,                                      	0},
/*  62  */                          {0x1e,                                      	1},
/*  63  */                          {0x1F,                                      	1},
/*  64  */                          {0x1e,                                      	2},
/*  65  */                          {0x1F,                                      	2},
/*  66  */                          {0x1e,                                      	3},
/*  67  */                          {0x1F,                                      	3},
/*  68  */                          {0x1e,                                      	4},
/*  69  */                          {0x1F,                                      	4},
/*  70  */                          {0x1e,                                      	5},
/*  71  */                          {0x1F,                                      	5},
/*  72  */                          {0x1e,                                      	6},
/*  73  */                          {0x1F,                                      	6},
/*  74  */                          {0x1e,                                      	7},
/*  75  */                          {0x1F,                                      	7},
/*  76  */                          {0x1e,                                      	8},
/*  77  */                          {0x1F,                                      	8},
/*  78  */                          {0x1e,                                      	9},
/*  79  */                          {0x1F,                                      	9},
/*  80  */                          {0x1e,                                      	10},
/*  81  */                          {0x1F,                                      	10},
/*  82  */                          {0x1e,                                      	11},
/*  83  */                          {0x1F,                                      	11},
/*  84  */                          {0x1e,                                      	12},
/*  85  */                          {0x1F,                                      	12},
/*  86  */                          {0x1e,                                      	13},
/*  87  */                          {0x1F,                                      	13},
/*  88  */                          {0x1e,                                      	14},
/*  89  */                          {0x1F,                                      	14},
/*  90  */                          {0x1e,                                      	15},
/*  91  */                          {0x1F,                                      	15},
};

/* The desired TSSI over CCK */
CHAR desiredTSSIOverCCK[4] = {0};

/* The desired TSSI over OFDM */
CHAR desiredTSSIOverOFDM[8] = {0};

/* The desired TSSI over HT */
CHAR desiredTSSIOverHT[16] = {0};

/* The desired TSSI over HT using STBC */
CHAR desiredTSSIOverHTUsingSTBC[8] = {0};
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef SINGLE_SKU_V2
/* The minimum power of every channel from the power table @ BW 40 */
UCHAR sku_min_pwr_40bw[11] = 
/* 	ch1 	ch2	ch3	ch4	ch5	ch6	ch7	ch8	ch9	ch10	 ch11  */
{	14,	16, 	11, 	12, 	14, 	14, 	14, 	12, 	10, 	14, 	 13	   };

/* The minimum power of every channel from the power table @ BW 20 */
UCHAR sku_min_pwr_20bw[11] = 
/* 	ch1 	ch2	ch3	ch4	ch5	ch6	ch7	ch8	ch9	ch10	 ch11  */
{	14,	16, 	16, 	16, 	16, 	16, 	16, 	16, 	16, 	14, 	 13	   };
#endif /* SINGLE_SKU_V2 */

INT32 pa_mode_table[4] =
{	
	    0,	/* PA mode = 0 :    0 * 8192  pa_mode_00*/	 	
	 4915,	/* PA mode = 1 :  0.6 * 8192  pa_mode_01*/
	-6554,  /* PA mode = 2 : -0.6 * 8192  pa_mode_10*/
	-6554,  /* PA mode = 3 : -0.8 * 8192  pa_mode_11*/
};

#define AMPDU_MAX_LEN_20M1S		0x1030
#define AMPDU_MAX_LEN_40M1S		0x1038
#define TXRX_MICS_CTRL			0x1608

/*
========================================================================
Routine Description:
	Initialize specific MAC registers for RT6352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
static VOID NICInitRT6352MacRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UINT32 IdReg;
	UINT32 Value = 0;
#ifdef APCLI_SUPPORT
	UINT32 pn_mode_value = 0;
#endif /* APCLI_SUPPORT */
#ifdef DESC_32B_SUPPORT
	WPDMA_GLO_CFG_STRUC	GloCfg;
#endif /* DESC_32B_SUPPORT */

	for(IdReg=0; IdReg<RT6352_NUM_MAC_REG_PARMS; IdReg++)
	{
		RTMP_IO_WRITE32(pAd, RT6352_MACRegTable[IdReg].Register,
								RT6352_MACRegTable[IdReg].Value);
	}

	if (pAd->CommonCfg.Chip_VerID <= 1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Initialize MAC Registers for E1 !!!\n"));
		RTMP_IO_WRITE32(pAd, TX_ALC_VGA3, 0x00); /* Gary,2012-03-15 */
		RTMP_IO_WRITE32(pAd, BB_PA_MODE_CFG0, 0x000055FF);
		RTMP_IO_WRITE32(pAd, BB_PA_MODE_CFG1, 0x00550055);
		RTMP_IO_WRITE32(pAd, RF_PA_MODE_CFG0, 0x000055FF);
		RTMP_IO_WRITE32(pAd, RF_PA_MODE_CFG1, 0x00550055);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x00150F0F);
		RTMP_IO_WRITE32(pAd, TX_ALC_VGA3, 0x06060606); /* WH, 2012-10-30 */
		RTMP_IO_WRITE32(pAd, TX0_BB_GAIN_ATTEN, 0x0);
		RTMP_IO_WRITE32(pAd, TX1_BB_GAIN_ATTEN, 0x0);
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_ATTEN, 0x6C6C666C); /* WH, 2012-12-26 */
		RTMP_IO_WRITE32(pAd, TX1_RF_GAIN_ATTEN, 0x6C6C666C); /* WH, 2012-12-26 */
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_CORRECT, 0x3630363A); /* WH, 2012-12-05 */
		RTMP_IO_WRITE32(pAd, TX1_RF_GAIN_CORRECT, 0x3630363A); /* WH, 2012-12-05 */
	}

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &Value);
	Value = Value & (~0x80000000);
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, Value);
#ifdef APCLI_SUPPORT
	/*
	*bit 0 :  R/W PN_PAD_MODE Padding IV/EIV in RX MPDU when packet is decrypted
	*0 : Disable                             1: Enable
	*/
	RTMP_IO_READ32(pAd, PN_PAD_MODE, &pn_mode_value);
	pn_mode_value |= PN_PAD_MODE_OFFSET;
	RTMP_IO_WRITE32(pAd, PN_PAD_MODE, pn_mode_value);
#endif /* APCLI_SUPPORT */
#ifdef DESC_32B_SUPPORT
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG , &GloCfg.word);
	GloCfg.field.Desc32BEn =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
#endif /* DESC_32B_SUPPORT */
}


/*
========================================================================
Routine Description:
	Initialize specific BBP registers for RT6352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
static VOID NICInitRT6352BbpRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UCHAR BbpReg = 0;
	UINT32 i;
	//USHORT k_count = 0;
	//UINT32 MacValue = 0, MacValue1 = 0;
	BBP_R105_STRUC BBPR105 = { { 0 } };

	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	/*	The channel estimation updates based on remodulation of L-SIG and HT-SIG symbols. */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R105, &BBPR105.byte);

	/* Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX) */
	if (pAd->Antenna.field.RxPath == 1) /* Single RX */
		BBPR105.field.MLDFor2Stream = 0;
	else
		BBPR105.field.MLDFor2Stream = 1;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R105, BBPR105.byte);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: BBP_R105: BBPR105.field.EnableSIGRemodulation = %d, BBPR105.field.MLDFor2Stream = %d\n",
			__FUNCTION__,
			BBPR105.field.EnableSIGRemodulation,
			BBPR105.field.MLDFor2Stream));
	
	/*	 Avoid data lost and CRC error */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
	BbpReg = ((BbpReg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);

	/* Fix I/Q swap issue */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpReg);
	BbpReg |= 0x04;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpReg);

	/* BBP for G band  */
	for (i = 0; i < RT6352_NUM_BBP_REG_PARMS; i++)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, RT6352_BBPRegTable[i].Register, RT6352_BBPRegTable[i].Value);

	if (pAd->CommonCfg.Chip_VerID > 1)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R91, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R92, 0x04);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R93, 0x54);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R99, 0x50);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R148, 0x84);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R167, 0x80);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R178, 0xFF);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R106, 0x13); /* James 2013/01/30 */
	}

	/* BBP for G band GLRT function */
	for (i = 0; i < RT6352_NUM_BBP_GLRT; i++)
	{
		/* Write index into BBP_R195 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, RT6352_BBP_GLRT[i].Register);

		/* Write value into BBP_R196 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, RT6352_BBP_GLRT[i].Value);
	}

	/* BBP for G band DCOC function */
	for (i = 0; i < RT6352_NUM_BBP_DCOC; i++)
	{
		/* Write index into BBP_R158 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, RT6352_BBP_DCOC[i].Register);

		/* Write value into BBP_R159 */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, RT6352_BBP_DCOC[i].Value);
	}

	/*	 Avoid data lost and CRC error */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpReg);
	BbpReg = ((BbpReg & ~0x40) | 0x40); /* MAC interface control (MAC_IF_80M, 1: 80 MHz) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpReg);

#ifdef MICROWAVE_OVEN_SUPPORT
	/* Backup BBP_R65  */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &BbpReg);
	pAd->CommonCfg.MO_Cfg.Stored_BBP_R65 = BbpReg;
	DBGPRINT(RT_DEBUG_TRACE, ("Stored_BBP_R65=%x @%s \n", pAd->CommonCfg.MO_Cfg.Stored_BBP_R65, __FUNCTION__));
#endif /* MICROWAVE_OVEN_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}


/*
========================================================================
Routine Description:
	Initialize specific RF registers for RT6352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
static VOID NICInitRT6352RFRegisters(
	IN	PRTMP_ADAPTER pAd)
{
	UCHAR RFValue = 0;
	int i;
	UINT32 SysCfgReg;

	DBGPRINT(RT_DEBUG_TRACE, ("--> %s\n", __FUNCTION__));

	// Initialize RF central register to default value
	for (i = 0; i < RT6352_NUM_RF_CENTRAL_REG_PARMS; i++)
	{
		RT635xWriteRFRegister(pAd, RF_BANK0, RT6352_RFCentralRegTable[i].Register, RT6352_RFCentralRegTable[i].Value);
	}

	if (pAd->CommonCfg.Chip_VerID > 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF Central Registers for E2 !!!\n"));
		RTMP_SYS_IO_READ32(0xb0000010, &SysCfgReg);
		SysCfgReg &= (1 << 6); /* XTAL_FREQ_SEL */
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R11, 0x21);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R13, (SysCfgReg) ? 0x00:0x03);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R14, 0x7C);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R16, 0x80);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R17, 0x99);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R18, 0x99);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R19, 0x09);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R20, 0x50);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R21, 0xB0);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R22, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R23, 0x06);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R24, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R25, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R26, 0x5D);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R27, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R28, 0x61);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R29, 0xB5);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R43, 0x02);
	}

	if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number >= 2))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF Central Registers for E3 !!!\n"));
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R28, 0x62);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R29, 0xAD);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x80);
#ifdef RTMP_TEMPERATURE_CALIBRATION
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R34, 0x23);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x01);
#endif /* RTMP_TEMPERATURE_CALIBRATION */
	}

    /* it is old RF_R17 */
	RFValue = pAd->RfFreqOffset & 0xFF;
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R12, RFValue);

	// Initialize RF channel register to default value
	for (i = 0; i < RT6352_NUM_RF_CHANNEL_REG_PARMS; i++)
	{
		RT635xWriteRFRegister(pAd, RF_BANK4, RT6352_RFChannelRegTable[i].Register, RT6352_RFChannelRegTable[i].Value);
		RT635xWriteRFRegister(pAd, RF_BANK6, RT6352_RFChannelRegTable[i].Register, RT6352_RFChannelRegTable[i].Value);
	}

	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R45, 0xC5);

	if (pAd->CommonCfg.Chip_VerID > 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF Channel Registers for E2 !!!\n"));
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R09, 0x47);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R09, 0x47);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R10, 0x71);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R10, 0x71);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R11, 0x33);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R11, 0x33);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R14, 0x0E);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R14, 0x0E);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R17, 0x23);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R17, 0x23);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R20, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R20, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R21, 0x12);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R21, 0x12);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R28, 0x1C);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R28, 0x1C);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R29, 0xEB);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R29, 0xEB);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R32, 0x7D);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R32, 0x7D);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R34, 0xD6);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R34, 0xD6);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R36, 0x08);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R36, 0x08);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R38, 0xB4);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R38, 0xB4);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R43, 0xD3);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R43, 0xD3);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R44, 0xB3);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R44, 0xB3);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R45, 0xD5);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R45, 0xD5);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R46, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R46, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x67);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0x69);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R48, 0xFF);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R48, 0xFF);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R54, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R54, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R56, 0xFF);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R56, 0xFF);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R57, 0x1C);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R57, 0x1C);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R58, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R58, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R59, 0x6B);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R59, 0x6B);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R60, 0xF7);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R60, 0xF7);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R61, 0x09);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R61, 0x09);
	}

	if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number >= 2))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF Channel Registers for E3 !!!\n"));

		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R10, 0x51);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R10, 0x51);

		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R14, 0x06);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R14, 0x06);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, 0xA7);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, 0xA7);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R28, 0x2C);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R28, 0x2C);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0x64);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0x64);

		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R08, 0x51);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R08, 0x51);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R09, 0x36);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R09, 0x36);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R11, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R11, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R14, 0x16);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R14, 0x16);

		//RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x27);
		//RT635xWriteRFRegister(pAd, RF_BANK4, RF_R49, 0x1F);

		//RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x2B);
		//RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0x2B);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x6C);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0x6C);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R48, 0xFC);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R48, 0xFC);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R49, 0x1F);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R49, 0x1F);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R54, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R54, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R59, 0x6B);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R59, 0x6B);
	}

	if ((pAd->CommonCfg.PKG_ID == 0) && (pAd->CommonCfg.Chip_VerID == 1))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF Channel Registers for DRQFN !!!\n"));

		// Initialize RF channel register for DRQFN
		for (i = 0; i < RT6352_NUM_RF_CHANNEL_REG_DRQFN_PARMS; i++)
		{
			RT635xWriteRFRegister(pAd, RF_BANK4, RT6352_RF_CHANNEL_REG_DRQFN[i].Register, RT6352_RF_CHANNEL_REG_DRQFN[i].Value);
			RT635xWriteRFRegister(pAd, RF_BANK6, RT6352_RF_CHANNEL_REG_DRQFN[i].Register, RT6352_RF_CHANNEL_REG_DRQFN[i].Value);
		}
	}

#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Reduce Power Consumption Support !!!\n"));

		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R43, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R43, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R44, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R44, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R45, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R45, 0x53);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x24);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0x64);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R48, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R48, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R49, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R49, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0x24);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0x64);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R56, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R56, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R57, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R57, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R58, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R58, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R59, 0x24);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R59, 0x64);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R60, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R60, 0x4F);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R61, 0x02);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R61, 0x02);
	}
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */

	// Initialize RF DC calibration register to default value
	for (i = 0; i < RT6352_NUM_RF_DCCAL_REG_PARMS; i++)
	{
		RT635xWriteRFRegister(pAd, RF_BANK5, RT6352_RFDCCalRegTable[i].Register, RT6352_RFDCCalRegTable[i].Value);
		RT635xWriteRFRegister(pAd, RF_BANK7, RT6352_RFDCCalRegTable[i].Register, RT6352_RFDCCalRegTable[i].Value);
	}

	if (pAd->CommonCfg.Chip_VerID > 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF DCCal Registers for E2 !!!\n"));
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, 0x08);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, 0x08);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x04);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x04);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R05, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R05, 0x20);
	}

	if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number >= 2))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Initialize RF DCCal Registers for E3 !!!\n"));
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R05, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R05, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x7C);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, 0x7C);
#ifdef RTMP_TEMPERATURE_CALIBRATION
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x07);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x07);
#endif /* RTMP_TEMPERATURE_CALIBRATION */
	}


	DBGPRINT(RT_DEBUG_TRACE, ("<-- %s\n", __FUNCTION__));
}

#ifdef CONFIG_STA_SUPPORT
static UCHAR RT6352_ChipAGCAdjust(
	IN PRTMP_ADAPTER		pAd,
	IN CHAR					Rssi,
	IN UCHAR				OrigR66Value)
{
	UCHAR R66 = OrigR66Value;
	CHAR lanGain = GET_LNA_GAIN(pAd);

	if (pAd->LatchRfRegs.Channel <= 14)
		R66 = 0x04 + 2 * GET_LNA_GAIN(pAd);

	if (OrigR66Value != R66)
		bbp_set_agc(pAd, R66, RX_CHAIN_ALL);
	
	return R66;
}
#endif // CONFIG_STA_SUPPORT //

static VOID RT6352_ChipBBPAdjust(
	IN RTMP_ADAPTER			*pAd)
{
	UINT32 Value;
	UCHAR byteValue = 0;

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE))
	{
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;

		/* TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/*	RX : control channel at lower */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
			
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : ExtAbove, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
									pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
									pAd->CommonCfg.Channel,
									pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
									pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
	}
	else if ((pAd->CommonCfg.Channel > 2) &&
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
			(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW))
	{
		pAd->CommonCfg.BBPCurrentBW = BW_40;

		if (pAd->CommonCfg.Channel == 14)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
		else
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;

		/*	TX : control channel at upper */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value |= (0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		/*	RX : control channel at upper */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &byteValue);
		byteValue |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, byteValue);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : ExtBlow, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
									pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
									pAd->CommonCfg.Channel,
									pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
									pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	
		/*	TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x1);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
	
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

#ifdef DOT11_N_SUPPORT
		DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp : 20MHz, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
									pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
									pAd->CommonCfg.Channel,
									pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
									pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
	}
}

static VOID RT6352_AsicAntennaDefaultReset(
	IN PRTMP_ADAPTER pAd,
	IN EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAntenna->field.RfIcType = RFIC_6352;
	pAntenna->field.TxPath = 2;
	pAntenna->field.RxPath = 2;
}

static VOID RT6352_ChipSwitchChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN BOOLEAN bScan) 
{
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; //Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER;
	UCHAR	index;
	UINT32 	Value = 0; //BbpReg, Value;
	UCHAR 	RFValue;
	UINT32 i = 0;

	i = i; /* avoid compile warning */
	RFValue = 0;
	// Search Tx power value

#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SkuChannel = Channel;
#endif /* SINGLE_SKU_V2 */	
	/*
		We can't use ChannelList to search channel, since some central channl's txpowr doesn't list 
		in ChannelList, so use TxPower array instead.
	*/
	for (index = 0; index < MAX_NUM_OF_CHANNELS; index++)
	{
		if (Channel == pAd->TxPower[index].Channel)
		{
			TxPwer = pAd->TxPower[index].Power;
			TxPwer2 = pAd->TxPower[index].Power2;
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Can't find the Channel#%d \n", __FUNCTION__, Channel));
	}

	for (index = 0; index < NUM_OF_6352_CHNL; index++)
	{
		if (Channel == FreqItems6352[index].Channel)
		{
			UINT32 macStatus, saveMacSysCtrl, SysCfgReg;
			USHORT k_count = 0;

			/* Frequeny plan setting */
			/*	
			  * Rdiv setting
			  * R13[1:0] 
			  */
			RTMP_SYS_IO_READ32(0xb0000010, &SysCfgReg);
			SysCfgReg &= (1 << 6); /* XTAL_FREQ_SEL */
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R13, &RFValue);
			RFValue = RFValue & (~0x03);
			if (!SysCfgReg)
			RFValue |= (FreqItems6352[index].Rdiv & 0x3);
			RT635xWriteRFRegister(pAd, RF_BANK0,RF_R13, RFValue);

			/*  
 			 * N setting
 			 * R21[0], R20[7:0] 
 			 */
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R20, &RFValue);
			RFValue = (FreqItems6352[index].N & 0x00ff);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R20, RFValue);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R21, &RFValue);
			RFValue = RFValue & (~0x01);
			RFValue |= ((FreqItems6352[index].N & 0x0100) >> 8);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R21, RFValue);

				
			/* 
			 * K setting 
			 * R16[3:0] (RF PLL freq selection)
			 */
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R16, &RFValue);
			RFValue = RFValue & (~0x0f);
			RFValue |= (FreqItems6352[index].K & 0x0f);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R16, RFValue);

			/* 
			 * D setting 
			 * R22[2:0] (D=15, R22[2:0]=<111>)
			 */
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R22, &RFValue);
			RFValue = RFValue & (~0x07);
			RFValue |= (FreqItems6352[index].D & 0x07);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R22, RFValue);

			/*	
			 * Ksd setting
			 * Ksd: R19<1:0>,R18<7:0>,R17<7:0>
			 */
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R17, &RFValue);
			RFValue = (FreqItems6352[index].Ksd & 0x000000ff);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R17, RFValue);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R18, &RFValue);
			RFValue = ((FreqItems6352[index].Ksd & 0x0000ff00) >> 8);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R18, RFValue);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R19, &RFValue);
			RFValue = RFValue & (~0x03);
			RFValue |= ((FreqItems6352[index].Ksd & 0x00030000) >> 16);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R19, RFValue);

			if (pAd->CommonCfg.Chip_VerID > 1)
			{
				/* Default: XO=20MHz , SDM mode */
				RT635xReadRFRegister(pAd, RF_BANK0, RF_R16, &RFValue);
				RFValue = RFValue & (~0xE0);
				RFValue |= 0x80;
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R16, RFValue);

				RT635xReadRFRegister(pAd, RF_BANK0, RF_R21, &RFValue);
				RFValue |= 0x80;
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R21, RFValue);
			}
	
			RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x2);
			else
				RFValue |= 0x2;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFValue);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x20);
			else
				RFValue |= 0x20;

			if (pAd->Antenna.field.RxPath == 1)
				RFValue &= (~0x02);
			else
				RFValue |= 0x02;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFValue);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x40);
			else
				RFValue |= 0x40;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFValue);

			/* RF for DC Cal BW */
			for (i = 0; i < RT6352_NUM_RF_DCCAL_BW; i++)
			{
				if(pAd->CommonCfg.BBPCurrentBW == RT6352_RFDCCal_BW[i].BW)
				{
					RT635xWriteRFRegister(pAd, RF_BANK5, RT6352_RFDCCal_BW[i].Register, RT6352_RFDCCal_BW[i].Value);
					RT635xWriteRFRegister(pAd, RF_BANK7, RT6352_RFDCCal_BW[i].Register, RT6352_RFDCCal_BW[i].Value);
				}
			}

			if (pAd->CommonCfg.Chip_VerID > 1)
			{
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
				{
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, 0x28);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, 0x28);
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, 0x28);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, 0x28);
				}
				else
				{
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, 0x08);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, 0x08);
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, 0x08);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, 0x08);
				}
			}

			if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number == 2))
			{
				RT635xReadRFRegister(pAd, RF_BANK0, RF_R28, &RFValue);
				if ((pAd->CommonCfg.BBPCurrentBW == BW_40) && (Channel == 11))
					RFValue |= 0x4;
				else
					RFValue &= (~0x4);
				RT635xWriteRFRegister(pAd, RF_BANK0, RF_R28, RFValue);
			}

			if (bScan == FALSE)
			{
				/* BandWidth Filter Calibration */
				if (pAd->CommonCfg.BBPCurrentBW == BW_20)
				{
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R06, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R06, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R07, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R07, RFValue);

					RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R58, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R59, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[0];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, RFValue);
				}
				else
				{
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R06, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R06, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R07, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->rx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R07, RFValue);

					RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R58, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, RFValue);
					RT635xReadRFRegister(pAd, RF_BANK7, RF_R59, &RFValue);
					RFValue &= (~0x3F);
					RFValue |= pAd->tx_bw_cal[1];
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, RFValue);
				}
			}

			RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &Value);
			Value = Value & (~0x3F3F);
			Value |= TxPwer;
			Value |= (TxPwer2 << 8);
			Value |= (0x2F << 16);
			Value |= (0x2F << 24);
			
#ifdef RTMP_INTERNAL_TX_ALC
			if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE) && (bScan == FALSE))
			{
				UCHAR target_power;
				//USHORT E2pValue;

				/* init base power by e2p target power */
				//RT28xx_EEPROM_READ16(pAd, 0xD0, E2pValue);
				target_power = (pAd->E2p_D0_Value & 0x3F);
				Value = Value & (~0x3F3F);
				Value |= target_power;
				Value |= (target_power << 8);
			}
#endif /* RTMP_INTERNAL_TX_ALC */
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, Value);

			RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &Value);
			Value = Value & (~0x3F);
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, Value);

#ifdef RTMP_INTERNAL_TX_ALC
			if ((pAd->TxPowerCtrl.bInternalTxALC == TRUE)&& (bScan == FALSE))
			{
				RT635xTssiDcCalibration(pAd);
			}
#endif /* RTMP_INTERNAL_TX_ALC */

			if (bScan == FALSE)
			{
				/* Save MAC SYS CTRL registers */
				RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);

				/* Disable Tx/Rx */
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

				/* Check MAC Tx/Rx idle */
				for (k_count = 0; k_count < 10000; k_count++)
				{
					RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
					if (macStatus & 0x3)
						RtmpusecDelay(50);
					else
						break;
				}

				if (k_count == 10000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("(%s) Wait MAC Status to MAX  !!!\n", __FUNCTION__));
				}

				if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number >= 2))
				{
					UINT8 BBPValue;

					/* ADC clcok selection */
					DBGPRINT(RT_DEBUG_TRACE, ("ADC clcok selection for E3 !!!\n"));
					{
						if (Channel > 10)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("Apr clock selection !!!\n"));
							RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPValue);
							BBPValue = 0x40;
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPValue);
							RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x00);
#ifdef RT6352_EL_SUPPORT
							if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0xFB);
							}
							else
#endif /* RT6352_EL_SUPPORT */
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x7B);
							}
						}
						else
						{
							DBGPRINT(RT_DEBUG_TRACE, ("Shielding clock selection !!!\n"));
							RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPValue);
							BBPValue = 0x1F;
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPValue);
							RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x80);
#ifdef RT6352_EL_SUPPORT
							if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0xDB);
							}
							else
#endif /* RT6352_EL_SUPPORT */
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x5B);
							}
						}
					}
				}

				/* Restore MAC SYS CTRL registers */
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
			}

			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R05, 0x40);
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, 0x0C);

			RT635xReadRFRegister(pAd, RF_BANK0, RF_R04, &RFValue);
			RFValue = ((RFValue & ~0x80) | 0x80); /* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. */
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, RFValue);
			RtmpOsMsDelay(2);

			/* latch channel for future usage.*/
			pAd->LatchRfRegs.Channel = Channel;

			DBGPRINT(RT_DEBUG_TRACE, ("RT6352: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), Rdiv=0x%02X, N=0x%02X, K=0x%02X, D=0x%02X, Ksd=0x%02X\n",
							Channel,
							pAd->RfIcType, 
							TxPwer,
							TxPwer2,
							pAd->Antenna.field.TxPath,
							FreqItems6352[index].Rdiv,
							FreqItems6352[index].N,
							FreqItems6352[index].K,
							FreqItems6352[index].D, 
							FreqItems6352[index].Ksd));
			break;
		}
	}

	if (bScan == FALSE)
	{
		pAd->Tx0_DPD_ALC_tag0 = 0;
		pAd->Tx0_DPD_ALC_tag1 = 0;
		pAd->Tx1_DPD_ALC_tag0 = 0;
		pAd->Tx1_DPD_ALC_tag1 = 0;

		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;


		/* DPD_Calibration */
#ifdef RT6352_EP_SUPPORT
		if (pAd->bExtPA == FALSE)
#endif /* RT6352_EP_SUPPORT */
		{
			DoDPDCalibration(pAd);
			pAd->DoDPDCurrTemperature = 0x7FFFFFFF;
		}

		/* Rx DCOC Calibration */		
		RxDCOC_Calibration(pAd);

#ifdef MICROWAVE_OVEN_SUPPORT
		/* B5.R6 and B5.R7 */
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &RFValue);
		pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R6 = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &RFValue);
		pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R7 = RFValue;
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			RT635xReadRFRegister(pAd, RF_BANK7, RF_R06, &RFValue);
			pAd->CommonCfg.MO_Cfg.Stored_RF_B7_R6 = RFValue;
			RT635xReadRFRegister(pAd, RF_BANK7, RF_R07, &RFValue);
			pAd->CommonCfg.MO_Cfg.Stored_RF_B7_R7 = RFValue;
		}
#endif /* RT6352 */
		DBGPRINT(RT_DEBUG_TRACE,("%s: Stored_RF_B5_R6=%x, and Stored_RF_B5_R7=%x \n", __FUNCTION__,
						pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R6, pAd->CommonCfg.MO_Cfg.Stored_RF_B5_R7));
#endif /* MICROWAVE_OVEN_SUPPORT */

	}

	/* BBP setting */
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00150F0F;/* Gary 2007/08/09 0x050A0A */

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Turn off unused PA or LNA when only 1T or 1R */
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg &= 0xFFFFFFF3;
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= 0xFFFFF3FF;

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		filter_coefficient_ctrl(pAd, Channel);
	}

	/* BBP for GLRT BW */
	for (i = 0; i < RT6352_NUM_BBP_GLRT_BW; i++)
	{
		if(pAd->CommonCfg.BBPCurrentBW == RT6352_BBP_GLRT_BW[i].BW)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, RT6352_BBP_GLRT_BW[i].Register);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, RT6352_BBP_GLRT_BW[i].Value);
		}
	}

#ifdef RT6352_EL_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
	{
		if(pAd->CommonCfg.BBPCurrentBW == BW_20)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R141);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
		}
	}
#endif /* RT6352_EL_SUPPORT */

	if (pAd->Antenna.field.RxPath == 1)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R91, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, 0x1A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R128);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0xA0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R170);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x12);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R171);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x10);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R91, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, 0x9A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R128);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0xE0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R170);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x30);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R171);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x30);
	}

	/* R66 should be set according to Channel and use 20MHz when scanning*/
	/*bbp_set_agc(pAd, (0x2E + lanGain), RX_CHAIN_ALL);*/
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	/* On 11A, We should delay and wait RF/BBP to be stable*/
	/* and the appropriate time should be 1000 micro seconds */
	/* 2005/06/05 - On 11G, We also need this delay time. Otherwise it's difficult to pass the WHQL.*/
	RtmpusecDelay(1000);
}

static VOID RT6352_RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR BandWidth)
{
	UCHAR R66 = 0;

	if (pAd->LatchRfRegs.Channel <= 14)
		R66 = 0x04 + 2 * GET_LNA_GAIN(pAd);
	bbp_set_agc(pAd, R66, RX_CHAIN_ALL);

#ifdef DYNAMIC_VGA_SUPPORT
	/* Backup BBP_R66  */
	pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0 = R66;
	pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_1 = R66;
#endif /* DYNAMIC_VGA_SUPPORT */

}

#ifdef RTMP_INTERNAL_TX_ALC
INT16 RT6352_lin2dBd(
	IN	unsigned short linearValue)
{
    short exp;
    unsigned int mantisa;
    int app,dBd;

	/* Default backoff ; to enhance leading bit searching time */
	mantisa = linearValue << DEFAULT_BO;
	exp = -(DEFAULT_BO);

	/* Leading bit searching */
	if (mantisa < (0x8000))
	{
		while (mantisa < (0x8000))
		{
			mantisa = mantisa << 1; /* no need saturation */
			exp--;
			if (exp < -20)
			{
				DBGPRINT_ERR(("input too small\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
	else 
	{
		while (mantisa > (0xFFFF))
		{
			mantisa = mantisa >> 1; /* no need saturation */
			exp ++;
			if (exp > 20)
			{
				DBGPRINT_ERR(("input too large\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
/*	printk("exp=0d%d,mantisa=0x%x\n",exp,mantisa); */

	if (mantisa <= 47104)
	{
		app=(mantisa+(mantisa>>3)+(mantisa>>4)-38400); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}
	else
	{
		app=(mantisa-(mantisa>>3)-(mantisa>>6)-23040); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}

	dBd=((15+exp)<<15)+app; /*since 2^15=1 here */
/*	printk("dBd1=%d\n",dBd); */
	dBd=(dBd<<2)+(dBd<<1)+(dBd>>6)+(dBd>>7);
	dBd=(dBd>>10); /* S10.5 */
/*	printk("app=%d,dBd=%d,dBdF=%f\n",app,dBd,(double)dBd/32); */

	return(dBd);
}

CHAR SignedExtension6To8(
	IN CHAR 		org_value)
{
	/* 6-bit --->  8-bit */
	CHAR value = org_value;

	DBGPRINT(RT_DEBUG_INFO, ("%s: original value is 0x%02x\n", __FUNCTION__, value));

	if ((value & 0x20) == 0x00) /* positive */
	{
		value = (value & ~0xE0);
	}
	else /* 0x20: negative number */
	{
		value = (value | 0xC0);
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: extended value is 0x%02x\n", __FUNCTION__, value));

	return value;
}

static INT32 rt635x_tssi_slope[2];
static INT32 rt635x_tssi_offset[3][2];
static INT32 rt635x_ant_pwr_offset[2];

/* read tssi_slope and tssi_offset from eeprom/efuse */
VOID RT635xGetTssiInfo(
	IN PRTMP_ADAPTER			pAd)
{
	USHORT offset, value;
	CHAR temp;
	
	/* get tssi slope 0 */
	offset = 0x6E;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	rt635x_tssi_slope[0] = (INT32)(value & 0x00FF);

	/* get tssi_offset 0 */
	temp = (CHAR)((value & 0xFF00)>>8);
	rt635x_tssi_offset[GROUP1_2G][0] = (INT32)temp;

	offset = 0x70;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	rt635x_tssi_offset[GROUP2_2G][0] = (INT32)temp;
	temp = (CHAR)((value & 0xFF00)>>8);
	rt635x_tssi_offset[GROUP3_2G][0] = (INT32)temp;

	/* get tssi slope 1 */
	offset = 0x72;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	rt635x_tssi_slope[1] = (INT32)(value & 0x00FF);

	/* get tssi_offset 1 */
	temp = (CHAR)((value & 0xFF00)>>8);
	rt635x_tssi_offset[GROUP1_2G][1] = (INT32)temp;

	offset = 0x74;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	rt635x_tssi_offset[GROUP2_2G][1] = (INT32)temp;
	temp = (CHAR)((value & 0xFF00)>>8);
	rt635x_tssi_offset[GROUP3_2G][1] = (INT32)temp;

	/* get tx0/tx1 power offset */
	offset = 0x76;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	rt635x_ant_pwr_offset[0] = (INT32)(temp * 1024); /* 8192/8 */
	temp = (CHAR)((value & 0xFF00)>>8);
	rt635x_ant_pwr_offset[1] = (INT32)(temp * 1024); /* 8192/8 */

    if((rt635x_tssi_slope[0] < 0x66) || (rt635x_tssi_slope[0] > 0x99)){
		rt635x_tssi_slope[0] = 0x80;
		rt635x_tssi_offset[GROUP1_2G][0] = 0x0;
		rt635x_tssi_offset[GROUP2_2G][0] = 0x0;
		rt635x_tssi_offset[GROUP3_2G][0] = 0x0;
    }
	
    if((rt635x_tssi_slope[1] < 0x66) || (rt635x_tssi_slope[1] > 0x99)){
		rt635x_tssi_slope[1] = 0x80;
		rt635x_tssi_offset[GROUP1_2G][1] = 0x0;
		rt635x_tssi_offset[GROUP2_2G][1] = 0x0;
		rt635x_tssi_offset[GROUP3_2G][1] = 0x0;
    }
	
	return;
}

/* results of TSSI DC calibration */
INT32 rt635x_tssi0_dc;
INT32 rt635x_tssi1_dc;
INT32 rt635x_tssi0_dc_hvga;
INT32 rt635x_tssi1_dc_hvga;
INT32 rt635x_tssi0_db_hvga;
INT32 rt635x_tssi1_db_hvga;

INT RT635xTssiDcCalibration(
	IN	PRTMP_ADAPTER	pAd)
{
#ifdef RALINK_ATE
	PATE_INFO 	pATEInfo = &(pAd->ate);
#endif /* RALINK_ATE */
	UINT32 MacValue = 0;
	INT32  MacValue_tmp = 0;
	UINT32 Mac1004_Org = 0;
	UINT32 index;
	UINT32 ant_idx;
	INT32 tssi0_linear = 0, tssi1_linear = 0;
	INT32 tssi0_db = 0, tssi1_db = 0, pwr=0, pwr1=0;
	CHAR BBPR49;
	UCHAR RFB5R3_Org, RFB7R3_Org, RFB4R39_Org, RFB6R39_Org,use_ant = 0;
	UCHAR RFValue, BBPValue, BBPR1_Org;
	CHAR  bbp49_read, wait = 0, max_recal = 0;
	UCHAR ch_group, channel;
	UCHAR RFB0R42_Org, RFB0R2_Org, RFB0R1_Org;	
	UCHAR BBPTXGAIN[2];
	UCHAR agc_loop_cnt;
	RT635xGetTssiInfo(pAd);

	RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &RFB0R1_Org);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &RFB0R2_Org);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFB0R42_Org);
	
	RFValue = 0x03;
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFValue);
	RFValue = 0x33;
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFValue);
	RFValue = RFB0R42_Org | 0x50;
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFValue);

	/* MAC Bypass */
	MacValue = 0x0004;
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacValue);
	MacValue = 0x3366;
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MacValue);
 
	MacValue = 0x0; /* ALC control */
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacValue);
	MacValue = 0xf1f1;
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacValue);

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Mac1004_Org);
	MacValue = 0x0;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1_Org);
	BBPValue = BBPR1_Org;
	BBPValue &= ~((1<<4)|(1<<3));
	BBPValue |= ((1<<4)|(0<<3));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);

	/* set VGA gain */
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFB5R3_Org);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFB7R3_Org);
	RFValue = 0x08;
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);

	/* mixer disable */
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R39, &RFB4R39_Org);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R39, &RFB6R39_Org);
	RFValue = 0x00;
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R39, RFValue);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R39, RFValue);

redo:
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);

	/* step 4 */
	for(ant_idx = 0; ant_idx < 2; ant_idx = ant_idx + 1){
		if(ant_idx == 0)
			BBPTXGAIN[0]= 0x8;
		else
			BBPTXGAIN[1]= 0x8;
		agc_loop_cnt = 0;

		for (index = 0; index < 4; index++)
		{
			switch(ant_idx){
				case 0: 
					RFValue = 0x01;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFValue);
					RFValue = 0x11;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFValue);
					// DAC 0 only
					RFValue = RFB0R42_Org & (~0x50);
					RFValue = RFB0R42_Org | 0x10;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFValue);
					MacValue = 0x1004;
					RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacValue);
					break;
				case 1: 				
					RFValue = 0x02;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFValue);
					RFValue = 0x22;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFValue);				
					// DAC 1 only
					RFValue = RFB0R42_Org & (~0x50);
					RFValue = RFB0R42_Org | 0x40;
					RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFValue);
					MacValue = 0x2004;
					RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacValue);
					break;
			}
		
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
			/* a */
			switch (index)
			{
				case 0:
				case 1:
					RFValue = 0x00;
					RT635xWriteRFRegister(pAd, RF_BANK4, RF_R39, RFValue);
					RT635xWriteRFRegister(pAd, RF_BANK6, RF_R39, RFValue);
					break;
				default:
					RFValue = 0xB3;
					RT635xWriteRFRegister(pAd, RF_BANK4, RF_R39, RFValue);
					RT635xWriteRFRegister(pAd, RF_BANK6, RF_R39, RFValue);
					break;
			}
			RT635xReadRFRegister(pAd, RF_BANK4, RF_R39, &RFValue);
			DBGPRINT(RT_DEBUG_TRACE, ("RF_B4R39 %08x\n", RFValue));
			RT635xReadRFRegister(pAd, RF_BANK6, RF_R39, &RFValue);
			DBGPRINT(RT_DEBUG_TRACE, ("RF_B6R39 %08x\n", RFValue));

			/* b */
			switch (index)
			{
				case 0:
					RFValue = 0x08;/* low tssi vga gain */
					break;
				case 1: 
					RFValue = 0x11;/* high tssi vga gain */
					break; 
				case 2: 
					RFValue = 0x08;
					break;
				case 3:
					RFValue = 0x11;
					break;
			}
			
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);

			/* c */
			/* BBP TSSI initial and soft reset */
			BBPValue = 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R58, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, BBPValue);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BBPValue);
			BBPValue |= 0x1;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BBPValue);
			RtmpusecDelay(1); /* wait 1 usec */
			BBPValue &= ~(0x1);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BBPValue);

			/* d */
			/* TSSI measurement */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, 0x50);

			switch (index)
			{
				case 0:
				case 1:
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x40);
					break;
				case 2:					
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x31);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, BBPTXGAIN[0]);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BBPTXGAIN[1]);
					break;
				default:
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x31);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, BBPTXGAIN[0]-0x6);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BBPTXGAIN[1]-0x6);
					break;
			}
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R244, &BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, ("BBP_R244 is 0x%02x\n", BBPValue));
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, ("BBP_R22 is 0x%02x\n", BBPValue));

			for (wait=0; wait < 100; wait++)
			{
				RtmpusecDelay(2000);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

				if (BBPValue != 0x50)
					break;
			}
			
			if ((wait >= 100) && (max_recal < 10))
			{
				ASSERT(wait < 100);
				max_recal++;
				goto redo;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("BBP_R47 is 0x%02x\n", BBPValue));

			/* e */
			/* TSSI read */
			
			BBPValue = (ant_idx == 0) ? 0x40 : 0x45;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
			RtmpusecDelay(10); /* wait 10 usec */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49); 							

#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT
			if((index==2) && (agc_loop_cnt < 10))
			{	
				DBGPRINT( RT_DEBUG_ERROR, 
					      ("BBPR49 %x BBPTXgain0 %x, BBPTXgain1 %x, loop cnt %d \n", BBPR49, BBPTXGAIN[0], BBPTXGAIN[1], agc_loop_cnt));
				agc_loop_cnt = agc_loop_cnt + 1;
				tssi0_linear = (INT32)BBPR49;
				tssi0_linear = tssi0_linear - ((ant_idx ==0) ? rt635x_tssi0_dc : rt635x_tssi1_dc);
				if(BBPTXGAIN[ant_idx]> 0xf)
				{
					tssi0_linear = tssi0_linear;
				}				
				else if(tssi0_linear < 0x4)
				{
					BBPTXGAIN[ant_idx]= BBPTXGAIN[ant_idx]+6;
					if(BBPTXGAIN[ant_idx]> 0x10)
						BBPTXGAIN[ant_idx]= 0x10;
					index = index - 1;
					continue;
				}
				else if(tssi0_linear < 0x8)
				{
					BBPTXGAIN[ant_idx]= BBPTXGAIN[ant_idx]+3;
					if(BBPTXGAIN[ant_idx]> 0x10)
						BBPTXGAIN[ant_idx]= 0x10;
					index = index - 1;
					continue;
				}
				else if(tssi0_linear < 0x10)
				{
					BBPTXGAIN[ant_idx]= BBPTXGAIN[ant_idx]+1;
					if(BBPTXGAIN[ant_idx]> 0x10)
						BBPTXGAIN[ant_idx] = 0x10;
					index = index - 1;
					continue;
				}	
			}

			if(index==2)
			{			
				DBGPRINT(RT_DEBUG_ERROR, ("BBPTXgain0 %x, BBPTXgain1 %x, loop cnt %d \n", BBPTXGAIN[0], BBPTXGAIN[1], agc_loop_cnt));
			}
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */

			if(ant_idx == 0){
				switch (index){
					case 0:
						rt635x_tssi0_dc = (INT32)BBPR49;
						/* don't remove it ,for TSSI Calibration */
						printk("rt635x_tssi0_dc is 0x%08x\n", rt635x_tssi0_dc);
						break;
					case 1: 								
						rt635x_tssi0_dc_hvga = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc_hvga is 0x%08x\n", rt635x_tssi0_dc_hvga));
						break;
					case 2:								
						tssi0_linear = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("index %d: tssi0_linear is 0x%08x\n", index, tssi0_linear));
								
						tssi0_db = RT6352_lin2dBd(tssi0_linear - rt635x_tssi0_dc);
						DBGPRINT(RT_DEBUG_TRACE, ("tssi0_db is %d\n", tssi0_db));
						break;
					case 3:								
						tssi0_linear = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("index %d: tssi0_linear hvga is 0x%08x\n", index, tssi0_linear));
								
						tssi0_linear = tssi0_linear - rt635x_tssi0_dc_hvga;
						tssi0_linear = tssi0_linear * 4;								
						rt635x_tssi0_db_hvga = RT6352_lin2dBd(tssi0_linear) - tssi0_db;

						DBGPRINT(RT_DEBUG_TRACE, ("tssi0_db_hvga is %d\n", rt635x_tssi0_db_hvga));
						break;
				}
			}
			else{
				switch (index){
					case 0:
						rt635x_tssi1_dc = (INT32)BBPR49;
						/* don't remove it ,for TSSI Calibration */
						printk("rt635x_tssi1_dc is 0x%08x\n", rt635x_tssi1_dc);
						break;
					case 1:					
						rt635x_tssi1_dc_hvga = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc_hvga is 0x%08x\n", rt635x_tssi1_dc_hvga));
						break;
					case 2:
						tssi1_linear = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("index %d: tssi1_linear is 0x%08x\n", index, tssi1_linear));
						
						tssi1_db = RT6352_lin2dBd(tssi1_linear - rt635x_tssi1_dc); 			
						DBGPRINT(RT_DEBUG_TRACE, ("tssi1_db is %d\n", tssi1_db));
						break;
					case 3:				
						tssi1_linear = (INT32)BBPR49;
						DBGPRINT(RT_DEBUG_TRACE, ("index %d: tssi1_linear hvga is 0x%08x\n", index, tssi1_linear));
						
						tssi1_linear = tssi1_linear - rt635x_tssi1_dc_hvga;
						tssi1_linear = tssi1_linear*4;

						rt635x_tssi1_db_hvga = RT6352_lin2dBd(tssi1_linear) - tssi1_db;
						DBGPRINT(RT_DEBUG_TRACE, ("tssi1_db_hvga is %d\n", rt635x_tssi1_db_hvga));						
						break;
				}
			}

			MacValue = 0x0004;
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacValue);
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
	}

#ifdef DOT11_N_SUPPORT				
	channel = pAd->CommonCfg.CentralChannel;
#else
	channel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		if (pATEInfo->TxAntennaSel == 0) // 2 antenna
			use_ant = 0;
		else if(pATEInfo->TxAntennaSel == 1) // 1st antenna
			use_ant = 1;
		else // 2nd antenna
			use_ant = 2;
	}
	else
#endif /* RALINK_ATE */
	{
		if(pAd->Antenna.field.TxPath == 2) // 2 antenna
			use_ant = 0;
		else // 1st antenna
			use_ant = 1;
	}

	ch_group = GET_2G_CHANNEL_GROUP(channel);		
	if(use_ant == 2) // only 2nd Tx
		pwr = (tssi1_db*(rt635x_tssi_slope[1])) + (rt635x_tssi_offset[ch_group][1] << 9);
	else // only  1st Tx or all Tx
		pwr = (tssi0_db*(rt635x_tssi_slope[0])) + (rt635x_tssi_offset[ch_group][0] << 9);
	
	pwr = (pwr + 2048) / 4096;
	pwr = -pwr+6;
#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT
	pwr = pwr+(BBPTXGAIN[0]-8)*2;
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */
	if(pwr > 0x1f)
		pwr = 0x1f; // + max
	else if(pwr < -0x1f)
		pwr = -0x1f; // - max

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);
	MacValue = MacValue | (pwr & 0x3f);
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);	

	if (use_ant == 0){ // All  Tx
		pwr1 = (tssi1_db*(rt635x_tssi_slope[1])) + (rt635x_tssi_offset[ch_group][1] << 9);
		pwr1 = (pwr1+ 2048) / 4096;
		pwr1 = -pwr1+6;
		if(pwr1 > 0x1f)
			pwr1 = 0x1f; // + max
		else if(pwr1 < -0x1f)
			pwr1 = -0x1f; // - max

		RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */	
		DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
		MacValue_tmp = (MacValue >> 8) & 0x3f;
		MacValue_tmp = MacValue_tmp + pwr1 - pwr;
		
		if(MacValue_tmp > 0x3f)
			MacValue_tmp = 0x3f;
		else if(MacValue_tmp < 0x0)
			MacValue_tmp = 0x0;
		
		MacValue_tmp = 	MacValue_tmp & 0x3f;
		MacValue = MacValue & (~0x3f00);   
		MacValue = MacValue | (MacValue_tmp << 8);
		DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
		RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);	
	}	

	/* recover */
	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BBPValue);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, BBPValue);
	/* BBP soft reset */
	BBPValue = 0x01;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BBPValue);
	RtmpusecDelay(1); /* wait 1 usec */
	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BBPValue);

	/* recover MAC */
	MacValue = 0x0;
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MacValue);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacValue);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacValue);
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacValue); /* ALC control */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Mac1004_Org);

	/* recover BBP */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1_Org);

	/* recover RF */
	
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFB0R1_Org);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFB0R2_Org);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFB0R42_Org);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFB5R3_Org);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFB7R3_Org);
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R39, RFB4R39_Org);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R39, RFB6R39_Org);

	return TRUE;
}

/* TSSI tables */
INT32 RT635x_McsPowerOverCCK[4] = {0};
INT32 RT635x_McsPowerOverOFDM[8] = {0};
INT32 RT635x_McsPowerOverHT[16] = {0};
INT32 RT635x_McsPowerOverHTSTBC[8] = {0};
UCHAR RT635x_RfPaModeOverCCK[4] = {0};
UCHAR RT635x_RfPaModeOverOFDM[8] = {0};
UCHAR RT635x_RfPaModeOverHT[16] = {0};
//UCHAR RT635x_RfPaModeOverHTSTBC[8] = {0};

VOID RT635xInitMcsPowerTable(
	IN PRTMP_ADAPTER			pAd)
{
	UINT32 MacValue;
	CHAR temp0, temp1, temp2, temp3;


	/* CCK */
	RTMP_IO_READ32(pAd, TX_PWR_CFG_0, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[13:8] */
	temp1 = (CHAR)((MacValue & 0x00003F00) >> 8);
	/* bit[21:16] */
	temp2 = (CHAR)((MacValue & 0x003F0000) >> 16);
	/* bit[29:24] */
	temp3 = (CHAR)((MacValue & 0x3F000000) >> 24);

	RT635x_McsPowerOverCCK[0] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverCCK[1] = RT635x_McsPowerOverCCK[0];
	RT635x_McsPowerOverCCK[2] = (INT32)SignedExtension6To8(temp1);
	RT635x_McsPowerOverCCK[3] = RT635x_McsPowerOverCCK[2];

	/* OFDM */
	RT635x_McsPowerOverOFDM[0] = (INT32)SignedExtension6To8(temp2);
	RT635x_McsPowerOverOFDM[1] = RT635x_McsPowerOverOFDM[0];
	RT635x_McsPowerOverOFDM[2] = (INT32)SignedExtension6To8(temp3);
	RT635x_McsPowerOverOFDM[3] = RT635x_McsPowerOverOFDM[2];

	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[13:8] */
	temp1 = (CHAR)((MacValue & 0x00003F00) >> 8);
	/* bit[21:16] */
	temp2 = (CHAR)((MacValue & 0x003F0000) >> 16);
	/* bit[29:24] */
	temp3 = (CHAR)((MacValue & 0x3F000000) >> 24);
	RT635x_McsPowerOverOFDM[4] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverOFDM[5] = RT635x_McsPowerOverOFDM[4];
	RT635x_McsPowerOverOFDM[6] = (INT32)SignedExtension6To8(temp1);

	/* MCS */
	RT635x_McsPowerOverHT[0] = (INT32)SignedExtension6To8(temp2);
	RT635x_McsPowerOverHT[1] = RT635x_McsPowerOverHT[0];
	RT635x_McsPowerOverHT[2] = (INT32)SignedExtension6To8(temp3);
	RT635x_McsPowerOverHT[3] = RT635x_McsPowerOverHT[2];

	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[13:8] */
	temp1 = (CHAR)((MacValue & 0x00003F00) >> 8);
	/* bit[21:16] */
	temp2 = (CHAR)((MacValue & 0x003F0000) >> 16);
	/* bit[29:24] */
	temp3 = (CHAR)((MacValue & 0x3F000000) >> 24);
	RT635x_McsPowerOverHT[4] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverHT[5] = RT635x_McsPowerOverHT[4];
	RT635x_McsPowerOverHT[6] = (INT32)SignedExtension6To8(temp1);

	RT635x_McsPowerOverHT[8] = (INT32)SignedExtension6To8(temp2);
	RT635x_McsPowerOverHT[9] = RT635x_McsPowerOverHT[8];
	RT635x_McsPowerOverHT[10] = (INT32)SignedExtension6To8(temp3);
	RT635x_McsPowerOverHT[11] = RT635x_McsPowerOverHT[10];

	RTMP_IO_READ32(pAd, TX_PWR_CFG_3, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[13:8] */
	temp1 = (CHAR)((MacValue & 0x00003F00) >> 8);
	/* bit[21:16] */
	temp2 = (CHAR)((MacValue & 0x003F0000) >> 16);
	/* bit[29:24] */
	temp3 = (CHAR)((MacValue & 0x3F000000) >> 24);
	RT635x_McsPowerOverHT[12] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverHT[13] = RT635x_McsPowerOverHT[12];
	RT635x_McsPowerOverHT[14] = (INT32)SignedExtension6To8(temp1);

	/* HT STBC */
	RT635x_McsPowerOverHTSTBC[0] = (INT32)SignedExtension6To8(temp2);
	RT635x_McsPowerOverHTSTBC[1] = RT635x_McsPowerOverHTSTBC[0];
	RT635x_McsPowerOverHTSTBC[2] = (INT32)SignedExtension6To8(temp3);
	RT635x_McsPowerOverHTSTBC[3] = RT635x_McsPowerOverHTSTBC[2];

	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[13:8] */
	temp1 = (CHAR)((MacValue & 0x00003F00) >> 8);
	RT635x_McsPowerOverHTSTBC[4] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverHTSTBC[5] = RT635x_McsPowerOverHTSTBC[4];
	RT635x_McsPowerOverHTSTBC[6] = (INT32)SignedExtension6To8(temp1);

	/* extra */
	RTMP_IO_READ32(pAd, TX_PWR_CFG_7, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	/* bit[21:16] */
	temp2 = (CHAR)((MacValue & 0x003F0000) >> 16);
	RT635x_McsPowerOverOFDM[7] = (INT32)SignedExtension6To8(temp0);
	RT635x_McsPowerOverHT[7] = (INT32)SignedExtension6To8(temp2);

	RTMP_IO_READ32(pAd, TX_PWR_CFG_8, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	RT635x_McsPowerOverHT[15] = (INT32)SignedExtension6To8(temp0);

	RTMP_IO_READ32(pAd, TX_PWR_CFG_9, &MacValue);
	/* bit[5:0] */
	temp0 = (CHAR)((MacValue & 0x0000003F));
	RT635x_McsPowerOverHTSTBC[7] = (INT32)SignedExtension6To8(temp0);

	return;
}


VOID RT635xInitRfPaModeTable(
	IN PRTMP_ADAPTER			pAd)
{
	UINT32 MacValue;
	UCHAR temp, bit;

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &MacValue); /* MAC 0x121C */

	/* CCK */
	for (bit = 0; bit < 8; bit += 2)
	{
		temp = (UCHAR)((MacValue >> bit) & (0x03));
		RT635x_RfPaModeOverCCK[bit/2] = temp;
	}

	/* OFDM */
	for (bit = 8; bit < 24; bit += 2)
	{
		temp = (UCHAR)((MacValue >> bit) & (0x03));
		RT635x_RfPaModeOverOFDM[(bit-8)/2] = temp;
	}

// TODO:MCS 32
		
	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &MacValue); /* MAC 0x1220 */
	/* HT */
	for (bit = 0; bit < 32; bit += 2)
	{
		temp = (UCHAR)((MacValue >> bit) & (0x03));
		RT635x_RfPaModeOverHT[bit/2] = temp;
	}

	return;	
}

static VOID RT6352_InitDesiredTSSITable(
	IN PRTMP_ADAPTER 		pAd)
{
	
	if (pAd->TxPowerCtrl.bInternalTxALC == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("internal ALC is not enabled in NVM !\n"));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("---> %s\n", __FUNCTION__));

	RT635xInitMcsPowerTable(pAd);
	RT635xInitRfPaModeTable(pAd);	
	
	DBGPRINT(RT_DEBUG_TRACE, ("<--- %s\n", __FUNCTION__));

	return;
}

static INT32 rt635x_pwr_diff_pre[2];

VOID RT635xTssiCompensation(
	IN	PRTMP_ADAPTER	pAd,
	IN	CHAR		percent_delta)
{
	UINT32 MacValue;
	INT32 base_power[max_ant];
	INT32 tssi_read[max_ant];
	INT32 pa_mode_00, pa_mode_01, pa_mode_11, pa_mode_used;
	INT32 target_power = 0, mcs_power = 0, _comp_power = 0, bbp_6db_power = 0;
	INT32 tssi_dc, tssi_m_dc, tssi_db;
	INT32 pwr, pwr_diff[max_ant], pwr_diff_pre, pkt_type_delta;
	INT32 comp_power[max_ant]; 
	INT32 cur_comp_power;
	UCHAR RFValue, BBPValue, BBPR4, tssi_use_hvga[max_ant];
	UCHAR wait, ch_group, mcs, pa_mode, chain, channel;
	CHAR temp[max_ant];
	CHAR BBPR49;
	UCHAR max_stream = 1;

#ifdef DOT11_N_SUPPORT				
	channel = pAd->CommonCfg.CentralChannel;
#else
	channel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

	/* TSSI compensation */
	RT635xInitMcsPowerTable(pAd);
	RT635xInitRfPaModeTable(pAd);

#ifndef SINGLE_SKU_V2
	/* init base power by e2p target power */
	//RT28xx_EEPROM_READ16(pAd, 0xD0, E2pValue);
	target_power = (INT32)(pAd->E2p_D0_Value & 0xFF);

	/* reduce target power if not 100% specified from UI */
	//target_power += (INT32)(percent_delta*2);
	target_power += (INT32)(percent_delta);
	target_power = (target_power < 0x0A) ? (0x0A) : target_power;

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);
	if ((MacValue & 0x3f) != target_power)
	{
		MacValue = MacValue & (~0x3F3F);
		MacValue |= target_power;
		MacValue |= (target_power << 8);
		RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);
	}
#endif /* SINGLE_SKU_V2 */

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
	{
		if (pAd->CommonCfg.TxStream == 2)
			max_stream = 2;
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		max_stream = 1;	
	}

#ifdef GREENAP_SUPPORT
	if ((pAd->ApCfg.bGreenAPEnable == TRUE) &&
		(pAd->ApCfg.GreenAPLevel!= GREENAP_11BGN_STAS))
	{
		max_stream = 1; 
	}
#endif /* GREENAP_SUPPORT */

	/* 0. get base power from 0x13B0 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);

	/* bit[5:0] */
	base_power[0] = (MacValue & 0x0000003F);
	/* bit[13:8] */
	base_power[1] = (MacValue & 0x00003F00) >> 8;

	for (chain=0; chain < max_stream; chain++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("\nchain %d: base_power is %d\n", chain, base_power[chain]));

		if (base_power[0] <= 20)
		{
			tssi_use_hvga[chain] = 1;
		}
		else
		{
			tssi_use_hvga[chain] = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("\ntssi_use_hvga %d \n", tssi_use_hvga[chain]));
	}

	/* 1. set TSSI mode */
	if (tssi_use_hvga[0] == 1)
	{
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
	}
	else
	{
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
	}

	/* for 2T2R */
	if (tssi_use_hvga[1] == 1)
	{
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
	}
	else
	{
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= ~((1<<6)|(1<<5));
	BBPValue |= ((1<<6)|(0<<5));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R58, 0x00);

	/* enable TSSI for next reading */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	
	if (!(BBPValue & (1<<4)))
	{
		BBPValue |= (1<<4);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	}

/*	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue); */

	/* 3. polling BBP_R47 */
	for (wait=0; wait<200; wait++)
	{
		RtmpusecDelay(2000);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

		if (!(BBPValue & (1<<4)))
		{
			break;
		}
	}

	ASSERT(wait < 200);
	ASSERT((BBPValue & 0x10) == 0);
	if ((BBPValue & 0x10) != 0)
		return;

	/* 4. read TSSI */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi0_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[0] = (INT32)BBPR49;  

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x05;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi1_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[1] = (INT32)BBPR49;  

	/* 5. read temperature */	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x04;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* save temperature */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	pAd->CurrTemperature = (INT32)BBPR49; 

	/* 6. estimate the target power */
	/* get packet type */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= (0x01);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* read BBP_R178 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &BBPValue);
	/* read BBP_R4 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	switch ((BBPR49 & 0x03))
	{
		case 0: /* CCK */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x03);
			mcs_power = RT635x_McsPowerOverCCK[mcs];
			pa_mode = RT635x_RfPaModeOverCCK[mcs];

			if (BBPValue == 0) /* BBP_R178 == 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 9831; /* 1.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
				pkt_type_delta = 3 * 8192;
				}
			}
			else /* BBP_R178 != 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 18023; /* 2.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
				pkt_type_delta = 819; /* 0.1 x 8192 */	
				}
			}
			break;
		case 1: /* OFDM */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x0F);
			switch (mcs)
			{
				case 0xb: mcs = 0; break;
				case 0xf: mcs = 1; break;
				case 0xa: mcs = 2; break;
				case 0xe: mcs = 3; break;
				case 0x9: mcs = 4; break;
				case 0xd: mcs = 5; break;
				case 0x8: mcs = 6; break;
				case 0xc: mcs = 7; break;
			}
			mcs_power = RT635x_McsPowerOverOFDM[mcs];
			pa_mode = RT635x_RfPaModeOverOFDM[mcs];
			pkt_type_delta = 0;	
			break;
		default: // TODO:STBC
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
			BBPValue &= (~0x07);
			BBPValue |= (0x02);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
			mcs = (UCHAR)(BBPR49 & 0x7F);
			mcs_power = RT635x_McsPowerOverHT[mcs];
			pa_mode = RT635x_RfPaModeOverHT[mcs];
			pkt_type_delta = 0;	
			break;
	}

#ifdef SPECIFIC_TX_POWER_SUPPORT
	if (((BBPR49 & 0x03) == 0) && (mcs == 0))
		return;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	/* 7. estimate delta power */
	/* read tssi_slope and tssi_offset from efuse */
	RT635xGetTssiInfo(pAd);

	ch_group = GET_2G_CHANNEL_GROUP(channel);		

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
	cur_comp_power = MacValue & 0x3f;
	if(cur_comp_power & 0x20) 
		cur_comp_power = cur_comp_power - 0x40;
	
	for (chain=0; chain < max_stream; chain++)
	{
//	    if ((base_power[chain] + cur_comp_power) >= 0)
		{
			pa_mode_00 =  0;
			pa_mode_01 =  4915; /*  0.6 * 8192 */
			pa_mode_11 = -6554; /* -0.8 * 8192 */
	    }
	pa_mode_used = (pa_mode == 0x00) ? pa_mode_00 :
					(pa_mode == 0x01) ? pa_mode_01 :
					pa_mode_11; /* pa_mode_11 is just default */

		DBGPRINT(RT_DEBUG_TRACE, ("\n============chain %u============\n", chain));

		if (chain == 0)
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi0_dc_hvga;
			else
				tssi_dc = rt635x_tssi0_dc;
		}
		else
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi1_dc_hvga;
			else
				tssi_dc = rt635x_tssi1_dc;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc %x\n", rt635x_tssi0_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc %x\n", rt635x_tssi1_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc_hvga %x\n", rt635x_tssi0_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc_hvga %x\n", rt635x_tssi1_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_dc %x\n", tssi_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_linear %x\n", tssi_read[chain]));
		tssi_m_dc = tssi_read[chain] - tssi_dc;
		tssi_db = RT6352_lin2dBd(tssi_m_dc);

		if (tssi_use_hvga[chain])
		{
			if (chain == 0)
				tssi_db = tssi_db - rt635x_tssi0_db_hvga;
			else
				tssi_db = tssi_db - rt635x_tssi1_db_hvga;
		}

		pwr = (tssi_db*(rt635x_tssi_slope[chain]))
			+ (rt635x_tssi_offset[ch_group][chain] << 9);

		DBGPRINT(RT_DEBUG_TRACE, ("tssi_slope  %x\n", rt635x_tssi_slope[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi offset %x\n", rt635x_tssi_offset[ch_group][chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("\npwr is %d \n", pwr));

		/* read BBP_R1 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPValue);
		switch ((BBPValue & 0x03))
		{
			case 0: bbp_6db_power = 0; break;
			case 1: bbp_6db_power = -49152; break; /* -6dB x 8192 */
			case 2: bbp_6db_power = -98304; break; /* -12dB x 8192 */
			case 3: bbp_6db_power = 49152; break; /* 6dB x 8192 */
		}
		DBGPRINT(RT_DEBUG_TRACE, ("mcs_power      %8d\n", mcs_power));
		DBGPRINT(RT_DEBUG_TRACE, ("pkt_type_delta %8d\n", pkt_type_delta));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode        %8d\n", pa_mode));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode used   %8d\n", pa_mode_used));
 
		target_power = base_power[0] + mcs_power;
		target_power = target_power * 4096;
		target_power = target_power + pa_mode_used + pkt_type_delta
			+ bbp_6db_power + rt635x_ant_pwr_offset[chain];
		DBGPRINT(RT_DEBUG_TRACE, ("target_power %d, pwr %d\n", target_power, pwr));
		pwr_diff[chain] = target_power - pwr;

		if ((tssi_read[chain] > 126) && (pwr_diff[chain] > 0)) /* upper saturation */
			pwr_diff[chain] = 0;
		if (((tssi_read[chain] - tssi_dc) < 1) && (pwr_diff[chain] < 0)) /* lower saturation */
			pwr_diff[chain] = 0;

		pwr_diff_pre = rt635x_pwr_diff_pre[chain];

		if (((pwr_diff_pre ^ pwr_diff[chain]) < 0)
			&& ((pwr_diff[chain] < 4096) && (pwr_diff[chain] > - 4096))
			&& ((pwr_diff_pre < 4096) && (pwr_diff_pre > - 4096)))
		{
			if ((pwr_diff[chain] > 0) && ((pwr_diff[chain] + pwr_diff_pre) >= 0))
				pwr_diff[chain] = 0;
			else if ((pwr_diff[chain] < 0) && ((pwr_diff[chain] + pwr_diff_pre) < 0))
				pwr_diff[chain] = 0;
			else 
				pwr_diff_pre = pwr_diff[chain];
		}
		else
		{
			pwr_diff_pre = pwr_diff[chain];
		}

		rt635x_pwr_diff_pre[chain] = pwr_diff_pre; 

		pwr_diff[chain] = pwr_diff[chain] + ((pwr_diff[chain] > 0) ? 2048 : -2048);
		pwr_diff[chain] = pwr_diff[chain] / 4096;

		if ((rt635x_tssi_slope[chain] > 0xa0) && (rt635x_tssi_slope[chain] < 0x60))
			pwr_diff[chain] = 0;

		/* get previous comp_power from 0x13B4 */
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
		/* chain == 0 ? bit[5:0] : bit[13:8] */
		temp[chain] = MacValue & 0x0000003F;
		DBGPRINT(RT_DEBUG_TRACE, ("temp[%d] is 0x%02x\n", chain, temp[chain]));
		_comp_power = (INT32)SignedExtension6To8(temp[chain]);

		if(chain == 0)
			_comp_power = _comp_power + pwr_diff[chain];
		else
			_comp_power = base_power[chain] + pwr_diff[chain] - pwr_diff[0];

		DBGPRINT(RT_DEBUG_TRACE, ("pwr_diff is %d\n", pwr_diff[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("_comp_power is 0x%08x\n", _comp_power));
		if (chain == 0)
		{
			if (_comp_power > 31)
				_comp_power = 31;
			else if(_comp_power < -31)
				_comp_power = -31;
		}
		else
		{
			if(_comp_power < 0)
				_comp_power = 0;
			else if(_comp_power > 0x3f)
				_comp_power = 0x3f;
		}
		comp_power[chain] = _comp_power; 
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[%d] is 0x%08x\n", chain, comp_power[chain]));
	}

	/* 8. write compensation value back */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);
	
	DBGPRINT(RT_DEBUG_TRACE, ("comp_power[0] is 0x%08x\n", comp_power[0]));
	if (max_stream == 2)
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[1] is 0x%08x\n", comp_power[1]));

		MacValue = MacValue | (comp_power[0] & 0x3f);

	DBGPRINT(RT_DEBUG_TRACE, ("\n================================\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);	

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	MacValue = MacValue & (~0xff00);
	if (max_stream == 2)
	{
		MacValue = MacValue | ((comp_power[1] & 0x3f) << 8);
	}
	else
	{
		MacValue = MacValue | ((base_power[0] & 0x3f) << 8);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);

	return;
}

BOOLEAN RT635xTriggerTssiCompensation(
	IN	PRTMP_ADAPTER pAd,
	IN	CHAR percent_delta)
{
	UINT32 MacValue;
	INT32 base_power[max_ant];
	INT32 tssi_read[max_ant];
	INT32 pa_mode_00, pa_mode_01, pa_mode_11, pa_mode_used;
	INT32 target_power = 0, mcs_power = 0, _comp_power = 0, bbp_6db_power = 0;
	INT32 tssi_dc, tssi_m_dc, tssi_db;
	INT32 pwr, pwr_diff[max_ant], pwr_diff_pre, pkt_type_delta;
	INT32 comp_power[max_ant]; 
	INT32 cur_comp_power;
	UCHAR RFValue, BBPValue, BBPR4, tssi_use_hvga[max_ant];
	UCHAR wait, ch_group, mcs, pa_mode, chain, channel;
	CHAR temp[max_ant];
	CHAR BBPR49;
	UCHAR max_stream = 1;

	pAd->CommonCfg.bEnTemperatureTrack = FALSE;
#ifdef DOT11_N_SUPPORT				
	channel = pAd->CommonCfg.CentralChannel;
#else
	channel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

	/* TSSI compensation */
	RT635xInitMcsPowerTable(pAd);
	RT635xInitRfPaModeTable(pAd);

#ifndef SINGLE_SKU_V2
	/* init base power by e2p target power */
	target_power = (INT32)(pAd->E2p_D0_Value & 0xFF);

	/* reduce target power if not 100% specified from UI */
	target_power += (INT32)(percent_delta);
	target_power = (target_power < 0x0A) ? (0x0A) : target_power;

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);
	if ((MacValue & 0x3f) != target_power)
	{
		MacValue = MacValue & (~0x3F3F);
		MacValue |= target_power;
		MacValue |= (target_power << 8);
		RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);
	}
#endif /* SINGLE_SKU_V2 */
	
#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
	{
		if (pAd->CommonCfg.TxStream == 2)
			max_stream = 2;
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		max_stream = 1; 
	}

#ifdef GREENAP_SUPPORT
	if ((pAd->ApCfg.bGreenAPEnable == TRUE) &&
		(pAd->ApCfg.GreenAPLevel!= GREENAP_11BGN_STAS))
	{
		max_stream = 1; 
	}
#endif /* GREENAP_SUPPORT */

	/* 0. get base power from 0x13B0 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);

	/* bit[5:0] */
	base_power[0] = (MacValue & 0x0000003F);
	/* bit[13:8] */
	base_power[1] = (MacValue & 0x00003F00) >> 8;

	for (chain=0; chain < max_stream; chain++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("\nchain %d: base_power is %d\n", chain, base_power[chain]));

		if (base_power[0] <= 20)
		{
			tssi_use_hvga[chain] = 1;
		}
		else
		{
			tssi_use_hvga[chain] = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("\ntssi_use_hvga %d \n", tssi_use_hvga[chain]));
	}

	/* 1. set TSSI mode */
	if (tssi_use_hvga[0] == 1)
	{
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
	}
	else
	{
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
	}

	/* for 2T2R */
	if (tssi_use_hvga[1] == 1)
	{
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
	}
	else
	{
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= ~((1<<6)|(1<<5));
	BBPValue |= ((1<<6)|(0<<5));
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R58, 0x00);

	/* enable TSSI for next reading */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	
	if (!(BBPValue & (1<<4)))
	{
		BBPValue |= (1<<4);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
	}

	/* 3. polling BBP_R47 */
	for (wait=0; wait < 2; wait++)
	{
		RtmpusecDelay(1000);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

		if (!(BBPValue & (1<<4)))
		{
			break;
		}
	}

	if ((BBPValue & 0x10) != 0)
	{
		pAd->CommonCfg.bEnTemperatureTrack = TRUE;
		return FALSE;
	}

	/* 4. read TSSI */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi0_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[0] = (INT32)BBPR49;  

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x05;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi1_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[1] = (INT32)BBPR49;  

	/* 5. read temperature */	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x04;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* save temperature */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	pAd->CurrTemperature = (INT32)BBPR49; 

	/* 6. estimate the target power */
	/* get packet type */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= (0x01);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* read BBP_R178 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &BBPValue);
	/* read BBP_R4 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	switch ((BBPR49 & 0x03))
	{
		case 0: /* CCK */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x03);
			mcs_power = RT635x_McsPowerOverCCK[mcs];
			pa_mode = RT635x_RfPaModeOverCCK[mcs];

			if (BBPValue == 0) /* BBP_R178 == 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 9831; /* 1.2 x 8192 */ 
				}
				else /* BBP_R4[5] == 0 */
				{
					pkt_type_delta = 3 * 8192;
				}
			}
			else /* BBP_R178 != 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 18023; /* 2.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
					pkt_type_delta = 819; /* 0.1 x 8192 */	
				}
			}
			break;

		case 1: /* OFDM */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x0F);
			switch (mcs)
			{
				case 0xb: mcs = 0; break;
				case 0xf: mcs = 1; break;
				case 0xa: mcs = 2; break;
				case 0xe: mcs = 3; break;
				case 0x9: mcs = 4; break;
				case 0xd: mcs = 5; break;
				case 0x8: mcs = 6; break;
				case 0xc: mcs = 7; break;
			}
			mcs_power = RT635x_McsPowerOverOFDM[mcs];
			pa_mode = RT635x_RfPaModeOverOFDM[mcs];
			pkt_type_delta = 0; 
			break;

		default: // TODO:STBC
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
			BBPValue &= (~0x07);
			BBPValue |= (0x02);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
			mcs = (UCHAR)(BBPR49 & 0x7F);
			mcs_power = RT635x_McsPowerOverHT[mcs];
			pa_mode = RT635x_RfPaModeOverHT[mcs];
			pkt_type_delta = 0; 
			break;
	}

#ifdef SPECIFIC_TX_POWER_SUPPORT
	if (((BBPR49 & 0x03) == 0) && (mcs == 0))
		return TRUE;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	
	/* 7. estimate delta power */
	/* read tssi_slope and tssi_offset from efuse */
	RT635xGetTssiInfo(pAd);

	ch_group = GET_2G_CHANNEL_GROUP(channel);		

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
	cur_comp_power = MacValue & 0x3f;
	if(cur_comp_power & 0x20) 
		cur_comp_power = cur_comp_power - 0x40;

	for (chain=0; chain < max_stream; chain++)
	{
		pa_mode_00 =  0;
		pa_mode_01 =  4915; /*	0.6 * 8192 */
		pa_mode_11 = -6554; /* -0.8 * 8192 */

		pa_mode_used = (pa_mode == 0x00) ? pa_mode_00 :
						(pa_mode == 0x01) ? pa_mode_01 :
						pa_mode_11; /* pa_mode_11 is just default */
	
		DBGPRINT(RT_DEBUG_TRACE, ("\n============chain %u============\n", chain));
	
		if (chain == 0)
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi0_dc_hvga;
			else
				tssi_dc = rt635x_tssi0_dc;
		}
		else
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi1_dc_hvga;
			else
				tssi_dc = rt635x_tssi1_dc;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc %x\n", rt635x_tssi0_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc %x\n", rt635x_tssi1_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc_hvga %x\n", rt635x_tssi0_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc_hvga %x\n", rt635x_tssi1_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_dc %x\n", tssi_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_linear %x\n", tssi_read[chain]));
		tssi_m_dc = tssi_read[chain] - tssi_dc;
		tssi_db = RT6352_lin2dBd(tssi_m_dc);

		if (tssi_use_hvga[chain])
		{
			if (chain == 0)
				tssi_db = tssi_db - rt635x_tssi0_db_hvga;
			else
				tssi_db = tssi_db - rt635x_tssi1_db_hvga;
		}

		pwr = (tssi_db*(rt635x_tssi_slope[chain]))
				+ (rt635x_tssi_offset[ch_group][chain] << 9);
	
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_slope  %x\n", rt635x_tssi_slope[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi offset %x\n", rt635x_tssi_offset[ch_group][chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("\npwr is %d \n", pwr));

		/* read BBP_R1 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPValue);
		switch ((BBPValue & 0x03))
		{
			case 0: bbp_6db_power = 0; break;
			case 1: bbp_6db_power = -49152; break; /* -6dB x 8192 */
			case 2: bbp_6db_power = -98304; break; /* -12dB x 8192 */
			case 3: bbp_6db_power = 49152; break; /* 6dB x 8192 */
		}
		DBGPRINT(RT_DEBUG_TRACE, ("mcs_power	  %8d\n", mcs_power));
		DBGPRINT(RT_DEBUG_TRACE, ("pkt_type_delta %8d\n", pkt_type_delta));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode		  %8d\n", pa_mode));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode used   %8d\n", pa_mode_used));
 
		target_power = base_power[0] + mcs_power;
		target_power = target_power * 4096;
		target_power = target_power + pa_mode_used + pkt_type_delta
						+ bbp_6db_power + rt635x_ant_pwr_offset[chain];
		DBGPRINT(RT_DEBUG_TRACE, ("target_power %d, pwr %d\n", target_power, pwr));
		pwr_diff[chain] = target_power - pwr;

		if ((tssi_read[chain] > 126) && (pwr_diff[chain] > 0)) /* upper saturation */
			pwr_diff[chain] = 0;

		if (((tssi_read[chain] - tssi_dc) < 1) && (pwr_diff[chain] < 0)) /* lower saturation */
			pwr_diff[chain] = 0;
	
		pwr_diff_pre = rt635x_pwr_diff_pre[chain];

		if (((pwr_diff_pre ^ pwr_diff[chain]) < 0)
			&& ((pwr_diff[chain] < 4096) && (pwr_diff[chain] > - 4096))
			&& ((pwr_diff_pre < 4096) && (pwr_diff_pre > - 4096)))
		{
			if ((pwr_diff[chain] > 0) && ((pwr_diff[chain] + pwr_diff_pre) >= 0))
				pwr_diff[chain] = 0;
			else if ((pwr_diff[chain] < 0) && ((pwr_diff[chain] + pwr_diff_pre) < 0))
				pwr_diff[chain] = 0;
			else 
				pwr_diff_pre = pwr_diff[chain];
		}
		else
		{
			pwr_diff_pre = pwr_diff[chain];
		}

		rt635x_pwr_diff_pre[chain] = pwr_diff_pre; 

		pwr_diff[chain] = pwr_diff[chain] + ((pwr_diff[chain] > 0) ? 2048 : -2048);
		pwr_diff[chain] = pwr_diff[chain] / 4096;

		if ((rt635x_tssi_slope[chain] > 0xa0) && (rt635x_tssi_slope[chain] < 0x60))
			pwr_diff[chain] = 0;

		/* get previous comp_power from 0x13B4 */
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
		/* chain == 0 ? bit[5:0] : bit[13:8] */
		temp[chain] = MacValue & 0x0000003F;
		DBGPRINT(RT_DEBUG_TRACE, ("temp[%d] is 0x%02x\n", chain, temp[chain]));
		_comp_power = (INT32)SignedExtension6To8(temp[chain]);

		if(chain == 0)
			_comp_power = _comp_power + pwr_diff[chain];
		else
			_comp_power = base_power[chain] + pwr_diff[chain] - pwr_diff[0];

		DBGPRINT(RT_DEBUG_TRACE, ("pwr_diff is %d\n", pwr_diff[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("_comp_power is 0x%08x\n", _comp_power));
		if (chain == 0)
		{
			if (_comp_power > 31)
				_comp_power = 31;
			else if(_comp_power < -31)
				_comp_power = -31;
		}
		else
		{
			if(_comp_power < 0)
				_comp_power = 0;
			else if(_comp_power > 0x3f)
				_comp_power = 0x3f;
		}
		comp_power[chain] = _comp_power; 
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[%d] is 0x%08x\n", chain, comp_power[chain]));
	}

	/* 8. write compensation value back */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);

	DBGPRINT(RT_DEBUG_TRACE, ("comp_power[0] is 0x%08x\n", comp_power[0]));
	if (max_stream == 2)
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[1] is 0x%08x\n", comp_power[1]));

	MacValue = MacValue | (comp_power[0] & 0x3f);

	DBGPRINT(RT_DEBUG_TRACE, ("\n================================\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);	

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	MacValue = MacValue & (~0xff00);
	if (max_stream == 2)
	{
		MacValue = MacValue | ((comp_power[1] & 0x3f) << 8);
	}
	else
	{
		MacValue = MacValue | ((base_power[0] & 0x3f) << 8);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);

	return TRUE;
}

BOOLEAN RT635xCheckTssiCompensation(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32 MacValue;
	INT32 base_power[max_ant];
	INT32 tssi_read[max_ant];
	INT32 pa_mode_00, pa_mode_01, pa_mode_11, pa_mode_used;
	INT32 target_power = 0, mcs_power = 0, _comp_power = 0, bbp_6db_power = 0;
	INT32 tssi_dc, tssi_m_dc, tssi_db;
	INT32 pwr, pwr_diff[max_ant], pwr_diff_pre, pkt_type_delta;
	INT32 comp_power[max_ant]; 
	INT32 cur_comp_power;
	UCHAR BBPValue, BBPR4, tssi_use_hvga[max_ant];
	UCHAR ch_group, mcs, pa_mode, chain, channel;
	CHAR temp[max_ant];
	CHAR BBPR49;
	UCHAR max_stream = 1;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	if ((BBPValue & 0x10) != 0)
		return FALSE;

	pAd->CommonCfg.bEnTemperatureTrack = FALSE;

#ifdef DOT11_N_SUPPORT				
	channel = pAd->CommonCfg.CentralChannel;
#else
	channel = pAd->CommonCfg.Channel;
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->Antenna.field.TxPath == 2))
	{
		if (pAd->CommonCfg.TxStream == 2)
			max_stream = 2;
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		max_stream = 1; 
	}

#ifdef GREENAP_SUPPORT
	if ((pAd->ApCfg.bGreenAPEnable == TRUE) &&
		(pAd->ApCfg.GreenAPLevel!= GREENAP_11BGN_STAS))
	{
		max_stream = 1; 
	}
#endif /* GREENAP_SUPPORT */

	/* 0. get base power from 0x13B0 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);

	/* bit[5:0] */
	base_power[0] = (MacValue & 0x0000003F);
	/* bit[13:8] */
	base_power[1] = (MacValue & 0x00003F00) >> 8;

	for (chain=0; chain < max_stream; chain++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("\nchain %d: base_power is %d\n", chain, base_power[chain]));

		if (base_power[0] <= 20)
		{
			tssi_use_hvga[chain] = 1;
		}
		else
		{
			tssi_use_hvga[chain] = 0;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("\ntssi_use_hvga %d \n", tssi_use_hvga[chain]));
	}

	/* 4. read TSSI */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi0_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[0] = (INT32)BBPR49;  

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x05;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi1_linear */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[1] = (INT32)BBPR49;  

	/* 5. read temperature */	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x04;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* save temperature */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	pAd->CurrTemperature = (INT32)BBPR49; 

	/* 6. estimate the target power */
	/* get packet type */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= (0x01);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* read BBP_R178 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &BBPValue);
	/* read BBP_R4 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	switch ((BBPR49 & 0x03))
	{
		case 0: /* CCK */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x03);
			mcs_power = RT635x_McsPowerOverCCK[mcs];
			pa_mode = RT635x_RfPaModeOverCCK[mcs];

			if (BBPValue == 0) /* BBP_R178 == 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 9831; /* 1.2 x 8192 */ 
				}
				else /* BBP_R4[5] == 0 */
				{
					pkt_type_delta = 3 * 8192;
				}
			}
			else /* BBP_R178 != 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 18023; /* 2.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
					pkt_type_delta = 819; /* 0.1 x 8192 */	
				}
			}
			break;

		case 1: /* OFDM */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x0F);
			switch (mcs)
			{
				case 0xb: mcs = 0; break;
				case 0xf: mcs = 1; break;
				case 0xa: mcs = 2; break;
				case 0xe: mcs = 3; break;
				case 0x9: mcs = 4; break;
				case 0xd: mcs = 5; break;
				case 0x8: mcs = 6; break;
				case 0xc: mcs = 7; break;
			}
			mcs_power = RT635x_McsPowerOverOFDM[mcs];
			pa_mode = RT635x_RfPaModeOverOFDM[mcs];
			pkt_type_delta = 0; 
			break;

		default: // TODO:STBC
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
			BBPValue &= (~0x07);
			BBPValue |= (0x02);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
			mcs = (UCHAR)(BBPR49 & 0x7F);
			mcs_power = RT635x_McsPowerOverHT[mcs];
			pa_mode = RT635x_RfPaModeOverHT[mcs];
			pkt_type_delta = 0; 
			break;
	}

#ifdef SPECIFIC_TX_POWER_SUPPORT
	if (((BBPR49 & 0x03) == 0) && (mcs == 0))
		return TRUE;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	/* 7. estimate delta power */
	/* read tssi_slope and tssi_offset from efuse */
	RT635xGetTssiInfo(pAd);

	ch_group = GET_2G_CHANNEL_GROUP(channel);		

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
	cur_comp_power = MacValue & 0x3f;
	if(cur_comp_power & 0x20) 
		cur_comp_power = cur_comp_power - 0x40;

	for (chain=0; chain < max_stream; chain++)
	{
		pa_mode_00 =  0;
		pa_mode_01 =  4915; /*	0.6 * 8192 */
		pa_mode_11 = -6554; /* -0.8 * 8192 */

		pa_mode_used = (pa_mode == 0x00) ? pa_mode_00 :
						(pa_mode == 0x01) ? pa_mode_01 :
						pa_mode_11; /* pa_mode_11 is just default */

		DBGPRINT(RT_DEBUG_TRACE, ("\n============chain %u============\n", chain));

		if (chain == 0)
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi0_dc_hvga;
			else
				tssi_dc = rt635x_tssi0_dc;
		}
		else
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi1_dc_hvga;
			else
				tssi_dc = rt635x_tssi1_dc;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc %x\n", rt635x_tssi0_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc %x\n", rt635x_tssi1_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi0_dc_hvga %x\n", rt635x_tssi0_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("rt635x_tssi1_dc_hvga %x\n", rt635x_tssi1_dc_hvga));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_dc %x\n", tssi_dc));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_linear %x\n", tssi_read[chain]));
		tssi_m_dc = tssi_read[chain] - tssi_dc;
		tssi_db = RT6352_lin2dBd(tssi_m_dc);

		if (tssi_use_hvga[chain])
		{
			if (chain == 0)
				tssi_db = tssi_db - rt635x_tssi0_db_hvga;
			else
				tssi_db = tssi_db - rt635x_tssi1_db_hvga;
		}

		pwr = (tssi_db*(rt635x_tssi_slope[chain]))
				+ (rt635x_tssi_offset[ch_group][chain] << 9);

		DBGPRINT(RT_DEBUG_TRACE, ("tssi_slope  %x\n", rt635x_tssi_slope[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi offset %x\n", rt635x_tssi_offset[ch_group][chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("\npwr is %d \n", pwr));

		/* read BBP_R1 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPValue);
		switch ((BBPValue & 0x03))
		{
			case 0: bbp_6db_power = 0; break;
			case 1: bbp_6db_power = -49152; break; /* -6dB x 8192 */
			case 2: bbp_6db_power = -98304; break; /* -12dB x 8192 */
			case 3: bbp_6db_power = 49152; break; /* 6dB x 8192 */
		}
		DBGPRINT(RT_DEBUG_TRACE, ("mcs_power	  %8d\n", mcs_power));
		DBGPRINT(RT_DEBUG_TRACE, ("pkt_type_delta %8d\n", pkt_type_delta));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode		  %8d\n", pa_mode));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode used   %8d\n", pa_mode_used));

		target_power = base_power[0] + mcs_power;
		target_power = target_power * 4096;
		target_power = target_power + pa_mode_used + pkt_type_delta
						+ bbp_6db_power + rt635x_ant_pwr_offset[chain];
		DBGPRINT(RT_DEBUG_TRACE, ("target_power %d, pwr %d\n", target_power, pwr));
		pwr_diff[chain] = target_power - pwr;

		if ((tssi_read[chain] > 126) && (pwr_diff[chain] > 0)) /* upper saturation */
			pwr_diff[chain] = 0;

		if (((tssi_read[chain] - tssi_dc) < 1) && (pwr_diff[chain] < 0)) /* lower saturation */
			pwr_diff[chain] = 0;

		pwr_diff_pre = rt635x_pwr_diff_pre[chain];

		if (((pwr_diff_pre ^ pwr_diff[chain]) < 0)
			&& ((pwr_diff[chain] < 4096) && (pwr_diff[chain] > - 4096))
			&& ((pwr_diff_pre < 4096) && (pwr_diff_pre > - 4096)))
		{
			if ((pwr_diff[chain] > 0) && ((pwr_diff[chain] + pwr_diff_pre) >= 0))
				pwr_diff[chain] = 0;
			else if ((pwr_diff[chain] < 0) && ((pwr_diff[chain] + pwr_diff_pre) < 0))
				pwr_diff[chain] = 0;
			else 
				pwr_diff_pre = pwr_diff[chain];
		}
		else
		{
			pwr_diff_pre = pwr_diff[chain];
		}

		rt635x_pwr_diff_pre[chain] = pwr_diff_pre; 

		pwr_diff[chain] = pwr_diff[chain] + ((pwr_diff[chain] > 0) ? 2048 : -2048);
		pwr_diff[chain] = pwr_diff[chain] / 4096;

		if ((rt635x_tssi_slope[chain] > 0xa0) && (rt635x_tssi_slope[chain] < 0x60))
			pwr_diff[chain] = 0;

		/* get previous comp_power from 0x13B4 */
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
		/* chain == 0 ? bit[5:0] : bit[13:8] */
		temp[chain] = MacValue & 0x0000003F;
		DBGPRINT(RT_DEBUG_TRACE, ("temp[%d] is 0x%02x\n", chain, temp[chain]));
		_comp_power = (INT32)SignedExtension6To8(temp[chain]);

		if(chain == 0)
			_comp_power = _comp_power + pwr_diff[chain];
		else
			_comp_power = base_power[chain] + pwr_diff[chain] - pwr_diff[0];
	
		DBGPRINT(RT_DEBUG_TRACE, ("pwr_diff is %d\n", pwr_diff[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("_comp_power is 0x%08x\n", _comp_power));
		if (chain == 0)
		{
			if (_comp_power > 31)
				_comp_power = 31;
			else if(_comp_power < -31)
				_comp_power = -31;
		}
		else
		{
			if(_comp_power < 0)
				_comp_power = 0;
			else if(_comp_power > 0x3f)
				_comp_power = 0x3f;
		}
		comp_power[chain] = _comp_power; 
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[%d] is 0x%08x\n", chain, comp_power[chain]));
	}

	/* 8. write compensation value back */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);

	DBGPRINT(RT_DEBUG_TRACE, ("comp_power[0] is 0x%08x\n", comp_power[0]));
	if (max_stream == 2)
		DBGPRINT(RT_DEBUG_TRACE, ("comp_power[1] is 0x%08x\n", comp_power[1]));

	MacValue = MacValue | (comp_power[0] & 0x3f);
	
	DBGPRINT(RT_DEBUG_TRACE, ("\n================================\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);	

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	MacValue = MacValue & (~0xff00);
	if (max_stream == 2)
	{
		MacValue = MacValue | ((comp_power[1] & 0x3f) << 8);
	}
	else
	{
		MacValue = MacValue | ((base_power[0] & 0x3f) << 8);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);

	return TRUE;
}

#endif /* RTMP_INTERNAL_TX_ALC */

/*
	========================================================================
	
	Routine Description:
		Read initial Tx power per MCS and BW from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID RT6352_RTMPReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd)
{
	ULONG		data, Gdata;
	USHORT		i, value, value2;
	USHORT		value_1, value_3;
	INT			Gpwrdelta;
	USHORT		t1,t3;
	BOOLEAN		bGpwrdeltaMinus = TRUE;
	CHAR		value_1_plus, value_3_plus;

	/* Get power delta for 20MHz and 40MHz.*/
	
	DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);

	Gpwrdelta = 0;
	DBGPRINT(RT_DEBUG_TRACE, ("Gpwrdelta = %x .\n", Gpwrdelta));
		
	/* Get Txpower per MCS for 20MHz in 2.4G.*/
	
	for (i=0; i<5; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
		data = value;

		/* use value_1 ~ value_4 for code size reduce */
		value_1 = value&0x3f;
		value_3 = (value&0x3f00)>>8;

		if (value_1 > 31)
			value_1_plus = value_1 - 64;
		else
			value_1_plus = value_1;

		if (value_3 > 31)
			value_3_plus = value_3 - 64;
		else
			value_3_plus = value_3;

		if (bGpwrdeltaMinus == FALSE)
		{
			value_1_plus = value_1_plus + (Gpwrdelta);

			if (value_1_plus < 0)
				t1 = value_1_plus + 64;
			else
			{
				t1 = value_1_plus;

				if (t1 > 0x1f)
					t1 = 0x1f;
			}

			if (t1 > 0x3f)
			{
				t1 = 0x3f;
			}

			value_3_plus = value_3_plus + (Gpwrdelta);

			if (value_3_plus < 0)
				t3 = value_3_plus + 64;
			else
			{
				t3 = value_3_plus;

				if (t3 > 0x1f)
					t3 = 0x1f;
			}

			if (t3 > 0x3f)
			{
				t3 = 0x3f;
			}
		}
		else
		{
			if ((value_1_plus - Gpwrdelta) <= -31)
				t1 = 0x21;
			else
			{
				value_1_plus = (value_1_plus - Gpwrdelta);

				if (value_1_plus < 0)
					t1 = value_1_plus + 64;
				else
					t1 = value_1_plus;
			}

			if ((value_3_plus - Gpwrdelta) <= -31)
				t3 = 0x21;
			else
			{
				value_3_plus = (value_3_plus - Gpwrdelta);

				if (value_3_plus < 0)
					t3 = value_3_plus + 64;
				else
					t3 = value_3_plus;
			}
		}				
		Gdata = t1 + (t3<<8);
	
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);

		/* use value_1 ~ value_4 for code size reduce */
		value_1 = value&0x3f;
		value_3 = (value&0x3f00)>>8;

		if (value_1 > 31)
			value_1_plus = value_1 - 64;
		else
			value_1_plus = value_1;

		if (value_3 > 31)
			value_3_plus = value_3 - 64;
		else
			value_3_plus = value_3;

		if (bGpwrdeltaMinus == FALSE)
		{
			value_1_plus = value_1_plus + (Gpwrdelta);

			if (value_1_plus < 0)
				t1 = value_1_plus + 64;
			else
			{
				t1 = value_1_plus;

				if (t1 > 0x1f)
					t1 = 0x1f;
			}

			if (t1 > 0x3f)
			{
				t1 = 0x3f;
			}

			value_3_plus = value_3_plus + (Gpwrdelta);

			if (value_3_plus < 0)
				t3 = value_3_plus + 64;
			else
			{
				t3 = value_3_plus;

				if (t3 > 0x1f)
					t3 = 0x1f;
			}

			if (t3 > 0x3f)
			{
				t3 = 0x3f;
			}
		}
		else
		{
			if ((value_1_plus - Gpwrdelta) <= -31)
				t1 = 0x21;
			else
			{
				value_1_plus = (value_1_plus - Gpwrdelta);
				if (value_1_plus < 0)
					t1 = value_1_plus + 64;
				else
					t1 = value_1_plus;
			}

			if ((value_3_plus - Gpwrdelta) <= -31)
				t3 = 0x21;
			else
			{
				value_3_plus = (value_3_plus - Gpwrdelta);
				if (value_3_plus < 0)
					t3 = value_3_plus + 64;
				else
					t3 = value_3_plus;
			}
		}			
		Gdata |= ((t1<<16) + (t3<<24));
		data |= (value<<16);

		/* For 20M/40M Power Delta issue */		
		pAd->Tx20MPwrCfgGBand[i] = data;
		pAd->Tx40MPwrCfgGBand[i] = Gdata;
	
		if (data != 0xffffffff)
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);

		DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx,  Gdata = %lx \n", data, Gdata));
	}

	/* Extra set MAC registers to compensate Tx power if any */
	RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
}

static VOID RT6352_AsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);

	/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);
		
	DBGPRINT(RT_DEBUG_INFO, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
	
	DBGPRINT(RT_DEBUG_INFO, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}

#ifdef SINGLE_SKU_V2
#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17

VOID RT6352_InitSkuRateDiffTable(
	IN PRTMP_ADAPTER 	pAd )
{
	USHORT		i, value;
	CHAR		BasePwr, Pwr;

	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4, value);
	BasePwr = (value >> 8) & 0xFF;
	BasePwr = (BasePwr > 0x1F ) ? BasePwr - 0x40: BasePwr;

	for ( i = 0 ; i < 9; i++ )
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*2, value);
		Pwr = value & 0xFF ;
		Pwr = (Pwr > 0x1F ) ? Pwr - 0x40: Pwr;
		pAd->SingleSkuRatePwrDiff[i *2] = Pwr - BasePwr;
		Pwr = (value >> 8) & 0xFF;
		Pwr = (Pwr > 0x1F ) ? Pwr - 0x40: Pwr;
		pAd->SingleSkuRatePwrDiff[i *2 + 1] = Pwr - BasePwr;
	}
}

UCHAR RT6352_GetSkuChannelBasePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR 			channel)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	UCHAR base_pwr = pAd->DefaultTargetPwr;
	UINT8 i, j;
	CHAR tx_pwr1;
	CHAR max_tx1_pwr;
	UINT16 TargetPwr = 0;
	UINT32 MacReg;

	if (IS_RT6352(pAd))
	{
		RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacReg);
		max_tx1_pwr = (MacReg >> 16) & 0x3F;

		tx_pwr1 = (pAd->E2p_D0_Value & 0x3F);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: EEPROM 0xD0 = 0x%x\n", __FUNCTION__, tx_pwr1));

		if ((tx_pwr1 == 0x0) || (tx_pwr1 > max_tx1_pwr) )
		{
			tx_pwr1 = 0x1E;
			DBGPRINT(RT_DEBUG_ERROR, ("%s: EEPROM 0xD0 Error! Use Default Target Power = 0x%x\n", __FUNCTION__, tx_pwr1));
		}

		base_pwr = tx_pwr1;
	}
	
	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;

		if ( channel >= start_ch )
		{
			for ( j = 0; j < ch->num; j++ )
			{
				if ( channel == ch->Channel[j] )
				{
					for ( i= 0 ; i < SINGLE_SKU_TABLE_CCK_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrCCK[i] )
							base_pwr = ch->PwrCCK[i];
					}

					for ( i= 0 ; i < SINGLE_SKU_TABLE_OFDM_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrOFDM[i] )
							base_pwr = ch->PwrOFDM[i];
					}

					for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
					{
						if ( base_pwr > ch->PwrHT20[i] )
							base_pwr = ch->PwrHT20[i];
					}
						
					if (pAd->CommonCfg.BBPCurrentBW == BW_40)
					{
						for ( i= 0 ; i < SINGLE_SKU_TABLE_HT_LENGTH ; i++ )
						{
							if ( ch->PwrHT40[i] == 0 )
								break;

							if ( base_pwr > ch->PwrHT40[i] )
								base_pwr = ch->PwrHT40[i];
						}
					}
					break;
				}
			}
		}
	}

	return base_pwr;

}

CHAR RT6352_AdjustChannelPwr(
	IN RTMP_ADAPTER *pAd,
	IN CHAR TotalDeltaPower,
	OUT CHAR *PreDiff)
{
	CHAR target_power;
	UCHAR sku_min_pwr = 0;
	UINT32 mac_value;
	USHORT e2p_data;
	
	//RT28xx_EEPROM_READ16(pAd, 0xD0, e2p_data);
	target_power = (pAd->E2p_D0_Value & 0x3F);
	sku_min_pwr = RT6352_GetSkuChannelBasePwr(pAd, pAd->CommonCfg.SkuChannel);

	/* Get delta power based on the percentage specified from UI */
	//target_power += TotalDeltaPower;
	
#ifdef RTMP_INTERNAL_TX_ALC	
	/* TSSI mode */
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		CHAR bw_delta = 0;
		CHAR target_power_ori = 0;
		target_power = (target_power > sku_min_pwr) ? sku_min_pwr : target_power;

		target_power_ori = target_power;
		target_power += TotalDeltaPower;

		if (target_power < 0)
			target_power = 0;
		if(pAd->bSingleSkuDebug)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s::channel_power =%d, TotalDeltaPower=%d, sku_min_pwr=%d\n", 
				__FUNCTION__, target_power, TotalDeltaPower, sku_min_pwr));	
		}
		
		RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &mac_value);
		if ((mac_value & 0x3f) != target_power)
		{
			mac_value &= 0xFFFFC0C0;
			mac_value |= (target_power & 0x3F);
			mac_value |= (target_power & 0x3F) << 8;
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, mac_value);	
		}
		if(pAd->bSingleSkuDebug)
			DBGPRINT(RT_DEBUG_TRACE, ("%s::mac 0x13B0 is 0x%08x\n", __FUNCTION__, mac_value));	
		
		target_power = target_power_ori;
	}
	else /* Manual/Temperature mode */
#endif /* RTMP_INTERNAL_TX_ALC */
	{
		INT32 diff = 0;

		if(pAd->bSingleSkuDebug)
			DBGPRINT(RT_DEBUG_TRACE, ("target_power=%d, sku_min_pwr=%d\n", target_power, sku_min_pwr));	

		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &mac_value);
		mac_value &= 0xFFFFFFC0;

		diff = sku_min_pwr - target_power;

		diff += TotalDeltaPower;
		if (target_power > sku_min_pwr)
		    target_power = sku_min_pwr;

		if (diff > 31)
			diff = 31;
		else if(diff < -31)
			diff = -31;

		mac_value |= (diff & 0x3F);
#ifdef RTMP_TEMPERATURE_COMPENSATION
		if (pAd->bAutoTxAgcG)
		{
			*PreDiff = (CHAR)diff;

			DBGPRINT(RT_DEBUG_TRACE, ("%s::diff is 0x%x\n", __FUNCTION__, *PreDiff));
		}
		else
#endif /* RTMP_TEMPERATURE_COMPENSATION */
		{
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, mac_value);
			if(pAd->bSingleSkuDebug)
				DBGPRINT(RT_DEBUG_TRACE, ("%s::mac 0x13B4 is 0x%08x\n", __FUNCTION__, mac_value));
		}
	}

	if (target_power > 0)
		return target_power;
	else
		return 0;
}

VOID RT6352_AdjustPerRatePwr(
	IN PRTMP_ADAPTER 	pAd,
	IN CHAR 				channelpower)
{
	INT i, j;
	INT32 channel_power = channelpower;
	CHAR bbp_reg, bbp_r4;
	INT32 mcs_digpwr_diff;
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC mcs_pwr_table = {0};	

	/* Get mcs_pwr_table from EEPROM 0xDEh ~ 0xEFh, unit 0.5 dBm */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&mcs_pwr_table);

	/* Calculate mcs_digpwr_diff from MCS digital power diff table */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &bbp_reg);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &bbp_r4);

	if (bbp_reg == 0) /* BBP_R178 == 0 */
	{
		if (bbp_r4 & (1 << 5)) /* BBP_R4[5] == 1 */
		{
			mcs_digpwr_diff = 9831; /* 1.2 x 8192 */	
		}
		else /* BBP_R4[5] == 0 */
		{
			mcs_digpwr_diff = 3 * 8192;
		}
	}
	else /* BBP_R178 != 0 */
	{
		if (bbp_r4 & (1 << 5)) /* BBP_R4[5] == 1 */
		{
			mcs_digpwr_diff = 18023; /* 2.2 x 8192 */	
		}
		else /* BBP_R4[5] == 0 */
		{
		mcs_digpwr_diff = 819; /* 0.1 x 8192 */
		}
	}

	if(pAd->bSingleSkuDebug)
		DBGPRINT(RT_DEBUG_TRACE, ("%s::mcs_digpwr_diff is %d\n", __FUNCTION__, mcs_digpwr_diff));

	for (i = 0; i < mcs_pwr_table.NumOfEntries; i++)
	{
		TX_POWER_CONTROL_OVER_MAC_ENTRY *per_rate_entry;
		ULONG reg_val;

		per_rate_entry = &mcs_pwr_table.TxPwrCtrlOverMAC[i];
		reg_val = per_rate_entry->RegisterValue;

		if (reg_val != 0xFFFFFFFF)
		{	
			for (j = 0; j < 8; j += 2)
			{
				CHAR diff = 0, mcs_pwr_value;
				INT32 rate_pwr = 0, rate_pwr_ori = 0, sku_rate_pwr, pa_mode = 0; 
				
				rate_pwr_ori = rate_pwr = (INT32)((reg_val >> j*4) & 0x000000FF);
				rate_pwr = ((rate_pwr + channel_power) << 12);

				/* Append mcs_digpwr_diff and PA_MODE to the corresponding rate power */
				switch (0x1314 + (i * 4))
				{
					case 0x1314: 						
						rate_pwr += ((j == 0) || (j == 2)) ? mcs_digpwr_diff : 0;
						pa_mode = ((j < 4)) ? 
							pAd->rf_pa_mode_over_cck[j] : pAd->rf_pa_mode_over_ofdm[j - 4];
					break;

					case 0x1318: 
						pa_mode = ((j < 4)) ? 
							pAd->rf_pa_mode_over_ofdm[j + 4] : pAd->rf_pa_mode_over_ht[j - 4];
					break;

					case 0x131C: 
						pa_mode = pAd->rf_pa_mode_over_ht[j + 4];
					break;

					case 0x1320: 
						pa_mode = ((j < 4)) ? pAd->rf_pa_mode_over_ht[j + 12] : 0;
					break;

					case 0x1324: 
					break;

					default: 
						/* Do nothing */
						DBGPRINT(RT_DEBUG_ERROR, ("%s::unknown register = 0x%x\n", __FUNCTION__, (0x1314 + (i * 4))));
					break;
				}

				rate_pwr = (rate_pwr + pa_mode_table[pa_mode]) >> 12;

				sku_rate_pwr = (INT32)GetSkuPerRatePwr(pAd, (i * 4) + (j / 2), pAd->CommonCfg.SkuChannel, pAd->CommonCfg.BBPCurrentBW, pa_mode_table[pa_mode]);
				/* Adjust the per rate power if enabling Single-SKU */
				diff = (CHAR)(sku_rate_pwr - rate_pwr);
				
				mcs_pwr_value = (CHAR)rate_pwr_ori + diff;
				if(pAd->bSingleSkuDebug)
				{
					DBGPRINT(RT_DEBUG_TRACE, 
						("[%d, %d] channel_power(%d) + mcs_pwr_table(%d) + pa_mode(%d, %d) = rate_pwr(%d)\n",
							i, j, channel_power, mcs_pwr_value, pa_mode, pa_mode_table[pa_mode], rate_pwr));

					DBGPRINT(RT_DEBUG_TRACE, 
						("[%d, %d] sku_rate_pwr(%d) - rate_pwr(%d) = diff(%d). rate_pwr_ori(%d)\n",
							i, j, sku_rate_pwr, rate_pwr, diff, rate_pwr_ori));
				}
				/* 8-bit representation ==> 6-bit representation (2's complement) */
				if ((mcs_pwr_value & 0x80) == 0x00) /* Positive number */
					mcs_pwr_value &= 0x3F;
				else /* 0x80: Negative number */
					mcs_pwr_value = (mcs_pwr_value & 0x1F) | 0x20;
				
				/* Write mcs_pwr_table to the corresponding MAC */
				if ((j % 2) == 0)
					per_rate_entry->RegisterValue = (per_rate_entry->RegisterValue & ~(0x000000FF << (j * 4))) | (mcs_pwr_value << (j * 4));
			}

			RTMP_IO_WRITE32(pAd, per_rate_entry->MACRegisterOffset, per_rate_entry->RegisterValue);

		}
	}

	/* Extra set MAC registers to compensate Tx power if any */
	RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
}

UCHAR GetSkuPerRatePwr(
	IN PRTMP_ADAPTER pAd,
	IN CHAR phymode,
	IN UCHAR channel,
	IN UCHAR bw,
	IN INT32 paValue)
{
	INT i = 0;
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch, end_ch;
	UCHAR rate_pwr, rate_pwr1;
	CHAR tx_pwr1;
	CHAR max_tx1_pwr;
	UINT16 TargetPwr = 0;
	UINT32 MacReg;
	INT32 pwr_diff = 0;

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacReg);
	max_tx1_pwr = (MacReg >> 16) & 0x3F;

	tx_pwr1 = (pAd->E2p_D0_Value & 0x3F);
	
	if(pAd->bSingleSkuDebug)
		DBGPRINT(RT_DEBUG_TRACE, ("%s: EEPROM 0xD0 = 0x%x\n", __FUNCTION__, tx_pwr1));

	if ((tx_pwr1 == 0x0) || (tx_pwr1 > max_tx1_pwr) )
	{
		tx_pwr1 = 0x1E;
		DBGPRINT(RT_DEBUG_ERROR, ("%s: EEPROM 0xD0 Error! Use Default Target Power = 0x%x\n", __FUNCTION__, tx_pwr1));
	}

	rate_pwr1 = rate_pwr = tx_pwr1;

	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;
		end_ch = start_ch + ch->num -1;

		if ( (channel >= start_ch) && ( channel <= end_ch ) )
		{
			switch ( phymode )
			{
				case SKU_PHYMODE_CCK_1M_2M:
					rate_pwr = ch->PwrCCK[0];
					break;
				case SKU_PHYMODE_CCK_5M_11M:
					rate_pwr = ch->PwrCCK[2];
					break;
				case SKU_PHYMODE_OFDM_6M_9M:
					rate_pwr = ch->PwrOFDM[0];
					break;
				case SKU_PHYMODE_OFDM_12M_18M:
					rate_pwr = ch->PwrOFDM[2];
					break;
				case SKU_PHYMODE_OFDM_24M_36M:
					rate_pwr = ch->PwrOFDM[4];
					break;
				case SKU_PHYMODE_OFDM_48M_54M:
					rate_pwr = ch->PwrOFDM[6];
					break;
				case SKU_PHYMODE_HT_MCS0_MCS1:
				case SKU_PHYMODE_STBC_MCS0_MCS1:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[0];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[0];
					break;
				case SKU_PHYMODE_HT_MCS2_MCS3:
				case SKU_PHYMODE_STBC_MCS2_MCS3:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[2];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[2];
					break;
				case SKU_PHYMODE_HT_MCS4_MCS5:
				case SKU_PHYMODE_STBC_MCS4_MCS5:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[4];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[4];
					break;
				case SKU_PHYMODE_HT_MCS6_MCS7:
				case SKU_PHYMODE_STBC_MCS6_MCS7:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[6];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[6];
					break;
				case SKU_PHYMODE_HT_MCS8_MCS9:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[8];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[8];
					break;
				case SKU_PHYMODE_HT_MCS10_MCS11:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[10];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[10];
					break;
				case SKU_PHYMODE_HT_MCS12_MCS13:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[12];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[12];
					break;
				case SKU_PHYMODE_HT_MCS14_MCS15:
					if ( bw == BW_20 )
						rate_pwr = ch->PwrHT20[14];
					else if ( bw == BW_40 )
						rate_pwr = ch->PwrHT40[14];
					break;
				default:
					break;
			}

			break;
		}
	}

	pwr_diff = paValue + (pAd->SingleSkuRatePwrDiff[phymode] << 12) + 2048;
	pwr_diff = pwr_diff >> 12;

	if(pAd->bSingleSkuDebug)
		DBGPRINT(RT_DEBUG_TRACE, ("%s: pwr_diff = 0x%x, rate_pwr = 0x%x, rate_pwr1 = 0x%x !!!\n",
					__FUNCTION__, pwr_diff, rate_pwr, rate_pwr1));
	return rate_pwr;

}
#endif /* SINGLE_SKU_V2 */


#ifdef RTMP_TEMPERATURE_COMPENSATION
static BOOLEAN RT6352_AsicGetTssiReport(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bResetTssiInfo,
	OUT PINT32 pTssiReport)
{
	INT		wait;
	UINT8	bbpval;
	CHAR	BBPR49;
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);

	if (bResetTssiInfo ==  TRUE)
	{
		/* set BBP R47[4]=1 to enable TSSI for next reading */
		//RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
		if (!(bbpval & 0x10))
		{
			bbpval |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
		}

		for (wait=0; wait < 4; wait++)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
			if (!(bbpval & 0x10))
				break;
			RtmpusecDelay(500);
		}	
	}

	if ((bbpval & 0x10) != 0)
	{
		/* Get temperature infomation failed */
		return FALSE;
	}
	else
	{
		bbpval &= 0xf8;
		bbpval |= 0x04;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
		
		/* save temperature */ 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
		*pTssiReport = (INT32) BBPR49;
		
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI report from BBP_R49=0x%x\n", *pTssiReport));
	}
	return TRUE;
}


/* MaxBoundaryLevel MUST not be greater than the array size of TssiBoundarys */
static BOOLEAN RT6352_GetTemperatureCompensationLevel(
	IN		PRTMP_ADAPTER	pAd,
	IN		BOOLEAN			bAutoTxAgc,
	IN		CHAR			TssiRef, /* e2p[75h]: the zero reference */
	IN		PCHAR			pTssiMinusBoundary,
	IN		PCHAR			pTssiPlusBoundary,
	IN		UINT8			MaxBoundaryLevel,
	IN		UINT8			TxAgcStep,
	IN		INT32			CurrTemperature,
	OUT		PCHAR			pCompensationLevel)
{
	INT			idx;

	/* Sanity Check */
	if (pTssiMinusBoundary == NULL ||
		pTssiPlusBoundary == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					("%s(): pTssiBoundary is NULL!\n",
					__FUNCTION__)); 
		return FALSE;
	}

	ASSERT(TssiRef == pAd->TssiCalibratedOffset);

	if (bAutoTxAgc)
	{
		if (CurrTemperature < pTssiMinusBoundary[1])
		{
			/* 	Reading is larger than the reference value check
				for how large we need to decrease the Tx power		*/
			for (idx = 1; idx < MaxBoundaryLevel; idx++)
			{
				if (CurrTemperature >= pTssiMinusBoundary[idx]) 
					break;	/* level range found */
			}

			/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
			*pCompensationLevel = -(TxAgcStep * (idx-1));
			DBGPRINT(RT_DEBUG_TRACE, 
						("-- Tx Power, CurrTemperature=%x, TssiRef=%x, TxAgcStep=%x, step = -%d, CompensationLevel = %d\n",
			    			CurrTemperature, TssiRef, TxAgcStep, idx-1, *pCompensationLevel));                    
		}
		else if (CurrTemperature > pTssiPlusBoundary[1])
		{
			/*	Reading is smaller than the reference value check
				for how large we need to increase the Tx power		*/
			for (idx = 1; idx < MaxBoundaryLevel; idx++)
			{
				if (CurrTemperature <= pTssiPlusBoundary[idx])
					break; /* level range found */
			}

			/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
			*pCompensationLevel = TxAgcStep * (idx-1);
			DBGPRINT(RT_DEBUG_TRACE,
						("++ Tx Power, CurrTemperature=%x, TssiRef=%x, TxAgcStep=%x, step = +%d, , CompensationLevel = %d\n",
				    		CurrTemperature, TssiRef, TxAgcStep, idx-1, *pCompensationLevel));
		}
		else
		{
			*pCompensationLevel = 0;
			DBGPRINT(RT_DEBUG_TRACE,
						("  Tx Power, CurrTemperature=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						CurrTemperature, TssiRef, TxAgcStep, 0));
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, 
					("%s(): bAutoTxAgc = %s\n",
					__FUNCTION__,
					(bAutoTxAgc) == TRUE ? "TRUE" : "FALSE")); 
		return FALSE;
	}

	return TRUE;
}


BOOLEAN RT6352_TemperatureCompensation(
		IN PRTMP_ADAPTER pAd,
		IN BOOLEAN bResetTssiInfo)
{
	BOOLEAN bCanDoRegAdjustment = FALSE;

	if (pAd->bCalibrationDone)
	{
		if (RT6352_AsicGetTssiReport(pAd,
									bResetTssiInfo,
									&pAd->CurrTemperature) == TRUE)
		{
			if (pAd->bAutoTxAgcG)
			{
				if (RT6352_GetTemperatureCompensationLevel(
									pAd,
									pAd->bAutoTxAgcG,
									pAd->TssiRefG,
									&pAd->TssiMinusBoundaryG[0],
									&pAd->TssiPlusBoundaryG[0],
									8, /* to do: make a definition */
									2/* pAd->TxAgcStepG; */, /* the unit of MAC 0x13B4 is 0.5 dB */
									pAd->CurrTemperature,
									&pAd->TxAgcCompensateG) == TRUE)
				{
					UINT32 MacValue;
					CHAR delta_pwr = 0;
					
					/* adjust compensation value by MP temperature readings(i.e., e2p[77h]) */
					delta_pwr = pAd->TxAgcCompensateG - pAd->mp_delta_pwr;
					delta_pwr += pAd->DeltaPwrBeforeTempComp;
					/* 8-bit representation ==> 6-bit representation (2's complement) */
					delta_pwr = (delta_pwr & 0x80) ? \
									((delta_pwr & 0x1f) | 0x20) : (delta_pwr & 0x3f);						
					/*	write compensation value into TX_ALG_CFG_1, 
						delta_pwr (unit: 0.5dB) will be compensated by TX_ALG_CFG_1 */     
					RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
					MacValue = (MacValue & (~0x3f)) | delta_pwr;
					RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);

					bCanDoRegAdjustment = TRUE;

					DBGPRINT(RT_DEBUG_TRACE, 
								("%s - delta_pwr = %d, TssiCalibratedOffset = %d,"
								 " TssiMpOffset = %d, Mac 0x13B4 = 0x%08x\n",
								__FUNCTION__,
								(delta_pwr & 0x20) ? (delta_pwr | 0x0c) : (delta_pwr),
								pAd->TssiCalibratedOffset,
								pAd->mp_delta_pwr,
								MacValue));	
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, 
								("%s(): Failed to get a compensation level!\n",
								__FUNCTION__)); 
					return FALSE;
				}
			}
			else
				bCanDoRegAdjustment = TRUE;
		}
		else
		{
			/* Failed to get current temperature */
			pAd->CommonCfg.bEnTemperatureTrack = TRUE;
			return FALSE;
		}
	}

#ifdef RTMP_TEMPERATURE_CALIBRATION
	if (bCanDoRegAdjustment == TRUE)
		RT6352_TemperatureCalibration(pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */

	return TRUE;
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */


VOID RT6352_AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd)
{
	//INT			i, j;
	//CHAR 		Value;
	CHAR		Rssi = -127;
	CHAR		DeltaPwr = 0;
	CHAR		DeltaPowerByBbpR1 = 0; 
	CHAR		TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};	


#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) || 
#ifdef RTMP_MAC_PCI
		(pAd->bPCIclkOff == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
#endif /* RTMP_MAC_PCI */
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if(INFRA_ON(pAd))
		{
			Rssi = RTMPMaxRssi(pAd, 
						   pAd->StaCfg.RssiSample.AvgRssi0, 
						   pAd->StaCfg.RssiSample.AvgRssi1, 
						   pAd->StaCfg.RssiSample.AvgRssi2);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Get Tx rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);

	/* Power will be compensated each 4 sec. */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
#ifdef SINGLE_SKU_V2
		CHAR PreDiff;
#endif /* SINGLE_SKU_V2 */

		/* Get delta power based on the percentage specified from UI */
		AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,&DeltaPowerByBbpR1);

		/* The transmit power controlled by the BBP */
		TotalDeltaPower += DeltaPowerByBbpR1;

		/* The transmit power controlled by the MAC */
		TotalDeltaPower += DeltaPwr; 	
		TotalDeltaPower = (TotalDeltaPower * 2); // because unit = 0.5 dbm

		if (pAd->CommonCfg.BBPCurrentBW == BW_40)
			TotalDeltaPower += pAd->BW_Power_Delta;

#ifdef SINGLE_SKU_V2
		if ((pAd->bCalibrationDone) && (pAd->bOpenFileSuccess))
		{
			CHAR channel_power = 0;

			channel_power = RT6352_AdjustChannelPwr(pAd, TotalDeltaPower, &PreDiff);

			RT6352_AdjustPerRatePwr(pAd, channel_power);
		}
#endif /* SINGLE_SKU_V2 */

#ifdef RTMP_INTERNAL_TX_ALC
		if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
		{
			if (pAd->bCalibrationDone)
			{
				if (RT635xTriggerTssiCompensation(pAd, TotalDeltaPower))
				{
#ifdef RTMP_TEMPERATURE_CALIBRATION
					RT6352_TemperatureCalibration(pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */
				}
			}

			return;
		} 
		else
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
		if (pAd->bAutoTxAgcG)
		{
			pAd->DeltaPwrBeforeTempComp = TotalDeltaPower;
#ifdef SINGLE_SKU_V2
			if (pAd->bOpenFileSuccess)
			pAd->DeltaPwrBeforeTempComp = PreDiff;
#endif /* SINGLE_SKU_V2 */
			if (pAd->bCalibrationDone)
				RT6352_TemperatureCompensation(pAd, TRUE);

			return;
		}
		else
#endif /* RTMP_TEMPERATURE_COMPENSATION */
		{
#ifdef RTMP_TEMPERATURE_CALIBRATION
			if (pAd->bCalibrationDone)
			{			
				UINT8 bbpval;
				CHAR BBPR49;			
				int wait;

				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
				if (!(bbpval & 0x10))
				{
					bbpval |= 0x10;
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
				}

				for (wait = 0; wait < 2; wait++)
				{
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
					if (!(bbpval & 0x10))
						break;
					RtmpusecDelay(1000);
				}

				if ((bbpval & 0x10) != 0)
				{
					pAd->CommonCfg.bEnTemperatureTrack = TRUE;
				}
				else
				{
					bbpval &= 0xf8;
					bbpval |= 0x04;
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
					
					/* save temperature */ 
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
					pAd->CurrTemperature = (INT32) BBPR49;
					
					DBGPRINT(RT_DEBUG_TRACE, ("Current Temperature from BBP_R49=0x%x\n", pAd->CurrTemperature));
					RT6352_TemperatureCalibration(pAd);
					pAd->CommonCfg.bEnTemperatureTrack = FALSE;
				}
			}
#endif /* RTMP_TEMPERATURE_CALIBRATION */
		}

#ifdef SINGLE_SKU_V2
		if (pAd->bOpenFileSuccess)
		return;
#endif /* SINGLE_SKU_V2 */

		{  
			INT32 Diff;
			UINT32 MacValue;
			RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */
			MacValue &= 0xffffffc0;

				Diff = TotalDeltaPower;
			if (Diff > 31)
				Diff = 31;
			else if(Diff < -31)
				Diff = -31;

			MacValue |= (Diff & 0x3F);
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue); 

			DBGPRINT(RT_DEBUG_TRACE, ("TotalDeltaPower=%d, Mac 0x13B4 is 0x%08x\n", TotalDeltaPower, MacValue));
		}

		/* Extra set MAC registers to compensate Tx power if any */
		RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
	}

}

#ifdef MICROWAVE_OVEN_SUPPORT
static VOID RT6352_AsicMeasureFalseCCA(
	IN PRTMP_ADAPTER pAd
)
{
	UINT32 reg;

	/* Set to high gain LAN */
	//printk("Stored_BBP_R65=%x @%s \n", pAd->CommonCfg.MO_Cfg.Stored_BBP_R65, __FUNCTION__);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, pAd->CommonCfg.MO_Cfg.Stored_BBP_R65);

	/* clear false cca counter */
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &reg);

	/* reset false CCA counter */
	pAd->CommonCfg.MO_Cfg.nFalseCCACnt = 0;
}

static VOID RT6352_AsicMitigateMicrowave(
	IN PRTMP_ADAPTER pAd)
{
	UINT8 RegValue;

	DBGPRINT(RT_DEBUG_OFF, ("Detect Microwave...\n"));

	/* set middle gain */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &RegValue);
	RegValue |= 0x08;
	RegValue &= 0xfd; /*BBP_R65[3:2] from 3 into 2 */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x28);
}
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RTMP_TEMPERATURE_CALIBRATION
VOID RT6352_Temperature_Init (
	IN PRTMP_ADAPTER pAd)
{
	int wait;
	UINT8 bbpval;
	CHAR BBPR49;
	UINT32 orig_RF_CONTROL0 = 0; // 0x0518
	UINT32 orig_RF_BYPASS0  = 0; // 0x051c

	RTMP_IO_READ32(pAd, RF_CONTROL0, &orig_RF_CONTROL0);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0 , &orig_RF_BYPASS0);

	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0 , 0x0);
	/* MAC Bypass */
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x0004);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x3366);

	if (pAd->bRef25CVaild == FALSE)
	{
		UINT8 RfB0R35Value;
		UINT8 RfB5R04Value,RfB5R17Value,RfB5R18Value;
		UINT8 RfB5R19Value, RfB5R20Value;

		RT635xReadRFRegister(pAd, RF_BANK0, RF_R35, &RfB0R35Value);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &RfB5R04Value);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &RfB5R17Value);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &RfB5R18Value);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &RfB5R19Value);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &RfB5R20Value);

		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x0);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x80);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0x83);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x20);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
		bbpval |= 0x50;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x40);
		
		for (wait=0; wait<200; wait++)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
			if (!(bbpval & 0x10))
				break;
			RtmpusecDelay(2000);
		}
		ASSERT(wait < 200);
		
		bbpval &= 0xf8;
		bbpval |= 0x04;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
		DBGPRINT(RT_DEBUG_ERROR, ("%s : BBPR49 = 0x%x\n", __FUNCTION__, BBPR49));
		pAd->TemperatureRef25C = BBPR49 - 0x0A;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x01);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, RfB0R35Value);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, RfB5R04Value);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, RfB5R17Value);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, RfB5R18Value);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, RfB5R19Value);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, RfB5R20Value);
		pAd->bRef25CVaild = TRUE;
		DBGPRINT(RT_DEBUG_ERROR, ("%s : TemperatureRef25C = 0x%x\n", __FUNCTION__, pAd->TemperatureRef25C));
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
	if (!(bbpval & 0x10))
	{	
		bbpval |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x31);

	for (wait=0; wait<200; wait++)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
		if (!(bbpval & 0x10))
			break;
		RtmpusecDelay(1000);
	}
	ASSERT(wait < 200);

	bbpval &= 0xf8;
	bbpval |= 0x04;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);

	/* save temperature */ 
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	pAd->CurrTemperature = (INT32)BBPR49; 

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x0);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &bbpval);
	bbpval |= 0x1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);	
	bbpval &= 0xfe;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);

	// recover
	if (wait >= 200) {
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbpval);
		bbpval &= 0xef;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbpval);
	}
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0 , 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0 , orig_RF_CONTROL0);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0 , orig_RF_BYPASS0);

	pAd->bLowTemperatureTrigger = FALSE;

	DBGPRINT(RT_DEBUG_OFF, ("Current Temperature from BBP_R49=0x%x\n", pAd->CurrTemperature));
}

VOID RT6352_TemperatureCalibration(
	IN PRTMP_ADAPTER pAd)
{
	INT32 CurrentTemper = pAd->CurrTemperature;

	CurrentTemper = (pAd->CurrTemperature - pAd->TemperatureRef25C) * 19;		// 319 * 10

        //                       HT    -> LT
	//B4/B6.R04=0x00->0x06
	//B4/B6.R10=0x51->0x41
	if (CurrentTemper < -50) // (20 - 25) * 10 = -50
	{
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R04, 0x06);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R04, 0x06);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R10, 0x41);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R10, 0x41);
		pAd->bLowTemperatureTrigger = TRUE;
		DBGPRINT(RT_DEBUG_INFO, ("%s:: CurrentTemper < 20 \n", __FUNCTION__));
	}
	else
	{
		if (pAd->bLowTemperatureTrigger)
		{
			if ( CurrentTemper > 50)
			{
				RT635xWriteRFRegister(pAd, RF_BANK4, RF_R04, 0x00);
				RT635xWriteRFRegister(pAd, RF_BANK6, RF_R04, 0x00);
				RT635xWriteRFRegister(pAd, RF_BANK4, RF_R10, 0x51);
				RT635xWriteRFRegister(pAd, RF_BANK6, RF_R10, 0x51);
				pAd->bLowTemperatureTrigger = FALSE;
				DBGPRINT(RT_DEBUG_INFO, ("%s::CurrentTemper > 30\n", __FUNCTION__));
			}
		}
		else
		{
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R04, 0x00);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R04, 0x00);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R10, 0x51);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R10, 0x51);
			DBGPRINT(RT_DEBUG_INFO, ("%s::CurrentTemper > 20\n", __FUNCTION__));
		}
	}
}
#endif /* RTMP_TEMPERATURE_CALIBRATION */

#ifdef RTMP_TEMPERATURE_COMPENSATION
/* 
	This procedure adjusts the boundary table by pAd->TssiCalibratedOffset.

	pAd->TssiCalibratedOffset: 
		production line temperature(e2p[77h]) - reference temperature(e2p[D1h])
*/				
VOID RT6352_TssiTableAdjust(
	IN PRTMP_ADAPTER pAd)
{
	INT			idx;
	CHAR 		upper_bound = 127, lower_bound = -128;

	DBGPRINT(RT_DEBUG_OFF,("%s: upper_bound = 0x%02X decimal: %d\n",
		__FUNCTION__, upper_bound, upper_bound));
	DBGPRINT(RT_DEBUG_OFF,("%s: lower_bound = 0x%02X decimal: %d\n",
		__FUNCTION__, lower_bound, lower_bound));

	DBGPRINT(RT_DEBUG_OFF,("*** %s: G Tssi[-7 .. +7] = %d %d %d %d %d %d %d\n - %d - %d %d %d %d %d %d %d, offset=%d, tuning=%d\n",
		__FUNCTION__,
		pAd->TssiMinusBoundaryG[7], pAd->TssiMinusBoundaryG[6], pAd->TssiMinusBoundaryG[5],
		pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
		pAd->TssiRefG,
		pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
		pAd->TssiPlusBoundaryG[5], pAd->TssiPlusBoundaryG[6], pAd->TssiPlusBoundaryG[7],
		pAd->TssiCalibratedOffset, pAd->bAutoTxAgcG));

	for (idx = 0; idx < 8; idx++ )
	{
		if ((lower_bound - pAd->TssiMinusBoundaryG[idx]) <= pAd->TssiCalibratedOffset)
		{
			pAd->TssiMinusBoundaryG[idx] += pAd->TssiCalibratedOffset;			
		}
		else
		{
			pAd->TssiMinusBoundaryG[idx] = lower_bound;
		}

		if ((upper_bound - pAd->TssiPlusBoundaryG[idx]) >= pAd->TssiCalibratedOffset)
		{
			pAd->TssiPlusBoundaryG[idx] += pAd->TssiCalibratedOffset;			
		}
		else
		{
			pAd->TssiPlusBoundaryG[idx] = upper_bound;			
		}

		ASSERT(pAd->TssiMinusBoundaryG[idx] >= lower_bound);
		ASSERT(pAd->TssiPlusBoundaryG[idx] <= upper_bound);
	}

	pAd->TssiRefG = pAd->TssiMinusBoundaryG[0];

	DBGPRINT(RT_DEBUG_OFF,("%s: G Tssi[-7 .. +7] = %d %d %d %d %d %d %d - %d - %d %d %d %d %d %d %d, offset=%d, tuning=%d\n",
		__FUNCTION__,
		pAd->TssiMinusBoundaryG[7], pAd->TssiMinusBoundaryG[6], pAd->TssiMinusBoundaryG[5],
		pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
		pAd->TssiRefG,
		pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
		pAd->TssiPlusBoundaryG[5], pAd->TssiPlusBoundaryG[6], pAd->TssiPlusBoundaryG[7],
		pAd->TssiCalibratedOffset, pAd->bAutoTxAgcG));

	return;
}


VOID RT6352_TssiMpAdjust(
	IN PRTMP_ADAPTER pAd)
{
	PCHAR pTssiMinusBoundary, pTssiPlusBoundary;
	EEPROM_TX_PWR_STRUC e2p_value;
	CHAR mp_temperature, idx, TxAgcMpOffset = 0;
	
	RT28xx_EEPROM_READ16(pAd, (EEPROM_G_TSSI_BOUND5), e2p_value.word);			
	mp_temperature = e2p_value.field.Byte1;						

	pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
	pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];

	if (mp_temperature < pTssiMinusBoundary[1])
	{
		/* mp_temperature is larger than the reference value */
		/* check for how large we need to adjust the Tx power */
		for (idx = 1; idx < 8; idx++)
		{
			if (mp_temperature >= pTssiMinusBoundary[idx]) /* the range has been found */
				break;
		}

		/* The index is the step we should decrease, idx = 0 means there is nothing to adjust */
		TxAgcMpOffset = -(2 * (idx-1));
		pAd->mp_delta_pwr = (TxAgcMpOffset);
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = -%d\n",
		    mp_temperature, idx-1));                    
	}
	else if (mp_temperature > pTssiPlusBoundary[1])
	{
		/* mp_temperature is smaller than the reference value */
		/* check for how large we need to adjust the Tx power */
		for (idx = 1; idx < 8; idx++)
		{
		    if (mp_temperature <= pTssiPlusBoundary[idx]) /* the range has been found */
	            break;
		}

		/* The index is the step we should increase, idx = 0 means there is nothing to adjust */
		TxAgcMpOffset = 2 * (idx-1);
		pAd->mp_delta_pwr = (TxAgcMpOffset);
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = +%d\n",
			    mp_temperature, idx-1));
	}
	else
	{
		pAd->mp_delta_pwr = 0;
		DBGPRINT(RT_DEBUG_OFF, ("mp_temperature=0x%02x, step = +%d\n",
				mp_temperature, 0));
	}

	return;
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
VOID RT6352_Init_ExtPA_ExtLNA(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN ReInit)
{
	ULONG GpioMode;

	/* Check the gpio setting first */
	RTMP_SYS_IO_READ32(0xb0000060, &GpioMode);
	if ((GpioMode & 0x100000) == 0x100000)
	{
		GpioMode &= (~0x100000);
		RTMP_SYS_IO_WRITE32(0xb0000060, GpioMode);
	
		DBGPRINT(RT_DEBUG_ERROR,("Change as Normal Mode(0x%x)\n", GpioMode));
		RTMP_SYS_IO_READ32(0xb0000060, &GpioMode);
		DBGPRINT(RT_DEBUG_ERROR,("After Change, now GPIO_MODE value is 0x%x\n", GpioMode));
	}

#ifdef RT6352_EP_SUPPORT
	if (pAd->bExtPA)
	{
		UINT32 MacValue;

		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Enable Ext-PA. init MAC \n", __FUNCTION__));
		RTMP_IO_READ32(pAd, RF_CONTROL3, &MacValue);
		MacValue |= 0x00000101;
		RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacValue);

		RTMP_IO_READ32(pAd, RF_BYPASS3, &MacValue);
		MacValue |= 0x00000101;
		RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacValue);
	}
#endif /* RT6352_EP_SUPPORT */

#ifdef RT6352_EL_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Enable Ext-LNA. \n", __FUNCTION__));

		/* TFBGA Ext-LNA */
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R14, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R14, 0x66);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R17, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R17, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R18, 0x42);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R18, 0x42);
	}
#endif /* RT6352_EL_SUPPORT */

#ifdef RT6352_EP_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->bExtPA))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Enable Ext-PA. \n", __FUNCTION__));

		/* TFBGA Ext-PA */
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R43, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R43, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R44, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R44, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R45, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R45, 0x73);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R46, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R46, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R48, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R48, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R49, 0x05);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R49, 0x05);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R54, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R54, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R56, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R56, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R57, 0x05);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R57, 0x05);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R58, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R58, 0x27);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R59, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R59, 0xC8);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R60, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R60, 0xA4);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R61, 0x05);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R61, 0x05);
	}
#endif /* RT6352_EP_SUPPORT */

#ifdef RT6352_EP_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->bExtPA))
	{
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R05, 0x00);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R05, 0x00);

	}
#endif /* RT6352_EP_SUPPORT */

#ifdef RT6352_EL_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Init Ext-LNA BBP. \n", __FUNCTION__));

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x68);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R76, 0x4C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R79, 0x1C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R80, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xB6);
	}
#endif /* RT6352_EL_SUPPORT */

#ifdef RT6352_EP_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->bExtPA))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Init Ext-PA MAC. \n", __FUNCTION__));

		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_CORRECT, 0x36303636); /* WH, 2012-12-28 */
		RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_ATTEN, 0x6C6C6B6C); /* WH, 2012-12-28 */
		RTMP_IO_WRITE32(pAd, TX1_RF_GAIN_ATTEN, 0x6C6C6B6C); /* WH, 2012-12-28 */
	}
#endif /* RT6352_EP_SUPPORT */

	return;
}

VOID RT6352_Restore_RF_BBP(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RT6352_EP_SUPPORT
	if (pAd->bExtPA)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Restore Ext-PA. init MAC \n", __FUNCTION__));
		RTMP_IO_WRITE32(pAd, RF_CONTROL3, 0x0);
		RTMP_IO_WRITE32(pAd, RF_BYPASS3, 0x0);
	}
#endif /* RT6352_EP_SUPPORT */
	
#ifdef RT6352_EL_SUPPORT
		if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Restore Ext-LNA. \n", __FUNCTION__));

			/* TFBGA Ext-LNA */
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R14, 0x16);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R14, 0x16);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R17, 0x23);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R17, 0x23);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R18, 0x02);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R18, 0x02);
		}
#endif /* RT6352_EL_SUPPORT */
	
#ifdef RT6352_EP_SUPPORT
		if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->bExtPA))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Restore Ext-PA. \n", __FUNCTION__));
	
			/* TFBGA Ext-PA */
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R43, 0xD3);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R43, 0xD3);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R44, 0xB3);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R44, 0xB3);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R45, 0xD5);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R45, 0xD5);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R46, 0x27);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R46, 0x27);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R47, 0x6C);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R47, 0x6C);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R48, 0xFC);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R48, 0xFC);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R49, 0x1F);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R49, 0x1F);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R54, 0x27);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R54, 0x27);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R55, 0x66);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R55, 0x66);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R56, 0xFF);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R56, 0xFF);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R57, 0x1C);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R57, 0x1C);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R58, 0x20);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R58, 0x20);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R59, 0x6B);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R59, 0x6B);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R60, 0xF7);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R60, 0xF7);
			RT635xWriteRFRegister(pAd, RF_BANK4, RF_R61, 0x09);
			RT635xWriteRFRegister(pAd, RF_BANK6, RF_R61, 0x09);
		}
#endif /* RT6352_EP_SUPPORT */
	
#ifdef RT6352_EL_SUPPORT
		if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Restore Ext-LNA BBP. \n", __FUNCTION__));
	
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x60);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R76, 0x44);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R79, 0x1C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R80, 0x0C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xB6);

			if(pAd->CommonCfg.BBPCurrentBW == BW_20)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R141);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x1A);
			}
		}
#endif /* RT6352_EL_SUPPORT */

#ifdef RT6352_EP_SUPPORT
		if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->bExtPA))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s: Restore Ext-PA MAC. \n", __FUNCTION__));
	
			RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_CORRECT, 0x3630363A); /* WH, 2012-12-28 */
			RTMP_IO_WRITE32(pAd, TX0_RF_GAIN_ATTEN, 0x6C6C666C); /* WH, 2012-12-28 */
			RTMP_IO_WRITE32(pAd, TX1_RF_GAIN_ATTEN, 0x6C6C666C); /* WH, 2012-12-28 */
		}
#endif /* RT6352_EP_SUPPORT */
}

VOID RT6352_ReCalibration(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 MTxCycle = 0;
	UINT32 MacValue = 0, MacSysCtrl = 0;
	UCHAR BBPR30Value, RFB0_R39, RFB0_R42, RFValue;

	DBGPRINT(RT_DEBUG_TRACE, (" Do ReCalibration !!!\n"));

	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacSysCtrl);

	/* MAC Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x04);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);

	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x1)
			RtmpusecDelay(50);
		else
			break;
	}

	if (MTxCycle >= 10000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Count MAC TX idle to MAX  !!!\n"));
	}

	/* MAC Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x08);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);

	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x2)
			RtmpusecDelay(50);
		else
			break;
	}

	if (MTxCycle >= 10000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Count MAC RX idle to MAX  !!!\n"));
	}

	/* Backup ADC clcok selection */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPR30Value);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R39, &RFB0_R39);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFB0_R42);

	/* change to Shielding clock */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, 0x1F);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x5B);

	RT6352_Restore_RF_BBP(pAd);

	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R05, 0x40);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, 0x0C);

	RT635xReadRFRegister(pAd, RF_BANK0, RF_R04, &RFValue);
	RFValue = RFValue | 0x80; /* bit 7=vcocal_en*/
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R04, RFValue);
	RtmpOsMsDelay(2);

	/* Do LOFT and IQ Calibration */
	LOFT_IQ_Calibration(pAd);

	/* Do RXIQ Calibration */
	RXIQ_Calibration(pAd);

	RT6352_Init_ExtPA_ExtLNA(pAd, TRUE);

#ifdef RT6352_EL_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
	{
		if(pAd->CommonCfg.BBPCurrentBW == BW_20)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R141);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
		}
	}
#endif /* RT6352_EL_SUPPORT */

	/* restore ADC clcok selection */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPR30Value);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, RFB0_R39);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFB0_R42);

	//RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	//MacValue |= 0xC;
	//RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacSysCtrl);

	return;
}
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */

#ifdef DYNAMIC_VGA_SUPPORT
void RT6352_UpdateRssiForChannelModel(RTMP_ADAPTER * pAd)
{
	INT32 rx0_rssi, rx1_rssi;
	
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		rx0_rssi = (CHAR)(pAd->StaCfg.RssiSample.LastRssi0);
		rx1_rssi = (CHAR)(pAd->StaCfg.RssiSample.LastRssi1);
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		rx0_rssi = (CHAR)(pAd->ApCfg.RssiSample.LastRssi0);
		rx1_rssi = (CHAR)(pAd->ApCfg.RssiSample.LastRssi1);
	}
#endif /* CONFIG_AP_SUPPORT */

	DBGPRINT(RT_DEBUG_INFO, ("%s:: rx0_rssi(%d), rx1_rssi(%d)\n", 
		__FUNCTION__, rx0_rssi, rx1_rssi));	

	/*
		RSSI_DUT(n) = RSSI_DUT(n-1)*15/16 + RSSI_R2320_100ms_sample*1/16
	*/
	pAd->chipCap.avg_rssi_0 = ((pAd->chipCap.avg_rssi_0) * 15)/16 + (rx0_rssi << 8)/16;
	pAd->chipCap.avg_rssi_1 = ((pAd->chipCap.avg_rssi_1) * 15)/16 + (rx1_rssi << 8)/16;

	if (pAd->MacTab.Size == 0)
		pAd->chipCap.avg_rssi_all = -75;
	else
		pAd->chipCap.avg_rssi_all = (pAd->chipCap.avg_rssi_0 + pAd->chipCap.avg_rssi_1)/512;

	DBGPRINT(RT_DEBUG_INFO, ("%s:: update rssi all(%d)\n", 
		__FUNCTION__, pAd->chipCap.avg_rssi_all));
}

static VOID RT6352_AsicDynamicVgaGainControl(
	IN PRTMP_ADAPTER pAd)
{
	if ((pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable) && 
		(pAd->chipCap.dynamic_vga_support) &&
		(pAd->bCalibrationDone)
		)
	{
		UCHAR BbpReg = 0;
        UCHAR VgaGainLowerBound = 0x10;
		

		if (((pAd->chipCap.avg_rssi_all <= -76) && (pAd->CommonCfg.BBPCurrentBW == BW_80))
		|| ((pAd->chipCap.avg_rssi_all <= -79) && (pAd->CommonCfg.BBPCurrentBW == BW_40))
		|| ((pAd->chipCap.avg_rssi_all <= -82) && (pAd->CommonCfg.BBPCurrentBW == BW_20)))
		{
			DBGPRINT(RT_DEBUG_INFO,("pAd->chipCap.avg_rssi_all = %d , no dync vga\n", pAd->chipCap.avg_rssi_all));
			return;
		}

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &BbpReg);
		
		DBGPRINT(RT_DEBUG_INFO,
			("RT6352 one second False CCA=%d, fixed R66 at 0x%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, BbpReg));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.lna_vga_ctl.nFalseCCATh)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xAD);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x28);

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xB1);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x20);

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
			if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			{
#ifdef RT6352_EL_SUPPORT
				if (pAd->NicConfig2.field.ExternalLNAForG)
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
				else
#endif /* RT6352_EL_SUPPORT */
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x1A);
			}
			else
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x10);
			}

			if (BbpReg < (pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0 + VgaGainLowerBound))
			{
				BbpReg += 4;
				bbp_set_agc(pAd, BbpReg, RX_CHAIN_ALL);
			}
			else if (BbpReg == (pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0 + VgaGainLowerBound))
			{
				if (pAd->RalinkCounters.OneSecFalseCCACnt > 1500)
				{
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9C);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x3D);

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9D);
					if (pAd->CommonCfg.BBPCurrentBW == BW_20)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x40);
					else
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x2F);
				}
			}
		}
		else if (pAd->RalinkCounters.OneSecFalseCCACnt < pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9D);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);

			if (BbpReg > pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0)
			{
				BbpReg -= 4;				
				bbp_set_agc(pAd, BbpReg, RX_CHAIN_ALL);				
			}
			else if (BbpReg == pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xAD);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0D);
				
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0xB1);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x16);

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0C);
			}
		}
		else
		{
			if (BbpReg == pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x0C);
			}
		}
#ifdef ED_MONITOR
		if(BbpReg >= (pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0 + VgaGainLowerBound))
			pAd->ed_vga_at_lowest_gain = TRUE;
		else
			pAd->ed_vga_at_lowest_gain = FALSE;
#endif
	}
}

VOID rt6352_dynamic_vga_enable(RTMP_ADAPTER *pAd)
{
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x83);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x70);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x86);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x70);
	
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9c);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9d);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x27);
}


VOID rt6352_dynamic_vga_disable(RTMP_ADAPTER *pAd)
{
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x83);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x32);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x86);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x19);
	
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9c);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x3d);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x9d);
	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x40);
	else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x2F);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, 0x8D);
	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
	{
#ifdef RT6352_EL_SUPPORT
		if (pAd->NicConfig2.field.ExternalLNAForG)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
		else
#endif /* RT6352_EL_SUPPORT */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x1A);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x10);
	}


	bbp_set_agc(pAd, pAd->CommonCfg.lna_vga_ctl.agc_vga_ori_0, RX_CHAIN_ALL);
}

#endif /* DYNAMIC_VGA_SUPPORT */

UINT32 rt6352_get_current_temp(
	IN RTMP_ADAPTER 	*pAd)
{
	UINT32 current_temp = 0;	
	current_temp = (19*(pAd->CurrTemperature - pAd->TemperatureRef25C))/10 + 25;
	return current_temp;
}

#ifdef THERMAL_PROTECT_SUPPORT
VOID thermal_pro_default_cond(
	IN RTMP_ADAPTER 	*pAd)
{
	UINT32 mac_val = 0;

	DBGPRINT(RT_DEBUG_OFF, ("----->%s\n", __FUNCTION__));

	/* Default A-MPDU length */
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M1S, 0x77777777);
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M1S, 0x77777777);

	/* 2SS */
	RTMP_IO_READ32(pAd, TXRX_MICS_CTRL, &mac_val);
	mac_val &= ~(0x00000008);
	RTMP_IO_WRITE32(pAd, TXRX_MICS_CTRL, mac_val);	

	pAd->force_one_tx_stream = FALSE;
}
	
VOID thermal_pro_1st_cond(
	IN RTMP_ADAPTER 	*pAd)
{
	UINT32 mac_val = 0;

	DBGPRINT(RT_DEBUG_OFF, ("----->%s\n", __FUNCTION__));

	/* Default A-MPDU length */
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M1S, 0x77777777);
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M1S, 0x77777777);

	/* 2SS -> 1SS */
	RTMP_IO_READ32(pAd, TXRX_MICS_CTRL, &mac_val);
	mac_val |= 0x00000008;
	RTMP_IO_WRITE32(pAd, TXRX_MICS_CTRL, mac_val);	

	pAd->force_one_tx_stream = TRUE;
}

VOID thermal_pro_2nd_cond(
	IN RTMP_ADAPTER 	*pAd)
{
	UINT32 mac_val = 0;

	DBGPRINT(RT_DEBUG_OFF, ("----->%s\n", __FUNCTION__));

	/* Reduce A-MPDU length */
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_20M1S, 0x33222222);
	RTMP_IO_WRITE32(pAd, AMPDU_MAX_LEN_40M1S, 0x33222222);

	/* 1SS */
	RTMP_IO_READ32(pAd, TXRX_MICS_CTRL, &mac_val);
	mac_val |= 0x00000008;
	RTMP_IO_WRITE32(pAd, TXRX_MICS_CTRL, mac_val);	

	pAd->force_one_tx_stream = TRUE;
}
#endif /* THERMAL_PROTECT_SUPPORT */

#ifdef ED_MONITOR
VOID RT6352_set_ed_cca(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	UINT32 mac_val;
	UCHAR bbp_val;

	if (enable)
	{
		RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
		mac_val &= (~0x01);
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

		RTMP_IO_READ32(pAd, CH_IDLE_STA, &mac_val);
		RTMP_IO_READ32(pAd, CH_BUSY_STA, &mac_val);
		RTMP_IO_READ32(pAd, CH_BUSY_STA_SEC, &mac_val);
		RTMP_IO_READ32(pAd, 0x1140, &mac_val);

		RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
		mac_val |= 0x05; // enable channel status check
		RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

		bbp_val = 0x1C; // bit 0~7 for high threshold
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R61, bbp_val);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R87, &bbp_val);
		bbp_val |= 0x84; // bit 0~2 for low threshold, bit 7 for enable ED_CCA
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R87, bbp_val);
		
		// enable bbp for ED_2nd_CCA status check
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R83, &bbp_val);
		bbp_val = 0x9a; // bit 0~3 for threshold
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, bbp_val);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &bbp_val);
		bbp_val &= (~0x02); // bit 1 for eanble/disable ED_2nd_CCA, 0: enable, 1: disable 
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, bbp_val);

		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
		mac_val |= ((1<<20) | (1<<7));		
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);		
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R87, &bbp_val);
		bbp_val &= (~0x80); // bit 7 for enable/disable ED_CCA
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R87, bbp_val);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &bbp_val);
		bbp_val |= (0x02); // bit 1 for eanble/disable ED_2nd_CCA, 0: enable, 1: disable 
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, bbp_val);
		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
		mac_val &= (~((1<<20) | (1<<7)));
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);
	}
}
#endif /* ED_MONITOR */


/*
========================================================================
Routine Description:
	Initialize RT6352.

Arguments:
	pAd					- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT6352_Init(
	IN PRTMP_ADAPTER		pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;


	pChipCap->hif_type = HIF_RTMP;
	pChipCap->rf_type = RF_RT;

	pAd->RfIcType = RFIC_UNKNOWN;
	rtmp_phy_probe(pAd);

	/*
		Init chip capabilities
	*/
	pChipCap->MCUType = SWMCU;
#ifdef MCS_LUT_SUPPORT
	pChipCap->asic_caps |= (fASIC_CAP_MCS_LUT);
#endif /* MCS_LUT_SUPPORT */
	pChipCap->phy_caps = (fPHY_CAP_24G);
	pChipCap->phy_caps |= (fPHY_CAP_HT);
	pChipCap->max_nss = 2;
	pChipCap->ba_max_cnt = 21;
	pChipCap->TXWISize = 20;
	pChipCap->RXWISize = 24;
	pChipCap->WPDMABurstSIZE = 2;
	pChipCap->DPDCalPassThres = 5;

	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_LEGACY);
	
	pChipCap->MaxNumOfRfId = 64;
	pChipCap->MaxNumOfBbpId = 255;
	pChipCap->bbpRegTbSize = 0;
	pChipCap->FlgIsVcoReCalMode = VCO_CAL_MODE_3;
	pChipCap->SnrFormula = SNR_FORMULA2;
	pChipCap->RfReg17WtMethod = RF_REG_WT_METHOD_NONE;
	pChipOps->AsicGetTxPowerOffset = AsicGetTxPowerOffset;
	pChipOps->AsicExtraPowerOverMAC = RT6352_AsicExtraPowerOverMAC;
	pChipOps->AsicAdjustTxPower = RT6352_AsicAdjustTxPower;

#ifdef RTMP_INTERNAL_TX_ALC
	pChipCap->TxAlcTxPowerUpperBound_2G = 61;
	pChipCap->TxPowerTuningTable_2G = NULL /* RT6352_TxPowerTuningTable */;
	pChipOps->InitDesiredTSSITable = RT6352_InitDesiredTSSITable;
	pChipOps->AsicTxAlcGetAutoAgcOffset = NULL /*RT6352_AsicTxAlcGetAutoAgcOffset*/;
#endif /* RTMP_INTERNAL_TX_ALC */

	pChipOps->ChipGetCurrentTemp = rt6352_get_current_temp;

#ifdef THERMAL_PROTECT_SUPPORT
	pChipCap->ThermalProtectSup = TRUE;
	pChipOps->ThermalProDefaultCond = thermal_pro_default_cond;
	pChipOps->ThermalPro1stCond = thermal_pro_1st_cond;
	pChipOps->ThermalPro2ndCond = thermal_pro_2nd_cond;
#endif /* THERMAL_PROTECT_SUPPORT */

	pChipCap->FlgIsHwWapiSup = TRUE;
	pChipCap->DPDCalPassLowThresTX0 = -999;
	pChipCap->DPDCalPassLowThresTX1 = 0;
	pChipCap->DPDCalPassHighThresTX0 = 5;
	pChipCap->DPDCalPassHighThresTX1 = 15;
#ifdef RTMP_FLASH_SUPPORT
	pChipCap->EEPROM_DEFAULT_BIN = RT6352_EeBuffer;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	pChipCap->MBSSIDMode = MBSSID_MODE4;
#else
	pChipCap->MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	pChipCap->MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */

#ifdef FIFO_EXT_SUPPORT
	pChipCap->FlgHwFifoExtCap = TRUE;
#endif /* FIFO_EXT_SUPPORT */

	/* init operator */
	pChipOps->AsicRfInit = NICInitRT6352RFRegisters;
	pChipOps->AsicBbpInit = NICInitRT6352BbpRegisters;
	pChipOps->AsicMacInit = NICInitRT6352MacRegisters;

#ifdef GREENAP_SUPPORT
	pChipOps->EnableAPMIMOPS = EnableAPMIMOPSv2;
	pChipOps->DisableAPMIMOPS = DisableAPMIMOPSv2;
#endif /* GREENAP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	pChipOps->ChipAGCAdjust = RT6352_ChipAGCAdjust;
#endif /* CONFIG_STA_SUPPORT */
	pChipOps->ChipBBPAdjust = RT6352_ChipBBPAdjust;
	pChipOps->AsicAntennaDefaultReset = RT6352_AsicAntennaDefaultReset;
	pChipOps->ChipSwitchChannel = RT6352_ChipSwitchChannel;
#ifdef ED_MONITOR
	pChipOps->ChipSetEDCCA = RT6352_set_ed_cca;
#else
	pChipOps->ChipSetEDCCA= NULL;
#endif /* ED_MONITOR */

	pChipOps->ChipAGCInit = RT6352_RTMPSetAGCInitValue;
#ifdef CARRIER_DETECTION_SUPPORT
	pAd->chipCap.carrier_func = TONE_RADAR_V2;
	pChipOps->ToneRadarProgram = ToneRadarProgram_v2;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef MICROWAVE_OVEN_SUPPORT
	pChipOps->AsicMeasureFalseCCA = RT6352_AsicMeasureFalseCCA;
	pChipOps->AsicMitigateMicrowave = RT6352_AsicMitigateMicrowave;
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
	pChipCap->dynamic_vga_support = TRUE;
	pChipCap->avg_rssi_all = -90;
	pChipCap->avg_rssi_0 = -90;
	pChipCap->avg_rssi_1 = -90;
	pChipOps->AsicDynamicVgaGainControl = RT6352_AsicDynamicVgaGainControl;
	pChipOps->UpdateRssiForDynamicVga = RT6352_UpdateRssiForChannelModel;
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RtmpChipBcnSpecInit(pAd);
#else
	RtmpChipBcnInit(pAd);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
}
#endif /* RT6352 */
/* End of rt3352.c */

