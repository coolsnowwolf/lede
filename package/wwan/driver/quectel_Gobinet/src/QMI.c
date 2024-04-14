#ifdef __QUEC_INCLUDE_QMI_C__
/*===========================================================================
FILE:
   QMI.c

DESCRIPTION:
   Qualcomm QMI driver code
   
FUNCTIONS:
   Generic QMUX functions
      ParseQMUX
      FillQMUX
   
   Generic QMI functions
      GetTLV
      ValidQMIMessage
      GetQMIMessageID

   Fill Buffers with QMI requests
      QMICTLGetClientIDReq
      QMICTLReleaseClientIDReq
      QMICTLReadyReq
      QMIWDSSetEventReportReq
      QMIWDSGetPKGSRVCStatusReq
      QMIDMSGetMEIDReq
      QMIWDASetDataFormatReq
      QMICTLSetDataFormatReq
      QMICTLSyncReq
      
   Parse data from QMI responses
      QMICTLGetClientIDResp
      QMICTLReleaseClientIDResp
      QMIWDSEventResp
      QMIDMSGetMEIDResp
      QMIWDASetDataFormatResp
      QMICTLSyncResp

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include <asm/unaligned.h>
#include <linux/kernel.h>
#include "Structs.h"
#include "QMI.h"

/*=========================================================================*/
// Get sizes of buffers needed by QMI requests
/*=========================================================================*/

/*===========================================================================
METHOD:
   QMUXHeaderSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMUXHeaderSize( void )
{
   return sizeof( sQMUX );
}

/*===========================================================================
METHOD:
   QMICTLGetClientIDReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMICTLGetClientIDReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMICTLGetClientIDReqSize( void )
{
   return sizeof( sQMUX ) + 10;
}

/*===========================================================================
METHOD:
   QMICTLReleaseClientIDReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMICTLReleaseClientIDReq
 
RETURN VALUE:
   u16 - size of header
===========================================================================*/
static u16 QMICTLReleaseClientIDReqSize( void )
{
   return sizeof( sQMUX ) + 11;
}

/*===========================================================================
METHOD:
   QMICTLReadyReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMICTLReadyReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMICTLReadyReqSize( void )
{
   return sizeof( sQMUX ) + 6;
}

/*===========================================================================
METHOD:
   QMIWDSSetEventReportReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMIWDSSetEventReportReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMIWDSSetEventReportReqSize( void )
{
   return sizeof( sQMUX ) + 15;
}

/*===========================================================================
METHOD:
   QMIWDSGetPKGSRVCStatusReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMIWDSGetPKGSRVCStatusReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMIWDSGetPKGSRVCStatusReqSize( void )
{
   return sizeof( sQMUX ) + 7;
}

/*===========================================================================
METHOD:
   QMIDMSGetMEIDReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMIDMSGetMEIDReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMIDMSGetMEIDReqSize( void )
{
   return sizeof( sQMUX ) + 7;
}

struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS
{
   u8  TLVType;
   u16 TLVLength;
   u8  QOSSetting;
} __packed;

struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV
{
   u8  TLVType;
   u16 TLVLength;
   u32  Value;
} __packed;

struct QMIWDS_ENDPOINT_TLV
{
   u8  TLVType;
   u16 TLVLength;
   u32  ep_type;
   u32  iface_id;
} __packed;

struct QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG
{
    u8  CtlFlags;      // 0: single QMUX Msg; 1:
    u16 TransactionId;
    u16 Type;
    u16 Length;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV_QOS QosDataFormatTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UnderlyingLinkLayerProtocolTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationProtocolTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationProtocolTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxDatagramsTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV DownlinkDataAggregationMaxSizeTlv;
    struct QMIWDS_ENDPOINT_TLV epTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV  dl_minimum_padding;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxDatagramsTlv;
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_TLV UplinkDataAggregationMaxSizeTlv;
} __packed;

/*===========================================================================
METHOD:
   QMIWDASetDataFormatReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMIWDASetDataFormatReq

RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16 QMIWDASetDataFormatReqSize( int qmap_version )
{
if (qmap_version)
   return sizeof( sQMUX ) + sizeof(struct QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG);
else
   return sizeof( sQMUX ) + 18;
}

/*===========================================================================
METHOD:
   QMICTLSyncReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMICTLSyncReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16  QMICTLSyncReqSize( void )
{
   return sizeof( sQMUX ) + 6; 
}

/*=========================================================================*/
// Generic QMUX functions
/*=========================================================================*/

