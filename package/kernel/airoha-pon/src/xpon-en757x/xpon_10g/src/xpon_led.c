/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include "common/xpon_global.h"
#include "common/xpon_led.h"

/*______________________________________________________________________________
**	function name
**		xpon_los_led_flicker
**	description:
**		LOS LED ficker function.
**	parameters:
**		data: not used.
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**		IO_GREG
**		IO_SREG
**		XPON_START_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
#if 0
#ifndef TCSUPPORT_XPON_LED
#define GPIO_GPIO17_OUTPUT_ENABLE				(1<<17)
void xpon_los_led_flicker(ulong data)
{
	uint read_data = 0;
	uint write_data = 0;
	uint inverse_data = 0;
#ifdef TCSUPPORT_CPU_ARMV8
	read_data = GET_GPIO_DATA();
#else
	read_data = IO_GREG(GPIO_CSR_DATA);
#endif
	if((read_data & GPIO_GPIO17_OUTPUT_ENABLE) == 0)
		inverse_data = GPIO_GPIO17_OUTPUT_ENABLE;
	else
		inverse_data = 0;
	write_data = (read_data & ~(GPIO_GPIO17_OUTPUT_ENABLE)) | inverse_data;
#ifdef TCSUPPORT_CPU_ARMV8
	SET_GPIO_DATA(write_data);
#else
	IO_SREG(GPIO_CSR_DATA, write_data);
#endif

    if( !gpPonSysData->flicker_stop ){
    	XPON_START_TIMER(gpPonSysData->led_timer,500) ; /* 500 ms */
    }
}

/*______________________________________________________________________________
**	function name
**		change_alarm_led_status
**	description:
**		This function is used to control the LOS LED lighting.
**	parameters:
**		los_led_status : LED_OFF; LED_ON; LED_FLICKER 
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**		ledTurnOff
**		ledTurnOn
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/

#define LOS_LED_GPIO_MASK						(1<<17)
void change_alarm_led_status(XPON_LED_Status_t los_led_status)
{
	uint read_data  = 0;
	uint write_data = 0;
	
#ifdef TCSUPPORT_CPU_ARMV8
	read_data = GET_GPIO_DATA();
#else
	read_data = IO_GREG(GPIO_CSR_DATA);
#endif

    switch(los_led_status)
    {
    	case LED_ON:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		//read_data = IO_GREG(GPIO_CSR_DATA);
    		write_data = read_data | (LOS_LED_GPIO_MASK);
#ifdef TCSUPPORT_CPU_ARMV8
			SET_GPIO_DATA(write_data);
#else
			IO_SREG(GPIO_CSR_DATA, write_data);
#endif
    		PON_MSG(MSG_DBG, "turn on LOS LED success. \n");
    		break;

    	case LED_OFF:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		//read_data = IO_GREG(GPIO_CSR_DATA);
    		write_data = read_data & ~(LOS_LED_GPIO_MASK);
#ifdef TCSUPPORT_CPU_ARMV8
			SET_GPIO_DATA(write_data);
#else
			IO_SREG(GPIO_CSR_DATA, write_data);
#endif
    		PON_MSG(MSG_DBG, "turn off LOS LED success. \n");
    		break;

    	case LED_FLICKER:
            gpPonSysData->flicker_stop = 0;
    		XPON_START_TIMER(gpPonSysData->led_timer,500) ; /* 500 ms */
    		PON_MSG(MSG_DBG, "LOS LED flicker success. \n");
    		break;

    	default:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		PON_MSG(MSG_ERR, "LOS LED fail. \n");
    		break;
    }


}

#endif

#endif
/*______________________________________________________________________________
**	function name
**		change_alarm_led_status
**	description:
**		This function is used to control the LOS LED lighting.
**	parameters:
**		los_led_status : LED_OFF; LED_ON; LED_FLICKER  
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**		ledTurnOff
**		ledTurnOn
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/

void change_alarm_led_status(XPON_LED_Status_t los_led_status)
{
#ifdef TCSUPPORT_XPON_LED
    switch(los_led_status)
    {
    	case LED_ON:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);      

#else
			ledTurnOff(LED_XPON_LOS_STATUS);
#endif
			ledTurnOn(LED_XPON_LOS_ON_STATUS);
    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case LED_OFF:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);

#else
			ledTurnOff(LED_XPON_LOS_ON_STATUS);
			ledTurnOff(LED_XPON_LOS_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case LED_FLICKER:
			ledTurnOff(LED_XPON_LOS_ON_STATUS);
#ifdef TCSUPPORT_KERNEL_API
    		ledBlinkOn(LED_XPON_LOS_ON_STATUS, 1);

#else
			
    		ledTurnOn(LED_XPON_LOS_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "LOS LED fail. \n");
    		break;
    }
#endif
}

/*______________________________________________________________________________
**	function name
**		change_pon_led_status
**	description:
**		This function is used to control the LOS LED lighting.
**	parameters:
**		los_led_status : LED_OFF; LED_ON; LED_FLICKER 
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**		ledTurnOff
**		ledTurnOn
**		PON_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/
void change_pon_led_status(XPON_LED_Status_t pon_led_status)
{
#ifdef TCSUPPORT_XPON_LED
    switch(pon_led_status)
    {
    	case LED_ON:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_STATUS);

#else
			ledTurnOff(LED_XPON_TRYING_STATUS);
#endif
    		ledTurnOn(LED_XPON_STATUS);
    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case LED_OFF:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_STATUS);

#else
			ledTurnOff(LED_XPON_TRYING_STATUS);
    		ledTurnOff(LED_XPON_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case LED_FLICKER:
			ledTurnOff(LED_XPON_STATUS);
#ifdef TCSUPPORT_KERNEL_API
			ledBlinkOn(LED_XPON_STATUS, 1);

#else
    		ledTurnOn(LED_XPON_TRYING_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "PON LED fail. \n");
    		break;
    }

#endif
}

/*______________________________________________________________________________
**	function name
**		xpon_los_led_read_proc
**	description:
**		Add a proc file to read current pon phy status(LOS/RDY).
**	parameters:
**		page :  
**		start :  
**		off :  
**		count :
**		eof :
**		data :
**	global:
**		gpPonSysData
**	return:
**		length of character size.
**	call:
**		None 
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_los_led_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "%d\n",  gpPonSysData->ponPhyStaus );

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
/*______________________________________________________________________________
**	function name
**		xpon_los_led_write_proc
**	description:
**		Add a proc file to  control los led status. 
**	parameters:
**		file :  
**		buffer :  
**		count :  
**		data : 
**	global:
**		None
**	return:
**		length of input character size.
**	call:
**		change_alarm_led_status 
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_los_led_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[8] = {0};
	unsigned long val  = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	val = simple_strtoul(val_string, NULL, 10);

	change_alarm_led_status(val);
	
	return count;
}
