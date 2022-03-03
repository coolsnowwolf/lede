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
	ee_efuse.c

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RTMP_EFUSE_SUPPORT

#include	"rt_config.h"

static VOID eFuseWritePhysical( 
	IN	PRTMP_ADAPTER	pAd,	
  	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
  	PUCHAR lpOutBuffer,
  	ULONG nOutBufferSize);


static VOID EFUSE_IO_READ32(PRTMP_ADAPTER pAd, UINT32 reg, UINT32 *value)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
#ifdef RTMP_MAC_PCI
		UINT32 RestoreValue;
		RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_EEF_BASE - 0x70000);
		RTMP_IO_READ32(pAd, 0x80000 + (reg - MT_EEF_BASE) + 0x70000, value);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
#endif

	}
	else
#endif /* MT_MAC */
		RTMP_IO_READ32(pAd, reg, value);
}


static VOID EFUSE_IO_WRITE32(PRTMP_ADAPTER pAd, UINT32 reg, UINT32 value)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
#ifdef RTMP_MAC_PCI
		UINT32 RestoreValue;
		RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_EEF_BASE - 0x70000);
		RTMP_IO_WRITE32(pAd, 0x80000 + (reg - MT_EEF_BASE) + 0x70000, value);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
#endif

	}
	else
#endif /* MT_MAC */
		RTMP_IO_WRITE32(pAd, reg, value);
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	Note:
	
========================================================================
*/
UCHAR eFuseReadRegisters(PRTMP_ADAPTER pAd, UINT16 Offset, UINT16 Length, UINT16* pData)
{
	EFUSE_CTRL_STRUC eFuseCtrlStruc;
	INT32 i;
	UINT32 efuseDataOffset;
	UINT32 data=0;
	UINT32 efuse_ctrl_reg = EFUSE_CTRL;
    int ret;
	RTMP_SEM_EVENT_WAIT(&(pAd->e2p_read_lock), ret);

	if(ret != 0)
		DBGPRINT(RT_DEBUG_ERROR, ("%s:(%d) RTMP_SEM_EVENT_WAIT failed!\n",__FUNCTION__,ret));
    


#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		efuse_ctrl_reg = MT_EFUSE_CTRL; 
#endif /* MT_MAC */

	EFUSE_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);
	
	/* Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	/*Use the eeprom logical address and covert to address to block number*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/* Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 0.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 0;

	/* Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;
	
	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	EFUSE_IO_WRITE32(pAd, efuse_ctrl_reg, data);

	
	/* Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{	
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			RTMP_SEM_EVENT_UP(&(pAd->e2p_read_lock));
			return 0x3f;
		}
		/*rtmp.HwMemoryReadDword(EFUSE_CTRL, (DWORD *) &eFuseCtrlStruc, 4);*/
		EFUSE_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);
		
		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RtmpusecDelay(2);
			break;
		}	
		RtmpusecDelay(2);
		i++;	
	}
	
	/*if EFSROM_AOUT is not found in physical address, write 0xffff*/
	if (eFuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
	{
		for(i = 0; i < Length / 2; i++) {
			*(pData +2 * i) = 0xffff;
        }
			
		RTMP_SEM_EVENT_UP(&(pAd->e2p_read_lock));
    	return 0x3f;
	} else {
#ifdef MT_MAC
        if (pAd->chipCap.hif_type == HIF_MT) {
            if (!eFuseCtrlStruc.field.EFSROM_DOUT_VLD) {
            
				for(i = 0; i < Length / 2; i++){
					*(pData +2 * i) = 0xffff;
				}

    	        RTMP_SEM_EVENT_UP(&(pAd->e2p_read_lock));
                return 0x3f;
            }
        }
#endif /* MT_MAC */

		/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x590-0x59C)*/
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			efuseDataOffset =  MT_EFUSE_RDATA0 + (Offset & 0xC);
		else
#endif /* MT_MAC */
            efuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC);
		

		/*data hold 4 bytes data.*/
		/*In EFUSE_IO_READ32 will automatically execute 32-bytes swapping*/
		
		EFUSE_IO_READ32(pAd, efuseDataOffset, &data);
		
		/*Decide the upper 2 bytes or the bottom 2 bytes.*/
		/* Little-endian		S	|	S	Big-endian*/
		/* addr	3	2	1	0	|	0	1	2	3*/
		/* Ori-V	D	C	B	A	|	A	B	C	D*/
		/*After swapping*/
		/*		D	C	B	A	|	D	C	B	A*/
		/*Return 2-bytes*/
		/*The return byte statrs from S. Therefore, the little-endian will return BA, the Big-endian will return DC.*/
		/*For returning the bottom 2 bytes, the Big-endian should shift right 2-bytes.*/
#ifdef RT_BIG_ENDIAN
		data = data << (8*((Offset & 0x3)^0x2));		  
#else
		data = data >> (8*(Offset & 0x3));		
