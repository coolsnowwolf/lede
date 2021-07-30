#include <fcntl.h>
#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define _BYTE_ORDER _BIG_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define _BYTE_ORDER _LITTLE_ENDIAN
#else
#error "__BYTE_ORDER undefined"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "qcsapi_output.h"
#include "./libqcsapi_client/qcsapi_rpc_common/client/find_host_addr.h"

#include "qcsapi.h"
#include "./libqcsapi_client/qcsapi_rpc/client/qcsapi_rpc_client.h"
#include "./libqcsapi_client/qcsapi_rpc/generated/qcsapi_rpc.h"
#include "./libqcsapi_client/qcsapi_rpc_common/common/rpc_pci.h"

#include "qcsapi_driver.h"
#include "call_qcsapi.h"

#include "iwinfo.h"
#include "iwinfo/utils.h"

#define MAX_RETRY_TIMES 5
#define WIFINAME "wifi0"

enum qtnawifi_client {
	QTNAWIFI_USE_TCP_CLIENT = 0,
	QTNAWIFI_USE_UDP_CLIENT,
	QTNAWIFI_USE_PCIE_CLIENT,
	QTNAWIFI_UNKNOW_CLIENT
};

static int qtnawifi_use_client = QTNAWIFI_USE_TCP_CLIENT;

static CLIENT *clnt;

int qtnawifi_get_pcie_client(void)
{
	int retry = 0;
	char *host;
	
	host = "localhost";
	while (retry++ < MAX_RETRY_TIMES) {
		clnt = clnt_pci_create(host, QCSAPI_PROG, QCSAPI_VERS, NULL);
		if (clnt == NULL) {
			clnt_pcreateerror(host);
			sleep(1);
			continue;
		} else
			client_qcsapi_set_rpcclient(clnt);
	}
	
	/* could not find host or create a client, exit */
	if (retry >= MAX_RETRY_TIMES)
		return -1;

	return 0;
}

int qtnawifi_get_rpc_client(void)
{
	int retry = 0;
	
	const char *host="1.1.1.2";

	/* setup RPC based on udp protocol */
	while (retry++ < MAX_RETRY_TIMES) {
		if (qtnawifi_use_client == QTNAWIFI_USE_TCP_CLIENT ) {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "tcp");
		} else {
			clnt = clnt_create(host, QCSAPI_PROG, QCSAPI_VERS, "udp");
		}
		if (clnt == NULL) {
			clnt_pcreateerror(host);
			sleep(1);
			continue;
		} else {
			client_qcsapi_set_rpcclient(clnt);
			break;
		}
	}

	/* could not find host or create a client, exit */
	if (retry >= MAX_RETRY_TIMES)
		return -1;

	return 0;
}

static char * qtnawifi_isvap(const char *ifname, const char *wifiname)
{

	return 0;
}

#define ADDR_LEN 18
static int qtnawifi_iswifi(const char *ifname)
{
	return 0;
}


static int qtnawifi_set_ipaddr(const char *ifname, const char *ip)
{
	struct ifreq ifr;
	struct sockaddr_in *addr;
	int fd = 0;
	int ret =-1;

	strcpy(ifr.ifr_name, ifname);

	if((fd = socket(AF_INET, SOCK_STREAM, 0))<0){
		return -1;
	}
 
	addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(ip);

	ret = ioctl(fd, SIOCSIFADDR, &ifr);
	close(fd);
 
	if(ret < 0)
		return -1;
 
	return 0;
}

static int qtnawifi_host_detect(const char *ifname)
{
        int skfd = 0;
		int ret = 0;
		
        struct ifreq ifr;

		if(strncmp(ifname, "host", 4) != 0)
			return 0;
		
        skfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(skfd < 0) 
			goto err;

        strcpy(ifr.ifr_name, ifname);

        if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0 ) 
			goto err;

        if(ifr.ifr_flags & IFF_UP) {
			ret = 1;
			goto err;
        } 
err:
	close(skfd);
	return ret;
}

int qtnawifi_probe(const char *ifname)
{
	int ret;
	if(qtnawifi_host_detect(ifname))
	{
		/* FIXME: unknow error if use PCIE */
		if(qtnawifi_use_client == QTNAWIFI_USE_PCIE_CLIENT) {
			ret = qtnawifi_get_pcie_client();
		} else {
			ret = qtnawifi_get_rpc_client();
		}
		
		if(!ret)
			return 1;
	}
	
	return 0;
}

