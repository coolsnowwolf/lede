#ifndef _ECNT_MACRO_COMPATIBLE_H
#define _ECNT_MACRO_COMPATIBLE_H

#ifdef __KERNEL__
#include <linux/libcompileoption.h>
#else
#include "libcompileoption.h"
#endif

static inline unsigned int dev_offset_macro_compatible(void)
{
	unsigned int dev_offset_macro =  28;
	
	if(TCSUPPORT_MULTI_USER_ITF_VAL)
	{
		dev_offset_macro = 27;
	}
	else
	{
		dev_offset_macro = 28;
	}
	
	return dev_offset_macro;
}

static inline unsigned int wlan_dev_offset_macro_compatible(void)
{
	unsigned int wlan_dev_offset_macro =  5;
	
	if(TCSUPPORT_MULTI_SWITCH_EXT_VAL || TCSUPPORT_MULTI_USER_ITF_VAL)
	{
		wlan_dev_offset_macro = 9;
	}
	else
	{
		wlan_dev_offset_macro = 5;
	}

	return wlan_dev_offset_macro;
}

static inline unsigned int wlan_ac_dev_offset_macro_compatible(void)
{
	unsigned int wlan_ac_dev_offset_macro =  11;
	
	if(TCSUPPORT_MULTI_SWITCH_EXT_VAL || TCSUPPORT_MULTI_USER_ITF_VAL)
	{
		wlan_ac_dev_offset_macro = 17;
	}
	else
	{
		wlan_ac_dev_offset_macro = 11;
	}
	
	return wlan_ac_dev_offset_macro;
}

static inline unsigned int usb_dev_offset_macro_compatible(void)
{
	unsigned int usb_dev_offset_macro =  9;
	
	if(TCSUPPORT_MULTI_SWITCH_EXT_VAL || TCSUPPORT_MULTI_USER_ITF_VAL)
	{
		usb_dev_offset_macro = 25;
	}
	else
	{
		usb_dev_offset_macro = 9;
	}
	
	return usb_dev_offset_macro;
}

static inline unsigned int lanif_mask_macro_compatible(void)
{
	unsigned int lanif_mask =  0xf0000000;
	
	if(TCSUPPORT_MULTI_SWITCH_EXT_VAL || TCSUPPORT_MULTI_USER_ITF_VAL)
	{
		lanif_mask = 0xf8000000;
	}
	else
	{
		lanif_mask = 0xf0000000;
	}
	
	return lanif_mask;
}

#endif

