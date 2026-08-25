#ifndef	__UAPI_INFO_IOCTL_H__
#define	__UAPI_INFO_IOCTL_H__

#define INFO_U_IOC_MAGIC		'i'
#define CMD_VPN_ADD_IP			_IOR(INFO_U_IOC_MAGIC, 0, unsigned long)
#define CMD_VPN_GET_COUNT		_IOR(INFO_U_IOC_MAGIC, 1, unsigned long)
#define CMD_VPN_CLEAR_IPS		_IOR(INFO_U_IOC_MAGIC, 2, unsigned long)
#define CMD_OB_INFO_RESTART		_IOR(INFO_U_IOC_MAGIC, 20, unsigned long)
#define CMD_OB_INFO_GET_CNT		_IOR(INFO_U_IOC_MAGIC, 21, unsigned long)
#define CMD_OB_INFO_GET_DATA	_IOR(INFO_U_IOC_MAGIC, 22, unsigned long)



#define Info_DEVNAME          	"vpn_info"
#define Info_MAJOR             	(249)
#define IP_TYPE					(1)
#define DOMAIN_TYPE 			(2)
#define MAX_IP_ADDR_SIZE		(32)


typedef struct _info_ioctl_data_s
{
	char ip[32];
	int vpn_entry_idx;
	/*domain_ip_type=1 represents ips_entry, domain_ip_type=2 represents domain_entry*/
	int domain_ip_type;
	int domain_ip_idx;
	int ip_mask;
	unsigned long long count;
}info_ioctl_data;

typedef struct _info_ioctl_return_data_s
{
	unsigned int vpn_entry_idx;
	unsigned long long *payload;
}info_ioctl_return_data;

#define D_TYPE_IP_IP6	0
#define D_TYPE_PPP		1

typedef struct _ob_monitor_data_s
{
	char ipaddr[40];
	unsigned char mac[6];
	unsigned short type; /* 0: IP/IPv6, 1: PPP */
	unsigned int mark;
}ob_monitor_data;

typedef struct _info_ioctl_OB_map_data_s
{
	int		wan_idx;
	int		data_len;
	void	*data;
}info_ioctl_OB_map_data;

#endif