#endif /* RT_BIG_ENDIAN */
		
		NdisMoveMemory(pData, &data, Length);
		
	}

	RTMP_SEM_EVENT_UP(&(pAd->e2p_read_lock));
    return eFuseCtrlStruc.field.EFSROM_AOUT;
}


VOID EfusePhysicalReadRegisters(PRTMP_ADAPTER pAd, UINT16 Offset, 
								UINT16 Length, UINT16* pData)
{
	EFUSE_CTRL_STRUC EfuseCtrlStruc;
	INT32 Index;
	UINT32 EfuseDataOffset;
	UINT32 Data;
	UINT32 EfuseCtrlReg = EFUSE_CTRL;


#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		EfuseCtrlReg = MT_EFUSE_CTRL; 
#endif /* MT_MAC */

	EFUSE_IO_READ32(pAd, EfuseCtrlReg, &EfuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	EfuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
	/*Read in physical view*/
	EfuseCtrlStruc.field.EFSROM_MODE = 1;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	EfuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&Data, &EfuseCtrlStruc, 4);	
	EFUSE_IO_WRITE32(pAd, EfuseCtrlReg, Data);	

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	Index = 0;
	
	while (Index < 500)
	{	
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;
		
		EFUSE_IO_READ32(pAd, EfuseCtrlReg, &EfuseCtrlStruc.word);
		
		if(EfuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RtmpusecDelay(2);
			break;
		}
		
		RtmpusecDelay(2);
		Index++;
	}

	DBGPRINT(RT_DEBUG_INFO, ("EfuseCtrlStruc.field.EFSROM_AOUT = %x\n", EfuseCtrlStruc.field.EFSROM_AOUT));

	DBGPRINT(RT_DEBUG_INFO, ("EfuseCtrlStruc.field.EFSROM_DOUT_VLD = %x\n", EfuseCtrlStruc.field.EFSROM_DOUT_VLD));

	/*if EFSROM_AOUT is not found in physical address, write 0xffff*/
	if (EfuseCtrlStruc.field.EFSROM_AOUT == 0x3f)
	{
		for(Index = 0; Index < Length / 2; Index++) 
		{
			*(pData + 2 * Index) = 0xffff;
        }
			DBGPRINT(RT_DEBUG_ERROR, ("EfuseCtrlStruc.field.EFSROM_AOUT = %x\n", EfuseCtrlStruc.field.EFSROM_AOUT));
        	return;
	} 
	else 
	{
        if (pAd->chipCap.hif_type == HIF_MT) 
		{
            if (!EfuseCtrlStruc.field.EFSROM_DOUT_VLD) {
				
				for(Index = 0; Index < Length / 2; Index++) 
				{
					*(pData + 2 * Index) = 0xffff;
        		}
				
				DBGPRINT(RT_DEBUG_OFF, ("EfuseCtrlStruc.field.EFSROM_DOUT_VLD = %x\n", EfuseCtrlStruc.field.EFSROM_DOUT_VLD));
				return;
            }
        }
	}


	/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
	/*Because the size of each EFUSE_DATA is 4 Bytes, the size of address of each is 2 bits.*/
	/*The previous 2 bits is the EFUSE_DATA number, the last 2 bits is used to decide which bytes*/
	/*Decide which EFUSE_DATA to read*/
	/*590:F E D C */
	/*594:B A 9 8 */
	/*598:7 6 5 4*/
	/*59C:3 2 1 0*/
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		EfuseDataOffset =  MT_EFUSE_RDATA0 + (Offset & 0xC);
	else
#endif /* MT_MAC */
	EfuseDataOffset =  EFUSE_DATA3 - (Offset & 0xC)  ;	

	EFUSE_IO_READ32(pAd, EfuseDataOffset , &Data);
	
#ifdef RT_BIG_ENDIAN
		Data = Data << (8 *((Offset & 0x3)^0x2));	
#else
	Data = Data >> (8 * (Offset & 0x3));
#endif /* RT_BIG_ENDIAN */

	NdisMoveMemory(pData, &Data, Length);
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
VOID eFuseReadPhysical( 
	IN	PRTMP_ADAPTER	pAd, 
  	IN	PUSHORT lpInBuffer,
  	IN	ULONG nInBufferSize,
  	OUT	PUSHORT lpOutBuffer,
  	IN	ULONG nOutBufferSize  
)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	USHORT* pOutBuf = (USHORT*)lpOutBuffer;

	USHORT Offset = pInBuf[0];					/*addr*/
	USHORT Length = pInBuf[1];					/*length*/
	INT32 i;
	
	for(i = 0; i < Length; i+=2)
	{
		EfusePhysicalReadRegisters(pAd, Offset + i, 2, &pOutBuf[i/2]);
	}
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
NTSTATUS eFuseRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUSHORT			pData,
	IN	USHORT			Length)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int	i;
	
	for(i = 0; i < Length; i+=2)
	{
		eFuseReadRegisters(pAd, Offset+i, 2, &pData[i/2]);
	} 
	return Status;
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
static VOID eFusePhysicalWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,	
	IN	USHORT Offset, 
	IN	USHORT Length, 
	OUT	USHORT* pData)
{
	EFUSE_CTRL_STRUC		eFuseCtrlStruc;
	int	i;
	UINT32	efuseDataOffset;
	UINT32 efuse_rdata = EFUSE_DATA3, efuse_wdata = EFUSE_DATA3;
	UINT32	data, eFuseDataBuffer[4];
	UINT32 efuse_ctrl_reg = EFUSE_CTRL;


#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		efuse_ctrl_reg = MT_EFUSE_CTRL;
		efuse_rdata = MT_EFUSE_RDATA0;
		efuse_wdata = MT_EFUSE_WDATA0;
	}
#endif /* MT_MAC */

	/*Step0. Write 16-byte of data to EFUSE_DATA0-3 (0x590-0x59C), where EFUSE_DATA0 is the LSB DW, EFUSE_DATA3 is the MSB DW.*/

	/*read current values of 16-byte block	*/
	EFUSE_IO_READ32(pAd, efuse_ctrl_reg,  &eFuseCtrlStruc.word);

	/*Step0. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step1. Write EFSROM_MODE (0x580, bit7:bit6) to 1.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 1;

	/*Step2. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical read procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);
	EFUSE_IO_WRITE32(pAd, efuse_ctrl_reg, data);	

	/*Step3. Polling EFSROM_KICK(0x580, bit30) until it become 0 again.*/
	i = 0;
	while(i < 500)
	{	
		EFUSE_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RtmpusecDelay(2);
			break;
		}
		RtmpusecDelay(2);
		i++;	
	}

	/*Step4. Read 16-byte of data from EFUSE_DATA0-3 (0x59C-0x590)*/
	efuseDataOffset =  efuse_rdata;
	
	for (i = 0; i < 4; i++)
	{
		EFUSE_IO_READ32(pAd, efuseDataOffset, (PUINT32) &eFuseDataBuffer[i]);
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			efuseDataOffset += 4;
		else
#endif /* MT_MAC */
			efuseDataOffset -= 4;
	}

	/*Update the value, the offset is multiple of 2, length is 2*/
	efuseDataOffset = (Offset & 0xc) >> 2;
	data = pData[0] & 0xffff;
	/*The offset should be 0x***10 or 0x***00*/
	if((Offset % 4) != 0)
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff) | (data << 16);
	}
	else
	{
		eFuseDataBuffer[efuseDataOffset] = (eFuseDataBuffer[efuseDataOffset] & 0xffff0000) | data;
	}

	efuseDataOffset =  efuse_wdata;

	for(i = 0; i < 4; i++)
	{
		EFUSE_IO_WRITE32(pAd, efuseDataOffset, eFuseDataBuffer[i]);
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			efuseDataOffset += 4;
		else
#endif /* MT_MAC */
			efuseDataOffset -= 4;		
	}

	/*Step1. Write 10-bit of address to EFSROM_AIN (0x580, bit25:bit16). The address must be 16-byte alignment.*/
	// TODO: shiang, for below line, windows driver didn't have this read, why we have ??
	EFUSE_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

	eFuseCtrlStruc.field.EFSROM_AIN = Offset & 0xfff0;

	/*Step2. Write EFSROM_MODE (0x580, bit7:bit6) to 3.*/
	eFuseCtrlStruc.field.EFSROM_MODE = 3;
	
	/*Step3. Write EFSROM_KICK (0x580, bit30) to 1 to kick-off physical write procedure.*/
	eFuseCtrlStruc.field.EFSROM_KICK = 1;

	NdisMoveMemory(&data, &eFuseCtrlStruc, 4);	
	EFUSE_IO_WRITE32(pAd, efuse_ctrl_reg, data);	

	/*Step4. Polling EFSROM_KICK(0x580, bit30) until it become 0 again. It's done.*/
	i = 0;

	while (i < 500)
	{	
		EFUSE_IO_READ32(pAd, efuse_ctrl_reg, &eFuseCtrlStruc.word);

		if(eFuseCtrlStruc.field.EFSROM_KICK == 0)
		{
			RtmpusecDelay(2);
			break;
		}
		
		RtmpusecDelay(2);	
		i++;	
	}
}

/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:

	Note:
	
