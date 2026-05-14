#ifndef _RTL8373_TYPES_H_
#define _RTL8373_TYPES_H_

//#include <stdio.h>

typedef unsigned long long      rtk_uint64;
typedef long long               rtk_int64;
typedef unsigned int            rtk_uint32;
typedef int                     rtk_int32;
typedef unsigned short          rtk_uint16;
typedef short                   rtk_int16;
typedef unsigned char           rtk_uint8;
typedef char                    rtk_int8;

#define CONST_T     const

#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1

#define RTK_MAX_NUM_OF_PORT                         10
#define RTK_PORT_ID_MAX                             (RTK_MAX_NUM_OF_PORT-1)
#define RTK_PHY_ID_MAX                              (RTK_MAX_NUM_OF_PORT-4)
#define RTK_MAX_PORT_MASK                           0x3FF
#define RTK_MAX_NUM_OF_PROTO_TYPE                   (0xFFFF)

#define RTK_WHOLE_SYSTEM                            0xFF

typedef struct rtk_portmask_s
{
    rtk_uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef enum rtk_valid_e
{
    INVALID = 0,
    VALID,
    RTK_VALID_END
} rtk_valid_t;


#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN      6
#endif

/* ethernet address type */
typedef struct  rtk_mac_s
{
    rtk_uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;

typedef rtk_uint32  rtk_pri_t;      /* priority vlaue */
typedef rtk_uint32  rtk_qid_t;      /* queue id type */
typedef rtk_uint32  rtk_data_t;
typedef rtk_uint32  rtk_dscp_t;     /* dscp vlaue */
typedef rtk_uint32  rtk_fid_t;      /* filter id type */
typedef rtk_uint32  rtk_vlan_t;     /* vlan id type */
typedef rtk_uint32  rtk_mac_cnt_t;  /* MAC count type  */
typedef rtk_uint32  rtk_meter_id_t; /* meter id type  */
typedef rtk_uint32  rtk_rate_t;     /* rate type  */

typedef enum rtk_port_e
{
    UTP_PORT0 = 0,
    UTP_PORT1,
    UTP_PORT2,
    UTP_PORT3,
    UTP_PORT4,
    UTP_PORT5,
    UTP_PORT6,
    UTP_PORT7,
    UTP_PORT8,
    UTP_PORT9,
    
    UNDEFINE_PORT = 30,
    RTK_PORT_MAX = 31
} rtk_port_t;


#ifndef _RTL_TYPES_H

#if 0
typedef unsigned long long      uint64;
typedef long long               int64;
typedef unsigned int            uint32;
typedef int                     int32;
typedef unsigned short          uint16;
typedef short                   int16;
typedef unsigned char           uint8;
typedef char                    int8;
#endif

typedef rtk_uint32                  ipaddr_t;
typedef rtk_uint32                  memaddr;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN      6
#endif

typedef struct ether_addr_s {
    rtk_uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#ifdef __KERNEL__
#include <linux/kernel.h>
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT           rtlglue_printf
#endif /*_RTL_TYPES_H*/

/* type abstraction */
#ifdef EMBEDDED_SUPPORT

typedef rtk_int16                   rtk_api_ret_t;
typedef rtk_int16                   ret_t;
typedef rtk_uint32                  rtk_u_long;

#else

typedef rtk_int32                   rtk_api_ret_t;
typedef rtk_int32                   ret_t;
typedef rtk_uint64                  rtk_u_long_t;

#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CONST           const
#endif /* _RTL8373_TYPES_H_ */