void qtnawifi_close(void)
{
	if(clnt)
		clnt_destroy(clnt);
	clnt = NULL;
	/* Nop */
}

int qtnawifi_get_mode(const char *ifname, int *buf)
{
	int ret;
	
	qcsapi_wifi_mode	 current_wifi_mode;
	
	ret = qcsapi_wifi_get_mode(WIFINAME, &current_wifi_mode);
	
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_mode error, return: %d\n", ret);
		return -1;
	}
	
	switch (current_wifi_mode)
	{
	  case qcsapi_mode_not_defined:
		*buf = IWINFO_OPMODE_UNKNOWN;
		break;

	  case qcsapi_access_point:
		*buf = IWINFO_OPMODE_MASTER;
		break;

	  case qcsapi_station:
		*buf = IWINFO_OPMODE_CLIENT;
		break;

	  case qcsapi_nosuch_mode:
	  default:
		*buf = IWINFO_OPMODE_UNKNOWN;
		break;
	}
	
	/* Current support AP Mode only */
// 	*buf = IWINFO_OPMODE_WDS;
// 	*buf = IWINFO_OPMODE_UNKNOWN;

    return 0;
}

int qtnawifi_get_ssid(const char *ifname, char *buf)
{
	int ret;
	ret = qcsapi_wifi_get_SSID(WIFINAME, (qcsapi_SSID *)buf);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_SSID error, return: %d\n", ret);
		return -1;
	}
	return 0;
}

int qtnawifi_get_bssid(const char *ifname, char *buf)
{
	int ret;
	uint8_t	bssid[MAC_ADDR_SIZE];
	
	ret = qcsapi_wifi_get_BSSID(WIFINAME, (qcsapi_mac_addr *)bssid);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_SSID error, return: %d\n", ret);
		return -1;
	}
	
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
			(uint8_t)bssid[0], (uint8_t)bssid[1],
			(uint8_t)bssid[2], (uint8_t)bssid[3],
			(uint8_t)bssid[4], (uint8_t)bssid[5]);
	return 0;
}

static int qtnawifi_freq2channel(int freq)
{
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq > 58319 && freq < 64801)
		return (freq - 56160) / 2160;
	else
		return (freq - 5000) / 5;
}

static int qtnawifi_channel2freq(int channel, const char *band)
{
	if (!band || band[0] != 'a')
	{
		if (channel == 14)
			return 2484;
		else if (channel < 14)
			return (channel * 5) + 2407;
	}
	else if ((channel < 5) && (band[0] == 'a' && band[1] == 'd'))
	{
		return (channel * 2160) + 56160;
	}
	else
	{
		if (channel >= 182 && channel <= 196)
			return (channel * 5) + 4000;
		else
			return (channel * 5) + 5000;
	}

	return 0;
}

int qtnawifi_get_channel(const char *ifname, int *buf)
{
	int ret;
	
	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	ret = qcsapi_wifi_get_channel(WIFINAME, (qcsapi_unsigned_int *)buf);
	if (ret < 0) {
		printf("Qcsapi qtnawifi_get_channel error, return: %d\n", ret);
		return -1;
	}
	return 0;
}

int qtnawifi_get_frequency(const char *ifname, int *buf)
{
	int ret;
	unsigned int channel;
	char *hwmode="ac";

	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	ret = qcsapi_wifi_get_channel(WIFINAME, (qcsapi_unsigned_int *)&channel);
	if (ret < 0) {
		printf("Qcsapi qtnawifi_get_channel error, return: %d\n", ret);
		return -1;
	}
	
	*buf = qtnawifi_channel2freq(channel, hwmode);
	
	return 0;
}

int qtnawifi_get_txpower(const char *ifname, int *buf)
{
	int ret;
	int txpower;

	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	unsigned int channel=149; /* use default channel 149 */
	
	ret = qcsapi_wifi_get_tx_power(WIFINAME, channel, &txpower);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_tx_power error, return: %d\n", ret);
		return -1;
	}
	
	*buf = txpower;
	
	return 0;
}

int qtnawifi_get_bitrate(const char *ifname, int *buf)
{
	*buf= 1733 * 1000;
	
	return 0;
}

int qtnawifi_get_signal(const char *ifname, int *buf)
{
	int ret;
	int rssi;

	if(!qtnawifi_host_detect(ifname))
		return -1;
	ret = qcsapi_wifi_get_rssi_by_chain(WIFINAME, 0, &rssi);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_rssi_by_chain error, return: %d\n", ret);
		return -1;
	}
	*buf = rssi;
	return 0;
}

