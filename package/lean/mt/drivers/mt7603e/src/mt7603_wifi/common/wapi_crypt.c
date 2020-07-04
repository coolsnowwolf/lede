
#ifdef LINUX
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>		/* Needed for the macros */
#endif /* LINUX */

#include "rtmp_type.h"
#include "wpa_cmm.h"
#include "wapi_def.h"
#include "wapi_sms4.h"

void dump_bin(char *name, void *data, int len)
{
	int i = 0;
	unsigned char *p = data;
	
	if(name != NULL)
	{
		printk("%s(%d): \n", name, len);
	}
	printk("\t");

	for(i=0; i<len; i++)
	{
		printk("%02x ", p[i]);
		if(((i+1)%16) ==0)
			printk("\n\t");
	}
	if(((i+1)%16 ) != 0)
		printk("\n");
}



/*
	========================================================================
	
	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address
		
	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL
	
	Note:
		
	========================================================================
*/
ULONG	WapiCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	/* Equal */
	return (0);
}

/************************************/
/* bitwise_xor()                    */
/* A 128 bit, bitwise exclusive or  */
/************************************/

void bitwise_xor_wpi(unsigned char *ina, unsigned char *inb, unsigned char *out)
{
	int i;
	for (i=0; i<16; i++)
	{
		out[i] = ina[i] ^ inb[i];
	}
}

/*cbc_mac*/
int wpi_cbc_mac_engine(
		unsigned char * maciv_in,
		unsigned char * in_data1,
		unsigned int 	in_data1_len,
		unsigned char * in_data2,
		unsigned int 	in_data2_len,
		unsigned char * pkey,
		unsigned char * mac_out)
{
	unsigned char  	mactmp[16];
	unsigned int 	i;
	unsigned int	num_blocks_1, num_blocks_2;
	unsigned int 	prmackey_in[32];

	num_blocks_1 = (in_data1_len) >> 4; 
	num_blocks_2 = (in_data2_len) >> 4; 	

	if((num_blocks_1 + num_blocks_2 ) < 1) return 1;
	if((num_blocks_1 + num_blocks_2 ) > 4096) return 1;

	SMS4KeyExt(pkey,  prmackey_in, 0);
	SMS4Crypt(maciv_in, mactmp, prmackey_in);	
	
	for (i = 0; i < num_blocks_1; i++) 
	{
		bitwise_xor_wpi(mactmp, in_data1, mactmp);
		in_data1 += 16;
		SMS4Crypt(mactmp, mactmp, prmackey_in);
	}
		
	for (i = 0; i < num_blocks_2; i++) 
	{
		bitwise_xor_wpi(mactmp, in_data2, mactmp);
		in_data2 += 16;
		SMS4Crypt(mactmp, mactmp, prmackey_in);
	}
	memcpy(mac_out, mactmp, 16);
	
	return 0;
}

/*ofb encrypt*/
int wpi_sms4_ofb_engine(
	unsigned char * pofbiv_in,
	unsigned char * pbw_in,
	unsigned int 	plbw_in,
	unsigned char * pkey,
	unsigned char * pcw_out)
{
	unsigned char ofbtmp[16];
	/*unsigned int * pint0, * pint1; */
	/*unsigned char * pchar0, * pchar1,* pchar2; */
	unsigned int counter,comp,i;
	unsigned int prkey_in[32];


	if(plbw_in<1)	return 1;
	/*if(plbw_in>65536) return 1; */

	SMS4KeyExt(pkey, prkey_in, 0);

	/* get the multiple of 16 */
	counter = plbw_in >> 4;
	/* get the remainder of 16 */
	comp = plbw_in & 0x0f;	

	/*get the iv */
	SMS4Crypt(pofbiv_in, ofbtmp, prkey_in);
	
	for(i = 0; i < counter; i++) 
	{
		bitwise_xor_wpi(pbw_in, ofbtmp, pcw_out);
		pbw_in += 16;
		pcw_out += 16;
		SMS4Crypt(ofbtmp, ofbtmp, prkey_in);		
	}

	for(i = 0; i < comp; i++) 
	{
		pcw_out[i]=pbw_in[i]^ofbtmp[i];
	}
	
	return 0;	
}


