#ifndef _PLOAM_RAW_H_
#define _PLOAM_RAW_H_


#define PLOAM_BROADCAST_ADDR					(0xFF)

#define PLOAM_DOWN_MSG_UPSTREAM_OVERHEAD		(0x01)
#define PLOAM_DOWN_MSG_ASSIGN_ONUID				(0x03)
#define PLOAM_DOWN_MSG_RANGING_TIME				(0x04)
#define PLOAM_DOWN_MSG_DEACTIVATE_ONUID			(0x05)
#define PLOAM_DOWN_MSG_DISABLE_SERIAL_NUM		(0x06)
#define PLOAM_DOWN_MSG_ENCRYPTED_PORTID			(0x08)
#define PLOAM_DOWN_MSG_REQUEST_PASSWORD			(0x09)
#define PLOAM_DOWN_MSG_ASSIGN_ALLOCID 			(0x0A)
#define PLOAM_DOWN_MSG_POPUP_MESSAGE			(0x0C)
#define PLOAM_DOWN_MSG_REQUEST_KEY				(0x0D)
#define PLOAM_DOWN_MSG_CONFIG_PORTID			(0x0E)
#define PLOAM_DOWN_MSG_PEE_MESSAGE				(0x0F)
#define PLOAM_DOWN_MSG_CPL_MESSAGE				(0x10)
#define PLOAM_DOWN_MSG_PST_MESSAGE				(0x11)
#define PLOAM_DOWN_MSG_BER_INTERVAL 			(0x12)
#define PLOAM_DOWN_MSG_KEY_SWITCHING_TIME		(0x13)
#define PLOAM_DOWN_MSG_EXTENDED_BURST_LENGTH	(0x14)
#define PLOAM_DOWN_MSG_PONID_MESSAGE			(0x15)
#define PLOAM_DOWN_MSG_SWIFT_POPUP				(0x16)
#define PLOAM_DOWN_MSG_RANGING_ADJUSTMENT		(0x17)
#define PLOAM_DOWN_MSG_SLEEP_ALLOW_MESSAGE		(0x18)
#define PLOAM_DOWN_MAX_TYPE						(0x19)

#define PLOAM_UP_MSG_SERIAL_NUMBER 				(0x01)
#define PLOAM_UP_MSG_PASSWORD_MESSAGE			(0x02)
#define PLOAM_UP_MSG_DYING_GASP					(0x03)
#define PLOAM_UP_MSG_NO_MESSAGE					(0x04)
#define PLOAM_UP_MSG_ENCRYPTION_KEY				(0x05)
#define PLOAM_UP_MSG_PEE_MESSAGE				(0x06)
#define PLOAM_UP_MSG_PST_MESSAGE				(0x07)
#define PLOAM_UP_MSG_REI_MESSAGE				(0x08)
#define PLOAM_UP_MSG_ACKNOWLEDGE				(0x09)
#define PLOAM_UP_MSG_SLEEP_REQUEST_MESSAGE		(0x0A)


/* PLOAM General message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar resv[10] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_General_T ;


typedef PLOAM_RAW_General_T PLOAM_RAW_Deactivate_OnuID_T, 
							 PLOAM_RAW_Request_Password_T, 
							 PLOAM_RAW_No_Message_T, 
							 PLOAM_RAW_Popup_T, 
							 PLOAM_RAW_Request_Key_T, 
							 PLOAM_RAW_PEE_T, 
							 PLOAM_RAW_Swift_Popup_T, 
							 PLOAM_RAW_Dying_Gasp_T ;




/***************************************
 Downstream PLOAM Message
***************************************/
/* Upstream_Overhead message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar gbits ;
		unchar t1_pbits ;
		unchar t2_pbits ;
		unchar t3_pbits ;
		unchar delimiter[3] ;
#ifdef __BIG_ENDIAN
		unchar resv1				:2 ;
		unchar delay_mode			:1 ;
		unchar sn_mask				:1 ;
		unchar sn_tran_num			:2 ;
		unchar tx_power				:2 ;
#else
		unchar tx_power				:2 ;
		unchar sn_tran_num			:2 ;
		unchar sn_mask				:1 ;
		unchar delay_mode			:1 ;
		unchar resv1				:2 ;
#endif /* __BIG_ENDIAN */
		unchar delay_time[2] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Upstream_Overhead_T ;

/* Assign_ONU-ID message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar onu_id ;		
		unchar sn[8] ;
		unchar resv ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Assign_OnuID_T ;

/* Ranging_Time message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1				:7 ;
		unchar eqd_type				:1 ;
#else
		unchar eqd_type				:1 ;
		unchar resv1				:7 ;
#endif /*__BIG_ENDIAN */
		unchar delay[4] ;
		unchar resv2[5] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Ranging_Time_T ;

