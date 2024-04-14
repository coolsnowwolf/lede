//https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/dataservices/tree/rmnetctl
#include <sys/socket.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <linux/gen_stats.h>
#include <net/if.h>
#include <asm/types.h>
#include <linux/rmnet_data.h>

#define RMNETCTL_SUCCESS 0
#define RMNETCTL_LIB_ERR 1
#define RMNETCTL_KERNEL_ERR 2
#define RMNETCTL_INVALID_ARG 3

enum rmnetctl_error_codes_e {
	RMNETCTL_API_SUCCESS = 0,

	RMNETCTL_API_FIRST_ERR = 1,
	RMNETCTL_API_ERR_MESSAGE_SEND = 3,
	RMNETCTL_API_ERR_MESSAGE_RECEIVE = 4,

	RMNETCTL_INIT_FIRST_ERR = 5,
	RMNETCTL_INIT_ERR_PROCESS_ID = RMNETCTL_INIT_FIRST_ERR,
	RMNETCTL_INIT_ERR_NETLINK_FD = 6,
	RMNETCTL_INIT_ERR_BIND = 7,

	RMNETCTL_API_SECOND_ERR = 9,
	RMNETCTL_API_ERR_HNDL_INVALID = RMNETCTL_API_SECOND_ERR,
	RMNETCTL_API_ERR_RETURN_TYPE = 13,
};