/*
	========================================================================
	
	Routine Description:
		Make the integrity check header for WPI MIC

	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:		
		
	Note:
		
	========================================================================
*/
UINT32 wpi_construct_mic_hdr(
		IN  PUCHAR			pHdr,
		IN	UINT32			data_len,
		IN	UCHAR			keyidx,		
		OUT	UCHAR 			*mic_hdr)
{
	UINT8 	from_ds, to_ds, a4_exists, qc_exists;
	//UINT8	frame_type;
	UINT8	frame_subtype;
	UINT8 	mhdr_len = 0;
	UCHAR 	ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	//frame_type = (((*pHdr) >> 2) & 0x03);
	frame_subtype = (((*pHdr) >> 4) & 0x0f);	

	/* Check if Addr4 is available */
	from_ds = ((*(pHdr + 1)) & 0x2) >> 1;
	to_ds = ((*(pHdr + 1)) & 0x1);
	a4_exists = (from_ds & to_ds);

	/* Check if Qos is available */
	qc_exists = ((frame_subtype == 0x08) ||    	/* QoS Data */
				  (frame_subtype == 0x09) ||   	/* QoS Data + CF-Ack */
				  (frame_subtype == 0x0a) || 	/* QoS Data + CF-Poll */
				  (frame_subtype == 0x0b)		/* QoS Data + CF-Ack + CF-Poll */
				 );

	/* Frame control */
	/* bit 4,5,6 must be zero */
	/* bit 11,12,13 must be zero */
	/* bit 14 must be 1 */
	mic_hdr[mhdr_len] = (*pHdr) & 0x8f;
	mic_hdr[mhdr_len + 1] = (*(pHdr + 1)) & 0xc7;
	mhdr_len = 2;
	
	/* Addr1 & 2 */
	WapiMoveMemory(&mic_hdr[mhdr_len], pHdr + 4, 2 * MAC_ADDR_LEN);
	mhdr_len += (2 * MAC_ADDR_LEN);
	
	/* In Sequence Control field, mute sequence numer bits (12-bit) */
	mic_hdr[mhdr_len] = (*(pHdr + 22)) & 0x0f;   
	mic_hdr[mhdr_len + 1] = 0x00;
	mhdr_len += 2;
	
	/* Addr3 */
	WapiMoveMemory(&mic_hdr[mhdr_len], pHdr + 16, MAC_ADDR_LEN);
	mhdr_len += MAC_ADDR_LEN;
		
	/* Fill the Addr4 field. */
	/* If the Addr4 is unused, it shall be padded zero. */
	if (a4_exists)		
		WapiMoveMemory(&mic_hdr[mhdr_len], pHdr + 24, MAC_ADDR_LEN);
	else
		WapiMoveMemory(&mic_hdr[mhdr_len], ZERO_MAC_ADDR, MAC_ADDR_LEN);
	mhdr_len += MAC_ADDR_LEN;

	/* Qos field */
	if (qc_exists & a4_exists)
	{
		mic_hdr[mhdr_len] = (*(pHdr + 30)) & 0x0f;   /* Qos_TC */
		mic_hdr[mhdr_len + 1] = 0x00;
		mhdr_len += 2;
	}
	else if (qc_exists & !a4_exists)
	{
		mic_hdr[mhdr_len] = (*(pHdr + 24)) & 0x0f;   /* Qos_TC */
		mic_hdr[mhdr_len + 1] = 0x00;
		mhdr_len += 2;
	}	

	/* fill the KeyIdx and reserve field */
	mic_hdr[mhdr_len] = keyidx;
	mic_hdr[mhdr_len + 1] = 0x00;
	mhdr_len += 2;

	/* Fill in data len field */
	mic_hdr[mhdr_len] = (data_len >> 8) & 0xff;
	mic_hdr[mhdr_len + 1] = (data_len) & 0xff;
	mhdr_len += 2;

	/* the len must be the multiple of 16 */
	/* If the Qos field doesn't exist, the header length shall be 32. */
	/* If the Qos exists, the header is 34. We need to expand it as 48 by padding zero. */
	/*DBGPRINT(RT_DEBUG_TRACE, ("mhdr_len(%d)\n", mhdr_len)); */
	mhdr_len = (mhdr_len > 32) ? 48 : 32;
				
	return mhdr_len;
}

/*
	========================================================================
	
	Routine Description:
		Calculate the MIC of WPI data.
		
	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:		
		TRUE	- success
		FLASE	- failure
		
	Note:
		
	========================================================================
*/
BOOLEAN RTMPCalculateWpiMic(
		IN	PUCHAR			pHeader,
		IN  PUCHAR			pData,
		IN	UINT32			data_len,
		IN	UINT8			key_idx,
		IN	PUCHAR 			pKey,
		IN	PUCHAR			pIV,
		OUT	PUCHAR			pOutMic)
{
	UCHAR	mic_header[48];
	UINT8	mic_hdr_len = 0;	
	UINT8	pad_len = 0;
	UINT8	remainder = 0;
		
	WapiZeroMemory(mic_header, 48);
		
	/* Construct integrity verify data header */
	mic_hdr_len = wpi_construct_mic_hdr(pHeader, data_len, key_idx, mic_header);

	/* calculate the padded length */
	if ((remainder = data_len & 0x0f) != 0)
		pad_len = (16 - remainder);

	/* Calculate MIC */
	wpi_cbc_mac_engine(pIV, 
			 mic_header, 
			 mic_hdr_len, 
			 pData, 
			 data_len + pad_len, 
			 pKey, 
			 pOutMic);
		
	return TRUE;
	
}