/* Disable_Serial_Number message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar mode ;
		unchar sn[8] ;
		unchar resv ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Disable_SN_T ;

/* Encrypted_Port-ID message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1				:6 ;
		unchar encrypt				:2 ;
#else
		unchar encrypt				:2 ;
		unchar resv1				:6 ;
#endif /* __BIG_ENDIAN */
		unchar port_id_m ;
#ifdef __BIG_ENDIAN
		unchar port_id_l			:4 ;
		unchar resv2				:4 ;
#else
		unchar resv2				:4 ;
		unchar port_id_l			:4 ;
#endif /* __BIG_ENDIAN */
		unchar resv3[7]	;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Encrypted_PortID_T ;

/* Assign_Alloc-ID message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar alloc_id_m ;
#ifdef __BIG_ENDIAN
		unchar alloc_id_l			:4 ;
		unchar resv1				:4 ;
#else
		unchar resv1				:4 ;
		unchar alloc_id_l			:4 ;
#endif /* __BIG_ENDIAN */
		unchar type ;
		unchar resv2[7] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Assign_AllocID_T ;

/* Configure_Port-ID message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1				:7 ;
		unchar activate 			:1 ;
#else
		unchar activate 			:1 ;
		unchar resv1				:7 ;
#endif /* __BIG_ENDIAN */
		unchar port_id_m ;
#ifdef __BIG_ENDIAN
		unchar port_id_l			:4 ;
		unchar resv2				:4 ;
#else
		unchar resv2				:4 ;
		unchar port_id_l			:4 ;
#endif /* __BIG_ENDIAN */
		unchar resv3[7] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Configure_PortID_T ;

/* Change_Power_Level message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar power_level ;
		unchar resv[9] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_CPL_T ;

/* PST message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar line_num ;
		unchar k1_ctrl ;
		unchar k2_ctrl ;
		unchar resv[7] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_PST_T ;

/* BER Interval message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar interval[4] ;
		unchar resv[6] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_BER_Interval_T ;

/* Key_Switching_Time message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar counter[4] ;
		unchar resv[6] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Key_Switching_Time_T ;

/* Extended_Burst_Length message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar o3_t3_preamble ;
		unchar o5_t3_preamble ;
		unchar resv[8] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Extended_Burst_Length_T ;

/* PON-ID message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar a_bit					:1 ;
		unchar b_class					:3 ;
		unchar resv						:4 ;
#else
		unchar resv						:4 ;
		unchar b_class					:3 ;
		unchar a_bit					:1 ;
#endif /* __BIG_ENDIAN */
		unchar pon_id[7] ;
		unchar tol ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_PonID_T ;

/* Ranging_Adjustment message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1					:6 ;
		unchar s_bit					:1 ;
		unchar resv2					:1 ;
#else
		unchar resv2					:1 ;
		unchar s_bit					:1 ;
		unchar resv1					:6 ;
#endif /* __BIG_ENDIAN */
		unchar eqd_value[4] ;
		unchar resv3[5] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Ranging_Adjustment_T ;

/* Sleep_Allow message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1				:7 ;
		unchar sleep_allowed		:1 ;
#else
		unchar sleep_allowed		:1 ;
		unchar resv1				:7 ;
#endif /* __BIG_ENDIAN */
		unchar resv2[9] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Sleep_Allow_T ;

/***************************************
 Upstream PLOAM Message
***************************************/
/* Serial_Number_ONU message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar vendor_id[4] ;
		unchar vendor_sn[4] ;
		unchar rd_m ;
#ifdef __BIG_ENDIAN
		unchar rd_l						:4 ;
		unchar a_bit					:1 ;
		unchar g_bit					:1 ;
		unchar tx_pl					:2 ;
#else
		unchar tx_pl					:2 ;
		unchar g_bit					:1 ;
		unchar a_bit					:1 ;
		unchar rd_l						:4 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Serial_Number_ONU_T ;

/* Password message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar passwd[10] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Password_T ;

/* Encryption_Key message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar key_idx ;
		unchar frag_idx ;
		unchar key[8] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Encryption_Key_T ;

/* REI message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar counter[4] ;
#ifdef __BIG_ENDIAN
		unchar resv1				:4 ;
		unchar seq_num 				:4 ;
#else
		unchar seq_num 				:4 ;
		unchar resv1				:4 ;
#endif /* __BIG_ENDIAN */
		unchar resv2[5] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_REI_T ;

/* Acknowledge message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
		unchar dm_id ;
		unchar dm_byte[9] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Acknowledge_T ;

/* Sleep_Request message */
typedef union {
	struct {
		unchar dest_id ;
		unchar msg_id ;
#ifdef __BIG_ENDIAN
		unchar resv1				:6 ;
		unchar sleep_mode			:2 ;
#else
		unchar sleep_mode 			:2 ;
		unchar resv1				:6 ;
#endif /* __BIG_ENDIAN */
		unchar resv2[9] ;
	} raw ;
	uint value[3] ;
} PLOAM_RAW_Sleep_Request_T ;

#endif /* _PLOAM_RAW_H_ */