/*===========================================================================
METHOD:
   ParseQMUX (Public Method)

DESCRIPTION:
   Remove QMUX headers from a buffer

PARAMETERS
   pClientID       [ O ] - On success, will point to Client ID
   pBuffer         [ I ] - Full Message passed in
   buffSize        [ I ] - Size of pBuffer

RETURN VALUE:
   int - Positive for size of QMUX header
         Negative errno for error
===========================================================================*/
static int ParseQMUX(
   u16 *    pClientID,
   void *   pBuffer,
   u16      buffSize )
{
   sQMUX * pQMUXHeader;
   
   if (pBuffer == 0 || buffSize < 12)
   {
      return -ENOMEM;
   }

   // QMUX Header
   pQMUXHeader = (sQMUX *)pBuffer;

   if (pQMUXHeader->mTF != 1
   ||  le16_to_cpu(get_unaligned(&pQMUXHeader->mLength)) != buffSize - 1
   ||  pQMUXHeader->mCtrlFlag != 0x80 )
   {
      return -EINVAL;
   }

   // Client ID   
   *pClientID = (pQMUXHeader->mQMIClientID << 8) + pQMUXHeader->mQMIService;
   
   return sizeof( sQMUX );
}

/*===========================================================================
METHOD:
   FillQMUX (Public Method)

DESCRIPTION:
   Fill buffer with QMUX headers

PARAMETERS
   clientID        [ I ] - Client ID
   pBuffer         [ O ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer (must be at least 6)

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int FillQMUX(
   u16      clientID,
   void *   pBuffer,
   u16      buffSize )
{
   sQMUX * pQMUXHeader;

   if (pBuffer == 0 ||  buffSize < sizeof( sQMUX ))
   {
      return -ENOMEM;
   }

   // QMUX Header
   pQMUXHeader = (sQMUX *)pBuffer;

   pQMUXHeader->mTF = 1;
   put_unaligned(cpu_to_le16(buffSize - 1), &pQMUXHeader->mLength);
   //DBG("pQMUXHeader->mLength = 0x%x, buffSize - 1 = 0x%x\n",pQMUXHeader->mLength, buffSize - 1);
   pQMUXHeader->mCtrlFlag = 0;

   // Service and Client ID   
   pQMUXHeader->mQMIService = clientID & 0xff;
   pQMUXHeader->mQMIClientID = clientID >> 8;

   return 0;
}

/*=========================================================================*/
// Generic QMI functions
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetTLV (Public Method)

DESCRIPTION:
   Get data buffer of a specified TLV from a QMI message

   QMI Message shall NOT include SDU
   
PARAMETERS
   pQMIMessage    [ I ] - QMI Message buffer
   messageLen     [ I ] - Size of QMI Message buffer
   type           [ I ] - Desired Type
   pOutDataBuf    [ O ] - Buffer to be filled with TLV
   messageLen     [ I ] - Size of QMI Message buffer

RETURN VALUE:
   u16 - Size of TLV for success
         Negative errno for error
===========================================================================*/
static int GetTLV(
   void *   pQMIMessage,
   u16      messageLen,
   u8       type,
   void *   pOutDataBuf,
   u16      bufferLen )
{
   u16 pos;
   u16 tlvSize = 0;
   u16 cpyCount;
   
   if (pQMIMessage == 0 || pOutDataBuf == 0)
   {
      return -ENOMEM;
   }   
   
   for (pos = 4; 
        pos + 3 < messageLen; 
        pos += tlvSize + 3)
   {
      tlvSize = le16_to_cpu( get_unaligned(((u16 *)(pQMIMessage + pos + 1) )) );
      if (*(u8 *)(pQMIMessage + pos) == type)
      {
         if (bufferLen < tlvSize)
         {
            return -ENOMEM;
         }
                
         for (cpyCount = 0; cpyCount < tlvSize; cpyCount++)
         {
            *((char*)(pOutDataBuf + cpyCount)) = *((char*)(pQMIMessage + pos + 3 + cpyCount));
         }
         
         return tlvSize;
      }
   }
   
   return -ENOMSG;
}