int qtnawifi_get_noise(const char *ifname, int *buf)
{
	int ret;
	int noise;
	
	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	ret = qcsapi_wifi_get_noise(WIFINAME, &noise);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_noise error, return: %d\n", ret);
		return -1;
	}
	*buf = noise;
	return 0;
}

int qtnawifi_get_quality(const char *ifname, int *buf)
{
	*buf = 90;
	return 0;
}

int qtnawifi_get_quality_max(const char *ifname, int *buf)
{
	*buf = 100;
	return 0;
}

int qtnawifi_get_encryption(const char *ifname, char *buf)
{
	int ret;
	uint32_t wsec, wauth, wpa;
	char	beacon_type[16];
	char	encryption_modes[36];
	char	authentication_mode[36];

	struct iwinfo_crypto_entry *c = (struct iwinfo_crypto_entry *)buf;
	
	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	ret = qcsapi_wifi_get_beacon_type(WIFINAME, &beacon_type[0]);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_beacon_type error, return: %d\n", ret);
		return -1;
	}
	
	ret = qcsapi_wifi_get_WPA_encryption_modes(WIFINAME, &encryption_modes[0]);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_WPA_encryption_modes error, return: %d\n", ret);
		return -1;
	}
	
	ret = qcsapi_wifi_get_IEEE11i_authentication_mode(WIFINAME, &authentication_mode[0]);
	if (ret < 0) {
		printf("Qcsapi qcsapi_wifi_get_WPA_encryption_modes error, return: %d\n", ret);
		return -1;
	}
	
	c->enabled = 0;
	c->wpa_version = 0;
	
    if(strstr(beacon_type, "Basic"))
	{
        c->auth_algs |= IWINFO_AUTH_OPEN;
        c->auth_suites |= IWINFO_KMGMT_NONE;
	}
	
    if(strstr(beacon_type, "WPA") ) {
		c->enabled = 1;
		c->wpa_version |= 0x1;
    }
    
    if(strstr(beacon_type, "11i") ) {
		c->enabled = 1;
		c->wpa_version |= 0x2;
    }
    
    if(strstr(encryption_modes, "AESEncryption") ) {
		 c->pair_ciphers |= IWINFO_CIPHER_CCMP;
    }
    
    if(strstr(encryption_modes, "TKIPEncryption") ) {
		 c->pair_ciphers |= IWINFO_CIPHER_TKIP;
    }
    c -> group_ciphers = c -> pair_ciphers;
	
    if(strstr(authentication_mode, "PSKAuthentication") ) {
		c->auth_suites |= IWINFO_KMGMT_PSK;
    }
    
    if(strstr(authentication_mode, "EAPAuthentication") ) {
		c->auth_suites |= IWINFO_KMGMT_8021x;
    }
	
	return 0;
}

int qtnawifi_get_phyname(const char *ifname, char *buf)
{
    if((strncmp(ifname,"host",4) == 0))
        strcpy(buf, ifname);
    return 0;
}

#define IP_ADDR_STR_LEN 16
int qtnawifi_rssi2signal(int signal)
{
		if (signal < -110)
			signal = -110;
		else if (signal > -40)
			signal = -40;

	return (signal + 110);
}

