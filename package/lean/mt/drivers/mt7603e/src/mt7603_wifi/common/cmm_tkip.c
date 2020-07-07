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
	cmm_tkip.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Paul Wu		02-25-02		Initial
*/

#include	"rt_config.h"

/* Rotation functions on 32 bit values */
#define ROL32( A, n ) \
	( ((A) << (n)) | ( ((A)>>(32-(n))) & ( (1UL << (n)) - 1 ) ) ) 
#define ROR32( A, n ) ROL32( (A), 32-(n) ) 

UINT Tkip_Sbox_Lower[256] = 
{ 
	0xA5,0x84,0x99,0x8D,0x0D,0xBD,0xB1,0x54, 
	0x50,0x03,0xA9,0x7D,0x19,0x62,0xE6,0x9A, 
	0x45,0x9D,0x40,0x87,0x15,0xEB,0xC9,0x0B, 
	0xEC,0x67,0xFD,0xEA,0xBF,0xF7,0x96,0x5B, 
	0xC2,0x1C,0xAE,0x6A,0x5A,0x41,0x02,0x4F, 
	0x5C,0xF4,0x34,0x08,0x93,0x73,0x53,0x3F, 
	0x0C,0x52,0x65,0x5E,0x28,0xA1,0x0F,0xB5, 
	0x09,0x36,0x9B,0x3D,0x26,0x69,0xCD,0x9F, 
	0x1B,0x9E,0x74,0x2E,0x2D,0xB2,0xEE,0xFB, 
	0xF6,0x4D,0x61,0xCE,0x7B,0x3E,0x71,0x97, 
	0xF5,0x68,0x00,0x2C,0x60,0x1F,0xC8,0xED, 
	0xBE,0x46,0xD9,0x4B,0xDE,0xD4,0xE8,0x4A, 
	0x6B,0x2A,0xE5,0x16,0xC5,0xD7,0x55,0x94, 
	0xCF,0x10,0x06,0x81,0xF0,0x44,0xBA,0xE3, 
	0xF3,0xFE,0xC0,0x8A,0xAD,0xBC,0x48,0x04, 
	0xDF,0xC1,0x75,0x63,0x30,0x1A,0x0E,0x6D, 
	0x4C,0x14,0x35,0x2F,0xE1,0xA2,0xCC,0x39, 
	0x57,0xF2,0x82,0x47,0xAC,0xE7,0x2B,0x95, 
	0xA0,0x98,0xD1,0x7F,0x66,0x7E,0xAB,0x83, 
	0xCA,0x29,0xD3,0x3C,0x79,0xE2,0x1D,0x76, 
	0x3B,0x56,0x4E,0x1E,0xDB,0x0A,0x6C,0xE4, 
	0x5D,0x6E,0xEF,0xA6,0xA8,0xA4,0x37,0x8B, 
	0x32,0x43,0x59,0xB7,0x8C,0x64,0xD2,0xE0, 
	0xB4,0xFA,0x07,0x25,0xAF,0x8E,0xE9,0x18, 
	0xD5,0x88,0x6F,0x72,0x24,0xF1,0xC7,0x51, 
	0x23,0x7C,0x9C,0x21,0xDD,0xDC,0x86,0x85, 
	0x90,0x42,0xC4,0xAA,0xD8,0x05,0x01,0x12, 
	0xA3,0x5F,0xF9,0xD0,0x91,0x58,0x27,0xB9, 
	0x38,0x13,0xB3,0x33,0xBB,0x70,0x89,0xA7, 
	0xB6,0x22,0x92,0x20,0x49,0xFF,0x78,0x7A, 
	0x8F,0xF8,0x80,0x17,0xDA,0x31,0xC6,0xB8, 
	0xC3,0xB0,0x77,0x11,0xCB,0xFC,0xD6,0x3A 
};

UINT Tkip_Sbox_Upper[256] = 
{ 
	0xC6,0xF8,0xEE,0xF6,0xFF,0xD6,0xDE,0x91, 
	0x60,0x02,0xCE,0x56,0xE7,0xB5,0x4D,0xEC, 
	0x8F,0x1F,0x89,0xFA,0xEF,0xB2,0x8E,0xFB, 
	0x41,0xB3,0x5F,0x45,0x23,0x53,0xE4,0x9B, 
	0x75,0xE1,0x3D,0x4C,0x6C,0x7E,0xF5,0x83, 
	0x68,0x51,0xD1,0xF9,0xE2,0xAB,0x62,0x2A, 
	0x08,0x95,0x46,0x9D,0x30,0x37,0x0A,0x2F, 
	0x0E,0x24,0x1B,0xDF,0xCD,0x4E,0x7F,0xEA, 
	0x12,0x1D,0x58,0x34,0x36,0xDC,0xB4,0x5B, 
	0xA4,0x76,0xB7,0x7D,0x52,0xDD,0x5E,0x13, 
	0xA6,0xB9,0x00,0xC1,0x40,0xE3,0x79,0xB6, 
	0xD4,0x8D,0x67,0x72,0x94,0x98,0xB0,0x85, 
	0xBB,0xC5,0x4F,0xED,0x86,0x9A,0x66,0x11, 
	0x8A,0xE9,0x04,0xFE,0xA0,0x78,0x25,0x4B, 
	0xA2,0x5D,0x80,0x05,0x3F,0x21,0x70,0xF1, 
	0x63,0x77,0xAF,0x42,0x20,0xE5,0xFD,0xBF, 
	0x81,0x18,0x26,0xC3,0xBE,0x35,0x88,0x2E, 
	0x93,0x55,0xFC,0x7A,0xC8,0xBA,0x32,0xE6, 
	0xC0,0x19,0x9E,0xA3,0x44,0x54,0x3B,0x0B, 
	0x8C,0xC7,0x6B,0x28,0xA7,0xBC,0x16,0xAD, 
	0xDB,0x64,0x74,0x14,0x92,0x0C,0x48,0xB8, 
	0x9F,0xBD,0x43,0xC4,0x39,0x31,0xD3,0xF2, 
	0xD5,0x8B,0x6E,0xDA,0x01,0xB1,0x9C,0x49, 
	0xD8,0xAC,0xF3,0xCF,0xCA,0xF4,0x47,0x10, 
	0x6F,0xF0,0x4A,0x5C,0x38,0x57,0x73,0x97, 
	0xCB,0xA1,0xE8,0x3E,0x96,0x61,0x0D,0x0F, 
	0xE0,0x7C,0x71,0xCC,0x90,0x06,0xF7,0x1C, 
	0xC2,0x6A,0xAE,0x69,0x17,0x99,0x3A,0x27, 
	0xD9,0xEB,0x2B,0x22,0xD2,0xA9,0x07,0x33, 
	0x2D,0x3C,0x15,0xC9,0x87,0xAA,0x50,0xA5, 
	0x03,0x59,0x09,0x1A,0x65,0xD7,0x84,0xD0, 
	0x82,0x29,0x5A,0x1E,0x7B,0xA8,0x6D,0x2C 
}; 


/* Expanded IV for TKIP function.*/

typedef	struct GNU_PACKED _IV_CONTROL_
{
	union GNU_PACKED
	{
		struct GNU_PACKED
		{
			UCHAR		rc0;
			UCHAR		rc1;
			UCHAR		rc2;

			union GNU_PACKED
			{
				struct GNU_PACKED
				{
#ifdef RT_BIG_ENDIAN
					UCHAR	KeyID:2;
					UCHAR	ExtIV:1;
					UCHAR	Rsvd:5;
#else
					UCHAR	Rsvd:5;
					UCHAR	ExtIV:1;
					UCHAR	KeyID:2;
#endif
				}	field;
				UCHAR		Byte;
			}	CONTROL;
		}	field;
		
		ULONG	word;
	}	IV16;
	
	ULONG	IV32;
}	TKIP_IV, *PTKIP_IV;


/*
	========================================================================

	Routine	Description:
		Convert from UCHAR[] to ULONG in a portable way 
		
	Arguments:
      pMICKey		pointer to MIC Key
		
	Return Value:
		None

	Note:
		
	========================================================================
*/
ULONG	RTMPTkipGetUInt32( 	
	IN	PUCHAR	pMICKey)
{  	
	ULONG	res = 0; 
	INT		i;
	
	for (i = 0; i < 4; i++) 
	{ 
		res |= (*pMICKey++) << (8 * i); 
	}

	return res; 
} 

