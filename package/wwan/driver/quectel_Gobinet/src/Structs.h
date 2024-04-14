/*===========================================================================
FILE:
   Structs.h

DESCRIPTION:
   Declaration of structures used by the Qualcomm Linux USB Network driver
   
FUNCTIONS:
   none

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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/poll.h>
#include <linux/completion.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>

#define QUECTEL_WWAN_QMAP 4 //MAX is 7
#ifdef QUECTEL_WWAN_QMAP
#define QUECTEL_QMAP_MUX_ID 0x81
#endif

//#define QUECTEL_QMI_MERGE

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE) 
#define QUECTEL_BRIDGE_MODE
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,21 ))
static inline void skb_reset_mac_header(struct sk_buff *skb)
{
    skb->mac.raw = skb->data;
}
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,22 ))
#define bool      u8
#ifndef URB_FREE_BUFFER
#define URB_FREE_BUFFER_BY_SELF //usb_free_urb will not free, should free by self
#define URB_FREE_BUFFER		0x0100	/* Free transfer buffer with the URB */
#endif

/**
 * usb_endpoint_type - get the endpoint's transfer type
 * @epd: endpoint to be checked
 *
 * Returns one of USB_ENDPOINT_XFER_{CONTROL, ISOC, BULK, INT} according
 * to @epd's transfer type.
 */
static inline int usb_endpoint_type(const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,18 ))
/**
 * usb_endpoint_dir_in - check if the endpoint has IN direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type IN, otherwise it returns false.
 */
static inline int usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

/**
 * usb_endpoint_dir_out - check if the endpoint has OUT direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type OUT, otherwise it returns false.
 */
static inline int usb_endpoint_dir_out(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

/**
 * usb_endpoint_xfer_int - check if the endpoint has interrupt transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type interrupt, otherwise it returns
 * false.
 */
static inline int usb_endpoint_xfer_int(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_INT);
}

static inline int usb_autopm_set_interface(struct usb_interface *intf)
{ return 0; }

static inline int usb_autopm_get_interface(struct usb_interface *intf)
{ return 0; }

static inline int usb_autopm_get_interface_async(struct usb_interface *intf)
{ return 0; }

static inline void usb_autopm_put_interface(struct usb_interface *intf)
{ }
static inline void usb_autopm_put_interface_async(struct usb_interface *intf)
{ }
static inline void usb_autopm_enable(struct usb_interface *intf)
{ }
static inline void usb_autopm_disable(struct usb_interface *intf)
{ }
static inline void usb_mark_last_busy(struct usb_device *udev)
{ }
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,24 ))
   #include "usbnet.h"
#else
   #include <linux/usb/usbnet.h>
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,25 ))
   #include <linux/fdtable.h>
#else
   #include <linux/file.h>
#endif

// Used in recursion, defined later below
struct sGobiUSBNet;


#if defined(QUECTEL_WWAN_QMAP)
#define QUECTEL_UL_DATA_AGG 1

#if defined(QUECTEL_UL_DATA_AGG)
struct ul_agg_ctx {
	/* QMIWDS_ADMIN_SET_DATA_FORMAT_RESP TLV_0x17 and TLV_0x18 */
	uint ul_data_aggregation_max_datagrams; //UplinkDataAggregationMaxDatagramsTlv
	uint ul_data_aggregation_max_size; //UplinkDataAggregationMaxSizeTlv
	uint dl_minimum_padding;
};
#endif
#endif

/*=========================================================================*/
// Struct sReadMemList
//
//    Structure that defines an entry in a Read Memory linked list
/*=========================================================================*/
typedef struct sReadMemList
{
   /* Data buffer */
   void *                     mpData;
   
   /* Transaction ID */
   u16                        mTransactionID;

   /* Size of data buffer */
   u16                        mDataSize;

   /* Next entry in linked list */
   struct sReadMemList *      mpNext;

} sReadMemList;

/*=========================================================================*/
// Struct sNotifyList
//
//    Structure that defines an entry in a Notification linked list
/*=========================================================================*/
typedef struct sNotifyList
{
   /* Function to be run when data becomes available */
   void                  (* mpNotifyFunct)(struct sGobiUSBNet *, u16, void *);
   
   /* Transaction ID */
   u16                   mTransactionID;

   /* Data to provide as parameter to mpNotifyFunct */
   void *                mpData;
   
   /* Next entry in linked list */
   struct sNotifyList *  mpNext;

} sNotifyList;