/*===========================================================================
METHOD:
   ValidQMIMessage (Public Method)

DESCRIPTION:
   Check mandatory TLV in a QMI message

   QMI Message shall NOT include SDU

PARAMETERS
   pQMIMessage    [ I ] - QMI Message buffer
   messageLen     [ I ] - Size of QMI Message buffer

RETURN VALUE:
   int - 0 for success (no error)
         Negative errno for error
         Positive for QMI error code
===========================================================================*/
static int ValidQMIMessage(
   void *   pQMIMessage,
   u16      messageLen )
{
   char mandTLV[4];

   if (GetTLV( pQMIMessage, messageLen, 2, &mandTLV[0], 4 ) == 4)
   {
      // Found TLV
      if (*(u16 *)&mandTLV[0] != 0)
      {
         return le16_to_cpu( get_unaligned(&mandTLV[2]) );
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return -ENOMSG;
   }
}      

/*===========================================================================
METHOD:
   GetQMIMessageID (Public Method)

DESCRIPTION:
   Get the message ID of a QMI message
   
   QMI Message shall NOT include SDU

PARAMETERS
   pQMIMessage    [ I ] - QMI Message buffer
   messageLen     [ I ] - Size of QMI Message buffer

RETURN VALUE:
   int - Positive for message ID
         Negative errno for error
===========================================================================*/
static int GetQMIMessageID(
   void *   pQMIMessage,
   u16      messageLen )
{
   if (messageLen < 2)
   {
      return -ENODATA;
   }
   else
   {
      return le16_to_cpu( get_unaligned((u16 *)pQMIMessage) );
   }
}

/*=========================================================================*/
// Fill Buffers with QMI requests
/*=========================================================================*/

/*===========================================================================
METHOD:
   QMICTLGetClientIDReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI CTL Get Client ID Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID
   serviceType     [ I ] - Service type requested

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMICTLGetClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u8       serviceType )
{
   if (pBuffer == 0 || buffSize < QMICTLGetClientIDReqSize() )
   {
      return -ENOMEM;
   }

   // QMI CTL GET CLIENT ID
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))= 0x00;
   // Transaction ID
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 1) = transactionID;
   // Message ID
   put_unaligned(cpu_to_le16(0x0022), (u16 *)(pBuffer + sizeof( sQMUX ) + 2));
   // Size of TLV's
   put_unaligned(cpu_to_le16(0x0004), (u16 *)(pBuffer + sizeof( sQMUX ) + 4));
      // QMI Service Type
      *(u8 *)(pBuffer + sizeof( sQMUX ) + 6)  = 0x01;
      // Size
   put_unaligned(cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 7));
      // QMI svc type
      *(u8 *)(pBuffer + sizeof( sQMUX ) + 9)  = serviceType;

   // success
   return sizeof( sQMUX ) + 10;
}

/*===========================================================================
METHOD:
   QMICTLReleaseClientIDReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI CTL Release Client ID Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID
   clientID        [ I ] - Service type requested

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMICTLReleaseClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u16      clientID )
{
   if (pBuffer == 0 || buffSize < QMICTLReleaseClientIDReqSize() )
   {
      return -ENOMEM;
   }

   DBG(  "buffSize: 0x%x, transactionID: 0x%x, clientID: 0x%x,\n",
         buffSize, transactionID, clientID );

   // QMI CTL RELEASE CLIENT ID REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 1 ) = transactionID;
   // Message ID
   put_unaligned( cpu_to_le16(0x0023), (u16 *)(pBuffer + sizeof( sQMUX ) + 2) );
   // Size of TLV's
   put_unaligned( cpu_to_le16(0x0005), (u16 *)(pBuffer + sizeof( sQMUX ) + 4) );
      // Release client ID
      *(u8 *)(pBuffer + sizeof( sQMUX ) + 6)  = 0x01;
      // Size
   put_unaligned( cpu_to_le16(0x0002), (u16 *)(pBuffer + sizeof( sQMUX ) + 7));
      // QMI svs type / Client ID
   put_unaligned(cpu_to_le16(clientID), (u16 *)(pBuffer + sizeof( sQMUX ) + 9));
      
   // success
   return sizeof( sQMUX ) + 11;
}

/*===========================================================================
METHOD:
   QMICTLReadyReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI CTL Get Version Info Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMICTLReadyReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID )
{
   if (pBuffer == 0 || buffSize < QMICTLReadyReqSize() )
   {
      return -ENOMEM;
   }

   DBG("buffSize: 0x%x, transactionID: 0x%x\n", buffSize, transactionID);

   // QMI CTL GET VERSION INFO REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 1) = transactionID;
   // Message ID
   put_unaligned( cpu_to_le16(0x0021), (u16 *)(pBuffer + sizeof( sQMUX ) + 2) );
   // Size of TLV's
   put_unaligned( cpu_to_le16(0x0000), (u16 *)(pBuffer + sizeof( sQMUX ) + 4) );

   // success
   return sizeof( sQMUX ) + 6;
}

/*===========================================================================
METHOD:
   QMIWDSSetEventReportReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI WDS Set Event Report Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMIWDSSetEventReportReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID )
{
   if (pBuffer == 0 || buffSize < QMIWDSSetEventReportReqSize() )
   {
      return -ENOMEM;
   }

   // QMI WDS SET EVENT REPORT REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   put_unaligned( cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1));
   // Message ID
   put_unaligned( cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 3));
   // Size of TLV's
   put_unaligned(cpu_to_le16(0x0008), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));
      // Report channel rate TLV
      *(u8 *)(pBuffer + sizeof( sQMUX ) + 7)  = 0x11;
      // Size
   put_unaligned( cpu_to_le16(0x0005), (u16 *)(pBuffer + sizeof( sQMUX ) + 8));
      // Stats period
      *(u8 *)(pBuffer + sizeof( sQMUX ) + 10)  = 0x01;
      // Stats mask
   put_unaligned( cpu_to_le32(0x000000ff), (u32 *)(pBuffer + sizeof( sQMUX ) + 11) );

   // success
   return sizeof( sQMUX ) + 15;
}

/*===========================================================================
METHOD:
   QMIWDSGetPKGSRVCStatusReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI WDS Get PKG SRVC Status Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMIWDSGetPKGSRVCStatusReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID )
{
   if (pBuffer == 0 || buffSize < QMIWDSGetPKGSRVCStatusReqSize() )
   {
      return -ENOMEM;
   }

   // QMI WDS Get PKG SRVC Status REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   put_unaligned(cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1));
   // Message ID
   put_unaligned(cpu_to_le16(0x0022), (u16 *)(pBuffer + sizeof( sQMUX ) + 3));
   // Size of TLV's
   put_unaligned(cpu_to_le16(0x0000), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));

   // success
   return sizeof( sQMUX ) + 7;
}

#if 0
static u16 QMIWDSSetQMUXBindMuxDataPortSize( void )
{
   return sizeof( sQMUX ) + 29;
}

static u16 QMIWDSSetQMUXBindMuxDataPortReq(
   void *   pBuffer,
   u16      buffSize,
   u8 MuxId,
   u16      transactionID )
{
   if (pBuffer == 0 || buffSize < QMIWDSSetQMUXBindMuxDataPortSize() )
   {
      return -ENOMEM;
   }

   // QMI WDS Set QMUX Bind Mux Data Port REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   put_unaligned(cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1));
   // Message ID
   put_unaligned(cpu_to_le16(0x00a2), (u16 *)(pBuffer + sizeof( sQMUX ) + 3));
   // Size of TLV's
   put_unaligned(cpu_to_le16(0x0016), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));

   *(u8 *)(pBuffer + sizeof( sQMUX ) +  7) = 0x10;
   put_unaligned(cpu_to_le16(0x08), (u16 *)(pBuffer + sizeof( sQMUX ) + 8));
   put_unaligned(cpu_to_le32(0x02), (u32 *)(pBuffer + sizeof( sQMUX ) + 10)); // ep_type
   put_unaligned(cpu_to_le32(0x04), (u32 *)(pBuffer + sizeof( sQMUX ) + 14)); // iface_id
   
   *(u8 *)(pBuffer + sizeof( sQMUX ) +  18) = 0x11;
   put_unaligned(cpu_to_le16(0x01), (u16 *)(pBuffer + sizeof( sQMUX ) + 19));
   *(u8 *)(pBuffer + sizeof( sQMUX ) +  21) = MuxId;  // MuxId
   
   *(u8 *)(pBuffer + sizeof( sQMUX ) +  22) = 0x13;
   put_unaligned(cpu_to_le16(0x04), (u16 *)(pBuffer + sizeof( sQMUX ) + 23));
   put_unaligned(cpu_to_le32(0x01), (u32 *)(pBuffer + sizeof( sQMUX ) + 25));
       
   // success
   return sizeof( sQMUX ) + 29;
}
#endif

/*===========================================================================
METHOD:
   QMIDMSGetMEIDReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI DMS Get Serial Numbers Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMIDMSGetMEIDReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID )
{
   if (pBuffer == 0 || buffSize < QMIDMSGetMEIDReqSize() )
   {
      return -ENOMEM;
   }

   // QMI DMS GET SERIAL NUMBERS REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   put_unaligned( cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1) );
   // Message ID
   put_unaligned( cpu_to_le16(0x0025), (u16 *)(pBuffer + sizeof( sQMUX ) + 3) );
   // Size of TLV's
   put_unaligned( cpu_to_le16(0x0000), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));

   // success
   return sizeof( sQMUX ) + 7;
}

/*===========================================================================
METHOD:
   QMIWDASetDataFormatReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI WDA Set Data Format Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMIWDASetDataFormatReq(
   void *   pBuffer,
   u16      buffSize,
   bool     bRawIPMode, int qmap_version, u32 rx_size,
   u16      transactionID )
{
if (qmap_version) {
    struct QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG *pMUXMsg = (struct QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG *)(pBuffer + sizeof( sQMUX ));

    pMUXMsg->CtlFlags = 0x00;
    put_unaligned( cpu_to_le16(transactionID), &pMUXMsg->TransactionId);
    put_unaligned( cpu_to_le16(0x0020), &pMUXMsg->Type);
    put_unaligned( cpu_to_le16(sizeof( struct QMIWDS_ADMIN_SET_DATA_FORMAT_REQ_MSG) - 7), &pMUXMsg->Length);

 //Indicates whether the Quality of Service(QOS) data format is used by the client.
    pMUXMsg->QosDataFormatTlv.TLVType = 0x10;
    pMUXMsg->QosDataFormatTlv.TLVLength = cpu_to_le16(0x0001);
    pMUXMsg->QosDataFormatTlv.QOSSetting = 0; /* no-QOS header */
//Underlying Link Layer Protocol
    pMUXMsg->UnderlyingLinkLayerProtocolTlv.TLVType = 0x11; 
    pMUXMsg->UnderlyingLinkLayerProtocolTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->UnderlyingLinkLayerProtocolTlv.Value = cpu_to_le32(0x02);     /* Set Ethernet  mode */
//Uplink (UL) data aggregation protocol to be used for uplink data transfer.
    pMUXMsg->UplinkDataAggregationProtocolTlv.TLVType = 0x12; 
    pMUXMsg->UplinkDataAggregationProtocolTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->UplinkDataAggregationProtocolTlv.Value = cpu_to_le32(qmap_version); //UL QMAP is enabled
//Downlink (DL) data aggregation protocol to be used for downlink data transfer
    pMUXMsg->DownlinkDataAggregationProtocolTlv.TLVType = 0x13; 
    pMUXMsg->DownlinkDataAggregationProtocolTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->DownlinkDataAggregationProtocolTlv.Value = cpu_to_le32(qmap_version); //UL QMAP is enabled
//Maximum number of datagrams in a single aggregated packet on downlink
    pMUXMsg->DownlinkDataAggregationMaxDatagramsTlv.TLVType = 0x15; 
    pMUXMsg->DownlinkDataAggregationMaxDatagramsTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->DownlinkDataAggregationMaxDatagramsTlv.Value = cpu_to_le32(rx_size/1024);
//Maximum size in bytes of a single aggregated packet allowed on downlink
    pMUXMsg->DownlinkDataAggregationMaxSizeTlv.TLVType = 0x16; 
    pMUXMsg->DownlinkDataAggregationMaxSizeTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->DownlinkDataAggregationMaxSizeTlv.Value = cpu_to_le32(rx_size);
//Peripheral End Point ID
    pMUXMsg->epTlv.TLVType = 0x17; 
    pMUXMsg->epTlv.TLVLength = cpu_to_le16(8);
    pMUXMsg->epTlv.ep_type = cpu_to_le32(0x02); // DATA_EP_TYPE_BAM_DMUX
    pMUXMsg->epTlv.iface_id = cpu_to_le32(0x04); 
//Specifies the minimum padding bytes to be added in between aggregated downlink QMAP packets.
    pMUXMsg->dl_minimum_padding.TLVType = 0x19; 
    pMUXMsg->dl_minimum_padding.TLVLength = cpu_to_le16(4);
    pMUXMsg->dl_minimum_padding.Value = cpu_to_le32(0);
//Maximum number of datagrams in a single aggregated packet on uplink
    pMUXMsg->UplinkDataAggregationMaxDatagramsTlv.TLVType = 27; 
    pMUXMsg->UplinkDataAggregationMaxDatagramsTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->UplinkDataAggregationMaxDatagramsTlv.Value = cpu_to_le32(11);
//Maximum size in bytes of a single aggregated packet allowed on uplink
    pMUXMsg->UplinkDataAggregationMaxSizeTlv.TLVType = 28; 
    pMUXMsg->UplinkDataAggregationMaxSizeTlv.TLVLength = cpu_to_le16(4);
    pMUXMsg->UplinkDataAggregationMaxSizeTlv.Value = cpu_to_le32(8*1024);
}
else {
   if (pBuffer == 0 || buffSize < QMIWDASetDataFormatReqSize(qmap_version) )
   {
      return -ENOMEM;
   }

   // QMI WDA SET DATA FORMAT REQ
   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;

   // Transaction ID
   put_unaligned( cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1) );

   // Message ID
   put_unaligned( cpu_to_le16(0x0020), (u16 *)(pBuffer + sizeof( sQMUX ) + 3) );

   // Size of TLV's
   put_unaligned( cpu_to_le16(0x000b), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));

   /* TLVType QOS Data Format 1 byte  */
   *(u8 *)(pBuffer + sizeof( sQMUX ) +  7) = 0x10; // type data format

   /* TLVLength  2 bytes - see spec */
   put_unaligned( cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 8)); 

   /* DataFormat: 0-default; 1-QoS hdr present 2 bytes */