========================================================================
*/
static NTSTATUS eFuseWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset, 
	IN	USHORT Length, 
	IN	USHORT* pData)
{
	USHORT	i,Loop=0, StartBlock=0, EndBlock=0;
	USHORT	eFuseData[2];
	USHORT	LogicalAddress, BlkNum = 0xffff;
	UCHAR	EFSROM_AOUT;

	USHORT addr,tmpaddr, InBuf[3], tmpOffset;
	USHORT buffer[8];
	BOOLEAN		bWriteSuccess = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters Offset=%x, pData=%x\n", Offset, *pData));
	/*set start block and end block number, start from tail of mapping table*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1; 
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_END; 
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1; 
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_START; 
	}
	/*Step 0. find the entry in the mapping table*/
	/*The address of EEPROM is 2-bytes alignment.*/
	/*The last bit is used for alignment, so it must be 0.*/
	tmpOffset = Offset & 0xfffe;
	EFSROM_AOUT = eFuseReadRegisters(pAd, tmpOffset, 2, &eFuseData[0]);
	if (EFSROM_AOUT == 0x3f)
	{	/*find available logical address pointer	*/
		/*the logical address does not exist, find an empty one*/
		/*from the first address of block 45=16*45=0x2d0 to the last address of block 47*/
		/*==>48*16-3(reserved)=2FC*/
		for (i=StartBlock; i >= EndBlock; i-=2)
		{
			/*Retrive the logical block nubmer form each logical address pointer*/
			/*It will access two logical address pointer each time.*/
			EfusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
			/*To avoid the odd byte problem, ex. We read the 21|20 bytes and if 21 is the */
			/* end byte. Then, the EFUSE_USAGE_MAP_END which is 21 is not equal to*/
			/* i which is 20. Therefore, this 21th byte could be used.*/
			/*Otherwise, if 20 is the stop byte, i which is 20 is equal EFUSE_USAGE_MAP_END.*/
			/* It means the 21th byte could not be used.*/
			if(( (LogicalAddress >> 8) & 0xff) == 0)
			{/*Not used logical address pointer*/
				if (i != pAd->chipCap.EFUSE_USAGE_MAP_END)
				{		
					BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START+1;	
					break;
				}				
				
			}
			
			if( (LogicalAddress & 0xff) == 0)
			{/*Not used logical address pointer*/
				if (i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
				{
					BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
					break;
				}
			}
			
		}
	}
	else
	{
		BlkNum = EFSROM_AOUT;
	}	

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters BlkNum = %d \n", BlkNum));

	if(BlkNum == 0xffff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
		return FALSE;
	}	

	/*Step 1. Save data of this block	which is pointed by the avaible logical address pointer*/
	/* read and save the original block data*/
	for(i =0; i<8; i++)
	{
		addr = BlkNum * 0x10 ;
		
		InBuf[0] = addr+2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;	
		
		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

		buffer[i] = InBuf[2];
	}

	/*Step 2. Update the data in buffer, and write the data to Efuse*/
	buffer[ (Offset >> 1) % 8] = pData[0];

	do
	{	Loop++;
		/*Step 3. Write the data to Efuse*/
		if(!bWriteSuccess)
		{
			for(i =0; i<8; i++)
			{
				addr = BlkNum * 0x10 ;
				
				InBuf[0] = addr+2*i;
				InBuf[1] = 2;
				InBuf[2] = buffer[i];	
				
				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);		
			}
		}
		else
		{
				addr = BlkNum * 0x10 ;
				
				InBuf[0] = addr+(Offset % 16);
				InBuf[1] = 2;
				InBuf[2] = pData[0];	
				
				eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 2);	
		}
	
		/*Step 4. Write mapping table*/
		addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;

		tmpaddr = addr;

		if(addr % 2 != 0)
			addr = addr -1; 
		InBuf[0] = addr;
		InBuf[1] = 2;

		/*convert the address from 10 to 8 bit ( bit7, 6 = parity and bit5 ~ 0 = bit9~4), and write to logical map entry*/
		tmpOffset = Offset;
		tmpOffset >>= 4;
		tmpOffset |= ((~((tmpOffset & 0x01) ^ ( tmpOffset >> 1 & 0x01) ^  (tmpOffset >> 2 & 0x01) ^  (tmpOffset >> 3 & 0x01))) << 6) & 0x40;
		tmpOffset |= ((~( (tmpOffset >> 2 & 0x01) ^ (tmpOffset >> 3 & 0x01) ^ (tmpOffset >> 4 & 0x01) ^ ( tmpOffset >> 5 & 0x01))) << 7) & 0x80;

		/* write the logical address*/
		if(tmpaddr%2 != 0) 	
			InBuf[2] = tmpOffset<<8;	
		else          
			InBuf[2] = tmpOffset;

		eFuseWritePhysical(pAd,&InBuf[0], 6, NULL, 0);

		/*Step 5. Compare data if not the same, invalidate the mapping entry, then re-write the data until E-fuse is exhausted*/
		bWriteSuccess = TRUE;
		for(i =0; i<8; i++)
		{
			addr = BlkNum * 0x10 ;
			
			InBuf[0] = addr+2*i;
			InBuf[1] = 2;
			InBuf[2] = 0x0;	
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);

			if(buffer[i] != InBuf[2])
			{
				bWriteSuccess = FALSE;
				break;
			}	
		}

		/*Step 6. invlidate mapping entry and find a free mapping entry if not succeed*/
		if (!bWriteSuccess)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess BlkNum = %d\n", BlkNum));

			/* the offset of current mapping entry*/
			addr = pAd->chipCap.EFUSE_USAGE_MAP_START+BlkNum;

			/*find a new mapping entry*/
			BlkNum = 0xffff;
			for (i=StartBlock; i >= EndBlock; i-=2)
			{
				EfusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				if(( (LogicalAddress >> 8) & 0xff) == 0)
				{
					if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
					{
						BlkNum = i+1-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}	
				}

				if( (LogicalAddress & 0xff) == 0)
				{
					if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
					{
						BlkNum = i-pAd->chipCap.EFUSE_USAGE_MAP_START;
						break;
					}
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Not bWriteSuccess and allocate new BlkNum = %d\n", BlkNum));	
			if(BlkNum == 0xffff)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("eFuseWriteRegisters: out of free E-fuse space!!!\n"));
				return FALSE;
			}

			/*invalidate the original mapping entry if new entry is not found*/
			tmpaddr = addr;

			if(addr % 2 != 0)
				addr = addr -1; 
			InBuf[0] = addr;
			InBuf[1] = 2;		
			
			eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);				

			/* write the logical address*/
			if(tmpaddr%2 != 0) 
			{
				/* Invalidate the high byte*/
				for (i=8; i<15; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}		
			}	
			else
			{
				/* invalidate the low byte*/
				for (i=0; i<8; i++)
				{
					if( ( (InBuf[2] >> i) & 0x01) == 0)
					{
						InBuf[2] |= (0x1 <<i);
						break;
					}	
				}					
			}
			eFuseWritePhysical(pAd, &InBuf[0], 6, NULL, 0);	
		}	
	}	
	while (!bWriteSuccess&&Loop<2);	
	if(!bWriteSuccess)
		DBGPRINT(RT_DEBUG_ERROR,("Efsue Write Failed!!\n"));
	return TRUE;
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
static VOID eFuseWritePhysical( 
	IN	PRTMP_ADAPTER	pAd,	
  	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
  	PUCHAR lpOutBuffer,
  	ULONG nOutBufferSize  
)
{
	USHORT* pInBuf = (USHORT*)lpInBuffer;
	int 		i;
	/*USHORT* pOutBuf = (USHORT*)ioBuffer;*/
	USHORT Offset = pInBuf[0];					/* addr*/
	USHORT Length = pInBuf[1];					/* length*/
	USHORT* pValueX = &pInBuf[2];				/* value ...		*/


	DBGPRINT(RT_DEBUG_INFO, ("eFuseWritePhysical Offset=0x%x, length=%d\n", Offset, Length));

	{


		/* Little-endian		S	|	S	Big-endian*/
		/* addr	3	2	1	0	|	0	1	2	3*/
		/* Ori-V	D	C	B	A	|	A	B	C	D*/
		/* After swapping*/
		/*		D	C	B	A	|	D	C	B	A*/
		/* Both the little and big-endian use the same sequence to write  data.*/
		/* Therefore, we only need swap data when read the data.*/
		for (i=0; i<Length; i+=2)
		{
			eFusePhysicalWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);
		}


	}
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
NTSTATUS eFuseWrite(  
   	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUSHORT			pData,
	IN	USHORT			length)
{
	int i;
	USHORT* pValueX = (PUSHORT) pData;				/*value ...		*/
	PUSHORT OddWriteByteBuf;

/*	OddWriteByteBuf=(PUSHORT)kmalloc(sizeof(USHORT)*2, MEM_ALLOC_FLAG);*/
	os_alloc_mem(NULL, (UCHAR **)&OddWriteByteBuf, sizeof(USHORT)*2);
	/* The input value=3070 will be stored as following*/
	/* Little-endian		S	|	S	Big-endian*/
	/* addr			1	0	|	0	1	*/
	/* Ori-V			30	70	|	30	70	*/
	/* After swapping*/
	/*				30	70	|	70	30*/
	/* Casting*/
	/*				3070	|	7030 (x)*/
	/* The swapping should be removed for big-endian*/
	if (OddWriteByteBuf == NULL)
		return FALSE;
	if((Offset%2)!=0)
	{
		length+=2;
		Offset-=1;
		eFuseRead(pAd,Offset,OddWriteByteBuf,2);
		eFuseRead(pAd,Offset+2,(OddWriteByteBuf+1),2);
		*OddWriteByteBuf&=0x00ff;
		*OddWriteByteBuf|=((*pData)&0xff)<<8;
		*(OddWriteByteBuf+1)&=0xff00;
		*(OddWriteByteBuf+1)|=(*pData&0xff00)>>8;
		pValueX=OddWriteByteBuf;
		
	}
	
	for(i=0; i<length; i+=2)
	{
		eFuseWriteRegisters(pAd, Offset+i, 2, &pValueX[i/2]);	
	}
	os_free_mem(NULL, OddWriteByteBuf);
	return TRUE;
}


/*
========================================================================
	
	Routine Description:

	Arguments:

	Return Value:
	
	Note:
	
========================================================================
*/
INT set_eFuseGetFreeBlockCount_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT free_num = 0;
	
	if (pAd->bUseEfuse == FALSE)
		return FALSE;
	
	eFuseGetFreeBlockCount(pAd, &free_num);
	DBGPRINT(RT_DEBUG_OFF, ("efuseFreeNumber = %d\n", free_num));
	return TRUE;
}


INT set_eFusedump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	USHORT InBuf[3];
	INT i = 0;
	
	if (pAd->bUseEfuse == FALSE)
		return FALSE;
	
	for (i = 0; i < pAd->chipCap.EFUSE_USAGE_MAP_END/2; i++)
	{
		InBuf[0] = 2*i;
		InBuf[1] = 2;
		InBuf[2] = 0x0;	
		
		eFuseReadPhysical(pAd, &InBuf[0], 4, &InBuf[2], 2);
		
		if (i%4 == 0)
			DBGPRINT(RT_DEBUG_OFF, ("\nBlock %x:", i/8));
		DBGPRINT(RT_DEBUG_OFF, ("%04x ", InBuf[2]));
	}
	return TRUE;
}


INT	set_eFuseLoadFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *src;
	RTMP_OS_FD				srcf;
	RTMP_OS_FS_INFO			osfsInfo;
	INT 						retval, memSize;
	RTMP_STRING *buffer, *memPtr;
	INT						TotalByte= 0;
	//USHORT					*PDATA;
	UCHAR					all_ff[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	UCHAR					*ptr;
	UCHAR					index;
	USHORT					offset = 0;
	USHORT 					value;
	
	memSize = 128 + MAX_EEPROM_BIN_FILE_SIZE + sizeof(USHORT) * 8;
/*	memPtr = kmalloc(memSize, MEM_ALLOC_FLAG);*/
	os_alloc_mem(NULL, (UCHAR **)&memPtr, memSize);
	if (memPtr == NULL)
		return FALSE;

	NdisZeroMemory(memPtr, memSize);
	src = memPtr; /* kmalloc(128, MEM_ALLOC_FLAG);*/
	buffer = src + 128;		/* kmalloc(MAX_EEPROM_BIN_FILE_SIZE, MEM_ALLOC_FLAG);*/
	//PDATA = (USHORT*)(buffer + MAX_EEPROM_BIN_FILE_SIZE);	/* kmalloc(sizeof(USHORT)*8,MEM_ALLOC_FLAG);*/
	ptr = buffer;
	
 	if(strlen(arg)>0)
		NdisMoveMemory(src, arg, strlen(arg));
	else
		NdisMoveMemory(src, EEPROM_DEFULT_BIN_FILE, strlen(EEPROM_DEFULT_BIN_FILE));
	DBGPRINT(RT_DEBUG_OFF, ("FileName=%s\n",src));

	RtmpOSFSInfoChange(&osfsInfo, TRUE);

	srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(srcf)) 
	{
		DBGPRINT_ERR(("--> Error opening file %s\n", src));
		retval = FALSE;
		goto recoverFS;
	}
	else 
	{
		/* The object must have a read method*/
		while(RtmpOSFileRead(srcf, &buffer[TotalByte], 1)==1)
		{
          		TotalByte++;
			if(TotalByte>MAX_EEPROM_BIN_FILE_SIZE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("--> Error reading file %s, file size too large[>%d]\n", src, MAX_EEPROM_BIN_FILE_SIZE));
				retval = FALSE;
				goto closeFile;
			}
		}

		retval = RtmpOSFileClose(srcf);
		if (retval)
			DBGPRINT(RT_DEBUG_TRACE, ("--> Error closing file %s\n", src));
  	}


	RtmpOSFSInfoChange(&osfsInfo, FALSE);

	for ( offset = 0 ; offset < TotalByte ; offset += 16 )
	{
		if ( memcmp( ptr, all_ff, 16 ) )
		{
			DBGPRINT(RT_DEBUG_TRACE, ("offset 0x%04x: ", offset));	
			for ( index = 0 ; index < 16 ; index += 2 )
			{
				value = *(USHORT *)(ptr + index);
				//eFuseWrite(pAd, offset + index ,&value, 2);
				DBGPRINT(RT_DEBUG_TRACE, ("0x%04x ", value));	
			}
			DBGPRINT(RT_DEBUG_TRACE, ("\n"));	
		}
		
		ptr += 16;
	}

	return TRUE;

