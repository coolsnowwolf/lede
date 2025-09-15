/****************************************************************************

    Module Name:
    OS/rtmp_osabl.h
 
    Abstract:
	Some structure/definitions for OS ABL function.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RTMP_OS_ABL_H__
#define __RTMP_OS_ABL_H__

#ifdef OS_ABL_FUNC_SUPPORT

#ifdef OS_ABL_OS_PCI_SUPPORT
#ifndef RTMP_MAC_PCI
#define RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */
#ifndef RTMP_PCI_SUPPORT
#define RTMP_PCI_SUPPORT
#endif /* RTMP_PCI_SUPPORT */
#endif /* OS_ABL_OS_PCI_SUPPORT */

#ifdef OS_ABL_OS_USB_SUPPORT
#include <linux/usb.h>

#define RTMP_MAC_USB
#define RTMP_USB_SUPPORT
#endif /* OS_ABL_OS_USB_SUPPORT */

#ifdef OS_ABL_OS_RBUS_SUPPORT
#define RTMP_RBUS_SUPPORT
#endif /* OS_ABL_OS_RBUS_SUPPORT */

#ifdef OS_ABL_OS_AP_SUPPORT
#ifndef CONFIG_AP_SUPPORT
#define CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#endif /* OS_ABL_OS_AP_SUPPORT */

#ifdef OS_ABL_OS_STA_SUPPORT
#define CONFIG_STA_SUPPORT
#endif /* OS_ABL_OS_STA_SUPPORT */

/* AP & STA con-current */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)		if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif

#endif /* OS_ABL_FUNC_SUPPORT */

#endif /* __RTMP_OS_ABL_H__ */