#ifdef QOS_MODE
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 10) = 1; /* QOS header */
#else
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 10) = 0; /* no-QOS header */
#endif

   /* TLVType Link-Layer Protocol  (Optional) 1 byte */
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 11) = 0x11;

   /* TLVLength 2 bytes */
   put_unaligned( cpu_to_le16(0x0004), (u16 *)(pBuffer + sizeof( sQMUX ) + 12));

   /* LinkProt: 0x1 - ETH; 0x2 - rawIP  4 bytes */
if (bRawIPMode) { //#ifdef DATA_MODE_RP
   /* Set RawIP mode */
   put_unaligned( cpu_to_le32(0x00000002), (u32 *)(pBuffer + sizeof( sQMUX ) + 14));
   DBG("Request RawIP Data Format\n");
} else { //#else
   /* Set Ethernet  mode */
   put_unaligned( cpu_to_le32(0x00000001), (u32 *)(pBuffer + sizeof( sQMUX ) + 14));
   DBG("Request Ethernet Data Format\n");
} //#endif

}

   // success
   return QMIWDASetDataFormatReqSize(qmap_version);
}

#if 0
static int QMIWDASetDataQmapReq(
		void *   pBuffer,
		u16      buffSize,
		u16      transactionID )
{
    // QMI WDA SET DATA FORMAT REQ
    // Request
    *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;

    // Transaction ID
    put_unaligned( cpu_to_le16(transactionID), (u16 *)(pBuffer + sizeof( sQMUX ) + 1) );

    // Message ID
    put_unaligned( cpu_to_le16(0x002B), (u16 *)(pBuffer + sizeof( sQMUX ) + 3) );

    // Size of TLV's
    put_unaligned( cpu_to_le16(0x0004), (u16 *)(pBuffer + sizeof( sQMUX ) + 5));

    /* TLVType QMAP In-Band Flow Control 1 byte  */
    *(u8 *)(pBuffer + sizeof( sQMUX ) +  7) = 0x10;
    put_unaligned( cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 8)); 
    *(u8 *)(pBuffer + sizeof( sQMUX ) +  10) = 0x01;

    // success
    return ( sizeof( sQMUX ) + 11);
}
#endif

