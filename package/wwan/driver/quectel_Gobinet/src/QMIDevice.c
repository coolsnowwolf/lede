/*===========================================================================
FILE:
   QMIDevice.c

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

   Internal userspace wrapper functions
      UserspaceunlockedIOCTL

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
#include <linux/module.h>
#include <linux/usb/cdc.h>
#include <linux/usb.h>

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define __QUEC_INCLUDE_QMI_C__
#include "QMI.c"
#define __QUECTEL_INTER__
#include "QMIDevice.h"

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,22 ))
static int s_interval;
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,14 ))
#include <linux/devfs_fs_kernel.h>
static char devfs_name[32];
static int device_create(struct class *class,  struct device *parent, dev_t devt, const char *fmt, ...)
{
   va_list vargs;
   struct class_device *class_dev;
   int err;

   va_start(vargs, fmt);
   vsnprintf(devfs_name, sizeof(devfs_name), fmt, vargs);
   va_end(vargs);

   class_dev = class_device_create(class, devt, parent, "%s", devfs_name);
   if (IS_ERR(class_dev)) {
      err = PTR_ERR(class_dev);
      goto out;
   }

   err = devfs_mk_cdev(devt, S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP, devfs_name);
   if (err) {
      class_device_destroy(class, devt);
      goto out;
   }

   return 0;
   
out:
   return err;   
}

static void device_destroy(struct class *class, dev_t devt)
{
   class_device_destroy(class, devt);
   devfs_remove(devfs_name);
}
#endif

#ifdef CONFIG_PM
// Prototype to GobiNetSuspend function
int QuecGobiNetSuspend(
   struct usb_interface *     pIntf,
   pm_message_t               powerEvent );
#endif /* CONFIG_PM */

// IOCTL to generate a client ID for this service type
#define IOCTL_QMI_GET_SERVICE_FILE 0x8BE0 + 1

// IOCTL to get the VIDPID of the device
#define IOCTL_QMI_GET_DEVICE_VIDPID 0x8BE0 + 2

// IOCTL to get the MEID of the device
#define IOCTL_QMI_GET_DEVICE_MEID 0x8BE0 + 3

#define IOCTL_QMI_RELEASE_SERVICE_FILE_IOCTL  (0x8BE0 + 4)

// CDC GET_ENCAPSULATED_RESPONSE packet
#define CDC_GET_ENCAPSULATED_RESPONSE_LE 0x01A1ll
#define CDC_GET_ENCAPSULATED_RESPONSE_BE 0xA101000000000000ll
/* The following masks filter the common part of the encapsulated response
 * packet value for Gobi and QMI devices, ie. ignore usb interface number
 */
#define CDC_RSP_MASK_BE 0xFFFFFFFF00FFFFFFll
#define CDC_RSP_MASK_LE 0xFFFFFFE0FFFFFFFFll

static const int i = 1;
#define is_bigendian() ( (*(char*)&i) == 0 )
#define CDC_GET_ENCAPSULATED_RESPONSE(pcdcrsp, pmask)\
{\
   *pcdcrsp  = is_bigendian() ? CDC_GET_ENCAPSULATED_RESPONSE_BE \
                          : CDC_GET_ENCAPSULATED_RESPONSE_LE ; \
   *pmask = is_bigendian() ? CDC_RSP_MASK_BE \
                           : CDC_RSP_MASK_LE; \
}

// CDC CONNECTION_SPEED_CHANGE indication packet
#define CDC_CONNECTION_SPEED_CHANGE_LE 0x2AA1ll
#define CDC_CONNECTION_SPEED_CHANGE_BE 0xA12A000000000000ll
/* The following masks filter the common part of the connection speed change
 * packet value for Gobi and QMI devices
 */
#define CDC_CONNSPD_MASK_BE 0xFFFFFFFFFFFF7FFFll
#define CDC_CONNSPD_MASK_LE 0XFFF7FFFFFFFFFFFFll
#define CDC_GET_CONNECTION_SPEED_CHANGE(pcdccscp, pmask)\
{\
   *pcdccscp  = is_bigendian() ? CDC_CONNECTION_SPEED_CHANGE_BE \
                          : CDC_CONNECTION_SPEED_CHANGE_LE ; \
   *pmask = is_bigendian() ? CDC_CONNSPD_MASK_BE \
                           : CDC_CONNSPD_MASK_LE; \
}

#define SET_CONTROL_LINE_STATE_REQUEST_TYPE        0x21
#define SET_CONTROL_LINE_STATE_REQUEST             0x22
#define CONTROL_DTR                     0x01
#define CONTROL_RTS                     0x02

/*=========================================================================*/
// UserspaceQMIFops
//    QMI device's userspace file operations
/*=========================================================================*/
static struct file_operations UserspaceQMIFops = 
{
   .owner     = THIS_MODULE,
   .read      = UserspaceRead,
   .write     = UserspaceWrite,
#ifdef CONFIG_COMPAT
   .compat_ioctl = UserspaceunlockedIOCTL,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,36 ))
   .unlocked_ioctl = UserspaceunlockedIOCTL,
#else
   .ioctl     = UserspaceIOCTL,
#endif
   .open      = UserspaceOpen,
#ifdef quectel_no_for_each_process
   .release      = UserspaceClose,
#else
   .flush     = UserspaceClose,
#endif
   .poll      = UserspacePoll,
};

/*=========================================================================*/
// Generic functions
/*=========================================================================*/
static u8 QMIXactionIDGet( sGobiUSBNet *pDev)
{
   u8 transactionID;

   if( 0 == (transactionID = atomic_add_return( 1, &pDev->mQMIDev.mQMICTLTransactionID)) )
   {
      transactionID = atomic_add_return( 1, &pDev->mQMIDev.mQMICTLTransactionID );
   }
   
#if 1 //free these ununsed qmi response, or when these transactionID re-used, they will be regarded as qmi response of the qmi request that have same transactionID
    if (transactionID) {
        unsigned long flags;
        void * pReadBuffer;
        u16 readBufferSize;
   
         spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
         while (PopFromReadMemList( pDev,
                                 QMICTL,
                                 transactionID,
                                 &pReadBuffer,
                                 &readBufferSize ) == true)
        {
            kfree( pReadBuffer );
        }
        spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
    }
#endif

   return transactionID;
}

static struct usb_endpoint_descriptor *GetEndpoint(
    struct usb_interface *pintf,
    int type,
    int dir )
{
   int i;
   struct usb_host_interface *iface = pintf->cur_altsetting;
   struct usb_endpoint_descriptor *pendp;

   for( i = 0; i < iface->desc.bNumEndpoints; i++)
   {
      pendp = &iface->endpoint[i].desc;
      if( ((pendp->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir)
          &&
          (usb_endpoint_type(pendp) == type) )
      {
         return pendp;
      }
   }

   return NULL;
}

/*===========================================================================
METHOD:
   IsDeviceValid (Public Method)

DESCRIPTION:
   Basic test to see if device memory is valid

PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   bool
===========================================================================*/
static bool IsDeviceValid( sGobiUSBNet * pDev )
{
   if (pDev == NULL)
   {
      return false;
   }

   if (pDev->mbQMIValid == false)
   {
      return false;
   }
   
   return true;
} 

/*===========================================================================
METHOD:
   PrintHex (Public Method)

DESCRIPTION:
   Print Hex data, for debug purposes

PARAMETERS:
   pBuffer       [ I ] - Data buffer
   bufSize       [ I ] - Size of data buffer

RETURN VALUE:
   None
===========================================================================*/
void QuecPrintHex(
   void *      pBuffer,
   u16         bufSize )
{
   char * pPrintBuf;
   u16 pos;
   int status;
   
   if (quec_debug != 1)
   {
       return;
   }

   pPrintBuf = kmalloc( bufSize * 3 + 1, GFP_ATOMIC );
   if (pPrintBuf == NULL)
   {
      DBG( "Unable to allocate buffer\n" );
      return;
   }
   memset( pPrintBuf, 0 , bufSize * 3 + 1 );
   
   for (pos = 0; pos < bufSize; pos++)
   {
      status = snprintf( (pPrintBuf + (pos * 3)), 
                         4, 
                         "%02X ", 
                         *(u8 *)(pBuffer + pos) );
      if (status != 3)
      {
         DBG( "snprintf error %d\n", status );
         kfree( pPrintBuf );
         return;
      }
   }
   
   DBG( "   : %s\n", pPrintBuf );

   kfree( pPrintBuf );
   pPrintBuf = NULL;
   return;   
}

/*===========================================================================
METHOD:
   GobiSetDownReason (Public Method)

DESCRIPTION:
   Sets mDownReason and turns carrier off

PARAMETERS
   pDev     [ I ] - Device specific memory
   reason   [ I ] - Reason device is down

RETURN VALUE:
   None
===========================================================================*/
void QuecGobiSetDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason )
{
   DBG("%s reason=%d, mDownReason=%x\n", __func__, reason, (unsigned)pDev->mDownReason);
   
#ifdef QUECTEL_WWAN_QMAP
   if (reason == NO_NDIS_CONNECTION)
      return;
#endif
   
   set_bit( reason, &pDev->mDownReason );
   
   netif_carrier_off( pDev->mpNetDev->net );
}

/*===========================================================================
METHOD:
   GobiClearDownReason (Public Method)

DESCRIPTION:
   Clear mDownReason and may turn carrier on

PARAMETERS
   pDev     [ I ] - Device specific memory
   reason   [ I ] - Reason device is no longer down

RETURN VALUE:
   None
===========================================================================*/
void QuecGobiClearDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason )
{
   clear_bit( reason, &pDev->mDownReason );
   
   DBG("%s reason=%d, mDownReason=%x\n", __func__, reason, (unsigned)pDev->mDownReason);
#if 0 //(LINUX_VERSION_CODE >= KERNEL_VERSION( 3,11,0 ))
    netif_carrier_on( pDev->mpNetDev->net );
#else
   if (pDev->mDownReason == 0)
   {
#ifdef QUECTEL_WWAN_QMAP
      if (pDev->qmap_mode && !pDev->link_state)
         ;
      else
#endif
      netif_carrier_on( pDev->mpNetDev->net );
   }
#endif
}

/*===========================================================================
METHOD:
   GobiTestDownReason (Public Method)

DESCRIPTION:
   Test mDownReason and returns whether reason is set

PARAMETERS
   pDev     [ I ] - Device specific memory
   reason   [ I ] - Reason device is down

RETURN VALUE:
   bool
===========================================================================*/
bool QuecGobiTestDownReason(
   sGobiUSBNet *    pDev,
   u8                 reason )
{
   return test_bit( reason, &pDev->mDownReason );
}

/*=========================================================================*/
// Driver level asynchronous read functions
/*=========================================================================*/