/*
	========================================================================

	Routine	Description:
		Convert from ULONG to UCHAR[] in a portable way 
		
	Arguments:
      pDst			pointer to destination for convert ULONG to UCHAR[]
      val			the value for convert
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPTkipPutUInt32(
	IN OUT	PUCHAR		pDst,
	IN		ULONG		val)					  
{ 	
	INT i;
	
	for(i = 0; i < 4; i++) 
	{ 
		*pDst++ = (UCHAR) (val & 0xff); 
		val >>= 8; 
	} 
} 

/*
	========================================================================

	Routine	Description:
		Set the MIC Key.
		
	Arguments:
      pAd		Pointer to our adapter
      pMICKey		pointer to MIC Key
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID RTMPTkipSetMICKey(  
	IN	PTKIP_KEY_INFO	pTkip,	
	IN	PUCHAR			pMICKey)
{ 
	/* Set the key */
	pTkip->K0 = RTMPTkipGetUInt32(pMICKey); 
	pTkip->K1 = RTMPTkipGetUInt32(pMICKey + 4); 
	/* and reset the message */
	pTkip->L = pTkip->K0;
	pTkip->R = pTkip->K1;
	pTkip->nBytesInM = 0;
	pTkip->M = 0;
} 

/*
	========================================================================

	Routine	Description:
		Calculate the MIC Value.
		
	Arguments:
      pAd		Pointer to our adapter
      uChar			Append this uChar
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPTkipAppendByte( 
	IN	PTKIP_KEY_INFO	pTkip,	
	IN	UCHAR 			uChar)
{ 
	/* Append the byte to our word-sized buffer */
	pTkip->M |= (uChar << (8* pTkip->nBytesInM)); 
	pTkip->nBytesInM++; 
	/* Process the word if it is full. */
	if( pTkip->nBytesInM >= 4 ) 
	{ 
		pTkip->L ^= pTkip->M; 
		pTkip->R ^= ROL32( pTkip->L, 17 ); 
		pTkip->L += pTkip->R; 
		pTkip->R ^= ((pTkip->L & 0xff00ff00) >> 8) | ((pTkip->L & 0x00ff00ff) << 8); 
		pTkip->L += pTkip->R; 
		pTkip->R ^= ROL32( pTkip->L, 3 ); 
		pTkip->L += pTkip->R; 
		pTkip->R ^= ROR32( pTkip->L, 2 ); 
		pTkip->L += pTkip->R; 
		/* Clear the buffer */
		pTkip->M = 0; 
		pTkip->nBytesInM = 0; 
	} 
} 

/*
	========================================================================

	Routine	Description:
		Calculate the MIC Value.
		
	Arguments:
      pAd		Pointer to our adapter
      pSrc			Pointer to source data for Calculate MIC Value
      Len			Indicate the length of the source data
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
VOID	RTMPTkipAppend( 
	IN	PTKIP_KEY_INFO	pTkip,	
	IN	PUCHAR			pSrc,
	IN	UINT			nBytes)						  
{ 
	/* This is simple */
	while(nBytes > 0) 
	{ 
		RTMPTkipAppendByte(pTkip, *pSrc++); 
		nBytes--; 
	} 
} 

/*
	========================================================================

	Routine	Description:
		Get the MIC Value.
		
	Arguments:
      pAd		Pointer to our adapter
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
		the MIC Value is store in pAd->PrivateInfo.MIC
	========================================================================
*/
VOID	RTMPTkipGetMIC( 
	IN	PTKIP_KEY_INFO	pTkip)
{ 
	/* Append the minimum padding*/
	RTMPTkipAppendByte(pTkip, 0x5a );	
	RTMPTkipAppendByte(pTkip, 0 ); 
	RTMPTkipAppendByte(pTkip, 0 ); 
	RTMPTkipAppendByte(pTkip, 0 ); 
	RTMPTkipAppendByte(pTkip, 0 ); 
	/* and then zeroes until the length is a multiple of 4 */
	while( pTkip->nBytesInM != 0 ) 
	{ 
		RTMPTkipAppendByte(pTkip, 0 ); 
	} 
	/* The appendByte function has already computed the result. */
	RTMPTkipPutUInt32(pTkip->MIC, pTkip->L);
	RTMPTkipPutUInt32(pTkip->MIC + 4, pTkip->R);
} 