/*=========================================================================*/
// Struct sURBList
//
//    Structure that defines an entry in a URB linked list
/*=========================================================================*/
typedef struct sURBList
{
   /* The current URB */
   struct urb *       mpURB;

   /* Next entry in linked list */
   struct sURBList *  mpNext;

} sURBList;

/*=========================================================================*/
// Struct sClientMemList
//
//    Structure that defines an entry in a Client Memory linked list
//      Stores data specific to a Service Type and Client ID
/*=========================================================================*/
typedef struct sClientMemList
{
   /* Client ID for this Client */
   u16                          mClientID;

   /* Linked list of Read entries */
   /*    Stores data read from device before sending to client */
   sReadMemList *               mpList;
   
   /* Linked list of Notification entries */
   /*    Stores notification functions to be run as data becomes 
         available or the device is removed */
   sNotifyList *                mpReadNotifyList;

   /* Linked list of URB entries */
   /*    Stores pointers to outstanding URBs which need canceled 
         when the client is deregistered or the device is removed */
   sURBList *                   mpURBList;
   
   /* Next entry in linked list */
   struct sClientMemList *      mpNext;

   /* Wait queue object for poll() */
   wait_queue_head_t    mWaitQueue;

} sClientMemList;

/*=========================================================================*/
// Struct sURBSetupPacket
//
//    Structure that defines a USB Setup packet for Control URBs
//    Taken from USB CDC specifications
/*=========================================================================*/
typedef struct sURBSetupPacket
{
   /* Request type */
   u8    mRequestType;

   /* Request code */
   u8    mRequestCode;

   /* Value */
   u16   mValue;

   /* Index */
   u16   mIndex;

   /* Length of Control URB */
   u16   mLength;

} sURBSetupPacket;

// Common value for sURBSetupPacket.mLength
#define DEFAULT_READ_URB_LENGTH 0x1000

#ifdef QUECTEL_QMI_MERGE
#define MERGE_PACKET_IDENTITY 0x2c7c
#define MERGE_PACKET_VERSION 0x0001
#define MERGE_PACKET_MAX_PAYLOAD_SIZE 56
typedef struct sQMIMsgHeader {
    u16 idenity;
    u16 version;
    u16 cur_len;
    u16 total_len;
} sQMIMsgHeader;

typedef struct sQMIMsgPacket {
    sQMIMsgHeader header;
    u16 len;
    char buf[DEFAULT_READ_URB_LENGTH];
} sQMIMsgPacket;
#endif

#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
/*=========================================================================*/
// Struct sAutoPM
//
//    Structure used to manage AutoPM thread which determines whether the
//    device is in use or may enter autosuspend.  Also submits net 
//    transmissions asynchronously.
/*=========================================================================*/
typedef struct sAutoPM
{
   /* Thread for atomic autopm function */
   struct task_struct *       mpThread;

   /* Signal for completion when it's time for the thread to work */
   struct completion          mThreadDoWork;

   /* Time to exit? */
   bool                       mbExit;

   /* List of URB's queued to be sent to the device */
   sURBList *                 mpURBList;

   /* URB list lock (for adding and removing elements) */
   spinlock_t                 mURBListLock;

   /* Length of the URB list */
   atomic_t                   mURBListLen;
   
   /* Active URB */
   struct urb *               mpActiveURB;

   /* Active URB lock (for adding and removing elements) */
   spinlock_t                 mActiveURBLock;
   
   /* Duplicate pointer to USB device interface */
   struct usb_interface *     mpIntf;

} sAutoPM;
#endif
#endif /* CONFIG_PM */