/*===========================================================================
METHOD:
   ResubmitIntURB (Public Method)

DESCRIPTION:
   Resubmit interrupt URB, re-using same values

PARAMETERS
   pIntURB       [ I ] - Interrupt URB 

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
static int ResubmitIntURB( struct urb * pIntURB )
{
   int status;
   int interval;

   // Sanity test
   if ( (pIntURB == NULL)
   ||   (pIntURB->dev == NULL) )
   {
      return -EINVAL;
   }
 
   // Interval needs reset after every URB completion
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,22 ))
    interval = max((int)(pIntURB->ep->desc.bInterval),
                  (pIntURB->dev->speed == USB_SPEED_HIGH) ? 7 : 3);
#else
    interval = s_interval;
#endif

   // Reschedule interrupt URB
   usb_fill_int_urb( pIntURB,
                     pIntURB->dev,
                     pIntURB->pipe,
                     pIntURB->transfer_buffer,
                     pIntURB->transfer_buffer_length,
                     pIntURB->complete,
                     pIntURB->context,
                     interval );
   status = usb_submit_urb( pIntURB, GFP_ATOMIC );
   if (status != 0)
   {
      DBG( "Error re-submitting Int URB %d\n", status );
   }

   return status;
}


#ifdef QUECTEL_QMI_MERGE
static int MergeRecQmiMsg( sQMIDev * pQMIDev, struct urb * pReadURB )
{
   sQMIMsgHeader * mHeader;
   sQMIMsgPacket * mPacket;

   DBG( "%s called \n", __func__ );
   mPacket = pQMIDev->mpQmiMsgPacket;

   if(pReadURB->actual_length < sizeof(sQMIMsgHeader))
   {
       return -1;
   }

   mHeader = (sQMIMsgHeader *)pReadURB->transfer_buffer;
   if(le16_to_cpu(mHeader->idenity) != MERGE_PACKET_IDENTITY || le16_to_cpu(mHeader->version) != MERGE_PACKET_VERSION || le16_to_cpu(mHeader->cur_len) > le16_to_cpu(mHeader->total_len)) 
       return -1;

   if(le16_to_cpu(mHeader->cur_len) == le16_to_cpu(mHeader->total_len)) {
        mPacket->len = le16_to_cpu(mHeader->total_len);
        memcpy(pReadURB->transfer_buffer, pReadURB->transfer_buffer + sizeof(sQMIMsgHeader), mPacket->len);
        pReadURB->actual_length = mPacket->len;
        mPacket->len = 0;
 
        return 0;
   } 

   memcpy(mPacket->buf + mPacket->len, pReadURB->transfer_buffer + sizeof(sQMIMsgHeader), le16_to_cpu(mHeader->cur_len));
   mPacket->len += le16_to_cpu(mHeader->cur_len);

   if (le16_to_cpu(mHeader->cur_len) < MERGE_PACKET_MAX_PAYLOAD_SIZE || mPacket->len >= le16_to_cpu(mHeader->total_len)) {       
        memcpy(pReadURB->transfer_buffer, mPacket->buf, mPacket->len);
        pReadURB->actual_length = mPacket->len;
        mPacket->len = 0;
        return 0;           
   }

   return -1;
}
#endif

/*===========================================================================
METHOD:
   ReadCallback (Public Method)

DESCRIPTION:
   Put the data in storage and notify anyone waiting for data

PARAMETERS
   pReadURB       [ I ] - URB this callback is run for

RETURN VALUE:
   None
===========================================================================*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
static void ReadCallback( struct urb * pReadURB )
#else
static void ReadCallback(struct urb *pReadURB, struct pt_regs *regs)
#endif
{
   int result;
   u16 clientID;
   sClientMemList * pClientMem;
   void * pData;
   void * pDataCopy;
   u16 dataSize;
   sGobiUSBNet * pDev;
   unsigned long flags;
   u16 transactionID;

   if (pReadURB == NULL)
   {
      DBG( "bad read URB\n" );
      return;
   }
   
   pDev = pReadURB->context;
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return;
   }   

#ifdef READ_QMI_URB_ERROR
   del_timer(&pDev->mQMIDev.mReadUrbTimer);
   if ((pReadURB->status == -ECONNRESET) && (pReadURB->actual_length > 0))
      pReadURB->status = 0;
#endif

   if (pReadURB->status != 0)
   {
      DBG( "Read status = %d\n", pReadURB->status );

      // Resubmit the interrupt URB
      ResubmitIntURB( pDev->mQMIDev.mpIntURB );

      return;
   }
   DBG( "Read %d bytes\n", pReadURB->actual_length );
   
#ifdef QUECTEL_QMI_MERGE
   if(MergeRecQmiMsg(&pDev->mQMIDev, pReadURB))
   {
      DBG( "not a full packet, read again\n");
      // Resubmit the interrupt URB
      ResubmitIntURB( pDev->mQMIDev.mpIntURB );
      return;
   }
#endif
 
   pData = pReadURB->transfer_buffer;
   dataSize = pReadURB->actual_length;

   PrintHex( pData, dataSize );

#ifdef READ_QMI_URB_ERROR
   if (dataSize < (le16_to_cpu(get_unaligned((u16*)(pData + 1))) + 1)) {
      dataSize = (le16_to_cpu(get_unaligned((u16*)(pData + 1))) + 1);
      memset(pReadURB->transfer_buffer + pReadURB->actual_length, 0x00, dataSize - pReadURB->actual_length);
      INFO( "Read %d / %d bytes\n", pReadURB->actual_length, dataSize);
   }
#endif

   result = ParseQMUX( &clientID,
                       pData,
                       dataSize );
   if (result < 0)
   {
      DBG( "Read error parsing QMUX %d\n", result );

      // Resubmit the interrupt URB
      ResubmitIntURB( pDev->mQMIDev.mpIntURB );

      return;
   }
   
   // Grab transaction ID

   // Data large enough?
   if (dataSize < result + 3)
   {
      DBG( "Data buffer too small to parse\n" );

      // Resubmit the interrupt URB
      ResubmitIntURB( pDev->mQMIDev.mpIntURB );

      return;
   }
   
   // Transaction ID size is 1 for QMICTL, 2 for others
   if (clientID == QMICTL)
   {
      transactionID = *(u8*)(pData + result + 1);
   }
   else
   {
      transactionID = le16_to_cpu( get_unaligned((u16*)(pData + result + 1)) );
   }
   
   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Find memory storage for this service and Client ID
   // Not using FindClientMem because it can't handle broadcasts
   pClientMem = pDev->mQMIDev.mpClientMemList;

   while (pClientMem != NULL)
   {
      if (pClientMem->mClientID == clientID 
      ||  (pClientMem->mClientID | 0xff00) == clientID)
      {
         // Make copy of pData
         pDataCopy = kmalloc( dataSize, GFP_ATOMIC );
         if (pDataCopy == NULL)
         {
            DBG( "Error allocating client data memory\n" );

            // End critical section
            spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

            // Resubmit the interrupt URB
            ResubmitIntURB( pDev->mQMIDev.mpIntURB );

            return;             
         }

         memcpy( pDataCopy, pData, dataSize );

         if (AddToReadMemList( pDev,
                               pClientMem->mClientID,
                               transactionID,
                               pDataCopy,
                               dataSize ) == false)
         {
            DBG( "Error allocating pReadMemListEntry "
                 "read will be discarded\n" );
            kfree( pDataCopy );
            
            // End critical section
            spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

            // Resubmit the interrupt URB
            ResubmitIntURB( pDev->mQMIDev.mpIntURB );

            return;
         }

         // Success
         VDBG( "Creating new readListEntry for client 0x%04X, TID %x\n",
              clientID,
              transactionID );

         // Notify this client data exists
         NotifyAndPopNotifyList( pDev,
                                 pClientMem->mClientID,
                                 transactionID );

         // Possibly notify poll() that data exists
         wake_up_interruptible_sync( &pClientMem->mWaitQueue );

         // Not a broadcast
         if (clientID >> 8 != 0xff)
         {
            break;
         }
      }
      
      // Next element
      pClientMem = pClientMem->mpNext;
   }
   
   // End critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
   
   // Resubmit the interrupt URB
   ResubmitIntURB( pDev->mQMIDev.mpIntURB );
}

/*===========================================================================
METHOD:
   IntCallback (Public Method)

DESCRIPTION:
   Data is available, fire off a read URB

PARAMETERS
   pIntURB       [ I ] - URB this callback is run for

RETURN VALUE:
   None
===========================================================================*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
static void IntCallback( struct urb * pIntURB )
{
#else
static void IntCallback(struct urb *pIntURB, struct pt_regs *regs)
{
#endif
   int status;
   struct usb_cdc_notification *dr;
   
   sGobiUSBNet * pDev = (sGobiUSBNet *)pIntURB->context;
   dr = (struct usb_cdc_notification *)pDev->mQMIDev.mpIntBuffer;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return;
   }

   // Verify this was a normal interrupt
   if (pIntURB->status != 0)
   {
        DBG( "IntCallback: Int status = %d\n", pIntURB->status );
      
      // Ignore EOVERFLOW errors
      if (pIntURB->status != -EOVERFLOW)
      {
         // Read 'thread' dies here
         return;
      }
   }
   else
   {
      //TODO cast transfer_buffer to struct usb_cdc_notification
      
      VDBG( "IntCallback: Encapsulated Response = 0x%llx\n",
          (*(u64*)pIntURB->transfer_buffer));

     switch (dr->bNotificationType) {
      case USB_CDC_NOTIFY_RESPONSE_AVAILABLE: //0x01
         {
          // Time to read
          usb_fill_control_urb( pDev->mQMIDev.mpReadURB,
                             pDev->mpNetDev->udev,
                             usb_rcvctrlpipe( pDev->mpNetDev->udev, 0 ),
                             (unsigned char *)pDev->mQMIDev.mpReadSetupPacket,
                             pDev->mQMIDev.mpReadBuffer,
                             DEFAULT_READ_URB_LENGTH,
                             ReadCallback,
                             pDev );
          #ifdef READ_QMI_URB_ERROR
          mod_timer( &pDev->mQMIDev.mReadUrbTimer, jiffies + msecs_to_jiffies(300) );
          #endif
          status = usb_submit_urb( pDev->mQMIDev.mpReadURB, GFP_ATOMIC );
          if (status != 0)
          {
            DBG("Error submitting Read URB %d\n", status);
            // Resubmit the interrupt urb
            ResubmitIntURB(pIntURB);
            return;
          }

           // Int URB will be resubmitted during ReadCallback
           return; 
         }
      case USB_CDC_NOTIFY_SPEED_CHANGE:   //0x2a
         {
             DBG( "IntCallback: Connection Speed Change = 0x%llx\n",
              (*(u64*)pIntURB->transfer_buffer));

           // if upstream or downstream is 0, stop traffic.  Otherwise resume it
           if ((*(u32*)(pIntURB->transfer_buffer + 8) == 0)
           ||  (*(u32*)(pIntURB->transfer_buffer + 12) == 0))
           {
              GobiSetDownReason( pDev, CDC_CONNECTION_SPEED );
              DBG( "traffic stopping due to CONNECTION_SPEED_CHANGE\n" );
           }
           else
           {
              GobiClearDownReason( pDev, CDC_CONNECTION_SPEED );
              DBG( "resuming traffic due to CONNECTION_SPEED_CHANGE\n" );
           }
         }
      break;
      default:
         {
             DBG( "ignoring invalid interrupt in packet\n" );
             PrintHex( pIntURB->transfer_buffer, pIntURB->actual_length );
         }
      }
      
        // Resubmit the interrupt urb
      ResubmitIntURB( pIntURB );

      return;
   }
}

#ifdef READ_QMI_URB_ERROR
static void ReadUrbTimerFunc( struct urb * pReadURB )
{
  int result;

  INFO( "%s called (%ld).\n", __func__, jiffies );

  if ((pReadURB != NULL) && (pReadURB->status == -EINPROGRESS))
  {
     // Asynchronously unlink URB. On success, -EINPROGRESS will be returned, 
     // URB status will be set to -ECONNRESET, and ReadCallback() executed
     result = usb_unlink_urb( pReadURB );
     INFO( "%s called usb_unlink_urb, result = %d\n", __func__, result);
  }
}
#endif

/*===========================================================================
METHOD:
   StartRead (Public Method)

DESCRIPTION:
   Start continuous read "thread" (callback driven)

   Note: In case of error, KillRead() should be run
         to remove urbs and clean up memory.
   
PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
int QuecStartRead( sGobiUSBNet * pDev )
{
   int interval;
   struct usb_endpoint_descriptor *pendp;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return -ENXIO;
   }

   // Allocate URB buffers
   pDev->mQMIDev.mpReadURB = usb_alloc_urb( 0, GFP_KERNEL );
   if (pDev->mQMIDev.mpReadURB == NULL)
   {
      DBG( "Error allocating read urb\n" );
      return -ENOMEM;
   }

#ifdef READ_QMI_URB_ERROR
   setup_timer( &pDev->mQMIDev.mReadUrbTimer, (void*)ReadUrbTimerFunc, (unsigned long)pDev->mQMIDev.mpReadURB );
#endif

   pDev->mQMIDev.mpIntURB = usb_alloc_urb( 0, GFP_KERNEL );
   if (pDev->mQMIDev.mpIntURB == NULL)
   {
      DBG( "Error allocating int urb\n" );
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENOMEM;
   }

   // Create data buffers
   pDev->mQMIDev.mpReadBuffer = kmalloc( DEFAULT_READ_URB_LENGTH, GFP_KERNEL );
   if (pDev->mQMIDev.mpReadBuffer == NULL)
   {
      DBG( "Error allocating read buffer\n" );
      usb_free_urb( pDev->mQMIDev.mpIntURB );
      pDev->mQMIDev.mpIntURB = NULL;
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENOMEM;
   }
   
   pDev->mQMIDev.mpIntBuffer = kmalloc( 64, GFP_KERNEL );
   if (pDev->mQMIDev.mpIntBuffer == NULL)
   {
      DBG( "Error allocating int buffer\n" );
      kfree( pDev->mQMIDev.mpReadBuffer );
      pDev->mQMIDev.mpReadBuffer = NULL;
      usb_free_urb( pDev->mQMIDev.mpIntURB );
      pDev->mQMIDev.mpIntURB = NULL;
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENOMEM;
   }      
   
   pDev->mQMIDev.mpReadSetupPacket = kmalloc( sizeof( sURBSetupPacket ), 
                                              GFP_KERNEL );
   if (pDev->mQMIDev.mpReadSetupPacket == NULL)
   {
      DBG( "Error allocating setup packet buffer\n" );
      kfree( pDev->mQMIDev.mpIntBuffer );
      pDev->mQMIDev.mpIntBuffer = NULL;
      kfree( pDev->mQMIDev.mpReadBuffer );
      pDev->mQMIDev.mpReadBuffer = NULL;
      usb_free_urb( pDev->mQMIDev.mpIntURB );
      pDev->mQMIDev.mpIntURB = NULL;
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENOMEM;
   }

   // CDC Get Encapsulated Response packet
   pDev->mQMIDev.mpReadSetupPacket->mRequestType = 0xA1;
   pDev->mQMIDev.mpReadSetupPacket->mRequestCode = 1;
   pDev->mQMIDev.mpReadSetupPacket->mValue = 0;
   pDev->mQMIDev.mpReadSetupPacket->mIndex =
      cpu_to_le16(pDev->mpIntf->cur_altsetting->desc.bInterfaceNumber);  /* interface number */
   pDev->mQMIDev.mpReadSetupPacket->mLength = cpu_to_le16(DEFAULT_READ_URB_LENGTH);

   pendp = GetEndpoint(pDev->mpIntf, USB_ENDPOINT_XFER_INT, USB_DIR_IN);
   if (pendp == NULL)
   {
      DBG( "Invalid interrupt endpoint!\n" );
      kfree(pDev->mQMIDev.mpReadSetupPacket);
      pDev->mQMIDev.mpReadSetupPacket = NULL;
      kfree( pDev->mQMIDev.mpIntBuffer );
      pDev->mQMIDev.mpIntBuffer = NULL;
      kfree( pDev->mQMIDev.mpReadBuffer );
      pDev->mQMIDev.mpReadBuffer = NULL;
      usb_free_urb( pDev->mQMIDev.mpIntURB );
      pDev->mQMIDev.mpIntURB = NULL;
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENXIO;
   }