/*
	========================================================================

	Routine	Description:
		Init MIC Value calculation function which include set MIC key & 
		calculate first 16 bytes (DA + SA + priority +  0)
		
	Arguments:
      pAd		Pointer to our adapter
		pTKey       Pointer to the Temporal Key (TK), TK shall be 128bits.
		pDA			Pointer to DA address
		pSA			Pointer to SA address
		pMICKey		pointer to MIC Key
		
	Return Value:
		None

	Note:
	
	========================================================================
*/
VOID	RTMPInitMICEngine(
	IN	PRTMP_ADAPTER	pAd,	
	IN	PUCHAR			pKey,
	IN	PUCHAR			pDA,
	IN	PUCHAR			pSA,
	IN  UCHAR           UserPriority,
	IN	PUCHAR			pMICKey)
{
	ULONG Priority = UserPriority;

	/* Init MIC value calculation*/
	RTMPTkipSetMICKey(&pAd->PrivateInfo.Tx, pMICKey);
	/* DA*/
	RTMPTkipAppend(&pAd->PrivateInfo.Tx, pDA, MAC_ADDR_LEN);
	/* SA*/
	RTMPTkipAppend(&pAd->PrivateInfo.Tx, pSA, MAC_ADDR_LEN);
	/* Priority + 3 bytes of 0*/
	RTMPTkipAppend(&pAd->PrivateInfo.Tx, (PUCHAR)&Priority, 4);
}

/*
	========================================================================

	Routine	Description:
		Compare MIC value of received MSDU
		
	Arguments:
		pAd	Pointer to our adapter
		pSrc        Pointer to the received Plain text data
		pDA			Pointer to DA address
		pSA			Pointer to SA address
		pMICKey		pointer to MIC Key
		Len         the length of the received plain text data exclude MIC value
		
	Return Value:
		TRUE        MIC value matched
		FALSE       MIC value mismatched
		
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
BOOLEAN	RTMPTkipCompareMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pSrc,
	IN	PUCHAR			pDA,
	IN	PUCHAR			pSA,
	IN	PUCHAR			pMICKey,
	IN	UCHAR			UserPriority,
	IN	UINT			Len)
{
	UCHAR	OldMic[8];
	ULONG	Priority = UserPriority;

	/* Init MIC value calculation*/
	RTMPTkipSetMICKey(&pAd->PrivateInfo.Rx, pMICKey);
	/* DA*/
	RTMPTkipAppend(&pAd->PrivateInfo.Rx, pDA, MAC_ADDR_LEN);
	/* SA*/
	RTMPTkipAppend(&pAd->PrivateInfo.Rx, pSA, MAC_ADDR_LEN);
	/* Priority + 3 bytes of 0*/
	RTMPTkipAppend(&pAd->PrivateInfo.Rx, (PUCHAR)&Priority, 4);
	
	/* Calculate MIC value from plain text data*/
	RTMPTkipAppend(&pAd->PrivateInfo.Rx, pSrc, Len);

	/* Get MIC valude from received frame*/
	NdisMoveMemory(OldMic, pSrc + Len, 8);
	
	/* Get MIC value from decrypted plain data*/
	RTMPTkipGetMIC(&pAd->PrivateInfo.Rx);
		
	/* Move MIC value from MSDU, this steps should move to data path.*/
	/* Since the MIC value might cross MPDUs.*/
	if(!NdisEqualMemory(pAd->PrivateInfo.Rx.MIC, OldMic, 8))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("RTMPTkipCompareMICValue(): TKIP MIC Error !\n"));  /*MIC error.*/

		
		return (FALSE);
	}
	return (TRUE);
}

/*
	========================================================================

	Routine	Description:
		Copy frame from waiting queue into relative ring buffer and set 
	appropriate ASIC register to kick hardware transmit function
		
	Arguments:
		pAd		Pointer	to our adapter
		PNDIS_PACKET	Pointer to Ndis Packet for MIC calculation
		pEncap			Pointer to LLC encap data
		LenEncap		Total encap length, might be 0 which indicates no encap
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID	RTMPCalculateMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR			pEncap,
	IN	PCIPHER_KEY		pKey,
	IN	UCHAR			apidx)
{
	PACKET_INFO		PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	PUCHAR			pSrc;
    UCHAR           UserPriority;
	UCHAR			vlan_offset = 0;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pSrc = pSrcBufVA;
    
	/* determine if this is a vlan packet */
	if (((*(pSrc + 12) << 8) + *(pSrc + 13)) == 0x8100)
		vlan_offset = 4;
	
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT	
	/* Start Calculate MIC Value*/
	if (apidx >= MIN_NET_DEVICE_FOR_APCLI && ((apidx - MIN_NET_DEVICE_FOR_APCLI) < MAX_APCLI_NUM) && (pAd->OpMode == OPMODE_AP))
	{	/* For packet which need to do MATConvert, we need to use the CurrentAddress of specific ApCli interface.*/
		RTMPInitMICEngine(
			pAd,
			pKey->Key,
			pSrc,
			pAd->ApCfg.ApCliTab[apidx-MIN_NET_DEVICE_FOR_APCLI].wdev.if_addr,
			UserPriority,
			pKey->TxMic);
	}
	else
#endif /* APCLI_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
	if ((RTMP_GET_PACKET_WCID(pPacket) != MCAST_WCID) && (*pSrc & 0x01) && (pAd->OpMode == OPMODE_AP))
	{
		UCHAR wcid = RTMP_GET_PACKET_WCID(pPacket);
		RTMPInitMICEngine(
			pAd,
			pKey->Key,
			pAd->MacTab.Content[wcid].Addr,
			pSrc + 6,
			UserPriority,
			pKey->TxMic);		
	}
	else
#endif /* IGMP_SNOOP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{
		RTMPInitMICEngine(
			pAd,
			pKey->Key,
			pSrc,
			pSrc + 6,
			UserPriority,
			pKey->TxMic);
	}
	

	if (pEncap != NULL)
	{
		/* LLC encapsulation*/
		RTMPTkipAppend(&pAd->PrivateInfo.Tx, pEncap, 6);
		/* Protocol Type*/
		RTMPTkipAppend(&pAd->PrivateInfo.Tx, pSrc + 12 + vlan_offset, 2);		
	}
	SrcBufLen -= (14 + vlan_offset);
	pSrc += (14 + vlan_offset);
	do
	{
		if (SrcBufLen > 0)
		{
			RTMPTkipAppend(&pAd->PrivateInfo.Tx, pSrc, SrcBufLen);
		}
		
		break;	/* No need handle next packet	*/

	} while (TRUE);

	/* Compute the final MIC Value*/
	RTMPTkipGetMIC(&pAd->PrivateInfo.Tx);
}


/************************************************************/ 
/* tkip_sbox()																*/ 
/* Returns a 16 bit value from a 64K entry table. The Table */ 
/* is synthesized from two 256 entry byte wide tables.		*/ 
/************************************************************/ 

UINT tkip_sbox(UINT index) 
{ 
	UINT index_low; 
	UINT index_high; 
	UINT left, right; 

	index_low = (index % 256); 
	index_high = ((index >> 8) % 256); 

	left = Tkip_Sbox_Lower[index_low] + (Tkip_Sbox_Upper[index_low] * 256); 
	right = Tkip_Sbox_Upper[index_high] + (Tkip_Sbox_Lower[index_high] * 256); 

	return (left ^ right); 
}

UINT rotr1(UINT a) 
{ 
	unsigned int b; 

	if ((a & 0x01) == 0x01) 
	{ 
		b = (a >> 1) | 0x8000; 
	} 
	else 
	{ 
		b = (a >> 1) & 0x7fff; 
	} 
	b = b % 65536; 
	return b; 
} 