closeFile:
	if (srcf)
		if (RtmpOSFileClose(srcf) != 0)
			retval = FALSE;

recoverFS:
	RtmpOSFSInfoChange(&osfsInfo, FALSE);
	

	if (memPtr)
/*		kfree(memPtr);*/
		os_free_mem(NULL, memPtr);
	
	return retval;
}



BOOLEAN rtmp_ee_efuse_read16(RTMP_ADAPTER *pAd, UINT16 Offset, UINT16 *pValue)
{
	UCHAR Value;

	Value = eFuseReadRegisters(pAd, Offset, 2, pValue);
	
	if (Value == 0x3F)
	{
		DBGPRINT(RT_DEBUG_INFO, ("%s:  Not found valid block for this offset (%x)\n", __FUNCTION__, Offset));
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


int rtmp_ee_efuse_write16(
	IN RTMP_ADAPTER *pAd, 
	IN USHORT Offset, 
	IN USHORT data)
{
	eFuseWrite(pAd,Offset ,&data, 2);
	return 0;
}

int RtmpEfuseSupportCheck(
	IN RTMP_ADAPTER *pAd)
{
	USHORT value;
	
	if (IS_RT30xx(pAd) || IS_RT3593(pAd))
	{
		EfusePhysicalReadRegisters(pAd, EFUSE_TAG, 2, &value);
		pAd->EFuseTag = (value & 0xff);
	}
	return 0;
}

INT rtmp_ee_write_to_efuse(
	IN PRTMP_ADAPTER 	pAd)
{
	USHORT	length = pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE;
	UCHAR	*ptr = pAd->EEPROMImage;
	UCHAR	index;
	UCHAR	all_ff[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	USHORT offset = 0;
	USHORT value;

	for ( offset = 0 ; offset < length ; offset += 16 )
	{
		if ( memcmp( ptr, all_ff, 16 ) )
		{
			DBGPRINT(RT_DEBUG_TRACE, ("offset 0x%04x: ", offset));	
			for ( index = 0 ; index < 16 ; index += 2 )
			{
				value = le2cpu16(*(USHORT *)(ptr + index));
				eFuseWrite(pAd, offset + index ,&value, 2);
				DBGPRINT(RT_DEBUG_TRACE, ("0x%04x ", value));	
			}
			DBGPRINT(RT_DEBUG_TRACE, ("\n"));	
		}
		
		ptr += 16;
	}

	return TRUE;

}


VOID eFuseGetFreeBlockCount(IN PRTMP_ADAPTER pAd, 
	PUINT EfuseFreeBlock)
{
	
	USHORT i=0, StartBlock=0, EndBlock=0;
	USHORT	LogicalAddress;
	USHORT	FirstFreeBlock = 0xffff, LastFreeBlock = 0xffff;

#ifndef CAL_FREE_IC_SUPPORT
	if(!pAd->bUseEfuse)
		{
		DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount Only supports efuse Mode\n"));
		return ;
		}
#endif /* CAL_FREE_IC_SUPPORT */
	*EfuseFreeBlock = 0;


	/* find first free block*/
	if( (pAd->chipCap.EFUSE_USAGE_MAP_START % 2) != 0)
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START-1; 
	}
	else
	{
		StartBlock = pAd->chipCap.EFUSE_USAGE_MAP_START; 
	}

	if( (pAd->chipCap.EFUSE_USAGE_MAP_END % 2) != 0)
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END-1; 
	}
	else
	{
		EndBlock = pAd->chipCap.EFUSE_USAGE_MAP_END; 
	}

	for (i = StartBlock; i <= EndBlock; i+=2)
	{
		EfusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
		
		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				FirstFreeBlock = i;
				break;
			}
		}		

		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				FirstFreeBlock = i+1;
				break;
			}	
		}			
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, FirstFreeBlock= 0x%x\n", FirstFreeBlock));

	/*if not find, return free block number = 0*/
	if(FirstFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;	
	}
	for (i = EndBlock; i >= StartBlock; i-=2)
	{
		EfusePhysicalReadRegisters(pAd, i, 2, &LogicalAddress);
				
		if(( (LogicalAddress >> 8) & 0xff) == 0)
		{
			if(i != pAd->chipCap.EFUSE_USAGE_MAP_END)
			{
				LastFreeBlock = i+1;
				break;
			}	
		}

		if( (LogicalAddress & 0xff) == 0)
		{
			if(i != (pAd->chipCap.EFUSE_USAGE_MAP_START-1))
			{
				LastFreeBlock = i;
				break;
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("eFuseGetFreeBlockCount, LastFreeBlock= 0x%x\n", LastFreeBlock));

	/*if not find last free block, return free block number = 0, this should not happen since we have checked first free block number previously*/
	if(LastFreeBlock == 0xffff)
	{
		*EfuseFreeBlock = 0;
		return;	
	}

	/* return total free block number, last free block number must >= first free block number*/
	if(LastFreeBlock < FirstFreeBlock)
	{
		*EfuseFreeBlock = 0;	
	}
	else
	{
		*EfuseFreeBlock = LastFreeBlock - FirstFreeBlock + 1;
	}
	
	DBGPRINT(RT_DEBUG_TRACE,("eFuseGetFreeBlockCount is %d\n",*EfuseFreeBlock));
}


INT eFuse_init(RTMP_ADAPTER *pAd)
{
	UINT EfuseFreeBlock=0;
	BOOLEAN bCalFree;

	/*RT3572 means 3062/3562/3572*/
	/*3593 means 3593*/
	DBGPRINT(RT_DEBUG_ERROR, ("NVM is Efuse and its size =%x[%x-%x] \n",pAd->chipCap.EFUSE_USAGE_MAP_SIZE,pAd->chipCap.EFUSE_USAGE_MAP_START,pAd->chipCap.EFUSE_USAGE_MAP_END));
	eFuseGetFreeBlockCount(pAd, &EfuseFreeBlock);
	/*If the used block of efuse is less than 5. We assume the default value*/
	/* of this efuse is empty and change to the buffer mode in odrder to */
	/*bring up interfaces successfully.*/
	
	
	if (EfuseFreeBlock >= pAd->chipCap.EFUSE_RESERVED_SIZE)
	{


		DBGPRINT(RT_DEBUG_OFF, ("NVM is efuse and the information is too less to bring up the interface\n"));
		DBGPRINT(RT_DEBUG_OFF, ("Load EEPROM buffer from BIN, and force to use BIN buffer mode\n"));

		pAd->bUseEfuse = FALSE;
		rtmp_ee_load_from_bin(pAd);

		/* Forse to use BIN eeprom buffer mode */
		RtmpChipOpsEepromHook(pAd, pAd->infType,E2P_BIN_MODE);

#ifdef CAL_FREE_IC_SUPPORT
		RTMP_CAL_FREE_IC_CHECK(pAd,bCalFree);
		if (bCalFree)
		{
			DBGPRINT(RT_DEBUG_OFF, ("Cal Free IC!!\n"));
			RTMP_CAL_FREE_DATA_GET(pAd);
		}
		else
		{
			DBGPRINT(RT_DEBUG_OFF, ("Non Cal Free IC!!\n"));
		}
#endif /* CAL_FREE_IC_SUPPORT */

	}
	else
	{
		rtmp_ee_load_from_efuse(pAd);
	}

	return 0;
}


INT efuse_probe(RTMP_ADAPTER *pAd)
{
	UINT32 eFuseCtrl, ctrl_reg;

	if (WaitForAsicReady(pAd) == FALSE)
		return -1;

	pAd->bUseEfuse=FALSE;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		ctrl_reg = MT_EE_CTRL;
	else
#endif /* MT_MAC */
		ctrl_reg = EFUSE_CTRL;

	EFUSE_IO_READ32(pAd, ctrl_reg, &eFuseCtrl);

	printk("%s: efuse = %x\n", __FUNCTION__, eFuseCtrl);

	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if ((eFuseCtrl & 0x40000000) == 0x40000000)
			pAd->bUseEfuse = 0;
		else
			pAd->bUseEfuse = 1;
	}
	else
	{
		pAd->bUseEfuse = ( (eFuseCtrl & 0x80000000) == 0x80000000) ? 1 : 0;
	}

	return 0;
}