#ifdef QUECTEL_QMI_MERGE
   pDev->mQMIDev.mpQmiMsgPacket = kmalloc( sizeof(sQMIMsgPacket), GFP_KERNEL );
   if (pDev->mQMIDev.mpQmiMsgPacket == NULL)
   {
      DBG( "Error allocating qmi msg merge packet buffer!\n" );
      kfree(pDev->mQMIDev.mpReadSetupPacket);
      pDev->mQMIDev.mpReadSetupPacket = NULL;
      kfree( pDev->mQMIDev.mpIntBuffer );
      pDev->mQMIDev.mpIntBuffer = NULL;
      kfree( pDev->mQMIDev.mpReadBuffer );
      pDev->mQMIDev.mpReadBuffer = NULL;
      usb_free_urb( pDev->mQMIDev.mpIntURB );
      pDev->mQMIDev.mpIntURB = NULL;
      usb_free_urb( pDev->mQMIDev.mpReadURB );
      pDev->mQMIDev.mpReadURB = NULL;
      return -ENOMEM;
   }   
#endif

   // Interval needs reset after every URB completion
   interval = max((int)(pendp->bInterval),
                  (pDev->mpNetDev->udev->speed == USB_SPEED_HIGH) ? 7 : 3);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,22 ))
    s_interval = interval;
#endif
   
   // Schedule interrupt URB
   usb_fill_int_urb( pDev->mQMIDev.mpIntURB,
                     pDev->mpNetDev->udev,
                     /* QMI interrupt endpoint for the following
                      * interface configuration: DM, NMEA, MDM, NET
                      */
                     usb_rcvintpipe( pDev->mpNetDev->udev,
                                     pendp->bEndpointAddress),
                     pDev->mQMIDev.mpIntBuffer,
                     min((int)le16_to_cpu(pendp->wMaxPacketSize), 64),
                     IntCallback,
                     pDev,
                     interval );
   return usb_submit_urb( pDev->mQMIDev.mpIntURB, GFP_KERNEL );
}

/*===========================================================================
METHOD:
   KillRead (Public Method)

DESCRIPTION:
   Kill continuous read "thread"
   
PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   None
===========================================================================*/
void QuecKillRead( sGobiUSBNet * pDev )
{
   // Stop reading
   if (pDev->mQMIDev.mpReadURB != NULL)
   {
      DBG( "Killng read URB\n" );
      usb_kill_urb( pDev->mQMIDev.mpReadURB );
   }

   if (pDev->mQMIDev.mpIntURB != NULL)
   {
      DBG( "Killng int URB\n" );
      usb_kill_urb( pDev->mQMIDev.mpIntURB );
   }

   // Release buffers
   kfree( pDev->mQMIDev.mpReadSetupPacket );
   pDev->mQMIDev.mpReadSetupPacket = NULL;
   kfree( pDev->mQMIDev.mpReadBuffer );
   pDev->mQMIDev.mpReadBuffer = NULL;
   kfree( pDev->mQMIDev.mpIntBuffer );
   pDev->mQMIDev.mpIntBuffer = NULL;
   
   // Release URB's
   usb_free_urb( pDev->mQMIDev.mpReadURB );
   pDev->mQMIDev.mpReadURB = NULL;
   usb_free_urb( pDev->mQMIDev.mpIntURB );
   pDev->mQMIDev.mpIntURB = NULL;

#ifdef QUECTEL_QMI_MERGE
   kfree( pDev->mQMIDev.mpQmiMsgPacket );
   pDev->mQMIDev.mpQmiMsgPacket = NULL;
#endif
}

/*=========================================================================*/
// Internal read/write functions
/*=========================================================================*/

/*===========================================================================
METHOD:
   ReadAsync (Public Method)

DESCRIPTION:
   Start asynchronous read
   NOTE: Reading client's data store, not device

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   transactionID     [ I ] - Transaction ID or 0 for any
   pCallback         [ I ] - Callback to be executed when data is available
   pData             [ I ] - Data buffer that willl be passed (unmodified) 
                             to callback

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
static int ReadAsync(
   sGobiUSBNet *      pDev,
   u16                clientID,
   u16                transactionID,
   void               (*pCallback)(sGobiUSBNet*, u16, void *),
   void *             pData )
{
   sClientMemList * pClientMem;
   sReadMemList ** ppReadMemList;
   
   unsigned long flags;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return -ENXIO;
   }

   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Find memory storage for this client ID
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find matching client ID 0x%04X\n",
           clientID );
           
      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      return -ENXIO;
   }
   
   ppReadMemList = &(pClientMem->mpList);
   
   // Does data already exist?
   while (*ppReadMemList != NULL)
   {
      // Is this element our data?
      if (transactionID == 0 
      ||  transactionID == (*ppReadMemList)->mTransactionID)
      {
         // End critical section
         spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

         // Run our own callback
         pCallback( pDev, clientID, pData );
         
         return 0;
      }
      
      // Next
      ppReadMemList = &(*ppReadMemList)->mpNext;
   }

   // Data not found, add ourself to list of waiters
   if (AddToNotifyList( pDev,
                        clientID,
                        transactionID, 
                        pCallback, 
                        pData ) == false)
   {
      DBG( "Unable to register for notification\n" );
   }

   // End critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

   // Success
   return 0;
}

/*===========================================================================
METHOD:
   UpSem (Public Method)

DESCRIPTION:
   Notification function for synchronous read

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   pData             [ I ] - Buffer that holds semaphore to be up()-ed

RETURN VALUE:
   None
===========================================================================*/
#define QUEC_SEM_MAGIC 0x12345678
struct QuecSem {
    struct semaphore readSem;
    int magic;
};

static void UpSem( 
   sGobiUSBNet * pDev,
   u16             clientID,
   void *          pData )
{
   struct QuecSem *pSem = (struct QuecSem *)pData;

   VDBG( "0x%04X\n", clientID );

   if (pSem->magic == QUEC_SEM_MAGIC)        
      up( &(pSem->readSem) );
   else
       kfree(pSem);
   return;
}

/*===========================================================================
METHOD:
   ReadSync (Public Method)

DESCRIPTION:
   Start synchronous read
   NOTE: Reading client's data store, not device

PARAMETERS:
   pDev              [ I ] - Device specific memory
   ppOutBuffer       [I/O] - On success, will be filled with a 
                             pointer to read buffer
   clientID          [ I ] - Requester's client ID
   transactionID     [ I ] - Transaction ID or 0 for any

RETURN VALUE:
   int - size of data read for success
         negative errno for failure
===========================================================================*/
static int ReadSync(
   sGobiUSBNet *    pDev,
   void **            ppOutBuffer,
   u16                clientID,
   u16                transactionID )
{
   int result;
   sClientMemList * pClientMem;
   sNotifyList ** ppNotifyList, * pDelNotifyListEntry;
   struct QuecSem readSem;
   void * pData;
   unsigned long flags;
   u16 dataSize;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return -ENXIO;
   }
   
   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Find memory storage for this Client ID
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find matching client ID 0x%04X\n",
           clientID );
      
      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      return -ENXIO;
   }
   
   // Note: in cases where read is interrupted, 
   //    this will verify client is still valid
   while (PopFromReadMemList( pDev,
                              clientID,
                              transactionID,
                              &pData,
                              &dataSize ) == false)
   {
      // Data does not yet exist, wait
      sema_init( &readSem.readSem, 0 );
      readSem.magic = QUEC_SEM_MAGIC;

      // Add ourself to list of waiters
      if (AddToNotifyList( pDev, 
                           clientID, 
                           transactionID, 
                           UpSem, 
                           &readSem ) == false)
      {
         DBG( "unable to register for notification\n" );
         spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
         return -EFAULT;
      }

      // End critical section while we block
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

      // Wait for notification
      result = down_interruptible( &readSem.readSem );
      //if (result) INFO("down_interruptible = %d\n", result);
      if (result == -EINTR) {
         result = down_timeout(&readSem.readSem, msecs_to_jiffies(200));
         //if (result) INFO("down_timeout = %d\n", result);
      }
      if (result != 0)
      {
         DBG( "Down Timeout %d\n", result );

         // readSem will fall out of scope, 
         // remove from notify list so it's not referenced
         spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
         ppNotifyList = &(pClientMem->mpReadNotifyList);
         pDelNotifyListEntry = NULL;

         // Find and delete matching entry
         while (*ppNotifyList != NULL)
         {
            if ((*ppNotifyList)->mpData == &readSem)
            {
               pDelNotifyListEntry = *ppNotifyList;
               *ppNotifyList = (*ppNotifyList)->mpNext;
               kfree( pDelNotifyListEntry );
               break;
            }

            // Next
            ppNotifyList = &(*ppNotifyList)->mpNext;
         }

         spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
         return -EINTR;
      }
      
      // Verify device is still valid
      if (IsDeviceValid( pDev ) == false)
      {
         DBG( "Invalid device!\n" );
         return -ENXIO;
      }
      
      // Restart critical section and continue loop
      spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
   }
   
   // End Critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

   // Success
   *ppOutBuffer = pData;

   return dataSize;
}

/*===========================================================================
METHOD:
   WriteSyncCallback (Public Method)

DESCRIPTION:
   Write callback

PARAMETERS
   pWriteURB       [ I ] - URB this callback is run for

RETURN VALUE:
   None
===========================================================================*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
static void WriteSyncCallback( struct urb * pWriteURB )
#else
static void WriteSyncCallback(struct urb *pWriteURB, struct pt_regs *regs)
#endif
{
   if (pWriteURB == NULL)
   {
      DBG( "null urb\n" );
      return;
   }

   DBG( "Write status/size %d/%d\n", 
        pWriteURB->status, 
        pWriteURB->actual_length );

   // Notify that write has completed by up()-ing semeaphore
   up( (struct semaphore * )pWriteURB->context );
   
   return;
}

/*===========================================================================
METHOD:
   WriteSync (Public Method)

DESCRIPTION:
   Start synchronous write

PARAMETERS:
   pDev                 [ I ] - Device specific memory
   pWriteBuffer         [ I ] - Data to be written
   writeBufferSize      [ I ] - Size of data to be written
   clientID             [ I ] - Client ID of requester

RETURN VALUE:
   int - write size (includes QMUX)
         negative errno for failure
===========================================================================*/
static int WriteSync(
   sGobiUSBNet *          pDev,
   char *                 pWriteBuffer,
   int                    writeBufferSize,
   u16                    clientID )
{
   int result;
   struct semaphore writeSem;
   struct urb * pWriteURB;
   sURBSetupPacket *writeSetup;
   unsigned long flags;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return -ENXIO;
   }

   pWriteURB = usb_alloc_urb( 0, GFP_KERNEL );
   if (pWriteURB == NULL)
   {
      DBG( "URB mem error\n" );
      return -ENOMEM;
   }

   // Fill writeBuffer with QMUX
   result = FillQMUX( clientID, pWriteBuffer, writeBufferSize );
   if (result < 0)
   {
      usb_free_urb( pWriteURB );
      return result;
   }

   // CDC Send Encapsulated Request packet
   writeSetup = kmalloc(sizeof(sURBSetupPacket), GFP_KERNEL);
   writeSetup->mRequestType = 0x21;
   writeSetup->mRequestCode = 0;
   writeSetup->mValue = 0;
   writeSetup->mIndex = cpu_to_le16(pDev->mpIntf->cur_altsetting->desc.bInterfaceNumber);
   writeSetup->mLength = cpu_to_le16(writeBufferSize);

   // Create URB   
   usb_fill_control_urb( pWriteURB,
                         pDev->mpNetDev->udev,
                         usb_sndctrlpipe( pDev->mpNetDev->udev, 0 ),
                         (unsigned char *)writeSetup,
                         (void*)pWriteBuffer,
                         writeBufferSize,
                         NULL,
                         pDev );

   DBG( "Actual Write:\n" );
   PrintHex( pWriteBuffer, writeBufferSize );

   sema_init( &writeSem, 0 );
   
   pWriteURB->complete = WriteSyncCallback;
   pWriteURB->context = &writeSem;

   // Wake device
   result = usb_autopm_get_interface( pDev->mpIntf );
   if (result < 0)
   {
      DBG( "unable to resume interface: %d\n", result );
      
      // Likely caused by device going from autosuspend -> full suspend
      if (result == -EPERM)
      {
#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,33 ))
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
         pDev->mpNetDev->udev->auto_pm = 0;
#endif
#endif
         QuecGobiNetSuspend( pDev->mpIntf, PMSG_SUSPEND );
#endif /* CONFIG_PM */
      }
      usb_free_urb( pWriteURB );
      kfree(writeSetup);

      return result;
   }

   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   if (AddToURBList( pDev, clientID, pWriteURB ) == false)
   {
      usb_free_urb( pWriteURB );
      kfree(writeSetup);

      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );   
      usb_autopm_put_interface( pDev->mpIntf );
      return -EINVAL;
   }

   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
   result = usb_submit_urb( pWriteURB, GFP_KERNEL );
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   if (result < 0)
   {
      DBG( "submit URB error %d\n", result );
      
      // Get URB back so we can destroy it
      if (PopFromURBList( pDev, clientID ) != pWriteURB)
      {
         // This shouldn't happen
         DBG( "Didn't get write URB back\n" );
         //advoid ReleaseClientID() free again (no PopFromURBList)
      }
      else
      {
      usb_free_urb( pWriteURB );
      kfree(writeSetup);
      }

      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      usb_autopm_put_interface( pDev->mpIntf );
      return result;
   }
   
   // End critical section while we block
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );   

   // Wait for write to finish
   if (1 != 0) //(interruptible != 0)
   {
      // Allow user interrupts
      result = down_interruptible( &writeSem );
      //if (result) INFO("down_interruptible = %d\n", result);
      if (result == -EINTR) {
         result = down_timeout(&writeSem, msecs_to_jiffies(200));
         //if (result) INFO("down_interruptible = %d\n", result);
      }
   }
   else
   {
      // Ignore user interrupts
      result = 0;
      down( &writeSem );
   }

   // Write is done, release device
   usb_autopm_put_interface( pDev->mpIntf );

   // Verify device is still valid
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );

      usb_kill_urb( pWriteURB );