#if 0
/*===========================================================================
METHOD:
   QMICTLSetDataFormatReqSize (Public Method)

DESCRIPTION:
   Get size of buffer needed for QMUX + QMICTLSetDataFormatReq
 
RETURN VALUE:
   u16 - size of buffer
===========================================================================*/
static u16  QMICTLSetDataFormatReqSize( void )
{
   return sizeof( sQMUX ) + 15; 
}

/*===========================================================================
METHOD:
   QMICTLSetDataFormatReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI CTL Set Data Format Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMICTLSetDataFormatReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID )
{
   if (pBuffer == 0 || buffSize < QMICTLSetDataFormatReqSize() )
   {
      return -ENOMEM;
   }

   /* QMI CTL Set Data Format Request */
   /* Request */
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00; // QMICTL_FLAG_REQUEST
   
   /* Transaction ID 1 byte */
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 1) = transactionID; /* 1 byte as in spec */

   /* QMICTLType  2 bytes */
   put_unaligned( cpu_to_le16(0x0026), (u16 *)(pBuffer + sizeof( sQMUX ) + 2));

   /* Length  2 bytes  of 2 TLVs  each - see spec */
   put_unaligned( cpu_to_le16(0x0009), (u16 *)(pBuffer + sizeof( sQMUX ) + 4));

   /* TLVType Data Format (Mandatory)  1 byte  */
   *(u8 *)(pBuffer + sizeof( sQMUX ) +  6) = 0x01; // type data format

   /* TLVLength  2 bytes - see spec */
   put_unaligned( cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 7)); 

   /* DataFormat: 0-default; 1-QoS hdr present 2 bytes */
