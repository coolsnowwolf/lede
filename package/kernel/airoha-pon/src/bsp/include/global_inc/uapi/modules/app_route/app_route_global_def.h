#ifndef _UAPI_APP_ROUTE_GLOBAL_DEF_H_
#define _UAPI_APP_ROUTE_GLOBAL_DEF_H_

#ifndef __KERNEL__
#include <stdint.h>
#endif

#define APPROUTE_IOC_MAGIC					'r'
#define APPROUTE_CMD_RULE_CLR_ALL			_IOR(APPROUTE_IOC_MAGIC, 0, unsigned long)
#define APPROUTE_CMD_RULE_INIT				_IOR(APPROUTE_IOC_MAGIC, 1, unsigned long)
#define APPROUTE_CMD_RULE_ADD_LIST			_IOR(APPROUTE_IOC_MAGIC, 2, unsigned long)
#define APPROUTE_CMD_RULE_CLR_ALL_WHOLE		_IOR(APPROUTE_IOC_MAGIC, 3, unsigned long)
#define APPROUTE_CMD_RULE_UPDATE_WAN		_IOR(APPROUTE_IOC_MAGIC, 4, unsigned long)
#define APPROUTE_CMD_RULE_IF_PREFIX			_IOR(APPROUTE_IOC_MAGIC, 5, unsigned long)

#define APPROUTE_MAX_MAP 				200

#define USE_BITS_CNT					32
#define MAX_32BIT_CNT					7

#define SET_MARK						1
#define CLR_MARK						0

#define APP_ROUTE_MODULE_NAME         	"app_route"
#define APP_ROUTE_MAJOR             	(245)
#define APP_ROUTE_DEV_NAME				"/dev/"APP_ROUTE_MODULE_NAME

#define	MAX_SAVE_SIZE	1000
#define MAX_ATTR_SIZE	1024

#if defined(TCSUPPORT_MULTI_USER_ITF)
#define SKB_MARK_LAN_MASK				0xF8000000
#else
#define SKB_MARK_LAN_MASK				0xF0000000
#endif
#define SKB_MARK_WAN_MASK				0x007F0000


#define FILE_APP_ROUTE_DATA_FOR_DOMAIN		"/opt/upt/apps/info/app_route_domain_data_%d"
#define FILE_APP_ROUTE_DATA_FOR_IP			"/opt/upt/apps/info/app_route_ip_data_%d"

#define BYTE_6_MASK1		(0x00010000)
#define BYTE_6_MASK2		(0x00020000)
#define BYTE_6_MASK3		(0x00040000)

#define IP_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1 | BYTE_6_MASK2 | BYTE_6_MASK3) )
#define MAC_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1 | BYTE_6_MASK2) )
#define DM_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1) )

#define IP_BYE_6_FUN_MASK		( (BYTE_6_MASK1 | BYTE_6_MASK2 | BYTE_6_MASK3) )
#define MAC_BYE_6_FUN_MASK		( (BYTE_6_MASK1 | BYTE_6_MASK2))
#define DM_BYE_6_FUN_MASK		( (BYTE_6_MASK1))

/* for IP and MAC node */
#define IS_DYNAMIC_NODE(x)	(ntohl(x->rule_index[6]) & BYTE_6_MASK1)
#define SET_DYNAMIC_NODE(x)	(x->rule_index[6] |= htonl(BYTE_6_MASK1))

/* for IP node */
#define IS_IPONLY_NODE(x)	(ntohl(x->rule_index[6]) & BYTE_6_MASK2)
#define SET_IPONLY_NODE(x)	(x->rule_index[6] |= htonl(BYTE_6_MASK2))
#define RM_IPONLY_NODE(x)	(x->rule_index[6] &= htonl(~(BYTE_6_MASK2)))

#define IS_IPV6_NODE(x)		(ntohl(x->rule_index[6]) & BYTE_6_MASK3)
#define SET_IPV6_NODE(x)	(x->rule_index[6] |= htonl(BYTE_6_MASK3))