VOID RTMPTkipMixKey(
	UCHAR *key, 
	UCHAR *ta, 
	ULONG pnl, /* Least significant 16 bits of PN */
	ULONG pnh, /* Most significant 32 bits of PN */ 
	UCHAR *rc4key, 
	UINT *p1k)
{

	UINT tsc0; 
	UINT tsc1;
	UINT tsc2; 

	UINT ppk0; 
	UINT ppk1; 
	UINT ppk2; 
	UINT ppk3; 
	UINT ppk4; 
	UINT ppk5; 

	INT i; 
	INT j; 

	tsc0 = (unsigned int)((pnh >> 16) % 65536); /* msb */ 
	tsc1 = (unsigned int)(pnh % 65536); 
	tsc2 = (unsigned int)(pnl % 65536); /* lsb */ 

	/* Phase 1, step 1 */ 
	p1k[0] = tsc1; 
	p1k[1] = tsc0; 
	p1k[2] = (UINT)(ta[0] + (ta[1]*256)); 
	p1k[3] = (UINT)(ta[2] + (ta[3]*256)); 
	p1k[4] = (UINT)(ta[4] + (ta[5]*256)); 

	/* Phase 1, step 2 */ 
	for (i=0; i<8; i++) 
	{ 
		j = 2*(i & 1); 
		p1k[0] = (p1k[0] + tkip_sbox( (p1k[4] ^ ((256*key[1+j]) + key[j])) % 65536 )) % 65536; 
		p1k[1] = (p1k[1] + tkip_sbox( (p1k[0] ^ ((256*key[5+j]) + key[4+j])) % 65536 )) % 65536; 
		p1k[2] = (p1k[2] + tkip_sbox( (p1k[1] ^ ((256*key[9+j]) + key[8+j])) % 65536 )) % 65536; 
		p1k[3] = (p1k[3] + tkip_sbox( (p1k[2] ^ ((256*key[13+j]) + key[12+j])) % 65536 )) % 65536; 
		p1k[4] = (p1k[4] + tkip_sbox( (p1k[3] ^ (((256*key[1+j]) + key[j]))) % 65536 )) % 65536; 
		p1k[4] = (p1k[4] + i) % 65536; 
	} 

	/* Phase 2, Step 1 */ 
	ppk0 = p1k[0]; 
	ppk1 = p1k[1]; 
	ppk2 = p1k[2]; 
	ppk3 = p1k[3]; 
	ppk4 = p1k[4]; 
	ppk5 = (p1k[4] + tsc2) % 65536; 

	/* Phase2, Step 2 */ 
	ppk0 = ppk0 + tkip_sbox( (ppk5 ^ ((256*key[1]) + key[0])) % 65536); 
	ppk1 = ppk1 + tkip_sbox( (ppk0 ^ ((256*key[3]) + key[2])) % 65536); 
	ppk2 = ppk2 + tkip_sbox( (ppk1 ^ ((256*key[5]) + key[4])) % 65536); 
	ppk3 = ppk3 + tkip_sbox( (ppk2 ^ ((256*key[7]) + key[6])) % 65536); 
	ppk4 = ppk4 + tkip_sbox( (ppk3 ^ ((256*key[9]) + key[8])) % 65536); 
	ppk5 = ppk5 + tkip_sbox( (ppk4 ^ ((256*key[11]) + key[10])) % 65536); 

	ppk0 = ppk0 + rotr1(ppk5 ^ ((256*key[13]) + key[12])); 
	ppk1 = ppk1 + rotr1(ppk0 ^ ((256*key[15]) + key[14])); 
	ppk2 = ppk2 + rotr1(ppk1); 
	ppk3 = ppk3 + rotr1(ppk2); 
	ppk4 = ppk4 + rotr1(ppk3); 
	ppk5 = ppk5 + rotr1(ppk4); 

	/* Phase 2, Step 3 */ 
    /* Phase 2, Step 3 */

	tsc0 = (unsigned int)((pnh >> 16) % 65536); /* msb */ 
	tsc1 = (unsigned int)(pnh % 65536); 
	tsc2 = (unsigned int)(pnl % 65536); /* lsb */ 

	rc4key[0] = (tsc2 >> 8) % 256; 
	rc4key[1] = (((tsc2 >> 8) % 256) | 0x20) & 0x7f; 
	rc4key[2] = tsc2 % 256; 
	rc4key[3] = ((ppk5 ^ ((256*key[1]) + key[0])) >> 1) % 256; 

	rc4key[4] = ppk0 % 256; 
	rc4key[5] = (ppk0 >> 8) % 256; 

	rc4key[6] = ppk1 % 256; 
	rc4key[7] = (ppk1 >> 8) % 256; 

	rc4key[8] = ppk2 % 256; 
	rc4key[9] = (ppk2 >> 8) % 256; 

	rc4key[10] = ppk3 % 256; 
	rc4key[11] = (ppk3 >> 8) % 256; 

	rc4key[12] = ppk4 % 256; 
	rc4key[13] = (ppk4 >> 8) % 256; 

	rc4key[14] = ppk5 % 256; 
	rc4key[15] = (ppk5 >> 8) % 256; 
}


