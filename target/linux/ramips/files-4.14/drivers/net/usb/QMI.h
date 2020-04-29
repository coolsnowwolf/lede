/*===========================================================================
FILE:
   QMI.h

DESCRIPTION:
   QTI QMI driver header
   
FUNCTIONS:
   Generic QMUX functions
      ParseQMUX
      FillQMUX
   
   Generic QMI functions
      GetTLV
      ValidQMIMessage
      GetQMIMessageID

   Get sizes of buffers needed by QMI requests
      QMUXHeaderSize
      QMICTLGetClientIDReqSize
      QMICTLReleaseClientIDReqSize
      QMICTLReadyReqSize
      QMIWDSSetEventReportReqSize
      QMIWDSGetPKGSRVCStatusReqSize
      QMIDMSGetMEIDReqSize

   Fill Buffers with QMI requests
      QMICTLGetClientIDReq
      QMICTLReleaseClientIDReq
      QMICTLReadyReq
      QMIWDSSetEventReportReq
      QMIWDSGetPKGSRVCStatusReq
      QMIDMSGetMEIDReq
      
   Parse data from QMI responses
      QMICTLGetClientIDResp
      QMICTLReleaseClientIDResp
      QMIWDSEventResp
      QMIDMSGetMEIDResp

Copyright (c) 2011,2015 The Linux Foundation. All rights reserved.

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

#pragma once

/*=========================================================================*/
// Definitions
/*=========================================================================*/

// QMI Service Types
#define QMICTL 0
#define QMIWDS 1
#define QMIDMS 2
#define QMIWDA 0x1A

#define u8        unsigned char
#define u16       unsigned short
#define u32       unsigned int
#define u64       unsigned long long

#define bool      u8
#define true      1
#define false     0

#define ENOMEM    12
#define EFAULT    14
#define EINVAL    22
#define ENOMSG    42
#define ENODATA   61

/*=========================================================================*/
// Struct sQMUX
//
//    Structure that defines a QMUX header
/*=========================================================================*/
typedef struct sQMUX
{
   /* T\F, always 1 */
   u8         mTF;

   /* Size of message */
   u16        mLength;

   /* Control flag */
   u8         mCtrlFlag;
   
   /* Service Type */
   u8         mQMIService;
   
   /* Client ID */
   u8         mQMIClientID;

}__attribute__((__packed__)) sQMUX;

/*=========================================================================*/
// Generic QMUX functions
/*=========================================================================*/

// Remove QMUX headers from a buffer
int ParseQMUX(
   u16 *    pClientID,
   void *   pBuffer,
   u16      buffSize );

// Fill buffer with QMUX headers
int FillQMUX(
   u16      clientID,
   void *   pBuffer,
   u16      buffSize );

/*=========================================================================*/
// Generic QMI functions
/*=========================================================================*/

// Get data bufffer of a specified TLV from a QMI message
u16 GetTLV(
   void *   pQMIMessage,
   u16      messageLen,
   u8       type,
   void *   pOutDataBuf,
   u16      bufferLen );

// Check mandatory TLV in a QMI message
int ValidQMIMessage(
   void *   pQMIMessage,
   u16      messageLen );

// Get the message ID of a QMI message
int GetQMIMessageID(
   void *   pQMIMessage,
   u16      messageLen );

/*=========================================================================*/
// Get sizes of buffers needed by QMI requests
/*=========================================================================*/

// Get size of buffer needed for QMUX
u16 QMUXHeaderSize( void );

// Get size of buffer needed for QMUX + QMICTLGetClientIDReq
u16 QMICTLGetClientIDReqSize( void );

// Get size of buffer needed for QMUX + QMICTLReleaseClientIDReq
u16 QMICTLReleaseClientIDReqSize( void );

// Get size of buffer needed for QMUX + QMICTLReadyReq
u16 QMICTLReadyReqSize( void );

// Get size of buffer needed for QMUX + QMIWDSSetEventReportReq
u16 QMIWDSSetEventReportReqSize( void );

// Get size of buffer needed for QMUX + QMIWDSGetPKGSRVCStatusReq
u16 QMIWDSGetPKGSRVCStatusReqSize( void );

// Get size of buffer needed for QMUX + QMIDMSGetMEIDReq
u16 QMIDMSGetMEIDReqSize( void );

// Get size of buffer needed for QMUX + QMIWDASetDataFormatReq
u16 QMIWDASetDataFormatReqSize( void );
u16 QMIWDASetDataFormatReqSettingsSize( void );

u16 QMIWDSStartNetworkReqSize(void);
u16 QMIWDSGetRuntimeSettingsReqSize(void);

/*=========================================================================*/
// Fill Buffers with QMI requests
/*=========================================================================*/

// Fill buffer with QMI CTL Get Client ID Request
int QMICTLGetClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u8       serviceType );

// Fill buffer with QMI CTL Release Client ID Request
int QMICTLReleaseClientIDReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID,
   u16      clientID );

// Fill buffer with QMI CTL Get Version Info Request
int QMICTLReadyReq(
   void *   pBuffer,
   u16      buffSize,
   u8       transactionID );

// Fill buffer with QMI WDS Set Event Report Request
int QMIWDSSetEventReportReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI WDS Get PKG SRVC Status Request
int QMIWDSGetPKGSRVCStatusReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI DMS Get Serial Numbers Request
int QMIDMSGetMEIDReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

// Fill buffer with QMI WDA QMAP Request
int QMIWDASetDataFormatReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID,
   sGobiUSBNet * pDev);

int QMIWDASetDataFormatReqSettings(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );



int QMIWDSStartNetworkReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

int QMIWDSGetRuntimeSettingsReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

int QMIWDSSetIPFamilyPrefReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID );

int QMIWDSBindMuxPortReq(
   void *   pBuffer,
   u16      buffSize,
   u16      transactionID,
   sGobiUSBNet *pDev,
   sQMIDev *QMIDev);


/*=========================================================================*/
// Parse data from QMI responses
/*=========================================================================*/

// Parse the QMI CTL Get Client ID Resp
int QMICTLGetClientIDResp(
   void * pBuffer,
   u16    buffSize,
   u16 *  pClientID );

// Verify the QMI CTL Release Client ID Resp is valid
int QMICTLReleaseClientIDResp(
   void *   pBuffer,
   u16      buffSize );

// Parse the QMI WDS Set Event Report Resp/Indication or
//    QMI WDS Get PKG SRVC Status Resp/Indication
int QMIWDSEventResp(
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
   bool *   pbReconfigure );

int QMIWDASetDataFormatResp(
   void *   pBuffer,
   u16      buffSize,
   u32 *    ULDatagram,
   u32 *    ULDatagramSize );


// Parse the QMI DMS Get Serial Numbers Resp
int QMIDMSGetMEIDResp(
   void *   pBuffer,
   u16      buffSize,
   char *   pMEID,
   int      meidSize );

int QMIWDSStartNetworkResp(
   void *   pBuffer,
   u16      buffSize,
   char *   pMEID,
   int      meidSize );

int QMIWDSGetRuntimeSettingsResp(
   sGobiUSBNet *pDev,
   void *   pBuffer,
   u16      buffSize,
   sQMIDev *QMIDev);

int ConfigureQMAP(sGobiUSBNet *pDev);

int PreparePacket(struct usbnet *dev, struct sk_buff *skb, gfp_t flags);

void ProcessARP(struct usbnet *dev, struct sk_buff *skb);

void ArpResponse(struct usbnet *dev, struct sk_buff *skb);

unsigned short GetTransactionID(sQMIDev *QMIDev);