int qtnawifi_get_assoclist(const char *ifname, char *buf, int *len)
{
	int ret;
	int i;
	int bl = 0;
	unsigned int association_count = 0;
	
	uint8_t		mac_addr[ETHER_ADDR_LEN];
	
	char		ip_addr[IP_ADDR_STR_LEN + 1];
	
	unsigned int 	link_quality;
	
	int	 rssi,noise;
	u_int64_t		 rx_bytes, tx_bytes;
	u_int32_t		 rx_packets, tx_packets;
	
	unsigned int 	 bw;
	
	unsigned int 	 tx_rate, rx_rate;
	unsigned int 	 tx_mcs, rx_mcs;
	unsigned int 	 time_associated = 0;
	
	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	ret = qcsapi_wifi_get_count_associations(WIFINAME, &association_count);
	if (ret < 0) {
		return -1;
	}
	
	if(association_count < 0 || association_count > 256)
		return -1;
	
	for(i = 0; i < association_count; i++) {
		int association_index = i;
		struct iwinfo_assoclist_entry entry;
		memset(&entry, 0, sizeof(entry)); 
		
		ret = qcsapi_wifi_get_associated_device_mac_addr(WIFINAME, association_index, mac_addr);
		//ret = qcsapi_wifi_get_associated_device_ip_addr(WIFINAME, association_index, ip_addr);
		ret = qcsapi_wifi_get_link_quality(WIFINAME, association_index, &link_quality);
		ret = qcsapi_wifi_get_rssi_in_dbm_per_association(WIFINAME, association_index, &rssi);
		ret = qcsapi_wifi_get_hw_noise_per_association(WIFINAME, association_index, &noise);
		ret = qcsapi_wifi_get_rx_bytes_per_association(WIFINAME, association_index, &rx_bytes);
		ret = qcsapi_wifi_get_tx_bytes_per_association(WIFINAME, association_index, &tx_bytes);
		ret = qcsapi_wifi_get_rx_packets_per_association(WIFINAME, association_index, &rx_packets);
		ret = qcsapi_wifi_get_tx_packets_per_association(WIFINAME, association_index, &tx_packets);
		ret = qcsapi_wifi_get_bw_per_association(WIFINAME, association_index, &bw);
		ret = qcsapi_wifi_get_tx_phy_rate_per_association(WIFINAME, association_index, &tx_rate);
		ret = qcsapi_wifi_get_rx_phy_rate_per_association(WIFINAME, association_index, &rx_rate);
		ret = qcsapi_wifi_get_tx_mcs_per_association(WIFINAME,association_index, &tx_mcs);
		ret = qcsapi_wifi_get_rx_mcs_per_association(WIFINAME,association_index, &rx_mcs);
		ret = qcsapi_wifi_get_time_associated_per_association(WIFINAME, association_index, &time_associated);
		
        entry.signal = rssi;
        entry.noise  = noise;
        entry.inactive  = time_associated * 1000;
        memcpy(&entry.mac, mac_addr, sizeof(entry.mac));
        entry.tx_packets = tx_packets;
        entry.rx_packets = rx_packets;
        entry.rx_bytes = rx_packets;
        entry.tx_bytes = tx_packets;
        entry.tx_retries = 0;
		
		if(bw <= 40)
			entry.tx_rate.is_40mhz = 1;
		if(bw > 40)
			entry.tx_rate.is_vht = 1;
		entry.tx_rate.rate = tx_rate * 1000;
		entry.tx_rate.mcs = tx_mcs;
		entry.tx_rate.mhz = bw;
		entry.tx_rate.nss = 0;
		entry.tx_rate.is_short_gi = 0;
		
		if(bw <= 40)
			entry.rx_rate.is_40mhz = 1;
		if(bw > 40)
			entry.rx_rate.is_vht = 1;
		entry.rx_rate.rate = rx_rate * 1000;
		entry.rx_rate.mcs = rx_mcs;
		entry.rx_rate.mhz = bw;
		entry.rx_rate.nss = 0;
		entry.rx_rate.is_short_gi = 0;
		
        memcpy(&buf[bl], &entry, sizeof(struct iwinfo_assoclist_entry));

        bl += sizeof(struct iwinfo_assoclist_entry);
	}
	
    *len = bl;
	return 0;
}

int qtnawifi_get_txpwrlist(const char *ifname, char *buf, int *len)
{
	struct iwinfo_txpwrlist_entry entry;
	uint8_t dbm[11] = { 0, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24 };
	uint8_t mw[11]  = { 1, 3, 6, 10, 15, 25, 39, 63, 100, 158, 251 };
	int i;

	for (i = 0; i < 11; i++)
	{
		entry.dbm = dbm[i];
		entry.mw  = mw[i];
		memcpy(&buf[i*sizeof(entry)], &entry, sizeof(entry));
	}

	*len = 11 * sizeof(entry);
	return 0;
}

int qtnawifi_get_scanlist(const char *ifname, char *buf, int *len)
{
	int ret;
	char *res;
	DIR *proc;
	struct dirent *e;

	ret = -1;

	return ret;
}