/*
	TRUE: Success!
	FALSE: Decrypt Error!
*/
BOOLEAN RTMPSoftDecryptTKIP(
	IN 		PRTMP_ADAPTER 	pAd,
	IN 		PUCHAR			pHdr,
	IN 		UCHAR    		UserPriority,
	IN 		PCIPHER_KEY		pKey,
	INOUT 	PUCHAR			pData,
	IN 		UINT16			*DataByteCnt)
{
	PHEADER_802_11	pFrame;
	//UINT8			frame_type;
	//UINT8			frame_subtype;
	UINT8			from_ds;
    UINT8			to_ds;
	//UINT8			a4_exists;
	//UINT8			qc_exists;
	UCHAR			TA[MAC_ADDR_LEN];
	UCHAR			DA[MAC_ADDR_LEN];
	UCHAR			SA[MAC_ADDR_LEN];	
	UCHAR			RC4Key[16];
	UINT			p1k[5]; /*for mix_key;*/
	ULONG			pnl;/* Least significant 16 bits of PN */
	ULONG			pnh;/* Most significant 32 bits of PN */ 
	ARC4_CTX_STRUC 	ARC4_CTX;
	PUCHAR			plaintext_ptr;
	UINT32			plaintext_len;
	PUCHAR			ciphertext_ptr;
	UINT32			ciphertext_len;
	UINT			crc32 = 0;
	UINT			trailfcs = 0;
	UCHAR			MIC[8];
	UCHAR			TrailMIC[8];

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif

	if (pKey->KeyLen == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : the key is empty)\n", __FUNCTION__));
		return FALSE;
	}

	/* Indicate type and subtype of Frame Control field */
	//frame_type = (((*pHdr) >> 2) & 0x03);
	//frame_subtype = (((*pHdr) >> 4) & 0x0f);	

	/* Indicate the fromDS and ToDS */
	from_ds = ((*(pHdr + 1)) & 0x2) >> 1;
	to_ds = ((*(pHdr + 1)) & 0x1);

	/* decide if the Address 4 exist or QoS exist */
	//a4_exists = (from_ds & to_ds);
	//qc_exists = ((frame_subtype == SUBTYPE_QDATA) || 
	//			 (frame_subtype == SUBTYPE_QDATA_CFACK) ||
	//			 (frame_subtype == SUBTYPE_QDATA_CFPOLL) ||
	//			 (frame_subtype == SUBTYPE_QDATA_CFACK_CFPOLL));


	/* pointer to 802.11 header */
	pFrame = (PHEADER_802_11)pHdr;

	/* Assign DA, SA and TA for TKIP calculation */
	if (to_ds == 0 && from_ds == 1)
	{
		NdisMoveMemory(DA, pFrame->Addr1, MAC_ADDR_LEN);
		NdisMoveMemory(TA, pFrame->Addr2, MAC_ADDR_LEN);  /*BSSID		*/
		NdisMoveMemory(SA, pFrame->Addr3, MAC_ADDR_LEN);
	}	
	else if (to_ds == 0 && from_ds == 0 )
	{
		NdisMoveMemory(DA, pFrame->Addr1, MAC_ADDR_LEN);
		NdisMoveMemory(TA, pFrame->Addr2, MAC_ADDR_LEN);		
		NdisMoveMemory(SA, pFrame->Addr2, MAC_ADDR_LEN);
	}
	else if (to_ds == 1 && from_ds == 0)
	{
		NdisMoveMemory(SA, pFrame->Addr2, MAC_ADDR_LEN);
		NdisMoveMemory(TA, pFrame->Addr2, MAC_ADDR_LEN);
		NdisMoveMemory(DA, pFrame->Addr3, MAC_ADDR_LEN);
	}
	else if (to_ds == 1 && from_ds == 1)
	{
		NdisMoveMemory(TA, pFrame->Addr2, MAC_ADDR_LEN);
		NdisMoveMemory(DA, pFrame->Addr3, MAC_ADDR_LEN);
		NdisMoveMemory(SA, pFrame->Octet, MAC_ADDR_LEN);
	}

	pnl = (*(pData)) << 8 | (*(pData + 2));	
	pnh = *((PULONG)(pData + 4));
	pnh = cpu2le32(pnh);	
	RTMPTkipMixKey(pKey->Key, TA, pnl, pnh, RC4Key, p1k);

	/* skip 8-bytes TKIP IV/EIV header */
	ciphertext_ptr = pData + LEN_TKIP_IV_HDR;
	ciphertext_len = *DataByteCnt - LEN_TKIP_IV_HDR;

	/* skip payload length is zero*/
	if ((*DataByteCnt) <= LEN_TKIP_IV_HDR)
		return FALSE;

	/* WEP Decapsulation */
	/* Generate an RC4 key stream */
	ARC4_INIT(&ARC4_CTX, &RC4Key[0], 16);

	/* Decrypt the TKIP MPDU by ARC4. 
	   It shall include plaintext, MIC and ICV.
	   The result output would overwrite the original TKIP IV/EIV header position */
	ARC4_Compute(&ARC4_CTX, ciphertext_ptr, ciphertext_len, pData);

	/* Point to the decrypted data frame and its length shall exclude ICV length */
	plaintext_ptr = pData;
	plaintext_len = ciphertext_len - LEN_ICV;

	/* Extract peer's ICV */	
	NdisMoveMemory(&trailfcs, plaintext_ptr + plaintext_len, LEN_ICV);
	
	/* Re-computes the ICV and 
	   bit-wise compares with the peer's ICV. */
	crc32 = RTMP_CALC_FCS32(PPPINITFCS32, plaintext_ptr, plaintext_len);
	crc32 ^= 0xffffffff;             /* complement */

    if(crc32 != cpu2le32(trailfcs))
    {
		DBGPRINT(RT_DEBUG_ERROR, ("! WEP Data CRC Error !\n"));	 /*CRC error.*/
		return FALSE;
	}

	/* Extract peer's MIC and subtract MIC length from total data length */
	plaintext_len -= LEN_TKIP_MIC;
	NdisMoveMemory(TrailMIC, plaintext_ptr + plaintext_len, LEN_TKIP_MIC);
	RTMPInitMICEngine(pAd, pKey->Key, DA, SA, UserPriority, pKey->RxMic);
	RTMPTkipAppend(&pAd->PrivateInfo.Tx, plaintext_ptr, plaintext_len);
	RTMPTkipGetMIC(&pAd->PrivateInfo.Tx);
	NdisMoveMemory(MIC, pAd->PrivateInfo.Tx.MIC, LEN_TKIP_MIC);

	if (!NdisEqualMemory(MIC, TrailMIC, LEN_TKIP_MIC))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("! TKIP MIC Error !\n"));	 /*MIC error.*/
		return FALSE;		
	}

	/* Update the total data length */
	*DataByteCnt = plaintext_len;

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif	
	return TRUE;
}