#if 0 //advoid ReleaseClientID() free again (no PopFromURBList)
      usb_free_urb( pWriteURB );
      kfree(writeSetup);
#endif
      return -ENXIO;
   }

   // Restart critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Get URB back so we can destroy it
   if (PopFromURBList( pDev, clientID ) != pWriteURB)
   {
      // This shouldn't happen
      DBG( "Didn't get write URB back\n" );
   
      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      usb_kill_urb( pWriteURB );
#if 0 //advoid ReleaseClientID() free again (fail PopFromURBList)
      usb_free_urb( pWriteURB );
      kfree(writeSetup);
#endif
      return -EINVAL;
   }

   // End critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );   

   if (result == 0)
   {
      // Write is finished
      if (pWriteURB->status == 0)
      {
         // Return number of bytes that were supposed to have been written,
         //   not size of QMI request
         result = writeBufferSize;
      }
      else
      {
         DBG( "bad status = %d\n", pWriteURB->status );
         
         // Return error value
         result = pWriteURB->status;
      }
   }
   else
   {
      // We have been forcibly interrupted
      DBG( "Interrupted %d !!!\n", result );
      DBG( "Device may be in bad state and need reset !!!\n" );

      // URB has not finished
      usb_kill_urb( pWriteURB );
   }

   usb_free_urb( pWriteURB );
   kfree(writeSetup);

   return result;
}

/*=========================================================================*/
// Internal memory management functions
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetClientID (Public Method)

DESCRIPTION:
   Request a QMI client for the input service type and initialize memory
   structure

PARAMETERS:
   pDev           [ I ] - Device specific memory
   serviceType    [ I ] - Desired QMI service type

RETURN VALUE:
   int - Client ID for success (positive)
         Negative errno for error
===========================================================================*/
static int GetClientID( 
   sGobiUSBNet *      pDev,
   u8                 serviceType )
{
   u16 clientID;
   sClientMemList ** ppClientMem;
   int result;
   void * pWriteBuffer;
   u16 writeBufferSize;
   void * pReadBuffer;
   u16 readBufferSize;
   unsigned long flags;
   u8 transactionID;
   
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device!\n" );
      return -ENXIO;
   }

   // Run QMI request to be asigned a Client ID
   if (serviceType != 0)
   {
      writeBufferSize = QMICTLGetClientIDReqSize();
      pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
      if (pWriteBuffer == NULL)
      {
         return -ENOMEM;
      }

      transactionID = QMIXactionIDGet( pDev );

      result = QMICTLGetClientIDReq( pWriteBuffer, 
                                     writeBufferSize,
                                     transactionID,
                                     serviceType );
      if (result < 0)
      {
         kfree( pWriteBuffer );
         return result;
      }

      
      result = WriteSync( pDev,
                          pWriteBuffer,
                          writeBufferSize,
                          QMICTL );
      kfree( pWriteBuffer );

      if (result < 0)
      {
         return result;
      }

      result = ReadSync( pDev,
                         &pReadBuffer,
                         QMICTL,
                         transactionID );
      if (result < 0)
      {
         DBG( "bad read data %d\n", result );
         return result;
      }
      readBufferSize = result;

      result = QMICTLGetClientIDResp( pReadBuffer,
                                      readBufferSize,
                                      &clientID );

     /* Upon return from QMICTLGetClientIDResp, clientID
      * low address contains the Service Number (SN), and
      * clientID high address contains Client Number (CN)
      * For the ReadCallback to function correctly,we swap
      * the SN and CN on a Big Endian architecture.
      */
      clientID = le16_to_cpu(clientID);

      kfree( pReadBuffer );

      if (result < 0)
      {
         return result;
      }
   }
   else
   {
      // QMI CTL will always have client ID 0
      clientID = 0;
   }

   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Verify client is not already allocated
   if (FindClientMem( pDev, clientID ) != NULL)
   {
      DBG( "Client memory already exists\n" );

      // End Critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      return -ETOOMANYREFS;
   }

   // Go to last entry in client mem list
   ppClientMem = &pDev->mQMIDev.mpClientMemList;
   while (*ppClientMem != NULL)
   {
      ppClientMem = &(*ppClientMem)->mpNext;
   }
   
   // Create locations for read to place data into
   *ppClientMem = kmalloc( sizeof( sClientMemList ), GFP_ATOMIC );
   if (*ppClientMem == NULL)
   {
      DBG( "Error allocating read list\n" );

      // End critical section
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      return -ENOMEM;
   }
      
   (*ppClientMem)->mClientID = clientID;
   (*ppClientMem)->mpList = NULL;
   (*ppClientMem)->mpReadNotifyList = NULL;
   (*ppClientMem)->mpURBList = NULL;
   (*ppClientMem)->mpNext = NULL;

   // Initialize workqueue for poll()
   init_waitqueue_head( &(*ppClientMem)->mWaitQueue );

   // End Critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
   
   return (int)( (*ppClientMem)->mClientID );
}

/*===========================================================================
METHOD:
   ReleaseClientID (Public Method)

DESCRIPTION:
   Release QMI client and free memory

PARAMETERS:
   pDev           [ I ] - Device specific memory
   clientID       [ I ] - Requester's client ID

RETURN VALUE:
   None
===========================================================================*/
static void ReleaseClientID(
   sGobiUSBNet *    pDev,
   u16                clientID )
{
   int result;
   sClientMemList ** ppDelClientMem;
   sClientMemList * pNextClientMem;
   struct urb * pDelURB;
   void * pDelData;
   u16 dataSize;
   void * pWriteBuffer;
   u16 writeBufferSize;
   void * pReadBuffer;
   u16 readBufferSize;
   unsigned long flags;
   u8 transactionID;

   // Is device is still valid?
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "invalid device\n" );
      return;
   }
   
   DBG( "releasing 0x%04X\n", clientID );

   // Run QMI ReleaseClientID if this isn't QMICTL   
   if (clientID != QMICTL && pDev->mpNetDev->udev->state)
   {
      // Note: all errors are non fatal, as we always want to delete 
      //    client memory in latter part of function
      
      writeBufferSize = QMICTLReleaseClientIDReqSize();
      pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
      if (pWriteBuffer == NULL)
      {
         DBG( "memory error\n" );
      }
      else
      {
         transactionID = QMIXactionIDGet( pDev );

         result = QMICTLReleaseClientIDReq( pWriteBuffer, 
                                            writeBufferSize,
                                            transactionID,
                                            clientID );
         if (result < 0)
         {
            kfree( pWriteBuffer );
            DBG( "error %d filling req buffer\n", result );
         }
         else
         {
            result = WriteSync( pDev,
                                pWriteBuffer,
                                writeBufferSize,
                                QMICTL );
            kfree( pWriteBuffer );

            if (result < 0)
            {
               DBG( "bad write status %d\n", result );
            }
            else
            {
               result = ReadSync( pDev,
                                  &pReadBuffer,
                                  QMICTL,
                                  transactionID );
               if (result < 0)
               {
                  DBG( "bad read status %d\n", result );
               }
               else
               {
                  readBufferSize = result;

                  result = QMICTLReleaseClientIDResp( pReadBuffer,
                                                      readBufferSize );
                  kfree( pReadBuffer );

                  if (result < 0)
                  {
                     DBG( "error %d parsing response\n", result );
                  }
               }
            }
         }
      }
   }

   // Cleaning up client memory
   
   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Can't use FindClientMem, I need to keep pointer of previous
   ppDelClientMem = &pDev->mQMIDev.mpClientMemList;
   while (*ppDelClientMem != NULL)
   {
      if ((*ppDelClientMem)->mClientID == clientID)
      {
         pNextClientMem = (*ppDelClientMem)->mpNext;

         // Notify all clients
         while (NotifyAndPopNotifyList( pDev,
                                        clientID,
                                        0 ) == true );         

         // Kill and free all URB's
         pDelURB = PopFromURBList( pDev, clientID );
         while (pDelURB != NULL)
         {
            spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
            usb_kill_urb( pDelURB );
            usb_free_urb( pDelURB );
            spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
            pDelURB = PopFromURBList( pDev, clientID );
         }

         // Free any unread data
         while (PopFromReadMemList( pDev, 
                                    clientID,
                                    0,
                                    &pDelData,
                                    &dataSize ) == true )
         {
            kfree( pDelData );
         }

         // Delete client Mem
         if (!waitqueue_active( &(*ppDelClientMem)->mWaitQueue))
            kfree( *ppDelClientMem );
         else
            INFO("memory leak!\n");

         // Overwrite the pointer that was to this client mem
         *ppDelClientMem = pNextClientMem;
      }
      else
      {
         // I now point to (a pointer of ((the node I was at)'s mpNext))
         ppDelClientMem = &(*ppDelClientMem)->mpNext;
      }
   }
   
   // End Critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

   return;
}

/*===========================================================================
METHOD:
   FindClientMem (Public Method)

DESCRIPTION:
   Find this client's memory

   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev           [ I ] - Device specific memory
   clientID       [ I ] - Requester's client ID

RETURN VALUE:
   sClientMemList - Pointer to requested sClientMemList for success
                    NULL for error
===========================================================================*/
static sClientMemList * FindClientMem( 
   sGobiUSBNet *      pDev,
   u16              clientID )
{
   sClientMemList * pClientMem;
   
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return NULL;
   }
   
#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif
   
   pClientMem = pDev->mQMIDev.mpClientMemList;
   while (pClientMem != NULL)
   {
      if (pClientMem->mClientID == clientID)
      {
         // Success
         VDBG("Found client's 0x%x memory\n", clientID);
         return pClientMem;
      }
      
      pClientMem = pClientMem->mpNext;
   }

   DBG( "Could not find client mem 0x%04X\n", clientID );
   return NULL;
}

/*===========================================================================
METHOD:
   AddToReadMemList (Public Method)

DESCRIPTION:
   Add Data to this client's ReadMem list
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev           [ I ] - Device specific memory
   clientID       [ I ] - Requester's client ID
   transactionID  [ I ] - Transaction ID or 0 for any
   pData          [ I ] - Data to add
   dataSize       [ I ] - Size of data to add

RETURN VALUE:
   bool
===========================================================================*/
static bool AddToReadMemList( 
   sGobiUSBNet *      pDev,
   u16              clientID,
   u16              transactionID,
   void *           pData,
   u16              dataSize )
{
   sClientMemList * pClientMem;
   sReadMemList ** ppThisReadMemList;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n",
           clientID );

      return false;
   }

   // Go to last ReadMemList entry
   ppThisReadMemList = &pClientMem->mpList;
   while (*ppThisReadMemList != NULL)
   {
      ppThisReadMemList = &(*ppThisReadMemList)->mpNext;
   }
   
   *ppThisReadMemList = kmalloc( sizeof( sReadMemList ), GFP_ATOMIC );
   if (*ppThisReadMemList == NULL)
   {
      DBG( "Mem error\n" );

      return false;
   }   
   
   (*ppThisReadMemList)->mpNext = NULL;
   (*ppThisReadMemList)->mpData = pData;
   (*ppThisReadMemList)->mDataSize = dataSize;
   (*ppThisReadMemList)->mTransactionID = transactionID;
   
   return true;
}

/*===========================================================================
METHOD:
   PopFromReadMemList (Public Method)

DESCRIPTION:
   Remove data from this client's ReadMem list if it matches 
   the specified transaction ID.
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   transactionID     [ I ] - Transaction ID or 0 for any
   ppData            [I/O] - On success, will be filled with a 
                             pointer to read buffer
   pDataSize         [I/O] - On succces, will be filled with the 
                             read buffer's size

RETURN VALUE:
   bool
===========================================================================*/
static bool PopFromReadMemList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   void **              ppData,
   u16 *                pDataSize )
{
   sClientMemList * pClientMem;
   sReadMemList * pDelReadMemList, ** ppReadMemList;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n",
           clientID );

      return false;
   }
   
   ppReadMemList = &(pClientMem->mpList);
   pDelReadMemList = NULL;
   
   // Find first message that matches this transaction ID
   while (*ppReadMemList != NULL)
   {
      // Do we care about transaction ID?
      if (transactionID == 0
      ||  transactionID == (*ppReadMemList)->mTransactionID )
      {
         pDelReadMemList = *ppReadMemList;
         VDBG(  "*ppReadMemList = 0x%p pDelReadMemList = 0x%p\n",
               *ppReadMemList, pDelReadMemList );
         break;
      }
      
      VDBG( "skipping 0x%04X data TID = %x\n", clientID, (*ppReadMemList)->mTransactionID );
      
      // Next
      ppReadMemList = &(*ppReadMemList)->mpNext;
   }
   VDBG(  "*ppReadMemList = 0x%p pDelReadMemList = 0x%p\n",
         *ppReadMemList, pDelReadMemList );
   if (pDelReadMemList != NULL)
   {
      *ppReadMemList = (*ppReadMemList)->mpNext;
      
      // Copy to output
      *ppData = pDelReadMemList->mpData;
      *pDataSize = pDelReadMemList->mDataSize;
      VDBG(  "*ppData = 0x%p pDataSize = %u\n",
            *ppData, *pDataSize );
      
      // Free memory
      kfree( pDelReadMemList );
      
      return true;
   }
   else
   {
      DBG( "No read memory to pop, Client 0x%04X, TID = %x\n", 
           clientID, 
           transactionID );
      return false;
   }
}

/*===========================================================================
METHOD:
   AddToNotifyList (Public Method)

DESCRIPTION:
   Add Notify entry to this client's notify List
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   transactionID     [ I ] - Transaction ID or 0 for any
   pNotifyFunct      [ I ] - Callback function to be run when data is available
   pData             [ I ] - Data buffer that willl be passed (unmodified) 
                             to callback

RETURN VALUE:
   bool
===========================================================================*/
static bool AddToNotifyList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   u16                  transactionID,
   void                 (* pNotifyFunct)(sGobiUSBNet *, u16, void *),
   void *               pData )
{
   sClientMemList * pClientMem;
   sNotifyList ** ppThisNotifyList;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n", clientID );
      return false;
   }

   // Go to last URBList entry
   ppThisNotifyList = &pClientMem->mpReadNotifyList;
   while (*ppThisNotifyList != NULL)
   {
      ppThisNotifyList = &(*ppThisNotifyList)->mpNext;
   }
   
   *ppThisNotifyList = kmalloc( sizeof( sNotifyList ), GFP_ATOMIC );
   if (*ppThisNotifyList == NULL)
   {
      DBG( "Mem error\n" );
      return false;
   }   
   
   (*ppThisNotifyList)->mpNext = NULL;
   (*ppThisNotifyList)->mpNotifyFunct = pNotifyFunct;
   (*ppThisNotifyList)->mpData = pData;
   (*ppThisNotifyList)->mTransactionID = transactionID;
   
   return true;
}

/*===========================================================================
METHOD:
   NotifyAndPopNotifyList (Public Method)

DESCRIPTION:
   Remove first Notify entry from this client's notify list 
   and Run function
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   transactionID     [ I ] - Transaction ID or 0 for any

RETURN VALUE:
   bool
===========================================================================*/
static bool NotifyAndPopNotifyList( 
   sGobiUSBNet *        pDev,
   u16                  clientID,
   u16                  transactionID )
{
   sClientMemList * pClientMem;
   sNotifyList * pDelNotifyList, ** ppNotifyList;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n", clientID );
      return false;
   }

   ppNotifyList = &(pClientMem->mpReadNotifyList);
   pDelNotifyList = NULL;

   // Remove from list
   while (*ppNotifyList != NULL)
   {
      // Do we care about transaction ID?
      if (transactionID == 0
      ||  (*ppNotifyList)->mTransactionID == 0
      ||  transactionID == (*ppNotifyList)->mTransactionID)
      {
         pDelNotifyList = *ppNotifyList;
         break;
      }
      
      DBG( "skipping data TID = %x\n", (*ppNotifyList)->mTransactionID );
      
      // next
      ppNotifyList = &(*ppNotifyList)->mpNext;
   }
   
   if (pDelNotifyList != NULL)
   {
      // Remove element
      *ppNotifyList = (*ppNotifyList)->mpNext;
      
      // Run notification function
      if (pDelNotifyList->mpNotifyFunct != NULL)
      {
         // Unlock for callback
         spin_unlock( &pDev->mQMIDev.mClientMemLock );
      
         pDelNotifyList->mpNotifyFunct( pDev,
                                        clientID,
                                        pDelNotifyList->mpData );

         // Restore lock
         spin_lock( &pDev->mQMIDev.mClientMemLock );
      }
      
      // Delete memory
      kfree( pDelNotifyList );

      return true;
   }
   else
   {
      DBG( "no one to notify for TID %x\n", transactionID );
      
      return false;
   }
}

/*===========================================================================
METHOD:
   AddToURBList (Public Method)

DESCRIPTION:
   Add URB to this client's URB list
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev              [ I ] - Device specific memory
   clientID          [ I ] - Requester's client ID
   pURB              [ I ] - URB to be added

RETURN VALUE:
   bool
===========================================================================*/
static bool AddToURBList( 
   sGobiUSBNet *      pDev,
   u16                  clientID,
   struct urb *     pURB )
{
   sClientMemList * pClientMem;
   sURBList ** ppThisURBList;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n", clientID );
      return false;
   }

   // Go to last URBList entry
   ppThisURBList = &pClientMem->mpURBList;
   while (*ppThisURBList != NULL)
   {
      ppThisURBList = &(*ppThisURBList)->mpNext;
   }
   
   *ppThisURBList = kmalloc( sizeof( sURBList ), GFP_ATOMIC );
   if (*ppThisURBList == NULL)
   {
      DBG( "Mem error\n" );
      return false;
   }   
   
   (*ppThisURBList)->mpNext = NULL;
   (*ppThisURBList)->mpURB = pURB;
   
   return true;
}

/*===========================================================================
METHOD:
   PopFromURBList (Public Method)

DESCRIPTION:
   Remove URB from this client's URB list
   
   Caller MUST have lock on mClientMemLock

PARAMETERS:
   pDev           [ I ] - Device specific memory
   clientID       [ I ] - Requester's client ID

RETURN VALUE:
   struct urb - Pointer to requested client's URB
                NULL for error
===========================================================================*/
static struct urb * PopFromURBList( 
   sGobiUSBNet *      pDev,
   u16                  clientID )
{
   sClientMemList * pClientMem;
   sURBList * pDelURBList;
   struct urb * pURB;

#ifdef CONFIG_SMP
   // Verify Lock
   if (spin_is_locked( &pDev->mQMIDev.mClientMemLock ) == 0)
   {
      DBG( "unlocked\n" );
      BUG();
   }
#endif

   // Get this client's memory location
   pClientMem = FindClientMem( pDev, clientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n", clientID );
      return NULL;
   }

   // Remove from list
   if (pClientMem->mpURBList != NULL)
   {
      pDelURBList = pClientMem->mpURBList;
      pClientMem->mpURBList = pClientMem->mpURBList->mpNext;
      
      // Copy to output
      pURB = pDelURBList->mpURB;
      
      // Delete memory
      kfree( pDelURBList );

      return pURB;
   }
   else
   {
      DBG( "No URB's to pop\n" );
      
      return NULL;
   }
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 3,19,0 ))
#ifndef f_dentry
#define f_dentry f_path.dentry
#endif
#endif

/*=========================================================================*/
// Internal userspace wrappers
/*=========================================================================*/

/*===========================================================================
METHOD:
   UserspaceunlockedIOCTL (Public Method)

DESCRIPTION:
   Internal wrapper for Userspace IOCTL interface

PARAMETERS
   pFilp        [ I ] - userspace file descriptor
   cmd          [ I ] - IOCTL command
   arg          [ I ] - IOCTL argument

RETURN VALUE:
   long - 0 for success
         Negative errno for failure
===========================================================================*/
static long UserspaceunlockedIOCTL(
   struct file *     pFilp,
   unsigned int      cmd, 
   unsigned long     arg )
{
   int result;
   u32 devVIDPID;

   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;

   if (pFilpData == NULL)
   {
      DBG( "Bad file data\n" );
      return -EBADF;
   }
   
   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      DBG( "Invalid device! Updating f_ops\n" );
      pFilp->f_op = pFilp->f_dentry->d_inode->i_fop;
      return -ENXIO;
   }

   switch (cmd)
   {
      case IOCTL_QMI_GET_SERVICE_FILE:    
         DBG( "Setting up QMI for service %lu\n", arg );
         if ((u8)arg == 0)
         {
            DBG( "Cannot use QMICTL from userspace\n" );
            return -EINVAL;
         }

         // Connection is already setup
         if (pFilpData->mClientID != (u16)-1)
         {
            DBG( "Close the current connection before opening a new one\n" );
            return -EBADR;
         }
         
         result = GetClientID( pFilpData->mpDev, (u8)arg );
// it seems QMIWDA only allow one client, if the last quectel-CM donot realese it (killed by SIGKILL).
// can force release it at here
#if 1
         if (result < 0 && (u8)arg == QMIWDA)
         {
             ReleaseClientID( pFilpData->mpDev, QMIWDA | (1 << 8) );
             result = GetClientID( pFilpData->mpDev, (u8)arg );
         }
#endif
         if (result < 0)
         {
            return result;
         }
         pFilpData->mClientID = (u16)result;
         DBG("pFilpData->mClientID = 0x%x\n", pFilpData->mClientID );
         return 0;
         break;


      case IOCTL_QMI_GET_DEVICE_VIDPID:
         if (arg == 0)
         {
            DBG( "Bad VIDPID buffer\n" );
            return -EINVAL;
         }
         
         // Extra verification
         if (pFilpData->mpDev->mpNetDev == 0)
         {
            DBG( "Bad mpNetDev\n" );
            return -ENOMEM;
         }
         if (pFilpData->mpDev->mpNetDev->udev == 0)
         {
            DBG( "Bad udev\n" );
            return -ENOMEM;
         }

         devVIDPID = ((le16_to_cpu( pFilpData->mpDev->mpNetDev->udev->descriptor.idVendor ) << 16)
                     + le16_to_cpu( pFilpData->mpDev->mpNetDev->udev->descriptor.idProduct ) );

         result = copy_to_user( (unsigned int *)arg, &devVIDPID, 4 );
         if (result != 0)
         {
            DBG( "Copy to userspace failure %d\n", result );
         }

         return result;
                 
         break;

      case IOCTL_QMI_GET_DEVICE_MEID:
         if (arg == 0)
         {
            DBG( "Bad MEID buffer\n" );
            return -EINVAL;
         }
         
         result = copy_to_user( (unsigned int *)arg, &pFilpData->mpDev->mMEID[0], 14 );
         if (result != 0)
         {
            DBG( "Copy to userspace failure %d\n", result );
         }

         return result;
                 
         break;
         
      default:
         return -EBADRQC;       
   }
}

/*=========================================================================*/
// Userspace wrappers
/*=========================================================================*/

/*===========================================================================
METHOD:
   UserspaceOpen (Public Method)

DESCRIPTION:
   Userspace open
      IOCTL must be called before reads or writes

PARAMETERS
   pInode       [ I ] - kernel file descriptor
   pFilp        [ I ] - userspace file descriptor

RETURN VALUE:
   int - 0 for success
         Negative errno for failure
===========================================================================*/
static int UserspaceOpen(
   struct inode *         pInode,
   struct file *          pFilp )
{
   sQMIFilpStorage * pFilpData;

   // Optain device pointer from pInode
   sQMIDev * pQMIDev = container_of( pInode->i_cdev,
                                     sQMIDev,
                                     mCdev );
   sGobiUSBNet * pDev = container_of( pQMIDev,
                                    sGobiUSBNet,
                                    mQMIDev );

   if (pDev->mbMdm9x07)
   {
      atomic_inc(&pDev->refcount);
      if (!pDev->mbQMIReady) {
         if (wait_for_completion_interruptible_timeout(&pDev->mQMIReadyCompletion, 15*HZ) <= 0) {
            if (atomic_dec_and_test(&pDev->refcount)) {
               kfree( pDev );
            }
            return -ETIMEDOUT;
         }
      }
      atomic_dec(&pDev->refcount);
   }

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return -ENXIO;
   }

   // Setup data in pFilp->private_data
   pFilp->private_data = kmalloc( sizeof( sQMIFilpStorage ), GFP_KERNEL );
   if (pFilp->private_data == NULL)
   {
      DBG( "Mem error\n" );
      return -ENOMEM;
   }

   pFilpData = (sQMIFilpStorage *)pFilp->private_data;
   pFilpData->mClientID = (u16)-1;
   pFilpData->mpDev = pDev;
   atomic_inc(&pFilpData->mpDev->refcount);

   return 0;
}

