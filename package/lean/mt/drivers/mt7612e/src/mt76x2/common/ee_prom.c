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
	ee_prom.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"



/* IRQL = PASSIVE_LEVEL*/
static inline VOID RaiseClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
	*x = *x | EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);				/* Max frequency = 1MHz in Spec. definition */
}

/* IRQL = PASSIVE_LEVEL*/
static inline VOID LowerClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
	*x = *x & ~EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RtmpusecDelay(1);
}

/* IRQL = PASSIVE_LEVEL*/
static inline USHORT ShiftInBits(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32		x,i;
	USHORT      data=0;

	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~( EEDO | EEDI);

	for(i=0; i<16; i++)
	{
		data = data << 1;
		RaiseClock(pAd, &x);

		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		LowerClock(pAd, &x); /*prevent read failed*/
		
		x &= ~(EEDI);
		if(x & EEDO)
		    data |= 1;
	}

	return data;
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID ShiftOutBits(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			data,
	IN USHORT			count)
{
	UINT32       x,mask;

	mask = 0x01 << (count - 1);
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~(EEDO | EEDI);

	do
	{
	    x &= ~EEDI;
	    if(data & mask)		x |= EEDI;

	    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	    RaiseClock(pAd, &x);
	    LowerClock(pAd, &x);

	    mask = mask >> 1;
	} while(mask);

	x &= ~EEDI;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
}


/* IRQL = PASSIVE_LEVEL*/
static inline VOID EEpromCleanup(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 x;

	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~(EECS | EEDI);
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);	
}


static inline VOID EWEN(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32	x;

	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWEN_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);    
}


static inline VOID EWDS(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32	x;

	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* kick a pulse*/
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	/* output the read_opcode and six pulse in that order    */
	ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);    
}


/* IRQL = PASSIVE_LEVEL*/
int rtmp_ee_prom_read16(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	OUT USHORT *pValue)
{
	UINT32 x;
	USHORT data;



	Offset /= 2;
	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* patch can not access e-Fuse issue*/
	if (IS_RT2860(pAd))
	{
		/* kick a pulse*/
		RaiseClock(pAd, &x);
		LowerClock(pAd, &x);
	}

	/* output the read_opcode and register number in that order    */
	ShiftOutBits(pAd, EEPROM_READ_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);

	/* Now read the data (16 bits) in from the selected EEPROM word*/
	data = ShiftInBits(pAd);

	EEpromCleanup(pAd);


	*pValue = data;

	return NDIS_STATUS_SUCCESS;
}


int rtmp_ee_prom_write16(
    IN RTMP_ADAPTER *pAd,
    IN USHORT Offset,
    IN USHORT Data)
{
	UINT32 x;



	Offset /= 2;

	EWEN(pAd);

	/* reset bits and set EECS*/
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	/* patch can not access e-Fuse issue*/
	if (IS_RT2860(pAd))
	{
		/* kick a pulse*/
		RaiseClock(pAd, &x);
		LowerClock(pAd, &x);
	}
	
	/* output the read_opcode ,register number and data in that order */
	ShiftOutBits(pAd, EEPROM_WRITE_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	ShiftOutBits(pAd, Data, 16);		/* 16-bit access*/

	/* read DO status*/
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	EEpromCleanup(pAd);

	RtmpusecDelay(10000);	/*delay for twp(MAX)=10ms*/

	EWDS(pAd);

	EEpromCleanup(pAd);


	return NDIS_STATUS_SUCCESS;
	
}


INT rtmp_ee_write_to_prom(
	IN PRTMP_ADAPTER 	pAd)
{
	USHORT value, offset = 0;
	
	{
		for (offset = 0; offset < (EEPROM_SIZE >> 1);)
		{
			value = ((PUSHORT)pAd->EEPROMImage)[offset];
			//RT28xx_EEPROM_WRITE16(pAd, (offset << 1), value);
			rtmp_ee_prom_write16(pAd,(offset << 1), value);
			offset++;
		}
	}

	return TRUE;
}