/*
	========================================================================
	
	Routine Description:
		Use RC4 to protect the Key Data field of EAPoL frame. 
		It's defined in IEEE 802.11i-2004 p.84 

	Arguments:
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID TKIP_GTK_KEY_WRAP( 
    IN UCHAR    *key,
    IN UCHAR	*iv,
    IN UCHAR    *input_text,
    IN UINT32    input_len,
    OUT UCHAR   *output_text)
{	
	UCHAR	ekey[LEN_KEY_DESC_IV + LEN_PTK_KEK];	
/*	ARC4_CTX_STRUC ARC4_CTX;*/
	ARC4_CTX_STRUC *pARC4_CTX = NULL;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pARC4_CTX, sizeof(ARC4_CTX_STRUC));
	if (pARC4_CTX == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	/* The encryption key is generated by concatenating the
	   EAPOL-Key IV field and the KEK. */
	NdisMoveMemory(ekey, iv, LEN_KEY_DESC_IV);
	NdisMoveMemory(&ekey[LEN_KEY_DESC_IV], key, LEN_PTK_KEK);

	/* RC4 stream cipher initialization with the KEK */	
	ARC4_INIT(pARC4_CTX, &ekey[0], LEN_KEY_DESC_IV + LEN_PTK_KEK);

	/* The first 256 octets of the RC4 key stream shall be discarded */
	ARC4_Discard_KeyLength(pARC4_CTX, 256);

	/* encryption begins using the 257th key stream octet */
	ARC4_Compute(pARC4_CTX, input_text, input_len, output_text);

	if (pARC4_CTX != NULL)
		os_free_mem(NULL, pARC4_CTX);
}

VOID TKIP_GTK_KEY_UNWRAP( 
    IN UCHAR    *key,
    IN UCHAR	*iv,
    IN UCHAR    *input_text,
    IN UINT32    input_len,
    OUT UCHAR   *output_text)
{
	TKIP_GTK_KEY_WRAP(key, iv, input_text, input_len, output_text);
}