/*===========================================================================
METHOD:
   UserspaceIOCTL (Public Method)

DESCRIPTION:
   Userspace IOCTL functions

PARAMETERS
   pUnusedInode [ I ] - (unused) kernel file descriptor
   pFilp        [ I ] - userspace file descriptor
   cmd          [ I ] - IOCTL command
   arg          [ I ] - IOCTL argument

RETURN VALUE:
   int - 0 for success
         Negative errno for failure
===========================================================================*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,36 ))
static int UserspaceIOCTL(
   struct inode *    pUnusedInode,
   struct file *     pFilp,
   unsigned int      cmd,
   unsigned long     arg ) 
{
   // call the internal wrapper function
   return (int)UserspaceunlockedIOCTL( pFilp, cmd, arg );  
}
#endif

#ifdef quectel_no_for_each_process
static int UserspaceClose(
   struct inode *         pInode,
   struct file *          pFilp )
{
   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;

   if (pFilpData == NULL)
   {
      DBG( "bad file data\n" );
      return -EBADF;
   }

   atomic_dec(&pFilpData->mpDev->refcount);
   
   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      return -ENXIO;
   }
   
   DBG( "0x%04X\n", pFilpData->mClientID );

   // Disable pFilpData so they can't keep sending read or write 
   //    should this function hang
   // Note: memory pointer is still saved in pFilpData to be deleted later
   pFilp->private_data = NULL;

   if (pFilpData->mClientID != (u16)-1)
   {
      if (pFilpData->mpDev->mbDeregisterQMIDevice)
         pFilpData->mClientID = (u16)-1; //DeregisterQMIDevice() will release this ClientID
      else
      ReleaseClientID( pFilpData->mpDev,
                       pFilpData->mClientID );
   }
      
   kfree( pFilpData );
   return 0;
}
#else
/*===========================================================================
METHOD:
   UserspaceClose (Public Method)

DESCRIPTION:
   Userspace close
      Release client ID and free memory

PARAMETERS
   pFilp           [ I ] - userspace file descriptor
   unusedFileTable [ I ] - (unused) file table

RETURN VALUE:
   int - 0 for success
         Negative errno for failure
===========================================================================*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,14 ))
int UserspaceClose( 
   struct file *       pFilp,
   fl_owner_t          unusedFileTable )
#else
int UserspaceClose( struct file *       pFilp )
#endif
{
   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;
   struct task_struct * pEachTask;
   struct fdtable * pFDT;
   int count = 0;
   int used = 0;
   unsigned long flags;

   if (pFilpData == NULL)
   {
      DBG( "bad file data\n" );
      return -EBADF;
   }

   // Fallthough.  If f_count == 1 no need to do more checks
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,24 ))
   if (atomic_read( &pFilp->f_count ) != 1)
#else
   if (atomic_long_read( &pFilp->f_count ) != 1)
#endif
   {
      rcu_read_lock();
      for_each_process( pEachTask )
      {
         task_lock(pEachTask);
         if (pEachTask == NULL || pEachTask->files == NULL)
         {
            // Some tasks may not have files (e.g. Xsession)
            task_unlock(pEachTask);
            continue;
         }
         spin_lock_irqsave( &pEachTask->files->file_lock, flags );
         task_unlock(pEachTask); //kernel/exit.c:do_exit() -> fs/file.c:exit_files()
         pFDT = files_fdtable( pEachTask->files );
         for (count = 0; count < pFDT->max_fds; count++)
         {
            // Before this function was called, this file was removed
            // from our task's file table so if we find it in a file
            // table then it is being used by another task
            if (pFDT->fd[count] == pFilp)
            {
               used++;
               break;
            }
         }
         spin_unlock_irqrestore( &pEachTask->files->file_lock, flags );
      }
      rcu_read_unlock();
      
      if (used > 0)
      {
         DBG( "not closing, as this FD is open by %d other process\n", used );
         return 0;
      }
   }

   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      DBG( "Invalid device! Updating f_ops\n" );
      pFilp->f_op = pFilp->f_dentry->d_inode->i_fop;
      return -ENXIO;
   }
   
   DBG( "0x%04X\n", pFilpData->mClientID );
   
   // Disable pFilpData so they can't keep sending read or write 
   //    should this function hang
   // Note: memory pointer is still saved in pFilpData to be deleted later
   pFilp->private_data = NULL;

   if (pFilpData->mClientID != (u16)-1)
   {
      if (pFilpData->mpDev->mbDeregisterQMIDevice)
         pFilpData->mClientID = (u16)-1; //DeregisterQMIDevice() will release this ClientID
      else
      ReleaseClientID( pFilpData->mpDev,
                       pFilpData->mClientID );
   }
   atomic_dec(&pFilpData->mpDev->refcount);
      
   kfree( pFilpData );
   return 0;
}
#endif

/*===========================================================================
METHOD:
   UserspaceRead (Public Method)

DESCRIPTION:
   Userspace read (synchronous)

PARAMETERS
   pFilp           [ I ] - userspace file descriptor
   pBuf            [ I ] - read buffer
   size            [ I ] - size of read buffer
   pUnusedFpos     [ I ] - (unused) file position

RETURN VALUE:
   ssize_t - Number of bytes read for success
             Negative errno for failure
===========================================================================*/
static ssize_t UserspaceRead( 
   struct file *          pFilp,
   char __user *          pBuf, 
   size_t                 size,
   loff_t *               pUnusedFpos )
{
   int result;
   void * pReadData = NULL;
   void * pSmallReadData;
   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;

   if (pFilpData == NULL)
   {
      DBG( "Bad file data\n" );
      return -EBADF;
   }

   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      DBG( "Invalid device! Updating f_ops\n" );
      pFilp->f_op = pFilp->f_dentry->d_inode->i_fop;
      return -ENXIO;
   }
   
   if (pFilpData->mClientID == (u16)-1)
   {
      DBG( "Client ID must be set before reading 0x%04X\n",
           pFilpData->mClientID );
      return -EBADR;
   }
   
   // Perform synchronous read
   result = ReadSync( pFilpData->mpDev,
                      &pReadData,
                      pFilpData->mClientID,
                      0 );
   if (result <= 0)
   {
      return result;
   }
   
   // Discard QMUX header
   result -= QMUXHeaderSize();
   pSmallReadData = pReadData + QMUXHeaderSize();

   if (result > size)
   {
      DBG( "Read data is too large for amount user has requested\n" );
      kfree( pReadData );
      return -EOVERFLOW;
   }

   DBG(  "pBuf = 0x%p pSmallReadData = 0x%p, result = %d",
         pBuf, pSmallReadData, result );

   if (copy_to_user( pBuf, pSmallReadData, result ) != 0)
   {
      DBG( "Error copying read data to user\n" );
      result = -EFAULT;
   }
   
   // Reader is responsible for freeing read buffer
   kfree( pReadData );
   
   return result;
}

/*===========================================================================
METHOD:
   UserspaceWrite (Public Method)

DESCRIPTION:
   Userspace write (synchronous)

PARAMETERS
   pFilp           [ I ] - userspace file descriptor
   pBuf            [ I ] - write buffer
   size            [ I ] - size of write buffer
   pUnusedFpos     [ I ] - (unused) file position

RETURN VALUE:
   ssize_t - Number of bytes read for success
             Negative errno for failure
===========================================================================*/
static ssize_t UserspaceWrite(
   struct file *        pFilp, 
   const char __user *  pBuf, 
   size_t               size,
   loff_t *             pUnusedFpos )
{
   int status;
   void * pWriteBuffer;
   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;

   if (pFilpData == NULL)
   {
      DBG( "Bad file data\n" );
      return -EBADF;
   }

   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      DBG( "Invalid device! Updating f_ops\n" );
      pFilp->f_op = pFilp->f_dentry->d_inode->i_fop;
      return -ENXIO;
   }

   if (pFilpData->mClientID == (u16)-1)
   {
      DBG( "Client ID must be set before writing 0x%04X\n",
           pFilpData->mClientID );
      return -EBADR;
   }
   
   // Copy data from user to kernel space
   pWriteBuffer = kmalloc( size + QMUXHeaderSize(), GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }
   status = copy_from_user( pWriteBuffer + QMUXHeaderSize(), pBuf, size );
   if (status != 0)
   {
      DBG( "Unable to copy data from userspace %d\n", status );
      kfree( pWriteBuffer );
      return status;
   }

   status = WriteSync( pFilpData->mpDev,
                       pWriteBuffer, 
                       size + QMUXHeaderSize(),
                       pFilpData->mClientID );

   kfree( pWriteBuffer );
   
   // On success, return requested size, not full QMI reqest size
   if (status == size + QMUXHeaderSize())
   {
      return size;
   }
   else
   {
      return status;
   }
}

/*===========================================================================
METHOD:
   UserspacePoll (Public Method)

DESCRIPTION:
   Used to determine if read/write operations are possible without blocking

PARAMETERS
   pFilp              [ I ] - userspace file descriptor
   pPollTable         [I/O] - Wait object to notify the kernel when data 
                              is ready

RETURN VALUE:
   unsigned int - bitmask of what operations can be done immediately
===========================================================================*/
static unsigned int UserspacePoll(
   struct file *                  pFilp,
   struct poll_table_struct *     pPollTable )
{
   sQMIFilpStorage * pFilpData = (sQMIFilpStorage *)pFilp->private_data;
   sClientMemList * pClientMem;
   unsigned long flags;

   // Always ready to write
   unsigned long status = POLLOUT | POLLWRNORM;

   if (pFilpData == NULL)
   {
      DBG( "Bad file data\n" );
      return POLLERR;
   }

   if (IsDeviceValid( pFilpData->mpDev ) == false)
   {
      DBG( "Invalid device! Updating f_ops\n" );
      pFilp->f_op = pFilp->f_dentry->d_inode->i_fop;
      return POLLERR;
   }

   if (pFilpData->mpDev->mbDeregisterQMIDevice)
   {
      DBG( "DeregisterQMIDevice ing\n" );
      return POLLHUP | POLLERR;
   }

   if (pFilpData->mClientID == (u16)-1)
   {
      DBG( "Client ID must be set before polling 0x%04X\n",
           pFilpData->mClientID );
      return POLLERR;
   }

   // Critical section
   spin_lock_irqsave( &pFilpData->mpDev->mQMIDev.mClientMemLock, flags );

   // Get this client's memory location
   pClientMem = FindClientMem( pFilpData->mpDev, 
                               pFilpData->mClientID );
   if (pClientMem == NULL)
   {
      DBG( "Could not find this client's memory 0x%04X\n",
           pFilpData->mClientID );

      spin_unlock_irqrestore( &pFilpData->mpDev->mQMIDev.mClientMemLock, 
                              flags );
      return POLLERR;
   }
   
   poll_wait( pFilp, &pClientMem->mWaitQueue, pPollTable );

   if (pClientMem->mpList != NULL)
   {
      status |= POLLIN | POLLRDNORM;
   }

   // End critical section
   spin_unlock_irqrestore( &pFilpData->mpDev->mQMIDev.mClientMemLock, flags );

   // Always ready to write 
   return (status | POLLOUT | POLLWRNORM);
}

/*=========================================================================*/
// Initializer and destructor
/*=========================================================================*/
static int QMICTLSyncProc(sGobiUSBNet *pDev)
{
   void *pWriteBuffer;
   void *pReadBuffer;
   int result;
   u16 writeBufferSize;
   u16 readBufferSize;
   u8 transactionID;
   unsigned long flags;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return -EFAULT;
   }

   writeBufferSize= QMICTLSyncReqSize();
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }

   transactionID = QMIXactionIDGet(pDev);

   /* send a QMI_CTL_SYNC_REQ (0x0027) */
   result = QMICTLSyncReq( pWriteBuffer,
                           writeBufferSize,
                           transactionID );
   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }

   result = WriteSync( pDev,
                       pWriteBuffer,
                       writeBufferSize,
                       QMICTL );

   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }

   // QMI CTL Sync Response
   result = ReadSync( pDev,
                      &pReadBuffer,
                      QMICTL,
                      transactionID );
   if (result < 0)
   {
      return result;
   }

   result = QMICTLSyncResp( pReadBuffer,
                            (u16)result );

   kfree( pReadBuffer );

   if (result < 0) /* need to re-sync */
   {
      DBG( "sync response error code %d\n", result );
      /* start timer and wait for the response */
      /* process response */
      return result;
   }

#if 1 //free these ununsed qmi response, or when these transactionID re-used, they will be regarded as qmi response of the qmi request that have same transactionID
   // Enter critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

   // Free any unread data
   while (PopFromReadMemList( pDev, QMICTL, 0, &pReadBuffer, &readBufferSize) == true) {	
       kfree( pReadBuffer ); 
   }
   
   // End critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );    
#endif
  
   // Success
   return 0;
}

static int qmi_sync_thread(void *data) {
    sGobiUSBNet * pDev = (sGobiUSBNet *)data;
    int result = 0;

#if 1  
   // Device is not ready for QMI connections right away
   //   Wait up to 30 seconds before failing
   if (QMIReady( pDev, 30000 ) == false)
   {
      DBG( "Device unresponsive to QMI\n" );
      goto __qmi_sync_finished;
   }

   // Initiate QMI CTL Sync Procedure
   DBG( "Sending QMI CTL Sync Request\n" );
   result = QMICTLSyncProc(pDev);
   if (result != 0)
   {
      DBG( "QMI CTL Sync Procedure Error\n" );
      goto __qmi_sync_finished;
   }
   else
   {
      DBG( "QMI CTL Sync Procedure Successful\n" );
   }

#if defined(QUECTEL_WWAN_QMAP)
if (pDev->qmap_mode) {
   // Setup Data Format
   result = QMIWDASetDataFormat (pDev, pDev->qmap_mode, &pDev->qmap_size);
   if (result != 0)
   {
      goto __qmi_sync_finished;
   }
   pDev->mpNetDev->rx_urb_size = pDev->qmap_size;
}
#endif

   // Setup WDS callback
   result = SetupQMIWDSCallback( pDev );
   if (result != 0)
   {
      goto __qmi_sync_finished;
   }

   // Fill MEID for device
   result = QMIDMSGetMEID( pDev );
   if (result != 0)
   {
      goto __qmi_sync_finished;
   }
#endif

__qmi_sync_finished:
   pDev->mbQMIReady = true;
   complete_all(&pDev->mQMIReadyCompletion);
   pDev->mbQMISyncIng = false;
   if (atomic_dec_and_test(&pDev->refcount)) {
      kfree( pDev );
   }
   return result;
}