/* for MAC node */
#define IS_MACONLY_NODE(x)	(ntohl(x->rule_index[6]) & BYTE_6_MASK2)
#define SET_MACONLY_NODE(x)	(x->rule_index[6] |= htonl(BYTE_6_MASK2))
#define RM_MACONLY_NODE(x)	(x->rule_index[6] &= htonl(~(BYTE_6_MASK2)) )

enum APPROUTE_DB_DATA_TYPE
{
	APPROUTE_BASE_OVER_WRITE_MAC,
	APPROUTE_BASE_OVER_WRITE_IP,
	APPROUTE_BASE_OVER_WRITE_DOMAIN,

	APPROUTE_BASE_APPEND_FEW_IP,	/* IPs must less than 10. */

	APPROUTE_BASE_MAX
};

enum APPROUTE_DOMAIN_IP_MATCH_TYPE
{
	APPROUTE_DM_IP_MAC_MDOE,
	APPROUTE_DM_IP_ONLY_MDOE,

	APPROUTE_DM_MAX
};


#define FOUND_BASEMAC_BUT_EMPTY	3

#define IPV4_LEN				4
#define IPV6_LEN				16
#define MAC_LEN					6
#define	MAX_DM_LEN				255
#define	MAX_DM_BUF_LEN			(MAX_DM_LEN + 1)


#define MAX_MAC_LIST_CNT		32
#define MAX_IP_LIST_CNT			2048
#define MAX_DOMAIN_LIST_CNT		256

#define APP_R_V4				1
#define APP_R_V6				2


/* for ip check mask. */
#define MASK_CHK_32BIT				(1<<0)
#define MASK_CHK_24BIT				(1<<1)
#define MASK_CHK_16BIT				(1<<2)
#define MASK_CHK_8BIT				(1<<3)

#define MASK_CHK_128BIT				(1<<4)
#define MASK_CHK_96BIT				(1<<5)
#define MASK_CHK_80BIT				(1<<6)
#define MASK_CHK_64BIT				(1<<7)



struct approute_ioctl_data
{
	int			rule_index;
	int			data_type;
	int			ifname_idx;
	char		ifname[12];
	int			ifname_state;
	int			dscp_up;
	int			dscp_down;
	int			list_cnt;
	int			data_len;
	void		*data;
	int			data2_len;
	void		*data2;
	uint32_t	bit_mask;
	int			dm_ip_type;
	/* support vpn interface. */
	int			is_vpn_iface;
	char		vpn_if_prefix[32];
};

struct approute_event_rule_update
{
	int rule_index;		/* approute start with 1, wan index start with 0 */
};

/* element data object. */
struct OBJ_M
{
	uint8_t		mac_key[MAC_LEN];
	uint32_t	rule_index[MAX_32BIT_CNT];
};

/* NOT support IP/23 IP/29 etc. */
struct OBJ_I
{
	uint32_t	ip_key[4];
	uint32_t	rule_index[MAX_32BIT_CNT];
};

/* domain rules*/
struct OBJ_D
{
	uint32_t	rule_index[MAX_32BIT_CNT];
};

#define APP_CTL_FRT_TITLE			"%-10s,%5s,%12s,%10s,%40s,%32s,%12s\n"
#define APP_CTL_FRT_ACTION			"%10d,%5d,%12s,%10d,%40s,%32s,%02X%02X%02X%02X%02X%02X\n"

enum APP_CTL_ACTION_TYPE
{
	APP_CTL_ACTION_ROUTE = 0,
	APP_CTL_ACTION_DSCP,
	APP_CTL_ACTION_DROP,

	APP_CTL_ACTION_MAX
};

enum APP_CTL_DIRECTION_TYPE
{
	APP_CTL_UPSTREAM = 0,
	APP_CTL_DOWNSTREAM,

	APP_CTL_MAX
};

struct approute_event_vpn_iface
{
	int			rule_index;
	int			is_wanup;
};

#define		APPROUTE_MATCH_TYPE_IPMAC	0
#define		APPROUTE_MATCH_TYPE_IP		1
#define		APPROUTE_MATCH_TYPE_MAC		2

#define		APPROUTE_APPEND_FEW_CNT		10

#endif