#ifdef QOS_MODE
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 9) = 1; /* QOS header */
#else
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 9) = 0; /* no-QOS header */
#endif

    /* TLVType Link-Layer Protocol  (Optional) 1 byte */
    *(u8 *)(pBuffer + sizeof( sQMUX ) + 10) = TLV_TYPE_LINK_PROTO;

    /* TLVLength 2 bytes */
    put_unaligned( cpu_to_le16(0x0002), (u16 *)(pBuffer + sizeof( sQMUX ) + 11));

   /* LinkProt: 0x1 - ETH; 0x2 - rawIP  2 bytes */
#ifdef DATA_MODE_RP
   /* Set RawIP mode */
   put_unaligned( cpu_to_le16(0x0002), (u16 *)(pBuffer + sizeof( sQMUX ) + 13));
   DBG("Request RawIP Data Format\n");
#else
   /* Set Ethernet  mode */
   put_unaligned( cpu_to_le16(0x0001), (u16 *)(pBuffer + sizeof( sQMUX ) + 13));
   DBG("Request Ethernet Data Format\n");
#endif

   /* success */
   return sizeof( sQMUX ) + 15;

}
#endif

/*===========================================================================
METHOD:
   QMICTLSyncReq (Public Method)

DESCRIPTION:
   Fill buffer with QMI CTL Sync Request

PARAMETERS
   pBuffer         [ 0 ] - Buffer to be filled
   buffSize        [ I ] - Size of pBuffer
   transactionID   [ I ] - Transaction ID

RETURN VALUE:
   int - Positive for resulting size of pBuffer
         Negative errno for error
===========================================================================*/
static int QMICTLSyncReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID )
{
   if (pBuffer == 0 || buffSize < QMICTLSyncReqSize() )
   {
      return -ENOMEM;
   }

   // Request
   *(u8 *)(pBuffer + sizeof( sQMUX ))  = 0x00;
   // Transaction ID
   *(u8 *)(pBuffer + sizeof( sQMUX ) + 1) = transactionID;
   // Message ID
   put_unaligned( cpu_to_le16(0x0027), (u16 *)(pBuffer + sizeof( sQMUX ) + 2) );
   // Size of TLV's
   put_unaligned( cpu_to_le16(0x0000), (u16 *)(pBuffer + sizeof( sQMUX ) + 4) );

  // success
  return sizeof( sQMUX ) + 6;
}

/*=========================================================================*/
// Parse data from QMI responses
/*=========================================================================*/