/*===========================================================================
METHOD:
   RegisterQMIDevice (Public Method)

DESCRIPTION:
   QMI Device initialization function

PARAMETERS:
   pDev     [ I ] - Device specific memory
   
RETURN VALUE:
   int - 0 for success
         Negative errno for failure
===========================================================================*/
int RegisterQMIDevice( sGobiUSBNet * pDev )
{
   int result;
   int GobiQMIIndex = 0;
   dev_t devno; 
   char * pDevName;

   if (pDev->mQMIDev.mbCdevIsInitialized == true)
   {
      // Should never happen, but always better to check
      DBG( "device already exists\n" );
      return -EEXIST;
   }
 
   pDev->mbQMIValid = true;
   pDev->mbDeregisterQMIDevice = false;

   // Set up for QMICTL
   //    (does not send QMI message, just sets up memory)
   result = GetClientID( pDev, QMICTL );
   if (result != 0)
   {
      pDev->mbQMIValid = false;
      return result;
   }
   atomic_set( &pDev->mQMIDev.mQMICTLTransactionID, 1 );

   // Start Async reading
   result = StartRead( pDev );
   if (result != 0)
   {
      pDev->mbQMIValid = false;
      return result;
   }

   if (pDev->mbMdm9x07)
   {
      usb_control_msg( pDev->mpNetDev->udev,
                            usb_sndctrlpipe( pDev->mpNetDev->udev, 0 ),
                             SET_CONTROL_LINE_STATE_REQUEST,
                             SET_CONTROL_LINE_STATE_REQUEST_TYPE,
                             CONTROL_DTR,
                             /* USB interface number to receive control message */
                             pDev->mpIntf->cur_altsetting->desc.bInterfaceNumber,
                             NULL,
                             0,
                             100 );
   }

    //for EC21&25, must wait about 15 seconds to wait QMI ready. it is too long for driver probe(will block other drivers probe).
   if (pDev->mbMdm9x07)
   {
      struct task_struct *qmi_sync_task;
      atomic_inc(&pDev->refcount);
      init_completion(&pDev->mQMIReadyCompletion);
      pDev->mbQMIReady = false;
      pDev->mbQMISyncIng = true;
      qmi_sync_task = kthread_run(qmi_sync_thread, (void *)pDev, "qmi_sync/%d", pDev->mpNetDev->udev->devnum);
       if (IS_ERR(qmi_sync_task)) {
         pDev->mbQMISyncIng = false;
         atomic_dec(&pDev->refcount);
         DBG( "Create qmi_sync_thread fail\n" );
         return PTR_ERR(qmi_sync_task);
      }
      goto __register_chardev_qccmi;
   }
   
   // Device is not ready for QMI connections right away
   //   Wait up to 30 seconds before failing
   if (QMIReady( pDev, 30000 ) == false)
   {
      DBG( "Device unresponsive to QMI\n" );
      return -ETIMEDOUT;
   }

   // Initiate QMI CTL Sync Procedure
   DBG( "Sending QMI CTL Sync Request\n" );
   result = QMICTLSyncProc(pDev);
   if (result != 0)
   {
      DBG( "QMI CTL Sync Procedure Error\n" );
      return result;
   }
   else
   {
      DBG( "QMI CTL Sync Procedure Successful\n" );
   }

   // Setup Data Format
#if defined(QUECTEL_WWAN_QMAP)   
   result = QMIWDASetDataFormat (pDev, pDev->qmap_mode, NULL);
#else
   result = QMIWDASetDataFormat (pDev, 0, NULL);
#endif
   if (result != 0)
   {
       return result;
   }

   // Setup WDS callback
   result = SetupQMIWDSCallback( pDev );
   if (result != 0)
   {
	  return result;
   }

   // Fill MEID for device
   result = QMIDMSGetMEID( pDev );
   if (result != 0)
   {
      return result;
   }

__register_chardev_qccmi:
   // allocate and fill devno with numbers
   result = alloc_chrdev_region( &devno, 0, 1, "qcqmi" );
   if (result < 0)
   {
 	  return result;
   }

   // Create cdev
   cdev_init( &pDev->mQMIDev.mCdev, &UserspaceQMIFops );
   pDev->mQMIDev.mCdev.owner = THIS_MODULE;
   pDev->mQMIDev.mCdev.ops = &UserspaceQMIFops;
   pDev->mQMIDev.mbCdevIsInitialized = true;

   result = cdev_add( &pDev->mQMIDev.mCdev, devno, 1 );
   if (result != 0)
   {
      DBG( "error adding cdev\n" );
      return result;
   }

   // Match interface number (usb# or eth#)
   if (!!(pDevName = strstr( pDev->mpNetDev->net->name, "eth" ))) {
       pDevName += strlen( "eth" );
   } else if (!!(pDevName = strstr( pDev->mpNetDev->net->name, "usb" ))) {
       pDevName += strlen( "usb" );
#if 1 //openWRT like use ppp# or lte#
   } else if (!!(pDevName = strstr( pDev->mpNetDev->net->name, "ppp" ))) {
       pDevName += strlen( "ppp" );
   } else if (!!(pDevName = strstr( pDev->mpNetDev->net->name, "lte" ))) {
       pDevName += strlen( "lte" );
#endif
   } else {
      DBG( "Bad net name: %s\n", pDev->mpNetDev->net->name );
      return -ENXIO;
   }
   GobiQMIIndex = simple_strtoul( pDevName, NULL, 10 );
   if (GobiQMIIndex < 0)
   {
      DBG( "Bad minor number\n" );
      return -ENXIO;
   }

   // Always print this output
   printk( KERN_INFO "creating qcqmi%d\n",
           GobiQMIIndex );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,27 ))
   // kernel 2.6.27 added a new fourth parameter to device_create
   //    void * drvdata : the data to be added to the device for callbacks
   device_create( pDev->mQMIDev.mpDevClass,
                  &pDev->mpIntf->dev, 
                  devno,
                  NULL,
                  "qcqmi%d", 
                  GobiQMIIndex );
#else
   device_create( pDev->mQMIDev.mpDevClass,
                  &pDev->mpIntf->dev, 
                  devno,
                  "qcqmi%d", 
                  GobiQMIIndex );
#endif
   
   pDev->mQMIDev.mDevNum = devno;

   // Success
   return 0;
}

/*===========================================================================
METHOD:
   DeregisterQMIDevice (Public Method)

DESCRIPTION:
   QMI Device cleanup function
   
   NOTE: When this function is run the device is no longer valid

PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   None
===========================================================================*/
void DeregisterQMIDevice( sGobiUSBNet * pDev )
{
#ifndef quectel_no_for_each_process
   struct inode * pOpenInode;
   struct list_head * pInodeList;
   struct task_struct * pEachTask;
   struct fdtable * pFDT;
   struct file * pFilp;
   int count = 0;
#endif
   unsigned long flags;
   int tries;
   int result;

   // Should never happen, but check anyway
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "wrong device\n" );
      return;
   }

   pDev->mbDeregisterQMIDevice = true;

   for (tries = 0; tries < 3000; tries += 10) {
      if (pDev->mbQMISyncIng == false)
         break;
      msleep(10);
   }

   if (pDev->mbQMISyncIng) {
      DBG( "QMI sync ing\n" );
   }

   // Release all clients
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
   while (pDev->mQMIDev.mpClientMemList != NULL)
   {
      u16 mClientID = pDev->mQMIDev.mpClientMemList->mClientID;
      if (waitqueue_active(&pDev->mQMIDev.mpClientMemList->mWaitQueue)) {
         DBG("WaitQueue 0x%04X\n", mClientID);
         wake_up_interruptible_sync( &pDev->mQMIDev.mpClientMemList->mWaitQueue );
         spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );      
         msleep(10);
         spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
         continue;
      }

      DBG( "release 0x%04X\n", pDev->mQMIDev.mpClientMemList->mClientID );
   
      spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      ReleaseClientID( pDev, mClientID );
      // NOTE: pDev->mQMIDev.mpClientMemList will 
      //       be updated in ReleaseClientID()
      spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
   }
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );

   // Stop all reads
   KillRead( pDev );

   pDev->mbQMIValid = false;

   if (pDev->mQMIDev.mbCdevIsInitialized == false)
   {
      return;
   }

#ifndef quectel_no_for_each_process
   // Find each open file handle, and manually close it
   
   // Generally there will only be only one inode, but more are possible
   list_for_each( pInodeList, &pDev->mQMIDev.mCdev.list )
   {
      // Get the inode
      pOpenInode = container_of( pInodeList, struct inode, i_devices );
      if (pOpenInode != NULL && (IS_ERR( pOpenInode ) == false))
      {
         // Look for this inode in each task

         rcu_read_lock();
         for_each_process( pEachTask )
         {
            task_lock(pEachTask);
            if (pEachTask == NULL || pEachTask->files == NULL)
            {
               // Some tasks may not have files (e.g. Xsession)
               task_unlock(pEachTask);
               continue;
            }
            // For each file this task has open, check if it's referencing
            // our inode.
            spin_lock_irqsave( &pEachTask->files->file_lock, flags );
            task_unlock(pEachTask);  //kernel/exit.c:do_exit() -> fs/file.c:exit_files()
            pFDT = files_fdtable( pEachTask->files );
            for (count = 0; count < pFDT->max_fds; count++)
            {
               pFilp = pFDT->fd[count];
               if (pFilp != NULL &&  pFilp->f_dentry != NULL)
               {
                  if (pFilp->f_dentry->d_inode == pOpenInode)
                  {
                     // Close this file handle
                     rcu_assign_pointer( pFDT->fd[count], NULL );                     
                     spin_unlock_irqrestore( &pEachTask->files->file_lock, flags );
                     
                     DBG( "forcing close of open file handle\n" );
                     filp_close( pFilp, pEachTask->files );

                     spin_lock_irqsave( &pEachTask->files->file_lock, flags );
                  }
               }
            }
            spin_unlock_irqrestore( &pEachTask->files->file_lock, flags );
         }
         rcu_read_unlock();
      }
   }
#endif

if (pDev->mpNetDev->udev->state) {
   // Send SetControlLineState request (USB_CDC)
   result = usb_control_msg( pDev->mpNetDev->udev,
                             usb_sndctrlpipe( pDev->mpNetDev->udev, 0 ),
                             SET_CONTROL_LINE_STATE_REQUEST,
                             SET_CONTROL_LINE_STATE_REQUEST_TYPE,
                             0, // DTR not present
                             /* USB interface number to receive control message */
                             pDev->mpIntf->cur_altsetting->desc.bInterfaceNumber,
                             NULL,
                             0,
                             100 );
   if (result < 0)
   {
      DBG( "Bad SetControlLineState status %d\n", result );
   }
}

   // Remove device (so no more calls can be made by users)
   if (IS_ERR( pDev->mQMIDev.mpDevClass ) == false)
   {
      device_destroy( pDev->mQMIDev.mpDevClass, 
                      pDev->mQMIDev.mDevNum );   
   }

   // Hold onto cdev memory location until everyone is through using it.
   // Timeout after 30 seconds (10 ms interval).  Timeout should never happen,
   // but exists to prevent an infinate loop just in case.
   for (tries = 0; tries < 30 * 100; tries++)
   {
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 4,11,0 ))
      int ref = atomic_read( &pDev->mQMIDev.mCdev.kobj.kref.refcount );
#else
      int ref = kref_read( &pDev->mQMIDev.mCdev.kobj.kref );
#endif
      if (ref > 1)
      {
         DBG( "cdev in use by %d tasks\n", ref - 1 ); 
         if (tries > 10)
            INFO( "cdev in use by %d tasks\n", ref - 1 ); 
         msleep( 10 );
      }
      else
      {
         break;
      }
   }

   cdev_del( &pDev->mQMIDev.mCdev );
   
   unregister_chrdev_region( pDev->mQMIDev.mDevNum, 1 );

   return;
}

/*=========================================================================*/
// Driver level client management
/*=========================================================================*/

/*===========================================================================
METHOD:
   QMIReady (Public Method)

DESCRIPTION:
   Send QMI CTL GET VERSION INFO REQ and SET DATA FORMAT REQ
   Wait for response or timeout

PARAMETERS:
   pDev     [ I ] - Device specific memory
   timeout  [ I ] - Milliseconds to wait for response

RETURN VALUE:
   bool
===========================================================================*/
static bool QMIReady(
   sGobiUSBNet *    pDev,
   u16                timeout )
{
   int result;
   void * pWriteBuffer;
   u16 writeBufferSize;
   void * pReadBuffer;
   u16 readBufferSize;
   u16 curTime;
   unsigned long flags;
   u8 transactionID;
   u16 interval = 2000;
   
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return false;
   }

   writeBufferSize = QMICTLReadyReqSize();
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return false;
   }

   // An implimentation of down_timeout has not been agreed on,
   //    so it's been added and removed from the kernel several times.
   //    We're just going to ignore it and poll the semaphore.

   // Send a write every 1000 ms and see if we get a response
   for (curTime = 0; curTime < timeout; curTime += interval)
   {
      // Start read
      struct QuecSem *readSem = kmalloc(sizeof(struct QuecSem ), GFP_KERNEL);
      readSem->magic = QUEC_SEM_MAGIC;
      sema_init( &readSem->readSem, 0 );
   
      transactionID = QMIXactionIDGet( pDev );

      result = ReadAsync( pDev, QMICTL, transactionID, UpSem, readSem );
      if (result != 0)
      {
         kfree( pWriteBuffer );
         return false;
      }

      // Fill buffer
      result = QMICTLReadyReq( pWriteBuffer, 
                               writeBufferSize,
                               transactionID );
      if (result < 0)
      {
         kfree( pWriteBuffer );
         return false;
      }

      // Disregard status.  On errors, just try again
      result = WriteSync( pDev,
                 pWriteBuffer,
                 writeBufferSize,
                 QMICTL );

      if (result < 0) //maybe caused by usb disconnect
      {
         kfree( pWriteBuffer );
         return false;
      }

#if 1
      if (down_timeout( &readSem->readSem, msecs_to_jiffies(interval) ) == 0)
#else
      msleep( interval );
      if (down_trylock( &readSem->readSem ) == 0)
#endif
      {
         kfree(readSem);
         // Enter critical section
         spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );

         // Pop the read data
         if (PopFromReadMemList( pDev,
                                 QMICTL,
                                 transactionID,
                                 &pReadBuffer,
                                 &readBufferSize ) == true)
         {
            // Success

            // End critical section
            spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
         
            // We don't care about the result
            kfree( pReadBuffer );

            break;
         }
         else
         {
            // Read mismatch/failure, unlock and continue
            spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
         }
      }
      else
      {
         readSem->magic = 0;
         // Enter critical section
         spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
         
         // Timeout, remove the async read
         NotifyAndPopNotifyList( pDev, QMICTL, transactionID );
         
         // End critical section
         spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags );
      }

      if (pDev->mbDeregisterQMIDevice)
      {
         kfree( pWriteBuffer );
         return false;
      }
   }

   kfree( pWriteBuffer );

   // Did we time out?   
   if (curTime >= timeout)
   {
      return false;
   }
   
   DBG( "QMI Ready after %u milliseconds\n", curTime );

   // Success
   return true;
}