/*=========================================================================*/
// Struct sQMIDev
//
//    Structure that defines the data for the QMI device
/*=========================================================================*/
typedef struct sQMIDev
{
   /* Device number */
   dev_t                      mDevNum;

   /* Device class */
   struct class *             mpDevClass;

   /* cdev struct */
   struct cdev                mCdev;

   /* is mCdev initialized? */
   bool                       mbCdevIsInitialized;

   /* Pointer to read URB */
   struct urb *               mpReadURB;

//#define READ_QMI_URB_ERROR
#ifdef READ_QMI_URB_ERROR
   struct timer_list mReadUrbTimer;
#endif

#ifdef QUECTEL_QMI_MERGE
   sQMIMsgPacket * mpQmiMsgPacket;
#endif

   /* Read setup packet */
   sURBSetupPacket *          mpReadSetupPacket;

   /* Read buffer attached to current read URB */
   void *                     mpReadBuffer;
   
   /* Inturrupt URB */
   /*    Used to asynchronously notify when read data is available */
   struct urb *               mpIntURB;

   /* Buffer used by Inturrupt URB */
   void *                     mpIntBuffer;
   
   /* Pointer to memory linked list for all clients */
   sClientMemList *           mpClientMemList;
   
   /* Spinlock for client Memory entries */
   spinlock_t                 mClientMemLock;

   /* Transaction ID associated with QMICTL "client" */
   atomic_t                   mQMICTLTransactionID;

} sQMIDev;

typedef struct {
	u32 qmap_enabled;
	u32 dl_data_aggregation_max_datagrams;
	u32 dl_data_aggregation_max_size ;
	u32 ul_data_aggregation_max_datagrams; 
	u32 ul_data_aggregation_max_size;
	u32 dl_minimum_padding;
} QMAP_SETTING;

/*=========================================================================*/
// Struct sGobiUSBNet
//
//    Structure that defines the data associated with the Qualcomm USB device
/*=========================================================================*/
typedef struct sGobiUSBNet
{
    atomic_t refcount;

   /* Net device structure */
   struct usbnet *        mpNetDev;
#ifdef QUECTEL_WWAN_QMAP
   unsigned link_state;
   int qmap_mode;
   int qmap_size;
   int qmap_version;
   struct net_device	*mpQmapNetDev[QUECTEL_WWAN_QMAP];
   struct tasklet_struct	txq;

   QMAP_SETTING qmap_settings;
#if defined(QUECTEL_UL_DATA_AGG)
   struct ul_agg_ctx agg_ctx;
#endif

#ifdef QUECTEL_BRIDGE_MODE
   int m_qmap_bridge_mode[QUECTEL_WWAN_QMAP];
#endif
#endif

#if 1 //def DATA_MODE_RP
    bool                   mbMdm9x07;  
    bool                   mbMdm9x06;	//for BG96
   /* QMI "device" work in IP Mode or ETH Mode */
   bool                   mbRawIPMode;
#ifdef QUECTEL_BRIDGE_MODE
   int m_bridge_mode;
   uint m_bridge_ipv4;
   unsigned char     mHostMAC[6];
#endif
   int m_qcrmcall_mode;
#endif

   struct completion mQMIReadyCompletion;
   bool                   mbQMIReady;
   bool                   mbProbeDone;
   bool                   mbQMISyncIng;

   /* Usb device interface */
   struct usb_interface * mpIntf;

   /* Pointers to usbnet_open and usbnet_stop functions */
   int                  (* mpUSBNetOpen)(struct net_device *);
   int                  (* mpUSBNetStop)(struct net_device *);
   
   /* Reason(s) why interface is down */
   /* Used by Gobi*DownReason */
   unsigned long          mDownReason;
#define NO_NDIS_CONNECTION    0
#define CDC_CONNECTION_SPEED  1
#define DRIVER_SUSPENDED      2
#define NET_IFACE_STOPPED     3

   /* QMI "device" status */
   bool                   mbQMIValid;

   bool                   mbDeregisterQMIDevice;

   /* QMI "device" memory */
   sQMIDev                mQMIDev;

   /* Device MEID */
   char                   mMEID[14];
    struct hrtimer timer;
    struct tasklet_struct bh;
    unsigned long
        pending_num : 8,
        pending_size : 16;
    struct sk_buff *pending_pool[16];

#ifdef CONFIG_PM
   #if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))   
   /* AutoPM thread */
   sAutoPM                mAutoPM;
#endif
#endif /* CONFIG_PM */
} sGobiUSBNet;

/*=========================================================================*/
// Struct sQMIFilpStorage
//
//    Structure that defines the storage each file handle contains
//       Relates the file handle to a client
/*=========================================================================*/
typedef struct sQMIFilpStorage
{
   /* Client ID */
   u16                  mClientID;
   
   /* Device pointer */
   sGobiUSBNet *        mpDev;

} sQMIFilpStorage;