/*===========================================================================
METHOD:
   QMICTLGetClientIDResp (Public Method)

DESCRIPTION:
   Parse the QMI CTL Get Client ID Resp

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer
   pClientID       [ 0 ] - Recieved client ID

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMICTLGetClientIDResp(
   void * pBuffer,
   u16    buffSize,
   u16 *  pClientID )
{
   int result;
   
   // Ignore QMUX and SDU
   //    QMI CTL SDU is 2 bytes, not 3
   u8 offset = sizeof( sQMUX ) + 2;

   if (pBuffer == 0 || buffSize < offset)
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   result = GetQMIMessageID( pBuffer, buffSize );
   if (result != 0x22)
   {
      return -EFAULT;
   }

   result = ValidQMIMessage( pBuffer, buffSize );
   if (result != 0)
   {
      return -EFAULT;
   }

   result = GetTLV( pBuffer, buffSize, 0x01, pClientID, 2 );
   if (result != 2)
   {
      return -EFAULT;
   }

   return 0;
}

/*===========================================================================
METHOD:
   QMICTLReleaseClientIDResp (Public Method)

DESCRIPTION:
   Verify the QMI CTL Release Client ID Resp is valid

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMICTLReleaseClientIDResp(
   void *   pBuffer,
   u16      buffSize )
{
   int result;
   
   // Ignore QMUX and SDU
   //    QMI CTL SDU is 2 bytes, not 3
   u8 offset = sizeof( sQMUX ) + 2;

   if (pBuffer == 0 || buffSize < offset)
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   result = GetQMIMessageID( pBuffer, buffSize );
   if (result != 0x23)
   {
      return -EFAULT;
   }

   result = ValidQMIMessage( pBuffer, buffSize );
   if (result != 0)
   {
      return -EFAULT;
   }

   return 0;
}

/*===========================================================================
METHOD:
   QMIWDSEventResp (Public Method)

DESCRIPTION:
   Parse the QMI WDS Set Event Report Resp/Indication or
      QMI WDS Get PKG SRVC Status Resp/Indication

   Return parameters will only be updated if value was received

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer
   pTXOk           [ O ] - Number of transmitted packets without errors
   pRXOk           [ O ] - Number of recieved packets without errors
   pTXErr          [ O ] - Number of transmitted packets with framing errors
   pRXErr          [ O ] - Number of recieved packets with framing errors
   pTXOfl          [ O ] - Number of transmitted packets dropped due to overflow
   pRXOfl          [ O ] - Number of recieved packets dropped due to overflow
   pTXBytesOk      [ O ] - Number of transmitted bytes without errors
   pRXBytesOk      [ O ] - Number of recieved bytes without errors
   pbLinkState     [ 0 ] - Is the link active?
   pbReconfigure   [ 0 ] - Must interface be reconfigured? (reset IP address)

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMIWDSEventResp(
   void *   pBuffer,
   u16      buffSize,
   u32 *    pTXOk,
   u32 *    pRXOk,
   u32 *    pTXErr,
   u32 *    pRXErr,
   u32 *    pTXOfl,
   u32 *    pRXOfl,
   u64 *    pTXBytesOk,
   u64 *    pRXBytesOk,
   bool *   pbLinkState,
   bool *   pbReconfigure )
{
   int result;
   u8 pktStatusRead[2];

   // Ignore QMUX and SDU
   u8 offset = sizeof( sQMUX ) + 3;

   if (pBuffer == 0 
   || buffSize < offset
   || pTXOk == 0
   || pRXOk == 0
   || pTXErr == 0
   || pRXErr == 0
   || pTXOfl == 0
   || pRXOfl == 0
   || pTXBytesOk == 0
   || pRXBytesOk == 0
   || pbLinkState == 0
   || pbReconfigure == 0 )
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   // Note: Indications.  No Mandatory TLV required

   result = GetQMIMessageID( pBuffer, buffSize );
   // QMI WDS Set Event Report Resp
   if (result == 0x01)
   {
      // TLV's are not mandatory
      GetTLV( pBuffer, buffSize, 0x10, (void*)pTXOk, 4 );
      put_unaligned( le32_to_cpu(*pTXOk), pTXOk);
      GetTLV( pBuffer, buffSize, 0x11, (void*)pRXOk, 4 );
      put_unaligned( le32_to_cpu(*pRXOk), pRXOk);
      GetTLV( pBuffer, buffSize, 0x12, (void*)pTXErr, 4 );
      put_unaligned( le32_to_cpu(*pTXErr), pTXErr);
      GetTLV( pBuffer, buffSize, 0x13, (void*)pRXErr, 4 );
      put_unaligned( le32_to_cpu(*pRXErr), pRXErr);
      GetTLV( pBuffer, buffSize, 0x14, (void*)pTXOfl, 4 );
      put_unaligned( le32_to_cpu(*pTXOfl), pTXOfl);
      GetTLV( pBuffer, buffSize, 0x15, (void*)pRXOfl, 4 );
      put_unaligned( le32_to_cpu(*pRXOfl), pRXOfl);
      GetTLV( pBuffer, buffSize, 0x19, (void*)pTXBytesOk, 8 );
      put_unaligned( le64_to_cpu(*pTXBytesOk), pTXBytesOk);
      GetTLV( pBuffer, buffSize, 0x1A, (void*)pRXBytesOk, 8 );
      put_unaligned( le64_to_cpu(*pRXBytesOk), pRXBytesOk);
   }
   // QMI WDS Get PKG SRVC Status Resp
   else if (result == 0x22)
   {
      result = GetTLV( pBuffer, buffSize, 0x01, &pktStatusRead[0], 2 );
      // 1 or 2 bytes may be received
      if (result >= 1)
      {
         if (pktStatusRead[0] == 0x02)
         {
            *pbLinkState = true;
         }
         else
         {
            *pbLinkState = false;
         }
      }
      if (result == 2)
      {
         if (pktStatusRead[1] == 0x01)
         {
            *pbReconfigure = true;
         }
         else
         {
            *pbReconfigure = false;
         }
      }
      
      if (result < 0)
      {
         return result;
      }
   }
   else
   {
      return -EFAULT;
   }

   return 0;
}

/*===========================================================================
METHOD:
   QMIDMSGetMEIDResp (Public Method)

DESCRIPTION:
   Parse the QMI DMS Get Serial Numbers Resp

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer
   pMEID           [ O ] - Device MEID
   meidSize        [ I ] - Size of MEID buffer (at least 14)

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMIDMSGetMEIDResp(
   void *   pBuffer,
   u16      buffSize,
   char *   pMEID,
   int      meidSize )
{
   int result;

   // Ignore QMUX and SDU
   u8 offset = sizeof( sQMUX ) + 3;

   if (pBuffer == 0 || buffSize < offset || meidSize < 14)
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   result = GetQMIMessageID( pBuffer, buffSize );
   if (result != 0x25)
   {
      return -EFAULT;
   }

   result = ValidQMIMessage( pBuffer, buffSize );
   if (result != 0)
   {
      return -EFAULT;
   }

   result = GetTLV( pBuffer, buffSize, 0x12, (void*)pMEID, 14 );
   if (result != 14)
   {
      return -EFAULT;
   }

   return 0;
}

/*===========================================================================
METHOD:
   QMIWDASetDataFormatResp (Public Method)

DESCRIPTION:
   Parse the QMI WDA Set Data Format Response

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMIWDASetDataFormatResp(
   void *   pBuffer,
   u16      buffSize, bool bRawIPMode, int *qmap_version, int *rx_size, int *tx_size, QMAP_SETTING *set)
{

   int result;

   u8 pktLinkProtocol[4];

   // Ignore QMUX and SDU
   // QMI SDU is 3 bytes
   u8 offset = sizeof( sQMUX ) + 3;

   if (pBuffer == 0 || buffSize < offset)
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   result = GetQMIMessageID( pBuffer, buffSize );
   if (result != 0x20)
   {
      return -EFAULT;
   }

   /* Check response message result TLV */
   result = ValidQMIMessage( pBuffer, buffSize );
   if (result != 0)
   {
      DBG("EFAULT: Data Format Mode Bad Response\n"); 
//      return -EFAULT;
      return 0;
   }

   /* Check response message link protocol */
   result = GetTLV( pBuffer, buffSize, 0x11,
                     &pktLinkProtocol[0], 4);
   if (result != 4)
   {
      DBG("EFAULT: Wrong TLV format\n"); 
      return 0;
   }