/*===========================================================================
METHOD:
   QMIWDSCallback (Public Method)

DESCRIPTION:
   QMI WDS callback function
   Update net stats or link state

PARAMETERS:
   pDev     [ I ] - Device specific memory
   clientID [ I ] - Client ID
   pData    [ I ] - Callback data (unused)

RETURN VALUE:
   None
===========================================================================*/
static void QMIWDSCallback(
   sGobiUSBNet *    pDev,
   u16                clientID,
   void *             pData )
{
   bool bRet;
   int result;
   void * pReadBuffer;
   u16 readBufferSize;

#if 0
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,31 ))
   struct net_device_stats * pStats = &(pDev->mpNetDev->stats);
#else
   struct net_device_stats * pStats = &(pDev->mpNetDev->net->stats);
#endif
#endif

   u32 TXOk = (u32)-1;
   u32 RXOk = (u32)-1;
   u32 TXErr = (u32)-1;
   u32 RXErr = (u32)-1;
   u32 TXOfl = (u32)-1;
   u32 RXOfl = (u32)-1;
   u64 TXBytesOk = (u64)-1;
   u64 RXBytesOk = (u64)-1;
   bool bLinkState;
   bool bReconfigure;
   unsigned long flags;
   
   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return;
   }

   // Critical section
   spin_lock_irqsave( &pDev->mQMIDev.mClientMemLock, flags );
   
   bRet = PopFromReadMemList( pDev,
                              clientID,
                              0,
                              &pReadBuffer,
                              &readBufferSize );
   
   // End critical section
   spin_unlock_irqrestore( &pDev->mQMIDev.mClientMemLock, flags ); 
   
   if (bRet == false)
   {
      DBG( "WDS callback failed to get data\n" );
      return;
   }
   
   // Default values
   bLinkState = ! GobiTestDownReason( pDev, NO_NDIS_CONNECTION );
   bReconfigure = false;

   result = QMIWDSEventResp( pReadBuffer,
                             readBufferSize,
                             &TXOk,
                             &RXOk,
                             &TXErr,
                             &RXErr,
                             &TXOfl,
                             &RXOfl,
                             &TXBytesOk,
                             &RXBytesOk,
                             &bLinkState,
                             &bReconfigure );
   if (result < 0)
   {
      DBG( "bad WDS packet\n" );
   }
   else
   {
#if 0 //usbbet.c will do this job
      // Fill in new values, ignore max values
      if (TXOfl != (u32)-1)
      {
         pStats->tx_fifo_errors = TXOfl;
      }
      
      if (RXOfl != (u32)-1)
      {
         pStats->rx_fifo_errors = RXOfl;
      }

      if (TXErr != (u32)-1)
      {
         pStats->tx_errors = TXErr;
      }
      
      if (RXErr != (u32)-1)
      {
         pStats->rx_errors = RXErr;
      }

      if (TXOk != (u32)-1)
      {
         pStats->tx_packets = TXOk + pStats->tx_errors;
      }
      
      if (RXOk != (u32)-1)
      {
         pStats->rx_packets = RXOk + pStats->rx_errors;
      }

      if (TXBytesOk != (u64)-1)
      {
         pStats->tx_bytes = TXBytesOk;
      }
      
      if (RXBytesOk != (u64)-1)
      {
         pStats->rx_bytes = RXBytesOk;
      }
#endif

      if (bReconfigure == true)
      {
         DBG( "Net device link reset\n" );
         GobiSetDownReason( pDev, NO_NDIS_CONNECTION );
         GobiClearDownReason( pDev, NO_NDIS_CONNECTION );
      }
      else
      {
         if (bLinkState == true)
         {
            if (GobiTestDownReason( pDev, NO_NDIS_CONNECTION )) {
                DBG( "Net device link is connected\n" );
                GobiClearDownReason( pDev, NO_NDIS_CONNECTION );
            }
         }
         else
         {
            if (!GobiTestDownReason( pDev, NO_NDIS_CONNECTION )) {
                DBG( "Net device link is disconnected\n" );
                GobiSetDownReason( pDev, NO_NDIS_CONNECTION );
            }
         }
      }
   }

   kfree( pReadBuffer );

   // Setup next read
   result = ReadAsync( pDev,
                       clientID,
                       0,
                       QMIWDSCallback,
                       pData );
   if (result != 0)
   {
      DBG( "unable to setup next async read\n" );
   }

   return;
}

/*===========================================================================
METHOD:
   SetupQMIWDSCallback (Public Method)

DESCRIPTION:
   Request client and fire off reqests and start async read for 
   QMI WDS callback

PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   int - 0 for success
         Negative errno for failure
===========================================================================*/
static int SetupQMIWDSCallback( sGobiUSBNet * pDev )
{
   int result;
   void * pWriteBuffer;
   u16 writeBufferSize;
   u16 WDSClientID;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return -EFAULT;
   }
   
   result = GetClientID( pDev, QMIWDS );
   if (result < 0)
   {
      return result;
   }
   WDSClientID = result;

#if 0 // add for "AT$QCRMCALL=1,1", be careful: donot enable these codes if use quectel-CM, or cannot obtain IP by udhcpc
    if (pDev->mbMdm9x07)
    {
       void * pReadBuffer;
       u16 readBufferSize;

        writeBufferSize = QMIWDSSetQMUXBindMuxDataPortSize();
        pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
        if (pWriteBuffer == NULL)
        {
            return -ENOMEM;
        }
   
        result = QMIWDSSetQMUXBindMuxDataPortReq( pWriteBuffer, 
                                         writeBufferSize,
                                         0x81,
                                         3 );
        if (result < 0)
        {
            kfree( pWriteBuffer );
            return result;
        }

       result = WriteSync( pDev,
                           pWriteBuffer,
                           writeBufferSize,
                           WDSClientID );
        kfree( pWriteBuffer );

        if (result < 0)
        {
            return result;
        }
       
        result = ReadSync( pDev,
                          &pReadBuffer,
                          WDSClientID,
                          3 );
        if (result < 0)
        {
            return result;
        }
        readBufferSize = result;

        kfree( pReadBuffer );
   }
#endif

   // QMI WDS Set Event Report
   writeBufferSize = QMIWDSSetEventReportReqSize();
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }
   
   result = QMIWDSSetEventReportReq( pWriteBuffer, 
                                     writeBufferSize,
                                     1 );
   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }

   result = WriteSync( pDev,
                       pWriteBuffer,
                       writeBufferSize,
                       WDSClientID );
   kfree( pWriteBuffer );

   if (result < 0)
   {
      return result;
   }

   // QMI WDS Get PKG SRVC Status
   writeBufferSize = QMIWDSGetPKGSRVCStatusReqSize();
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }

   result = QMIWDSGetPKGSRVCStatusReq( pWriteBuffer, 
                                       writeBufferSize,
                                       2 );
   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }
   
   result = WriteSync( pDev,
                       pWriteBuffer,
                       writeBufferSize,
                       WDSClientID );
   kfree( pWriteBuffer );

   if (result < 0)
   {
      return result;
   }

   // Setup asnyc read callback
   result = ReadAsync( pDev,
                       WDSClientID,
                       0,
                       QMIWDSCallback,
                       NULL );
   if (result != 0)
   {
      DBG( "unable to setup async read\n" );
      return result;
   }

   // Send SetControlLineState request (USB_CDC)
   //   Required for Autoconnect
   result = usb_control_msg( pDev->mpNetDev->udev,
                             usb_sndctrlpipe( pDev->mpNetDev->udev, 0 ),
                             SET_CONTROL_LINE_STATE_REQUEST,
                             SET_CONTROL_LINE_STATE_REQUEST_TYPE,
                             CONTROL_DTR,
                             /* USB interface number to receive control message */
                             pDev->mpIntf->cur_altsetting->desc.bInterfaceNumber,
                             NULL,
                             0,
                             100 );
   if (result < 0)
   {
      DBG( "Bad SetControlLineState status %d\n", result );
      return result;
   }

   return 0;
}

/*===========================================================================
METHOD:
   QMIDMSGetMEID (Public Method)

DESCRIPTION:
   Register DMS client
   send MEID req and parse response
   Release DMS client

PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   None
===========================================================================*/
static int QMIDMSGetMEID( sGobiUSBNet * pDev )
{
   int result;
   void * pWriteBuffer;
   u16 writeBufferSize;
   void * pReadBuffer;
   u16 readBufferSize;
   u16 DMSClientID;

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return -EFAULT;
   }

   result = GetClientID( pDev, QMIDMS );
   if (result < 0)
   {
      return result;
   }
   DMSClientID = result;

   // QMI DMS Get Serial numbers Req
   writeBufferSize = QMIDMSGetMEIDReqSize();
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }

   result = QMIDMSGetMEIDReq( pWriteBuffer, 
                              writeBufferSize,
                              1 );
   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }

   result = WriteSync( pDev,
                       pWriteBuffer,
                       writeBufferSize,
                       DMSClientID );
   kfree( pWriteBuffer );

   if (result < 0)
   {
      return result;
   }

   // QMI DMS Get Serial numbers Resp
   result = ReadSync( pDev,
                      &pReadBuffer,
                      DMSClientID,
                      1 );
   if (result < 0)
   {
      return result;
   }
   readBufferSize = result;

   result = QMIDMSGetMEIDResp( pReadBuffer,
                               readBufferSize,
                               &pDev->mMEID[0],
                               14 );
   kfree( pReadBuffer );

   if (result < 0)
   {
      DBG( "bad get MEID resp\n" );
      
      // Non fatal error, device did not return any MEID
      //    Fill with 0's
      memset( &pDev->mMEID[0], '0', 14 );
   }

   ReleaseClientID( pDev, DMSClientID );

   // Success
   return 0;
}

/*===========================================================================
METHOD:
   QMIWDASetDataFormat (Public Method)

DESCRIPTION:
   Register WDA client
   send Data format request and parse response
   Release WDA client

PARAMETERS:
   pDev     [ I ] - Device specific memory

RETURN VALUE:
   None
===========================================================================*/
static int QMIWDASetDataFormat( sGobiUSBNet * pDev, int qmap_mode, int *rx_urb_size )
{
   int result;
   void * pWriteBuffer;
   u16 writeBufferSize;
   void * pReadBuffer;
   u16 readBufferSize;
   u16 WDAClientID;

   DBG("\n");

   if (IsDeviceValid( pDev ) == false)
   {
      DBG( "Invalid device\n" );
      return -EFAULT;
   }

   result = GetClientID( pDev, QMIWDA );
   if (result < 0)
   {
      return result;
   }
   WDAClientID = result;

   // QMI WDA Set Data Format Request
   writeBufferSize = QMIWDASetDataFormatReqSize(qmap_mode);
   pWriteBuffer = kmalloc( writeBufferSize, GFP_KERNEL );
   if (pWriteBuffer == NULL)
   {
      return -ENOMEM;
   }

   result = QMIWDASetDataFormatReq( pWriteBuffer,
                              writeBufferSize, pDev->mbRawIPMode,
                              qmap_mode ? pDev->qmap_version : 0, (31*1024),
                              1 );
   
   if (result < 0)
   {
      kfree( pWriteBuffer );
      return result;
   }

   result = WriteSync( pDev,
                       pWriteBuffer,
                       writeBufferSize,
                       WDAClientID );
   kfree( pWriteBuffer );

   if (result < 0)
   {
      return result;
   }

   // QMI DMS Get Serial numbers Resp
   result = ReadSync( pDev,
                      &pReadBuffer,
                      WDAClientID,
                      1 );
   if (result < 0)
   {
      return result;
   }
   readBufferSize = result;

if (qmap_mode && rx_urb_size) {
   int qmap_version = 0, rx_size = 0, tx_size = 0;
   result = QMIWDASetDataFormatResp( pReadBuffer,
                                     readBufferSize, pDev->mbRawIPMode, &qmap_version, &rx_size, &tx_size, &pDev->qmap_settings);
    INFO( "qmap settings qmap_version=%d, rx_size=%d, tx_size=%d\n",
        le32_to_cpu(qmap_version), le32_to_cpu(rx_size), le32_to_cpu(tx_size));

    if (le32_to_cpu(qmap_version)) {
#if defined(QUECTEL_UL_DATA_AGG)
        struct ul_agg_ctx *ctx = &pDev->agg_ctx;

        if (le32_to_cpu(pDev->qmap_settings.ul_data_aggregation_max_datagrams) > 1) {
            ctx->ul_data_aggregation_max_size = le32_to_cpu(pDev->qmap_settings.ul_data_aggregation_max_size);
            ctx->ul_data_aggregation_max_datagrams = le32_to_cpu(pDev->qmap_settings.ul_data_aggregation_max_datagrams);
            ctx->dl_minimum_padding = le32_to_cpu(pDev->qmap_settings.dl_minimum_padding);
        }
        INFO( "qmap settings ul_data_aggregation_max_size=%d, ul_data_aggregation_max_datagrams=%d\n",
                ctx->ul_data_aggregation_max_size, ctx->ul_data_aggregation_max_datagrams);
	if (ctx->ul_data_aggregation_max_datagrams > 11)
		ctx->ul_data_aggregation_max_datagrams = 11;
#endif
        *rx_urb_size = le32_to_cpu(rx_size);
    } else {
        *rx_urb_size = 0;
        result = -EFAULT;
    }
} else {
   int qmap_enabled = 0, rx_size = 0, tx_size = 0;
   result = QMIWDASetDataFormatResp( pReadBuffer,
                                     readBufferSize, pDev->mbRawIPMode, &qmap_enabled, &rx_size, &tx_size, NULL);
}

   kfree( pReadBuffer );

   if (result < 0)
   {
      DBG( "Data Format Cannot be set\n" );
   }

   ReleaseClientID( pDev, WDAClientID );

   // Success
    return 0;
}

int QuecQMIWDASetDataFormat( sGobiUSBNet * pDev, int qmap_mode, int *rx_urb_size ) {
	return QMIWDASetDataFormat(pDev, qmap_mode, rx_urb_size);
}
