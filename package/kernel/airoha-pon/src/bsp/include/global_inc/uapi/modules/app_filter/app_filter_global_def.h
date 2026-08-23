#ifndef _UAPI_APP_FILTER_GLOBAL_DEF_H_
#define _UAPI_APP_FILTER_GLOBAL_DEF_H_

#ifndef __KERNEL__
#include <stdint.h>
#endif


#define APPFILTER_IOC_MAGIC						'f'
#define APPFILTER_CMD_RULE_CLR_ALL				_IOR(APPFILTER_IOC_MAGIC, 0, unsigned long)
#define APPFILTER_CMD_RULE_DEL_ONE_IP			_IOR(APPFILTER_IOC_MAGIC, 1, unsigned long)
#define APPFILTER_CMD_RULE_INIT					_IOR(APPFILTER_IOC_MAGIC, 2, unsigned long)
#define APPFILTER_CMD_RULE_ADD_LIST				_IOR(APPFILTER_IOC_MAGIC, 3, unsigned long)
#define APPFILTER_CMD_RULE_ADD_ONE_IP			_IOR(APPFILTER_IOC_MAGIC, 4, unsigned long)
#define APPFILTER_CMD_RULE_CLR_ALL_WHOLE		_IOR(APPFILTER_IOC_MAGIC, 5, unsigned long)
#define APPFILTER_CMD_RULE_RESET_BLOCKEDTIMES	_IOR(APPFILTER_IOC_MAGIC, 6, unsigned long)
#define APPFILTER_CMD_RULE_GET_BLOCKEDTIMES		_IOR(APPFILTER_IOC_MAGIC, 7, unsigned long)
#define APPFILTER_CMD_SET_LANIP					_IOR(APPFILTER_IOC_MAGIC, 8, unsigned long)
#define APPFILTER_CMD_SET_TIMEZONE				_IOR(APPFILTER_IOC_MAGIC, 9, unsigned long)


#define APPFILTER_MAX_MAP 				200

#define USE_BITS_CNT					32
#define MAX_32BIT_CNT					7

#define SET_MARK						1
#define CLR_MARK						0

#define MAX_DAYTIME_PERIOD_CNT			3

#define APP_FILTER_MODULE_NAME         	"app_filter"
#define APP_FILTER_MAJOR             	(246)
#define APP_FILTER_DEV_NAME				"/dev/"APP_FILTER_MODULE_NAME

#define	MAX_SAVE_SIZE	1000
#define MAX_ATTR_SIZE	1024

#define FILE_APP_FILTER_DATA_FOR_DOMAIN		"/opt/upt/apps/info/app_filter_domain_data_%d"
#define FILE_APP_FILTER_DATA_FOR_IP			"/opt/upt/apps/info/app_filter_ip_data_%d"

#define APP_FILTER_SOCK_PATH 				"/tmp/app_filter_sock"

#if 0
#define IS_MACONLY_NODE(x) (ntohl(x->rule_index[6]) & 0x80000000)
#define IS_IPONLY_NODE(x) (ntohl(x->rule_index[6]) & 0x80000000)

#define SET_MACONLY_NODE(x) (x->rule_index[6] |= htonl(0x80000000))
#define SET_IPONLY_NODE(x) (x->rule_index[6] |= htonl(0x80000000))
#endif

#define IP_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1 | BYTE_6_MASK2 | BYTE_6_MASK3) )
#define MAC_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1 | BYTE_6_MASK2) )
#define DM_BYE_6_RULE_MASK		( ~(BYTE_6_MASK1 | BYTE_6_MASK2) )

#define IP_BYE_6_FUN_MASK		( (BYTE_6_MASK1 | BYTE_6_MASK2 | BYTE_6_MASK3) )
#define MAC_BYE_6_FUN_MASK		( (BYTE_6_MASK1 | BYTE_6_MASK2))
#define DM_BYE_6_FUN_MASK		( (BYTE_6_MASK1 | BYTE_6_MASK2))




#define BYTE_6_MASK1		(0x00010000)
#define BYTE_6_MASK2		(0x00020000)
#define BYTE_6_MASK3		(0x00040000)


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

/* for Domain node */
#define IS_DMONLY_NODE(x)	(ntohl(x->rule_index[6]) & BYTE_6_MASK2)
#define SET_DMONLY_NODE(x)	(x->rule_index[6] |= htonl(BYTE_6_MASK2))
#define RM_DMONLY_NODE(x)	(x->rule_index[6] &= htonl(~(BYTE_6_MASK2)))


enum APPFILTER_DB_DATA_TYPE
{
	APPFILTER_BASE_OVER_WRITE_MAC,
	APPFILTER_BASE_OVER_WRITE_IP,
	APPFILTER_BASE_OVER_WRITE_DOMAIN,

	/* for support add in tail. */
	APPFILTER_BASE_APPEND_MAC,
	APPFILTER_BASE_APPEND_IP,

	APPFILTER_BASE_MAX
};

enum APPFILTER_DOMAIN_IP_MATCH_TYPE
{
	APPFILTER_DM_IP_MAC_MDOE,
	APPFILTER_DM_IP_ONLY_MDOE,

	APPFILTER_DM_MAX
};


#define FOUND_BASEMAC_BUT_EMPTY	3

#define IPV4_LEN				4
#define IPV6_LEN				16

#define MAC_LEN					6
#define	MAX_DM_LEN				255

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
#define MASK_CHK_64BIT				(1<<5)


struct appfilter_ioctl_data
{
	int			rule_index;
	int			data_type;
	uint8_t		mode;
	uint8_t		domain_action;
	uint8_t		dip_action;
	uint8_t		weekdays;
	uint32_t	daytime_start[MAX_DAYTIME_PERIOD_CNT];
	uint32_t	daytime_stop[MAX_DAYTIME_PERIOD_CNT];
	uint32_t 	blocked_times;
	uint8_t		signal_enable;
	int			list_cnt;
	int			data_len;
	void		*data;
	int			data2_len;
	void		*data2;
	uint32_t	bit_mask;
	int			dm_ip_type;
	uint32_t	lanip;
	uint32_t	lanip_mask;
	uint8_t		lanip6[16];
	int			tz_hour;
};

struct appfilter_event_rule_update
{
	int rule_index;		/* start with 1, if 0 means all. */
};

#if 1
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
#endif

/*
* for dbus FUN AddAppRouteList/DeleteAppRouteList part process
*/
struct OBJ_PART_DATA
{
	char	file_path[MAX_ATTR_SIZE]; /* GVariant data for file name or ip/mac/domain. */
};


#define APP_CTL_FRT_ACTION			"%10d,%5d,%12s,%10d,%40s,%32s,%02X%02X%02X%02X%02X%02X\n"
#define APP_FILTER_FRT_MSG			"%10d,%d,%s,%s,%02X%02X%02X%02X%02X%02X"

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


#endif