if (bRawIPMode) { ////#ifdef DATA_MODE_RP
   if (pktLinkProtocol[0] != 2)
   {
      DBG("EFAULT: Data Format Cannot be set to RawIP Mode\n"); 
      return pktLinkProtocol[0];
   }
   DBG("Data Format Set to RawIP\n");
} else { ////#else
   if (pktLinkProtocol[0] != 1)
   {
      DBG("EFAULT: Data Format Cannot be set to Ethernet Mode\n"); 
      return pktLinkProtocol[0];
   }
   DBG("Data Format Set to Ethernet Mode \n");
} //#endif

    GetTLV( pBuffer, buffSize, 0x12, qmap_version, 4);
    if (le32_to_cpu(*qmap_version))
        GetTLV( pBuffer, buffSize, 0x13, qmap_version, 4);

    GetTLV( pBuffer, buffSize, 0x16, rx_size, 4);
    GetTLV( pBuffer, buffSize, 0x18, tx_size, 4);

    if (set) {
        GetTLV( pBuffer, buffSize, 0x15, &set->dl_data_aggregation_max_datagrams, 4);
        GetTLV( pBuffer, buffSize, 0x16, &set->dl_data_aggregation_max_size, 4);
        GetTLV( pBuffer, buffSize, 0x17, &set->ul_data_aggregation_max_datagrams, 4);
        GetTLV( pBuffer, buffSize, 0x18, &set->ul_data_aggregation_max_size, 4);
        GetTLV( pBuffer, buffSize, 0x1a, &set->dl_minimum_padding, 4);
    }

   return pktLinkProtocol[0];
}

/*===========================================================================
METHOD:
   QMICTLSyncResp (Public Method)

DESCRIPTION:
   Validate the QMI CTL Sync Response

PARAMETERS
   pBuffer         [ I ] - Buffer to be parsed
   buffSize        [ I ] - Size of pBuffer

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int QMICTLSyncResp(
   void *pBuffer,
   u16  buffSize )
{
   int result;

   // Ignore QMUX (2 bytes for QMI CTL) and SDU
   u8 offset = sizeof( sQMUX ) + 2;

   if (pBuffer == 0 || buffSize < offset)
   {
      return -ENOMEM;
   }

   pBuffer = pBuffer + offset;
   buffSize -= offset;

   result = GetQMIMessageID( pBuffer, buffSize );
   if (result != 0x27)
   {
      return -EFAULT;
   }

   result = ValidQMIMessage( pBuffer, buffSize );

   return result;
}
#endif
