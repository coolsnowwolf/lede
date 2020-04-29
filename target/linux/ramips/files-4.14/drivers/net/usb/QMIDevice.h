/*===========================================================================
FILE:
   QMIDevice.h

DESCRIPTION:
   Functions related to the QMI interface device
   
FUNCTIONS:
   Generic functions
      IsDeviceValid
      PrintHex
      GobiSetDownReason
      GobiClearDownReason
      GobiTestDownReason

   Driver level asynchronous read functions
      ResubmitIntURB
      ReadCallback
      IntCallback
      StartRead
      KillRead

   Internal read/write functions
      ReadAsync
      UpSem
      ReadSync
      WriteSyncCallback
      WriteSync

   Internal memory management functions
      GetClientID
      ReleaseClientID
      FindClientMem
      AddToReadMemList
      PopFromReadMemList
      AddToNotifyList
      NotifyAndPopNotifyList
      AddToURBList
      PopFromURBList

   Userspace wrappers
      UserspaceOpen
      UserspaceIOCTL
      UserspaceClose
      UserspaceRead
      UserspaceWrite
      UserspacePoll

   Initializer and destructor
      RegisterQMIDevice
      DeregisterQMIDevice

   Driver level client management
      QMIReady
      QMIWDSCallback
      SetupQMIWDSCallback
      QMIDMSGetMEID

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

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "Structs.h"
#include "QMI.h"

/*=========================================================================*/
// Generic functions
/*=========================================================================*/

// Basic test to see if device memory is valid
bool IsDeviceValid( sGobiUSBNet * pDev );

// Print Hex data, for debug purposes
void PrintHex(
   void *         pBuffer,
   u16            bufSize );
void PrintHex_RX(
   void *         pBuffer,
   u16            bufSize );


// Sets mDownReason and turns carrier off
void GobiSetDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason );

// Clear mDownReason and may turn carrier on
void GobiClearDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason );

// Tests mDownReason and returns whether reason is set
bool GobiTestDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason );

/*=========================================================================*/
// Driver level asynchronous read functions
/*=========================================================================*/

// Resubmit interrupt URB, re-using same values
int ResubmitIntURB( struct urb * pIntURB );

// Read callback
//    Put the data in storage and notify anyone waiting for data
void ReadCallback( struct urb * pReadURB );

// Inturrupt callback
//    Data is available, start a read URB
void IntCallback( struct urb * pIntURB );

// Start continuous read "thread"
int StartRead( sGobiUSBNet * pDev );

// Kill continuous read "thread"
void KillRead( sGobiUSBNet * pDev );

/*=========================================================================*/
// Internal read/write functions
/*=========================================================================*/

// Start asynchronous read
//     Reading client's data store, not device
int ReadAsync(
   sGobiUSBNet *    pDev,
   u16                clientID,
   u16                transactionID,
   void               (*pCallback)(sGobiUSBNet *, u16, void *, sQMIDev *),
   void *             pData,
   sQMIDev *QMIDev);

// Notification function for synchronous read
void UpSem( 
   sGobiUSBNet *    pDev,
   u16                clientID,
   void *             pData,
   sQMIDev *QMIDev );

// Start synchronous read
//     Reading client's data store, not device
int ReadSync(
   sGobiUSBNet *    pDev,
   void **            ppOutBuffer,
   u16                clientID,
   u16                transactionID,
   sQMIDev *QMIDev );

// Write callback
void WriteSyncCallback( struct urb * pWriteURB );

// Start synchronous write
int WriteSync(
   sGobiUSBNet *    pDev,
   char *             pInWriteBuffer,
   int                size,
   u16                clientID,
   sQMIDev *QMIDev );

/*=========================================================================*/
// Internal memory management functions
/*=========================================================================*/

// Create client and allocate memory
int GetClientID( 
   sGobiUSBNet *      pDev,
   u8                 serviceType,
   sQMIDev *          QMIDev );

// Release client and free memory
void ReleaseClientID(
   sGobiUSBNet *      pDev,
   u16                  clientID,
   sQMIDev *QMIDev );

// Find this client's memory
sClientMemList * FindClientMem(
   sGobiUSBNet *      pDev,
   u16                  clientID,
   sQMIDev *QMIDev );

// Add Data to this client's ReadMem list
bool AddToReadMemList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   void *               pData,
   u16                  dataSize,
   sQMIDev *QMIDev );

// Remove data from this client's ReadMem list if it matches 
// the specified transaction ID.
bool PopFromReadMemList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   void **              ppData,
   u16 *                pDataSize,
   sQMIDev *QMIDev );

// Add Notify entry to this client's notify List
bool AddToNotifyList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   void                 (* pNotifyFunct)(sGobiUSBNet *, u16, void *, sQMIDev *QMIDev),
   void *               pData,
   sQMIDev *QMIDev );

// Remove first Notify entry from this client's notify list 
//    and Run function
bool NotifyAndPopNotifyList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   sQMIDev *QMIDev );

// Add URB to this client's URB list
bool AddToURBList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   struct urb *         pURB,
   sQMIDev *QMIDev );

// Remove URB from this client's URB list
struct urb * PopFromURBList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   sQMIDev *QMIDev );

/*=========================================================================*/
// Userspace wrappers
/*=========================================================================*/

// Userspace open
int UserspaceOpen( 
   struct inode *   pInode, 
   struct file *    pFilp );

// Userspace ioctl
int UserspaceIOCTL( 
   struct inode *    pUnusedInode, 
   struct file *     pFilp,
   unsigned int      cmd, 
   unsigned long     arg );

// Userspace close
int UserspaceClose( 
   struct file *       pFilp,
   fl_owner_t          unusedFileTable );

// Userspace read (synchronous)
ssize_t UserspaceRead( 
   struct file *        pFilp,
   char __user *        pBuf, 
   size_t               size,
   loff_t *             pUnusedFpos );

// Userspace write (synchronous)
ssize_t UserspaceWrite(
   struct file *        pFilp, 
   const char __user *  pBuf, 
   size_t               size,
   loff_t *             pUnusedFpos );

unsigned int UserspacePoll(
   struct file *                  pFilp,
   struct poll_table_struct *     pPollTable );

/*=========================================================================*/
// Initializer and destructor
/*=========================================================================*/

// QMI Device initialization function
int RegisterQMIDevice( sGobiUSBNet * pDev );

// QMI Device cleanup function
void DeregisterQMIDevice( sGobiUSBNet * pDev );

/*=========================================================================*/
// Driver level client management
/*=========================================================================*/

// Check if QMI is ready for use
bool QMIReady(
   sGobiUSBNet *    pDev,
   u16                timeout );

// QMI WDS callback function
void QMIWDSCallback(
   sGobiUSBNet *    pDev,
   u16                clientID,
   void *             pData,
   sQMIDev *QMIDev);

// Fire off reqests and start async read for QMI WDS callback
int SetupQMIWDSCallback( sGobiUSBNet * pDev, sQMIDev *QMIDev );

// Register client, send req and parse MEID response, release client
int QMIDMSGetMEID( sGobiUSBNet * pDev, sQMIDev *QMIDev );

// Register client, Set QMAP req and parse response, release client
int QMIWDASetQMAP( sGobiUSBNet * pDev , u16 ClientID, sQMIDev *QMIDev);
int QMIWDASetQMAPSettings( sGobiUSBNet * pDev , u16 ClientID, sQMIDev *QMIDev);

int WDSConnect(sGobiUSBNet *pDev, sQMIDev *QMIDev);
void WDSDisConnect(int id, sGobiUSBNet *pDev, sQMIDev *QMIDev);