struct rmnetctl_hndl_s {
	 uint32_t pid;
	 uint32_t transaction_id;
	 int netlink_fd;
	 struct sockaddr_nl src_addr, dest_addr;
};
typedef struct rmnetctl_hndl_s rmnetctl_hndl_t;

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((char *)(nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

struct nlmsg {
	struct nlmsghdr nl_addr;
	struct ifinfomsg ifmsg;
	char data[500];
};

#define MIN_VALID_PROCESS_ID 0
#define MIN_VALID_SOCKET_FD 0
#define KERNEL_PROCESS_ID 0
#define UNICAST 0

enum {
	IFLA_RMNET_UL_AGG_PARAMS = __IFLA_RMNET_MAX,
	__IFLA_RMNET_EXT_MAX,
};

struct rmnet_egress_agg_params {
	uint16_t agg_size;
	uint16_t agg_count;
	uint32_t agg_time;
};

static int rmnet_get_ack(rmnetctl_hndl_t *hndl, uint16_t *error_code)
{
	struct nlack {
		struct nlmsghdr ackheader;
		struct nlmsgerr ackdata;
		char   data[256];

	} ack;
	int i;

	if (!hndl || !error_code)
		return RMNETCTL_INVALID_ARG;

	if ((i = recv(hndl->netlink_fd, &ack, sizeof(ack), 0)) < 0) {
		*error_code = errno;
		return RMNETCTL_API_ERR_MESSAGE_RECEIVE;
	}

	/*Ack should always be NLMSG_ERROR type*/
	if (ack.ackheader.nlmsg_type == NLMSG_ERROR) {
		if (ack.ackdata.error == 0) {
			*error_code = RMNETCTL_API_SUCCESS;
			return RMNETCTL_SUCCESS;
		} else {
			*error_code = -ack.ackdata.error;
			return RMNETCTL_KERNEL_ERR;
		}
	}

	*error_code = RMNETCTL_API_ERR_RETURN_TYPE;
	return RMNETCTL_API_FIRST_ERR;
}

static int rtrmnet_ctl_init(rmnetctl_hndl_t **hndl, uint16_t *error_code)
{
	struct sockaddr_nl __attribute__((__may_alias__)) *saddr_ptr;
	int netlink_fd = -1;
	pid_t pid = 0;

	if (!hndl || !error_code)
		return RMNETCTL_INVALID_ARG;

	*hndl = (rmnetctl_hndl_t *)malloc(sizeof(rmnetctl_hndl_t));
	if (!*hndl) {
		*error_code = RMNETCTL_API_ERR_HNDL_INVALID;
		return RMNETCTL_LIB_ERR;
	}

	memset(*hndl, 0, sizeof(rmnetctl_hndl_t));

	pid = getpid();
	if (pid  < MIN_VALID_PROCESS_ID) {
		free(*hndl);
		*error_code = RMNETCTL_INIT_ERR_PROCESS_ID;
		return RMNETCTL_LIB_ERR;
	}
	(*hndl)->pid = KERNEL_PROCESS_ID;
	netlink_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (netlink_fd < MIN_VALID_SOCKET_FD) {
		free(*hndl);
		*error_code = RMNETCTL_INIT_ERR_NETLINK_FD;
		return RMNETCTL_LIB_ERR;
	}

	(*hndl)->netlink_fd = netlink_fd;

	memset(&(*hndl)->src_addr, 0, sizeof(struct sockaddr_nl));

	(*hndl)->src_addr.nl_family = AF_NETLINK;
	(*hndl)->src_addr.nl_pid = (*hndl)->pid;

	saddr_ptr = &(*hndl)->src_addr;
	if (bind((*hndl)->netlink_fd,
		(struct sockaddr *)saddr_ptr,
		sizeof(struct sockaddr_nl)) < 0) {
		close((*hndl)->netlink_fd);
		free(*hndl);
		*error_code = RMNETCTL_INIT_ERR_BIND;
		return RMNETCTL_LIB_ERR;
	}

	memset(&(*hndl)->dest_addr, 0, sizeof(struct sockaddr_nl));

	(*hndl)->dest_addr.nl_family = AF_NETLINK;
	(*hndl)->dest_addr.nl_pid = KERNEL_PROCESS_ID;
	(*hndl)->dest_addr.nl_groups = UNICAST;

	return RMNETCTL_SUCCESS;
}

static int rtrmnet_ctl_deinit(rmnetctl_hndl_t *hndl)
{
	if (!hndl)
		return RMNETCTL_SUCCESS;

	close(hndl->netlink_fd);
	free(hndl);

	return RMNETCTL_SUCCESS;
}

static int rtrmnet_ctl_newvnd(rmnetctl_hndl_t *hndl, char *devname, char *vndname,
		       uint16_t *error_code, uint8_t  index,
		       uint32_t flagconfig, uint32_t ul_agg_cnt, uint32_t ul_agg_size)
{
	struct rtattr *attrinfo, *datainfo, *linkinfo;
	struct ifla_vlan_flags flags;
	int devindex = 0, val = 0;
	char *kind = "rmnet";
	struct nlmsg req;
	short id;

	if (!hndl || !devname || !vndname || !error_code)
		return RMNETCTL_INVALID_ARG;

	memset(&req, 0, sizeof(req));
	req.nl_addr.nlmsg_type = RTM_NEWLINK;
	req.nl_addr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.nl_addr.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL |
				  NLM_F_ACK;
	req.nl_addr.nlmsg_seq = hndl->transaction_id;
	hndl->transaction_id++;

	/* Get index of devname*/
	devindex = if_nametoindex(devname);
	if (devindex < 0) {
		*error_code = errno;
		return RMNETCTL_KERNEL_ERR;
	}

	/* Setup link attr with devindex as data */
	val = devindex;
	attrinfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	attrinfo->rta_type = IFLA_LINK;
	attrinfo->rta_len = RTA_ALIGN(RTA_LENGTH(sizeof(val)));
	memcpy(RTA_DATA(attrinfo), &val, sizeof(val));
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(sizeof(val)));

	/* Set up IFLA info kind  RMNET that has linkinfo and type */
	attrinfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	attrinfo->rta_type =  IFLA_IFNAME;
	attrinfo->rta_len = RTA_ALIGN(RTA_LENGTH(strlen(vndname) + 1));
	memcpy(RTA_DATA(attrinfo), vndname, strlen(vndname) + 1);
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(strlen(vndname) + 1));

	linkinfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	linkinfo->rta_type = IFLA_LINKINFO;
	linkinfo->rta_len = RTA_ALIGN(RTA_LENGTH(0));
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(0));

	attrinfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	attrinfo->rta_type =  IFLA_INFO_KIND;
	attrinfo->rta_len = RTA_ALIGN(RTA_LENGTH(strlen(kind)));
	memcpy(RTA_DATA(attrinfo), kind, strlen(kind));
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(strlen(kind)));

	datainfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	datainfo->rta_type =  IFLA_INFO_DATA;
	datainfo->rta_len = RTA_ALIGN(RTA_LENGTH(0));
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(0));

	id = index;
	attrinfo = (struct rtattr *)(((char *)&req) +
				     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
	attrinfo->rta_type =  IFLA_VLAN_ID;
	attrinfo->rta_len = RTA_LENGTH(sizeof(id));
	memcpy(RTA_DATA(attrinfo), &id, sizeof(id));
	req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
				RTA_ALIGN(RTA_LENGTH(sizeof(id)));

	if (flagconfig != 0) {
		flags.mask  = flagconfig;
		flags.flags = flagconfig;

		attrinfo = (struct rtattr *)(((char *)&req) +
					     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
		attrinfo->rta_type =  IFLA_VLAN_FLAGS;
		attrinfo->rta_len = RTA_LENGTH(sizeof(flags));
		memcpy(RTA_DATA(attrinfo), &flags, sizeof(flags));
		req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
					RTA_ALIGN(RTA_LENGTH(sizeof(flags)));
	}

	if (ul_agg_cnt > 1) {
            struct rmnet_egress_agg_params agg_params;

            agg_params.agg_size = ul_agg_size;
            agg_params.agg_count = ul_agg_cnt;
            agg_params.agg_time = 3000000;

		attrinfo = (struct rtattr *)(((char *)&req) +
					     NLMSG_ALIGN(req.nl_addr.nlmsg_len));
		attrinfo->rta_type =  IFLA_RMNET_UL_AGG_PARAMS;
		attrinfo->rta_len = RTA_LENGTH(sizeof(agg_params));
		memcpy(RTA_DATA(attrinfo), &agg_params, sizeof(agg_params));
		req.nl_addr.nlmsg_len = NLMSG_ALIGN(req.nl_addr.nlmsg_len) +
					RTA_ALIGN(RTA_LENGTH(sizeof(agg_params)));
	}

	datainfo->rta_len = (char *)NLMSG_TAIL(&req.nl_addr) - (char *)datainfo;

	linkinfo->rta_len = (char *)NLMSG_TAIL(&req.nl_addr) - (char *)linkinfo;

	if (send(hndl->netlink_fd, &req, req.nl_addr.nlmsg_len, 0) < 0) {
		*error_code = RMNETCTL_API_ERR_MESSAGE_SEND;
		return RMNETCTL_LIB_ERR;
	}

	return rmnet_get_ack(hndl, error_code);
}

int rtrmnet_ctl_create_vnd(char *devname, char *vndname, uint8_t muxid,
		       uint32_t qmap_version, uint32_t ul_agg_cnt, uint32_t ul_agg_size)
{
    struct rmnetctl_hndl_s *handle;
    uint16_t error_code;
    int return_code;
    uint32_t flagconfig = RMNET_FLAGS_INGRESS_DEAGGREGATION;

    printf("%s devname: %s, vndname: %s, muxid: %d, qmap_version: %d\n",
        __func__, devname, vndname, muxid, qmap_version);

    ul_agg_cnt = 0; //TODO

    if (ul_agg_cnt > 1)
        flagconfig |= RMNET_EGRESS_FORMAT_AGGREGATION;

    if (qmap_version == 9) { //QMAPV5
#ifdef RMNET_FLAGS_INGRESS_MAP_CKSUMV5
        flagconfig |= RMNET_FLAGS_INGRESS_MAP_CKSUMV5;
        flagconfig |= RMNET_FLAGS_EGRESS_MAP_CKSUMV5;
#else
        return -1001;
#endif
    }
    else if (qmap_version == 8) { //QMAPV4
        flagconfig |= RMNET_FLAGS_INGRESS_MAP_CKSUMV4;
        flagconfig |= RMNET_FLAGS_EGRESS_MAP_CKSUMV4;
    }
    else if (qmap_version == 5) { //QMAPV1
    }
    else {
        flagconfig = 0;
    }
    
    return_code = rtrmnet_ctl_init(&handle, &error_code);
    if (return_code) {
        printf("rtrmnet_ctl_init error_code: %d, return_code: %d, errno: %d (%s)\n",
            error_code, return_code, errno, strerror(errno));
    }
    if (return_code == RMNETCTL_SUCCESS) {
        return_code = rtrmnet_ctl_newvnd(handle, devname, vndname, &error_code,
            muxid, flagconfig, ul_agg_cnt, ul_agg_size);
            if (return_code) {
                printf("rtrmnet_ctl_newvnd error_code: %d, return_code: %d, errno: %d (%s)\n",
                    error_code, return_code, errno, strerror(errno));
            }
        rtrmnet_ctl_deinit(handle);
    }

    return return_code;
}