VOID  rtmp_ee_load_from_efuse(RTMP_ADAPTER *pAd)
{
	UINT16 efuse_val=0;
	UINT free_blk = 0;
	UINT i;

	DBGPRINT(RT_DEBUG_OFF,("Load EEPROM buffer from efuse, and change to BIN buffer mode\n"));

	/* If the number of the used block is less than 5, assume the efuse is not well-calibrated, and force to use buffer mode */
	eFuseGetFreeBlockCount(pAd, &free_blk);
	if (free_blk > (pAd->chipCap.EFUSE_USAGE_MAP_SIZE - 5))
		return ;

	NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
	for(i=0; i<MAX_EEPROM_BIN_FILE_SIZE; i+=2)
	{
			eFuseRead(pAd, i,&efuse_val, 2);
			efuse_val = cpu2le16 (efuse_val);
			NdisMoveMemory(&pAd->EEPROMImage[i],&efuse_val,2);
	}		
	
	/* Change to BIN eeprom buffer mode */
	RtmpChipOpsEepromHook(pAd, pAd->infType,E2P_BIN_MODE);
}


#ifdef CONFIG_ATE
INT Set_LoadEepromBufferFromEfuse_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);

	
	if (bEnable < 0)
		return FALSE;
	else
	{
		rtmp_ee_load_from_efuse(pAd);
		return TRUE;
	}
}


INT set_eFuseBufferModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	
	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s::Write EEPROM buffer back to eFuse\n", __FUNCTION__));	
		Set_EepromBufferWriteBack_Proc(pAd, "1");
		return TRUE;
	}
}


INT set_BinModeWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT bEnable = simple_strtol(arg, 0, 10);
	
	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s::Write EEPROM buffer back to BIN\n", __FUNCTION__));	
		Set_EepromBufferWriteBack_Proc(pAd, "4");
		
		return TRUE;
	}
}

#endif /* CONFIG_ATE */
#endif /* RTMP_EFUSE_SUPPORT */