int qtnawifi_get_freqlist(const char *ifname, char *buf, int *len)
{
	int i, bl;
	
	struct iwinfo_freqlist_entry qtnawifi_channel_entry[] = {
			{.mhz=5180, .channel=36},
			{.mhz=5200, .channel=40},
			{.mhz=5220, .channel=44},
			{.mhz=5240, .channel=48},
			{.mhz=5260, .channel=52},
			{.mhz=5280, .channel=56},
			{.mhz=5300, .channel=60},
			{.mhz=5320, .channel=64},
			{.mhz=5500, .channel=100},
			{.mhz=5520, .channel=104},
			{.mhz=5540, .channel=108},
			{.mhz=5560, .channel=112},
			{.mhz=5580, .channel=116},
			{.mhz=5600, .channel=120},
			{.mhz=5620, .channel=124},
			{.mhz=5640, .channel=128},
			{.mhz=5660, .channel=132},
			{.mhz=5680, .channel=136},
			{.mhz=5700, .channel=140},
			{.mhz=5745, .channel=149},
			{.mhz=5765, .channel=153},
			{.mhz=5785, .channel=157},
			{.mhz=5805, .channel=161},
			{.mhz=5825, .channel=165}
	};
	
	if(!qtnawifi_host_detect(ifname))
		return -1;
	
	bl = 0;
	
	for(i = 0; i < 	ARRAY_SIZE(qtnawifi_channel_entry); i++)
	{

		memcpy(&buf[bl], &qtnawifi_channel_entry[i], sizeof(struct iwinfo_freqlist_entry));
		bl += sizeof(struct iwinfo_freqlist_entry);
	}

	*len = bl;
	return 0;
}

int qtnawifi_get_country(const char *ifname, char *buf)
{
	return -1;
}

int qtnawifi_get_countrylist(const char *ifname, char *buf, int *len)
{
	return -1;
}

int qtnawifi_get_hwmodelist(const char *ifname, int *buf)
{
	*buf |= IWINFO_80211_A;
	*buf |= IWINFO_80211_N;
	*buf |= IWINFO_80211_AC;
	
	return 0;
}

#define QCA_HTMODE_ADD(MODESTR, MODEVAL) if(strstr(prot, MODESTR)) *buf |= MODEVAL;
int qtnawifi_get_htmodelist(const char *ifname, int *buf)
{
	
	*buf |= IWINFO_HTMODE_VHT20 | IWINFO_HTMODE_VHT40 |
				IWINFO_HTMODE_VHT80;
	return 0;
}

int qtnawifi_get_mbssid_support(const char *ifname, int *buf)
{
    *buf = 0;
    return 0;
}

int qtnawifi_get_hardware_id(const char *ifname, char *buf)
{
	return -1;
}

static const struct iwinfo_hardware_entry *
qtnawifi_get_hardware_entry(const char *ifname)
{
	struct iwinfo_hardware_id id;

	if (qtnawifi_get_hardware_id(ifname, (char *)&id))
		return NULL;

	return iwinfo_hardware(&id);
}

int qtnawifi_get_hardware_name(const char *ifname, char *buf)
{
	const struct iwinfo_hardware_entry *hw;

	if (!(hw = qtnawifi_get_hardware_entry(ifname)))
		sprintf(buf, "Quantenna");
	else
		sprintf(buf, "%s %s", hw->vendor_name, hw->device_name);

	return 0;
}

int qtnawifi_get_txpower_offset(const char *ifname, int *buf)
{
	return -1;
}

int qtnawifi_get_frequency_offset(const char *ifname, int *buf)
{
	return -1;
}

const struct iwinfo_ops qtnawifi_ops = {
	.name             = "qtnawifi",
	.probe            = qtnawifi_probe,
	.channel          = qtnawifi_get_channel,
	.frequency        = qtnawifi_get_frequency,
	.frequency_offset = qtnawifi_get_frequency_offset,
	.txpower          = qtnawifi_get_txpower,
	.txpower_offset   = qtnawifi_get_txpower_offset,
	.bitrate          = qtnawifi_get_bitrate,
	.signal           = qtnawifi_get_signal,
	.noise            = qtnawifi_get_noise,
	.quality          = qtnawifi_get_quality,
	.quality_max      = qtnawifi_get_quality_max,
	.mbssid_support   = qtnawifi_get_mbssid_support,
	.hwmodelist       = qtnawifi_get_hwmodelist,
	.htmodelist       = qtnawifi_get_htmodelist,
	.mode             = qtnawifi_get_mode,
	.ssid             = qtnawifi_get_ssid,
	.bssid            = qtnawifi_get_bssid,
	.country          = qtnawifi_get_country,
	.hardware_id      = qtnawifi_get_hardware_id,
	.hardware_name    = qtnawifi_get_hardware_name,
	.encryption       = qtnawifi_get_encryption,
	.phyname          = qtnawifi_get_phyname,
	.assoclist        = qtnawifi_get_assoclist,
	.txpwrlist        = qtnawifi_get_txpwrlist,
	.scanlist         = qtnawifi_get_scanlist,
	.freqlist         = qtnawifi_get_freqlist,
	.countrylist      = qtnawifi_get_countrylist,
	.close            = qtnawifi_close
};