/*
	========================================================================
	
	Routine Description:
		Encrypt WPI-data by software.
		
	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:		
		TRUE	- success
		FLASE	- failure
		
	Note:
		
	========================================================================
*/
INT RTMPSoftEncryptSMS4(
		IN	PUCHAR			pHeader,
		IN  PUCHAR			pData,
		IN	UINT32			data_len,				
		IN	UCHAR			key_id,
		IN	PUCHAR 			pKey,
		IN	PUCHAR			pIv)
{
		INT		i;
		UCHAR	out_mic[LEN_WPI_MIC];
		UCHAR	iv_inv[LEN_WAPI_TSC];

		/* Inverse IV byte order */
		for (i=0; i < LEN_WAPI_TSC; i++)
		{
			iv_inv[LEN_WAPI_TSC - 1 - i] = (*(pIv + i)) & 0xFF;
		}

		WapiZeroMemory(out_mic, LEN_WPI_MIC);
		/* clear enough space for calculating WPI MIC later */
		WapiZeroMemory(pData + data_len, LEN_WPI_MIC);
		/* Calculate WPI MIC */
		RTMPCalculateWpiMic(pHeader, 
							pData, 
							data_len, 
							key_id, 
							pKey + 16, 
							iv_inv, 
							out_mic);

		/* append MIC to the data tail */
		WapiMoveMemory(pData + data_len, out_mic, LEN_WPI_MIC);		
		
		/* Encrypt WPI data by software */
		wpi_sms4_ofb_engine(iv_inv, pData, data_len + LEN_WPI_MIC, pKey, pData);			

		return TRUE;
							
}

/*
	========================================================================
	
	Routine Description:
		Decrypt WPI-data by software.
		
	Arguments:		
		pAd			-	pointer to our pAdapter context	
  				
	Return Value:		
		TRUE	- success
		FLASE	- failure
		
	Note:
		
	========================================================================
*/
INT	RTMPSoftDecryptSMS4(
		IN		PUCHAR			pHdr,
		IN		BOOLEAN			bSanityIV,
		IN 		PCIPHER_KEY		pKey,
		INOUT 	PUCHAR			pData,
		INOUT 	UINT16			*DataByteCnt)
{	
	UCHAR	i;
	UCHAR	key_idx;
	PUCHAR 	iv_ptr;
	UCHAR	iv_inv[LEN_WAPI_TSC];	
	UCHAR	MIC[LEN_WPI_MIC];
	UCHAR	TrailMIC[LEN_WPI_MIC];
	/*PUCHAR	mic_ptr; */
	/*UINT32	mic_data_len; */
	/*PUCHAR	buf_ptr = NULL; */
	PUCHAR	plaintext_ptr;
	UINT16	plaintext_len;
	PUCHAR	ciphertext_ptr;
	UINT16	ciphertext_len;
		
	/* Check the key is valid */
	if (pKey->KeyLen == 0)
	{
		return STATUS_WAPI_KEY_INVALID;
	}

	/* extract key index from the IV header */
	key_idx = *(pData) & 0xFF;
	
	/* Skip 2-bytes(key_idx and reserve field) and point to IV field */	
	iv_ptr = pData + 2;

	/* IV sanity check */
	if (bSanityIV)
	{
		if (WapiCompareMemory(iv_ptr, pKey->RxTsc, LEN_WAPI_TSC) > 1)
		{
			return STATUS_WAPI_IV_MISMATCH;
		}
		else
		{
			/* Record the received IV */
			WapiMoveMemory(pKey->RxTsc, iv_ptr, LEN_WAPI_TSC);
		}
	}
 
	/* Inverse IV byte order for SMS4 calculation */
	for (i = 0; i < LEN_WAPI_TSC; i++)
	{
		iv_inv[LEN_WAPI_TSC - 1 - i] = (*(iv_ptr + i)) & 0xFF;
	}

	/* Skip the WPI IV header (18-bytes) */
	ciphertext_ptr = pData + LEN_WPI_IV_HDR;
	ciphertext_len = *DataByteCnt - LEN_WPI_IV_HDR;

	/* skip payload length is zero */
	if ((*DataByteCnt ) <= LEN_WPI_IV_HDR)
		return FALSE;

	/* Decrypt the WPI MPDU. It shall include plaintext and MIC.
	   The result output would overwrite the original WPI IV header position */
	wpi_sms4_ofb_engine(iv_inv, ciphertext_ptr, ciphertext_len, pKey->Key, pData);
			
	/* Point to the plainext data frame and its length shall exclude MIC length */
	plaintext_ptr = pData;
	plaintext_len = ciphertext_len - LEN_WPI_MIC;
	
	/* Extract peer's MIC and zero the MIC field of received frame */
	WapiMoveMemory(TrailMIC, plaintext_ptr + plaintext_len, LEN_WPI_MIC);
	WapiZeroMemory(plaintext_ptr + plaintext_len, LEN_WPI_MIC);
	
	/* Calculate WPI MIC */
	WapiZeroMemory(MIC, LEN_WPI_MIC);
	RTMPCalculateWpiMic(pHdr, 
						plaintext_ptr, 
						plaintext_len, 
						key_idx, 
						pKey->TxMic, 
						iv_inv, 
						MIC);

	/* Compare the MIC field */
	if (!WapiEqualMemory(MIC, TrailMIC, LEN_WPI_MIC))
	{			
		return STATUS_WAPI_MIC_DIFF;
    }

	/* Update the total data length */
	*DataByteCnt = plaintext_len;
	
	return STATUS_SUCCESS;		
}

//#ifndef RTMP_RBUS_SUPPORT

