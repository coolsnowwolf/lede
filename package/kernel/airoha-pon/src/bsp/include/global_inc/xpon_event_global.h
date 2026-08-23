#ifndef _XPON_EVENT_GLOBAL_H
#define _XPON_EVENT_GLOBAL_H  

/**
* \file  xpon_event_global.h
* \brief This file is xpon event global header file that will be exported for others to use.
* \author jun.wu
* \date     2020-09-22
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

/************************************************************************
*		   I N C L U D E S
*************************************************************************
*/

/************************************************************************
*		   D E F I N E S	&	C O N S T A N T S
*************************************************************************
*/

/************************************************************************
* 		   M A C R O S
*************************************************************************
*/
#define XMCS_EVENT_MAX_ITEMS			32

/************************************************************************
*		   D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*		   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
typedef enum {
	XMCS_EVENT_TYPE_GPON = 1,
	XMCS_EVENT_TYPE_EPON
} XMCS_EventType_t ;

#define XMCS_MAX_BUFFER_LEN   (52)

typedef enum {
	XMCS_EVENT_GPON_LOS = 0,
	XMCS_EVENT_GPON_PHY_READY,
	XMCS_EVNET_GPON_DEACTIVATE,
	XMCS_EVENT_GPON_ACTIVATE,
	XMCS_EVENT_GPON_TCONT_ALLOCED,
	XMCS_EVENT_GPON_MODE_CHANGE,
	XMCS_EVENT_GPON_PHY_TRANS_ALARM,
	XMCS_EVENT_GPON_MAX_ID, /* no use, just for backward compatiblity*/
	XMCS_EVENT_GPON_DISABLE,
	XMCS_EVENT_GPON_ENABLE,
	XMCS_EVENT_GPON_STATE_CHANGE,
	XMCS_EVENT_GPON_DYING_GASP,
	XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE,
	XMCS_EVENT_GPON_PRIV_PLOAM,
	XMCS_EVENT_GPON_REBOOT_ONU,

    /* all new events must be added above this line! */
    XMCS_EVENT_GPON_MAX
} XMCS_GponEventId_t ;

typedef enum {
	XMCS_EVENT_EPON_LOS = XMCS_EVENT_GPON_MAX +1,
	XMCS_EVENT_EPON_PHY_READY,
	XMCS_EVENT_EPON_REGISTER,
	XMCS_EVENT_EPON_DEREGISTER,
	XMCS_EVENT_EPON_DENIED,
	XMCS_EVENT_EPON_MODE_CHANGE,
	XMCS_EVENT_EPON_PHY_TRANS_ALARM,
	XMCS_EVENT_EPON_TX_POWER_OFF,	
	XMCS_EVENT_EPON_DYING_GASP,
	XMCS_EVENT_EPON_MAC_CHANGED,
	XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE,
	XMCS_EVENT_EPON_REGISTERING,
	XMCS_EVENT_EPON_dpoe_key,
	XMCS_EVENT_CHANGE_TO_TURBO_EPON,
	XMCS_EVENT_CHANGE_TO_1G_1G_EPON,

    /* all new events must be added above this line! */
	XMCS_EVENT_EPON_MAX_ID
} XMCS_EponEventId_t ;


typedef struct {
	XMCS_EventType_t	type ;
	unsigned int		id ;
	unsigned int		value ;
	unsigned char		buf[XMCS_MAX_BUFFER_LEN];
} XPON_EVENT_t;

typedef struct {
	unsigned char		depth ;
	unsigned char		image ;
	unsigned char		state;	
	unsigned char		flags;
	unsigned char		placeholder[XMCS_MAX_BUFFER_LEN - 4];
} XPON_REBOOT_t;

/* struct definition for fault detection command */
struct XMCS_PonEventStatus_S {
	XPON_EVENT_t event[XMCS_EVENT_MAX_ITEMS] ;
	unsigned int				items ;
};



/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif

