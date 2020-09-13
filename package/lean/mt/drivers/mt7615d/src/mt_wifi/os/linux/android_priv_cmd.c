
#define RTMP_MODULE_OS

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT

#define ANDROID_CMD_P2P_DEV_ADDR		"P2P_DEV_ADDR"
#define ANDROID_CMD_SET_AP_WPS_P2P_IE	"SET_AP_WPS_P2P_IE"

typedef struct android_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_priv_cmd;

static INT priv_cmd_get_p2p_dev_addr(PNET_DEV net_dev, PCHAR command, INT total_len)
{
	VOID *pAdSrc;

	pAdSrc = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	ASSERT(pAdSrc);
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	INT bytes_written = 0;

	COPY_MAC_ADDR(command, pAd->cfg80211_ctrl.P2PCurrentAddress, MAC_ADDR_LEN);
	bytes_written = MAC_ADDR_LEN;
	return bytes_written;
}

static INT priv_cmd_set_ap_wps_p2p_ie(PNET_DEV net_dev, PCHAR buf, INT len, INT type)
{
	/* 0x1: BEACON */
	/* 0x2: PROBE_RESP */
	/* 0x4: ASSOC_RESP */
	INT ret = 0;
	return ret;
}

int rt_android_private_command_entry(
	VOID *pAdSrc, PNET_DEV net_dev, struct ifreq *ifr, INT cmd)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	android_priv_cmd priv_cmd;
	UCHAR *command = NULL;
	INT ret = 0, bytes_written = 0;

	if (!ifr->ifr_data)
		return -EINVAL;

	if (copy_from_user(&priv_cmd, ifr->ifr_data, sizeof(android_wifi_priv_cmd)))
		return -EFAULT;

	os_alloc_mem(NULL, (UCHAR **)&command, priv_cmd.total_len);

	if (!command) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: MEM ALLOC ERROR\n", __func__));
		return -ENOMEM;
	}

	if (copy_from_user(command, priv_cmd.buf, priv_cmd.total_len)) {
		ret = -EFAULT;
		goto FREE;
	}

	if (strnicmp(command, ANDROID_CMD_SET_AP_WPS_P2P_IE, strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE)) == 0) {
		int skip = strlen(ANDROID_CMD_SET_AP_WPS_P2P_IE) + 3;

		bytes_written = priv_cmd_set_ap_wps_p2p_ie(net_dev, command + skip,
						priv_cmd.total_len - skip, *(command + skip - 2) - '0');
	} else if (strnicmp(command, ANDROID_CMD_P2P_DEV_ADDR, strlen(ANDROID_CMD_P2P_DEV_ADDR)) == 0)
		bytes_written = priv_cmd_get_p2p_dev_addr(net_dev, command, priv_cmd.total_len);
	else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unsupport priv_cmd !!!\n", command));
		snprintf(command, 3, "OK");
		bytes_written = strlen("OK");
	}

RESPONSE:

	if (bytes_written >= 0) {
		/* priv_cmd in but no response */
		if ((bytes_written == 0) && (priv_cmd.total_len > 0))
			command[0] = '\0';

		if (bytes_written >= priv_cmd.total_len)
			bytes_written = priv_cmd.total_len;
		else
			bytes_written++;

		priv_cmd.used_len = bytes_written;

		if (copy_to_user(priv_cmd.buf, command, bytes_written))
			ret = -EFAULT;
	} else
		ret = bytes_written;

FREE:

	if (command)
		os_free_mem(command);

	return ret;
}
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */
