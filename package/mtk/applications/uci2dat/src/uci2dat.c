/*****************************************************************************
 * $File:   uci2dat.c
 *
 * $Author: Hua Shao
 * $Date:   Feb, 2014
 *
 * Boring, Boring, Boring, Boring, Boring........
 *
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <getopt.h>

#include <uci.h>


#ifdef OK
#undef OK
#endif
#define OK (0)

#ifdef NG
#undef NG
#endif
#define NG (-1)

#ifdef SHDBG
#undef SHDBG
#endif
#define SHDBG(...)   printf(__VA_ARGS__)
#define DEVNUM_MAX (4)
#define MBSSID_MAX (4)


#define VFPRINT(fp, e, ...) \
            do {\
                char buffer[128] = {0}; \
                snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
                if (strlen(buffer) > 0) { \
                    SHDBG("%s(),%s=", __FUNCTION__, e->dat_key); \
                    fprintf(fp, "%s", buffer); \
                    SHDBG("%s(),%s=%s, def=\"%s\"\n", __FUNCTION__, \
                        e->dat_key, buffer, e->defvalue?e->defvalue:""); \
                } \
            }while(0)

#define FPRINT(fp, e, v) \
            do {\
                if (v && strlen(v) > 0) { \
                    SHDBG("%s(),%s=", __FUNCTION__, e->dat_key); \
                    fprintf(fp, "%s", v); \
                    SHDBG("%s", v); \
                    SHDBG(", def=\"%s\"\n", e->defvalue?e->defvalue:""); \
                } \
            }while(0)

#define WIFI_UCI_FILE "/etc/config/wireless"

#define PARSE_UCI_OPTION(dst, src) \
    do { \
        src = NULL; \
        src = uci_lookup_option_string(uci_ctx, s, dst.uci_key); \
        if(src) { \
            dst.value = strdup(src); \
            SHDBG("%s(),    %s=%s\n", __FUNCTION__, dst.uci_key, dst.value); \
        } \
    }while(0)


struct _param;

typedef void (*ucihook)(FILE *,struct _param *, const char * devname);

typedef struct  _param
{
    const char *    dat_key;
    const char *    uci_key;
    char       *    value;
    ucihook         hook;
    const char *    defvalue;
} param;


typedef struct __extra_cfg
{
    param cfg;
    struct __extra_cfg * next;
} extra_cfg;

typedef struct _vif
{
    param ssid;
    param authmode;        /* wep, wpa, ... */
    param hidden;          /* Hidden SSID */
    param cipher;          /* ccmp(aes),tkip */
    param key;             /* wpa psk */

    param wepkey[4];       /* wep key, ugly, yep! */

    param auth_server;
    param auth_port;
    param auth_secret;
    param rekeyinteval;
    param preauth;
    param pmkcacheperiod;
} vif;

typedef struct
{
    char    devname[16];
    param * params;
    int     vifnum;
    vif     vifs[MBSSID_MAX];
} wifi_params;

void hooker(FILE * fp, param * p, const char * devname);

/* these are separated from CFG_ELEMENTS because they
   use a different represention structure.
*/
vif VIF =
{
    .ssid               = {NULL, "ssid", NULL, NULL,  NULL},
    .authmode           = {NULL, "encryption", NULL, NULL,  NULL},
    .hidden             = {NULL, "hidden", NULL, NULL,  NULL},
    .cipher             = {NULL, "cipher", NULL, NULL,  NULL},

    /* wpa key, or wep key index */
    .key                = {NULL, "key", NULL, NULL,  NULL},

    /* wep key group */
    .wepkey             = {
        {NULL, "key1", NULL, NULL,  NULL},
        {NULL, "key2", NULL, NULL,  NULL},
        {NULL, "key3", NULL, NULL,  NULL},
        {NULL, "key4", NULL, NULL,  NULL}
    },

    /* wpa & 8021x */
    .auth_server        = {NULL, "auth_server", "0", NULL,  NULL},
    .auth_port          = {NULL, "auth_port", "1812", NULL,  NULL},
    .auth_secret        = {NULL, "auth_secret", NULL, NULL,  NULL},
    .pmkcacheperiod     = {NULL, "pmkcacheperiod", NULL, NULL,  NULL},
    .preauth            = {NULL, "preauth", NULL, NULL,  NULL},
    .rekeyinteval       = {NULL, "rekeyinteval", NULL, NULL,  NULL},
};

param CFG_ELEMENTS[] =
{
    /* Default configurations described in :
           MTK_Wi-Fi_SoftAP_Software_Programmming_Guide_v3.6.pdf
    */
    {"CountryRegion", "region", NULL, hooker,  "1"},
    {"CountryRegionABand", "aregion", NULL, hooker, "7"},
    {"CountryCode", "country", NULL, hooker, NULL},
    {"BssidNum", NULL, NULL, hooker,  "1"},
    {"SSID1", NULL, NULL, hooker,  "OpenWrt"},
    {"SSID2", NULL, NULL, hooker,  NULL},
    {"SSID3", NULL, NULL, hooker,  NULL},
    {"SSID4", NULL, NULL, hooker,  NULL},
    {"WirelessMode", "wifimode", NULL, hooker,  "9"},
    {"TxRate", "txrate", NULL, hooker, "0"},
    {"Channel", "channel", NULL, hooker,  "0"},
    {"BasicRate", "basicrate", NULL, hooker, "15"},
    {"BeaconPeriod", "beacon", NULL, hooker,  "100"},
    {"DtimPeriod", "dtim", NULL, hooker,  "1"},
    {"TxPower", "txpower", NULL, hooker,  "100"},
    {"DisableOLBC", "disolbc", NULL, hooker, "0"},
    {"BGProtection", "bgprotect", NULL, hooker,  "0"},
    {"TxAntenna", "txantenna", NULL, hooker, NULL},
    {"RxAntenna", "rxantenna", NULL, hooker, NULL},
    {"TxPreamble", "txpreamble", NULL, hooker,  "0"},
    {"RTSThreshold", "rtsthres", NULL, hooker,  "2347"},
    {"FragThreshold", "fragthres", NULL, hooker,  "2346"},
    {"TxBurst", "txburst", NULL, hooker,  "1"},
    {"PktAggregate", "pktaggre", NULL, hooker,  "0"},
    {"TurboRate", "turborate", NULL, hooker, "0"},
    {"WmmCapable", "wmm", NULL, hooker, "1"},
    {"APSDCapable", "apsd", NULL, hooker, "1"},
    {"DLSCapable", "dls", NULL, hooker, "0"},
    {"APAifsn", "apaifsn", NULL, hooker, "3;7;1;1"},
    {"APCwmin", "apcwmin", NULL, hooker, "4;4;3;2"},
    {"APCwmax", "apcwmax", NULL, hooker, "6;10;4;3"},
    {"APTxop", "aptxop", NULL, hooker, "0;0;94;47"},
    {"APACM", "apacm", NULL, hooker, "0;0;0;0"},
    {"BSSAifsn", "bssaifsn", NULL, hooker, "3;7;2;2"},
    {"BSSCwmin", "bsscwmin", NULL, hooker, "4;4;3;2"},
    {"BSSCwmax", "bsscwmax", NULL, hooker, "10;10;4;3"},
    {"BSSTxop", "bsstxop", NULL, hooker, "0;0;94;47"},
    {"BSSACM", "bssacm", NULL, hooker, "0;0;0;0"},
    {"AckPolicy", "ackpolicy", NULL, hooker, "0;0;0;0"},
    {"NoForwarding", "noforward", NULL, hooker, "0"},
    {"NoForwardingBTNBSSID", NULL, NULL, NULL, "0"},
    {"HideSSID", "hidden", NULL, hooker,  "0"},
    {"StationKeepAlive", NULL, NULL, NULL, "0"},
    {"ShortSlot", "shortslot", NULL, hooker,  "1"},
    {"AutoChannelSelect", "channel", NULL, hooker, "2"},
    {"IEEE8021X", "ieee8021x", NULL, hooker, "0"},
    {"IEEE80211H", "ieee80211h", NULL, hooker, "0"},
    {"CSPeriod", "csperiod", NULL, hooker, "10"},
    {"WirelessEvent", "wirelessevent", NULL, hooker, "0"},
    {"IdsEnable", "idsenable", NULL, hooker, "0"},
    {"AuthFloodThreshold", NULL, NULL, NULL, "32"},
    {"AssocReqFloodThreshold", NULL, NULL, NULL, "32"},
    {"ReassocReqFloodThreshold", NULL, NULL, NULL, "32"},
    {"ProbeReqFloodThreshold", NULL, NULL, NULL, "32"},
    {"DisassocFloodThreshold", NULL, NULL, NULL, "32"},
    {"DeauthFloodThreshold", NULL, NULL, NULL, "32"},
    {"EapReqFooldThreshold", NULL, NULL, NULL, "32"},
    {"PreAuth", "preauth", NULL, hooker, "0"},
    {"AuthMode", NULL, NULL, hooker,  "OPEN"},
    {"EncrypType", NULL, NULL, hooker,  "NONE"},
    {"RekeyInterval", "rekeyinteval", NULL, hooker, "0"},
    {"PMKCachePeriod", "pmkcacheperiod", NULL, hooker, "10"},
    {"WPAPSK1", NULL, NULL, hooker,  NULL},
    {"WPAPSK2", NULL, NULL, hooker,  NULL},
    {"WPAPSK3", NULL, NULL, hooker,  NULL},
    {"WPAPSK4", NULL, NULL, hooker,  NULL},
    {"DefaultKeyID", NULL, NULL, hooker, "1"},
    {"Key1Type", NULL, NULL, hooker, "1"},
    {"Key1Str1", NULL, NULL, hooker, NULL},
    {"Key1Str2", NULL, NULL, hooker, NULL},
    {"Key1Str3", NULL, NULL, hooker, NULL},
    {"Key1Str4", NULL, NULL, hooker, NULL},
    {"Key2Type", NULL, NULL, hooker, "1"},
    {"Key2Str1", NULL, NULL, hooker, NULL},
    {"Key2Str2", NULL, NULL, hooker, NULL},
    {"Key2Str3", NULL, NULL, hooker, NULL},
    {"Key2Str4", NULL, NULL, hooker, NULL},
    {"Key3Type", NULL, NULL, hooker, "1"},
    {"Key3Str1", NULL, NULL, hooker, NULL},
    {"Key3Str2", NULL, NULL, hooker, NULL},
    {"Key3Str3", NULL, NULL, hooker, NULL},
    {"Key3Str4", NULL, NULL, hooker, NULL},
    {"Key4Type", NULL, NULL, hooker, "1"},
    {"Key4Str1", NULL, NULL, hooker, NULL},
    {"Key4Str2", NULL, NULL, hooker, NULL},
    {"Key4Str3", NULL, NULL, hooker, NULL},
    {"Key4Str4", NULL, NULL, hooker, NULL},
    {"AccessPolicy0", NULL, NULL, NULL, "0"},
    {"AccessControlList0", NULL, NULL, NULL, NULL},
    {"AccessPolicy1", NULL, NULL, NULL, "0"},
    {"AccessControlList1", NULL, NULL, NULL, NULL},
    {"AccessPolicy2", NULL, NULL, NULL, "0"},
    {"AccessControlList2", NULL, NULL, NULL, NULL},
    {"AccessPolicy3", NULL, NULL, NULL, "0"},
    {"AccessControlList3", NULL, NULL, NULL, NULL},
    {"WdsEnable", "wds_enable", NULL, hooker, "0"},
    {"WdsEncrypType", "wds_enctype", NULL, hooker, "NONE"},
    {"WdsList", NULL, NULL, NULL, NULL},
    {"Wds0Key", NULL, NULL, NULL, NULL},
    {"Wds1Key", NULL, NULL, NULL, NULL},
    {"Wds2Key", NULL, NULL, NULL, NULL},
    {"Wds3Key", NULL, NULL, NULL, NULL},
    {"RADIUS_Server", "auth_server", NULL, hooker, NULL},
    {"RADIUS_Port", "auth_port", NULL, hooker, NULL},
    {"RADIUS_Key1", "radius_key1", NULL, hooker, NULL},
    {"RADIUS_Key2", "radius_key2", NULL, hooker, NULL},
    {"RADIUS_Key3", "radius_key2", NULL, hooker, NULL},
    {"RADIUS_Key4", "radius_key4", NULL, hooker, NULL},
    {"own_ip_addr", "own_ip_addr", NULL, hooker, "192.168.5.234"},
    {"EAPifname", "eapifname", NULL, hooker, NULL},
    {"PreAuthifname", "preauthifname", NULL, hooker, "br-lan"},
    {"HT_HTC", "ht_htc", NULL, hooker, "0"},
    {"HT_RDG", "ht_rdg", NULL, hooker,  "0"},
    {"HT_EXTCHA", "ht_extcha", NULL, hooker, "0"},
    {"HT_LinkAdapt", "ht_linkadapt", NULL, hooker, "0"},
    {"HT_OpMode", "ht_opmode", NULL, hooker, "0"},
    {"HT_MpduDensity", NULL, NULL, hooker, "5"},
    {"HT_BW", "bw", NULL, hooker,  "0"},
    {"VHT_BW", "bw", NULL, hooker,  "0"},
    {"VHT_Sec80_Channel", "vht2ndchannel", NULL, hooker, NULL},
    {"VHT_SGI", "vht_sgi", NULL, hooker,  "1"},
    {"VHT_STBC", "vht_stbc", NULL, hooker, "0"},
    {"VHT_BW_SIGNAL", "vht_bw_sig", NULL, hooker,  "0"},
    {"VHT_DisallowNonVHT", "vht_disnonvht", NULL, hooker, NULL},
    {"VHT_LDPC", "vht_ldpc", NULL, hooker, "1"},
    {"HT_AutoBA", "ht_autoba", NULL, hooker, "1"},
    {"HT_AMSDU", "ht_amsdu", NULL, hooker, NULL},
    {"HT_BAWinSize", "ht_bawinsize", NULL, hooker, "64"},
    {"HT_GI", "ht_gi", NULL, hooker,  "1"},
    {"HT_MCS", "ht_mcs", NULL, hooker, "33"},
    {"WscManufacturer", "wscmanufacturer", NULL, hooker, NULL},
    {"WscModelName", "wscmodelname", NULL, hooker, NULL},
    {"WscDeviceName", "wscdevicename", NULL, hooker, NULL},
    {"WscModelNumber", "wscmodelnumber", NULL, hooker, NULL},
    {"WscSerialNumber", "wscserialnumber", NULL, hooker, NULL},

    /* Extra configurations found in 76x2e */
    {"FixedTxMode", "fixedtxmode", NULL, hooker, "0"},
    {"AutoProvisionEn", "autoprovision", NULL, hooker, "0"},
    {"FreqDelta", "freqdelta", NULL, hooker, "0"},
    {"CarrierDetect", "carrierdetect", NULL, hooker, "0"},
    {"ITxBfEn", NULL, NULL, hooker, "0"},
    {"PreAntSwitch", "preantswitch", NULL, hooker, "1"},
    {"PhyRateLimit", "phyratelimit", NULL, hooker, "0"},
    {"DebugFlags", "debugflags", NULL, hooker, "0"},
    {"ETxBfEnCond", NULL, NULL, hooker, "0"},
    {"ITxBfTimeout", NULL, NULL, NULL, "0"},
    {"ETxBfNoncompress", NULL, NULL, NULL, "0"},
    {"ETxBfIncapable", NULL, NULL, hooker, "1"},
    {"FineAGC", "fineagc", NULL, hooker, "0"},
    {"StreamMode", "streammode", NULL, hooker, "0"},
    {"StreamModeMac0", NULL, NULL, NULL, NULL},
    {"StreamModeMac1", NULL, NULL, NULL, NULL},
    {"StreamModeMac2", NULL, NULL, NULL, NULL},
    {"StreamModeMac3", NULL, NULL, NULL, NULL},
    {"RDRegion", NULL, NULL, NULL, NULL},
    {"DfsLowerLimit", "dfs_lowlimit", NULL, hooker, "0"},
    {"DfsUpperLimit", "dfs_uplimit", NULL, hooker, "0"},
    {"DfsOutdoor", "dfs_outdoor", NULL, hooker, "0"},
    {"SymRoundFromCfg", NULL, NULL, NULL, "0"},
    {"BusyIdleFromCfg", NULL, NULL, NULL, "0"},
    {"DfsRssiHighFromCfg", NULL, NULL, NULL, "0"},
    {"DfsRssiLowFromCfg", NULL, NULL, NULL, "0"},
    {"DFSParamFromConfig", NULL, NULL, NULL, "0"},
    {"FCCParamCh0", NULL, NULL, NULL, NULL},
    {"FCCParamCh1", NULL, NULL, NULL, NULL},
    {"FCCParamCh2", NULL, NULL, NULL, NULL},
    {"FCCParamCh3", NULL, NULL, NULL, NULL},
    {"CEParamCh0", NULL, NULL, NULL, NULL},
    {"CEParamCh1", NULL, NULL, NULL, NULL},
    {"CEParamCh2", NULL, NULL, NULL, NULL},
    {"CEParamCh3", NULL, NULL, NULL, NULL},
    {"JAPParamCh0", NULL, NULL, NULL, NULL},
    {"JAPParamCh1", NULL, NULL, NULL, NULL},
    {"JAPParamCh2", NULL, NULL, NULL, NULL},
    {"JAPParamCh3", NULL, NULL, NULL, NULL},
    {"JAPW53ParamCh0", NULL, NULL, NULL, NULL},
    {"JAPW53ParamCh1", NULL, NULL, NULL, NULL},
    {"JAPW53ParamCh2", NULL, NULL, NULL, NULL},
    {"JAPW53ParamCh3", NULL, NULL, NULL, NULL},
    {"FixDfsLimit", "fixdfslimit", NULL, hooker, "0"},
    {"LongPulseRadarTh", "lpsradarth", NULL, hooker, "0"},
    {"AvgRssiReq", "avgrssireq", NULL, hooker, "0"},
    {"DFS_R66", "dfs_r66", NULL, hooker, "0"},
    {"BlockCh", "blockch", NULL, hooker, NULL},
    {"GreenAP", "greenap", NULL, hooker, "0"},
    {"WapiPsk1", NULL, NULL, NULL, NULL},
    {"WapiPsk2", NULL, NULL, NULL, NULL},
    {"WapiPsk3", NULL, NULL, NULL, NULL},
    {"WapiPsk4", NULL, NULL, NULL, NULL},
    {"WapiPsk5", NULL, NULL, NULL, NULL},
    {"WapiPsk6", NULL, NULL, NULL, NULL},
    {"WapiPsk7", NULL, NULL, NULL, NULL},
    {"WapiPsk8", NULL, NULL, NULL, NULL},
    {"WapiPskType", NULL, NULL, NULL, NULL},
    {"Wapiifname", NULL, NULL, NULL, NULL},
    {"WapiAsCertPath", NULL, NULL, NULL, NULL},
    {"WapiUserCertPath", NULL, NULL, NULL, NULL},
    {"WapiAsIpAddr", NULL, NULL, NULL, NULL},
    {"WapiAsPort", NULL, NULL, NULL, NULL},
    {"RekeyMethod", "rekeymethod", NULL, hooker, "TIME"},
    {"MeshAutoLink", "mesh_autolink", NULL, hooker, "0"},
    {"MeshAuthMode", "mesh_authmode", NULL, hooker, NULL},
    {"MeshEncrypType", "mesh_enctype", NULL, hooker, NULL},
    {"MeshDefaultkey", "mesh_defkey", NULL, hooker, "0"},
    {"MeshWEPKEY", "mesh_wepkey", NULL, hooker, NULL},
    {"MeshWPAKEY", "mesh_wpakey", NULL, hooker, NULL},
    {"MeshId", "mesh_id", NULL, hooker, NULL},
    {"HSCounter", "hscount", NULL, hooker, "0"},
    {"HT_BADecline", "ht_badec", NULL, hooker, "0"},
    {"HT_STBC", "ht_stbc", NULL, hooker, "0"},
    {"HT_LDPC", "ht_ldpc", NULL, hooker, "1"},
    {"HT_TxStream", "ht_txstream", NULL, hooker, "1"},
    {"HT_RxStream", "ht_rxstream", NULL, hooker, "1"},
    {"HT_PROTECT", "ht_protect", NULL, hooker, "1"},
    {"HT_DisallowTKIP", "ht_distkip", NULL, hooker, "0"},
    {"HT_BSSCoexistence", "ht_bsscoexist", NULL, hooker, "0"},
    {"WscConfMode", "wsc_confmode", NULL, hooker, "0"},
    {"WscConfStatus", "wsc_confstatus", NULL, hooker, "2"},
    {"WCNTest", "wcntest", NULL, hooker, "0"},
    {"WdsPhyMode", "wds_phymode", NULL, hooker, NULL},
    {"RADIUS_Acct_Server", "radius_acctserver", NULL, hooker, NULL},
    {"RADIUS_Acct_Port", "radius_acctport", NULL, hooker, "1813"},
    {"RADIUS_Acct_Key", "radius_acctkey", NULL, hooker, NULL},
    {"Ethifname", "ethifname", NULL, hooker, NULL},
    {"session_timeout_interval", "session_intv", NULL, hooker, "0"},
    {"idle_timeout_interval", "idle_intv", NULL, hooker, "0"},
    {"WiFiTest", NULL, NULL, NULL, "0"},
    {"TGnWifiTest", "tgnwifitest", NULL, hooker, "0"},
    {"ApCliEnable", NULL, NULL, NULL, "0"},
    {"ApCliSsid", NULL, NULL, NULL, NULL},
    {"ApCliBssid", NULL, NULL, NULL, NULL},
    {"ApCliAuthMode", NULL, NULL, NULL, NULL},
    {"ApCliEncrypType", NULL, NULL, NULL, NULL},
    {"ApCliWPAPSK", NULL, NULL, NULL, NULL},
    {"ApCliDefaultKeyID", NULL, NULL, NULL, "0"},
    {"ApCliKey1Type", NULL, NULL, NULL, "0"},
    {"ApCliKey1Str", NULL, NULL, NULL, NULL},
    {"ApCliKey2Type", NULL, NULL, NULL, "0"},
    {"ApCliKey2Str", NULL, NULL, NULL, NULL},
    {"ApCliKey3Type", NULL, NULL, NULL, "0"},
    {"ApCliKey3Str", NULL, NULL, NULL, NULL},
    {"ApCliKey4Type", NULL, NULL, NULL, "0"},
    {"ApCliKey4Str", NULL, NULL, NULL, NULL},
    {"EfuseBufferMode", "efusebufmode", NULL, hooker, "0"},
    {"E2pAccessMode", "e2paccmode", NULL, hooker, "1"},
    {"RadioOn", "radio", NULL, hooker, "1"},
    {"BW_Enable", "bw_enable", NULL, hooker, "0"},
    {"BW_Root", "bw_root", NULL, hooker, "0"},
    {"BW_Priority", "bw_priority", NULL, hooker, NULL},
    {"BW_Guarantee_Rate", "bw_grtrate", NULL, hooker, NULL},
    {"BW_Maximum_Rate", "bw_maxrate", NULL, hooker, NULL},

    /* add more configurations */
    {"AutoChannelSkipList", "autoch_skip", NULL, hooker, NULL},
    {"WscConfMethod", "wsc_confmethod", NULL, hooker, NULL},
    {"WscKeyASCII", "wsc_keyascii", NULL, hooker, NULL},
    {"WscSecurityMode", "wsc_security", NULL, hooker, NULL},
    {"Wsc4digitPinCode", "wsc_4digitpin", NULL, hooker, NULL},
    {"WscVendorPinCode", "wsc_vendorpin", NULL, hooker, NULL},
    {"WscV2Support", "wsc_v2", NULL, hooker, NULL},
    {"HT_MIMOPS", "mimops", NULL, hooker, "3"},
    {"G_BAND_256QAM", "g256qam", NULL, hooker, "1"},
    {"DBDC_MODE", "dbdc", NULL, hooker, "0"},
    {"IgmpSnEnable", "igmpsnoop", NULL, hooker, "1"},
    {"MUTxRxEnable", "mutxrxenable", NULL, hooker, "1"},
    {"ITxBfEnCond", "itxbfencond", NULL, hooker, "0"},
    {"BandSteering", "bandsteering", NULL, hooker, "0"},

    {NULL, "txbf", NULL, hooker, "0"}, // all txbf params use this hook
};

extra_cfg * __extra_cfgs__ = NULL;

static struct uci_context * uci_ctx;
static struct uci_package * uci_wireless;
static wifi_params wifi_cfg[DEVNUM_MAX];


char * __get_value_by_datkey(char * datkey, wifi_params * wifi)
{
    int i;
    for(i=0; i<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); i++)
        if(wifi->params[i].dat_key && 0 == strcmp(datkey, wifi->params[i].dat_key))
            return wifi->params[i].value;
    return NULL;
}


char * __get_value_by_ucikey(char * ucikey, wifi_params * wifi)
{
    int i;
    for(i=0; i<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); i++)
        if(wifi->params[i].uci_key && 0 == strcmp(ucikey, wifi->params[i].uci_key))
            return wifi->params[i].value;
    return NULL;
}


int strmatch(const char * str, const char * pattern)
{
    int i = strlen(str);
    int j = 0;
    if (i != strlen(pattern))
        return -1;

    for(j=0; j<i; j++)
        if(pattern[j] != '?' && pattern[j] != str[j])
            return -1;

    return 0;
}



char * __dump_all(void)
{
    int i, j;
    param * p = NULL;

    for(i=0; i<DEVNUM_MAX; i++)
    {
        if(strlen(wifi_cfg[i].devname) == 0) break;
        printf("%s      %-16s\t%-16s\t%-16s\t%-8s\t%s\n",
               wifi_cfg[i].devname, "[dat-key]", "[uci-key]", "[value]", "[hook]", "[default]");
        for(j=0; j<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); j++)
        {
            p = &wifi_cfg[i].params[j];
            printf("%s %3d. %-16s\t%-16s\t%-16s\t%-8s\t%s\n",
                   wifi_cfg[i].devname, j, p->dat_key, p->uci_key,
                   p->value?p->value:"(null)", p->hook?"Y":"-", p->defvalue);
        }
    }
    return NULL;
}


void parse_dat(char * devname, char * datpath)
{
    FILE * fp = NULL;
    char buffer[1024] = {0};
    char k[512] = {0};
    char v[512] = {0};
    int i = 0, n = 0;
    char * p = NULL;
    char * q = NULL;
    wifi_params * cfg = NULL;

    for (i=0; i<DEVNUM_MAX; i++)
    {
        if(0 == strcmp(devname, wifi_cfg[i].devname))
            cfg = &wifi_cfg[i];
    }

    if(!cfg)
    {
        fprintf(stderr, "%s(), device (%s) not found!\n", __FUNCTION__, devname);
        goto __error;
    }

    fp = fopen(datpath, "rb");
    if(!fp)
    {
        fprintf(stderr, "%s() error: %s!\n", __FUNCTION__, strerror(errno));
        goto __error;
    }

    memset(buffer, 0, sizeof(buffer));
    i=0;
    do
    {
        memset(buffer, 0, sizeof(buffer));
        p = fgets(buffer, sizeof(buffer), fp);
        if(!p) break;

        // skip empty lines
        while(*p == ' '|| *p == '\t') p++;
        if(*p == 0 || *p == '\r' || *p == '\n') continue;
        // skipe lines starts with "#"
        if(*p == '#') continue;

        // cut the \r\n tail!
        q = strchr(buffer, '\n');
        if(q) *q = 0;
        q = strchr(buffer, '\r');
        if(q) *q = 0;


        q = strstr(buffer, "=");
        if(!q) continue; // a valid line should contain "="

        i++;

        *q = 0;
        q++; // split it!
        strncpy(k, p, sizeof(k));
        strncpy(v, q, sizeof(v));

        for ( n=0; n<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); n++)
        {
            if(CFG_ELEMENTS[n].dat_key && 0 == strcmp(CFG_ELEMENTS[n].dat_key, k))
            {
                cfg->params[n].value = strdup(v);
                //printf("%s(), <%s>=<%s>", __FUNCTION__,CFG_ELEMENTS[n].dat_key,cfg->params[n].value);
                break;
            }
        }
        if (n >= sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]))
        {
            fprintf(stderr, "<%s> not supported by uci2dat yet, ignored.\n", k);
            extra_cfg * c = (extra_cfg *)malloc(sizeof(extra_cfg));
            memset(c, 0, sizeof(extra_cfg));
            c->cfg.dat_key = strdup(k);
            c->cfg.uci_key = strdup(k);
            c->cfg.value = strdup(v);
            c->cfg.defvalue = NULL;
            c->cfg.hook = NULL;
            c->next = __extra_cfgs__;
            __extra_cfgs__ = c;
        }
        // else
        //     printf("<%s>=<%s>\n", k, v);

    } while(1);

__error:
    if(fp) fclose(fp);

    return;
}


void parse_uci(char * arg)
{
    struct uci_element *e   = NULL;
    const char * value = NULL;
    int i = 0;
    int cur_dev, cur_vif;

    if (!uci_wireless || !uci_ctx)
    {
        fprintf(stderr, "%s() uci context not ready!\n", __FUNCTION__);
        return;
    }

    /* scan wireless devices ! */
    uci_foreach_element(&uci_wireless->sections, e)
    {
        struct uci_section *s = uci_to_section(e);

        if(0 == strcmp(s->type, "wifi-device"))
        {
            printf("%s(), wifi-device: %s\n", __FUNCTION__, s->e.name);
            for(cur_dev=0; cur_dev<DEVNUM_MAX; cur_dev++)
            {
                if(0 == strcmp(s->e.name, wifi_cfg[cur_dev].devname))
                    break;
            }

            if(cur_dev>=DEVNUM_MAX)
            {
                printf("%s(), device (%s) not found!\n", __FUNCTION__, s->e.name);
                break;
            }

            for( i = 0; i<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); i++)
            {
                if (CFG_ELEMENTS[i].uci_key)
                {
                    value = NULL;
                    value = uci_lookup_option_string(uci_ctx, s, CFG_ELEMENTS[i].uci_key);
                    if(value)
                    {
                        wifi_cfg[cur_dev].params[i].value = strdup(value);
                        printf("%s(),    %s=%s\n", __FUNCTION__, CFG_ELEMENTS[i].uci_key, value);
                    }
                }
            }
        }
    }

    /* scan wireless network interfaces ! */
    uci_foreach_element(&uci_wireless->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        if(0 == strcmp(s->type, "wifi-iface"))
        {
            value = NULL;
            value = uci_lookup_option_string(uci_ctx, s, "device");
            for(cur_dev=0; cur_dev<DEVNUM_MAX; cur_dev++)
            {
                if(0 == strcmp(value, wifi_cfg[cur_dev].devname))
                    break;
            }
            if(cur_dev >= DEVNUM_MAX)
            {
                fprintf(stderr, "%s(), device (%s) not found!\n", __FUNCTION__, value);
                break;
            }
            value = NULL;
            value = uci_lookup_option_string(uci_ctx, s, "ifname");
            printf("%s(), wifi-iface: %s\n", __FUNCTION__, value);

            cur_vif = wifi_cfg[cur_dev].vifnum;

            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].ssid, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].hidden, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].key, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].wepkey[0], value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].wepkey[1], value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].wepkey[2], value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].wepkey[3], value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].auth_server, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].auth_port, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].auth_secret, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].rekeyinteval, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].preauth, value);
            PARSE_UCI_OPTION(wifi_cfg[cur_dev].vifs[cur_vif].pmkcacheperiod, value);

            #define STRNCPPP(dst,src) \
                do {\
                    dst.value = strdup(src);\
                    printf("%s(),    %s=%s\n", __FUNCTION__, dst.uci_key, src); \
                } while(0)

            /* cipher */
            value = uci_lookup_option_string(uci_ctx, s, "encryption");
            if(value)
            {
                const char * p = NULL;
                if (0 == strncmp("8021x", value, strlen("8021x")))
                {
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "8021x");
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "wep");
                }

                if (0 == strncmp("none", value, strlen("none")))
                {
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "none");
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "NONE");
                }
                else if (0 == strncmp("wep-open", value, strlen("wep-open")))
                {
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wep-open");
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "wep");
                }
                else if (0 == strncmp("wep-shared", value, strlen("wep-shared")))
                {
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wep-shared");
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "wep");
                }
                else if (0 == strncmp("mixed-psk", value, strlen("mixed-psk")))
                {
                    STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "psk-mixed");
                    p = value+strlen("mixed-psk");
                    if (*p == '+' && *(p+1) != 0)
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, p+1);
                    else
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "tkip+ccmp");
                }
                else if(0 == strncmp("psk", value, strlen("psk")))
                {
                    if (0 == strncmp("psk-mixed", value, strlen("psk-mixed")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "psk-mixed");
                        p = value+strlen("psk-mixed");
                    }
                    else if (0 == strncmp("psk+psk2", value, strlen("psk+psk2")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "psk-mixed");
                        p = value+strlen("psk+psk2");
                    }
                    else if (0 == strncmp("psk2", value, strlen("psk2")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "psk2");
                        p = value+strlen("psk2");
                    }
                    else if (0 == strncmp("psk", value, strlen("psk")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "psk");
                        p = value+strlen("psk");
                    }

                    if (*p == '+' && *(p+1) != 0)
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, p+1);
                    else
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "tkip+ccmp");
                }
                else if(0 == strncmp("wpa", value, strlen("wpa")))
                {
                    if (0 == strncmp("wpa-mixed", value, strlen("wpa-mixed")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wpa-mixed");
                        p = value+strlen("wpa-mixed");
                    }
                    else if (0 == strncmp("wpa+wpa2", value, strlen("wpa+wpa2")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wpa-mixed");
                        p = value+strlen("wpa+wpa2");
                    }
                    else if (0 == strncmp("wpa2", value, strlen("wpa2")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wpa2");
                        p = value+strlen("wpa2");
                    }
                    else if (0 == strncmp("wpa", value, strlen("wpa")))
                    {
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].authmode, "wpa");
                        p = value+strlen("wpa");
                    }

                    if (*p == '+' && *(p+1) != 0)
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, p+1);
                    else
                        STRNCPPP(wifi_cfg[cur_dev].vifs[cur_vif].cipher, "tkip+ccmp");
                }

            }
            wifi_cfg[cur_dev].vifnum++;
        }
    }
    return;
}


void hooker(FILE * fp, param * p, const char * devname)
{
    int N = 0;
    int i = 0;

    if (!uci_wireless || !uci_ctx)
    {
        fprintf(stderr, "%s() uci context not ready!\n", __FUNCTION__);
        return;
    }

    for(N=0; N<MBSSID_MAX; N++)
    {
        if(0 == strcmp(devname, wifi_cfg[N].devname))
            break;
    }
    if(N >= MBSSID_MAX)
    {
        fprintf(stderr, "%s() device (%s) not found!\n", __FUNCTION__, devname);
        return;
    }

    if(0 == strmatch(p->dat_key, "SSID?"))
    {
        i = atoi(p->dat_key+4)-1;
        if(i<0 || i >= MBSSID_MAX)
        {
            fprintf(stderr, "%s() array index error, L%d\n", __FUNCTION__, __LINE__);
            return;
        }
        FPRINT(fp, p, wifi_cfg[N].vifs[i].ssid.value);
    }
    else if(0 == strcmp(p->dat_key, "BssidNum"))
    {
        VFPRINT(fp, p, "%d", wifi_cfg[N].vifnum);
    }
    else if(0 == strcmp(p->dat_key, "EncrypType"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");

            if (!wifi_cfg[N].vifs[i].authmode.value) continue;
            if (!wifi_cfg[N].vifs[i].cipher.value) continue;

            if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "none"))
                FPRINT(fp, p, "NONE");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-open")
                     || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-shared")
                     || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "8021x"))
                FPRINT(fp, p, "WEP");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].cipher.value, "ccmp"))
                FPRINT(fp, p, "AES");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].cipher.value, "tkip"))
                FPRINT(fp, p, "TKIP");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].cipher.value, "ccmp+tkip")
                     || 0 == strcasecmp(wifi_cfg[N].vifs[i].cipher.value, "tkip+ccmp"))
                FPRINT(fp, p, "TKIPAES");
            else
                FPRINT(fp, p, "NONE");
        }
    }
    else if(0 == strcmp(p->dat_key, "AuthMode"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            if (!wifi_cfg[N].vifs[i].authmode.value) continue;

            if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "none"))
                FPRINT(fp, p, "OPEN");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-open"))
                FPRINT(fp, p, "OPEN");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "8021x"))
                FPRINT(fp, p, "OPEN");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-shared"))
                FPRINT(fp, p, "SHARED");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-auto"))
                FPRINT(fp, p, "WEPAUTO");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk"))
                FPRINT(fp, p, "WPAPSK");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk2"))
                FPRINT(fp, p, "WPA2PSK");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk-mixed")
                     || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk+psk2"))
                FPRINT(fp, p, "WPAPSKWPA2PSK");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wpa"))
                FPRINT(fp, p, "WPA");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wpa2"))
                FPRINT(fp, p, "WPA2");
            else if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wpa-mixed")
                     || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wpa+wpa2"))
                FPRINT(fp, p, "WPA1WPA2");
            else
                FPRINT(fp, p, "OPEN");
        }

    }
    else if(0 == strcmp(p->dat_key, "HideSSID"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if (i>0) FPRINT(fp, p, ";");
            if (!wifi_cfg[N].vifs[i].hidden.value)
                FPRINT(fp, p, "0");
            else
                FPRINT(fp, p, wifi_cfg[N].vifs[i].hidden.value);
        }
    }
    else if(0 == strcmp(p->dat_key, "Channel"))
    {
        if(0 == strcmp(p->value, "auto"))
            FPRINT(fp, p, "0");
        else
            FPRINT(fp, p, p->value);
    }
    else if(0 == strcmp(p->dat_key, "AutoChannelSelect"))
    {
        if(0 == strcmp(p->value, "0"))
            FPRINT(fp, p, "2");
        else
            FPRINT(fp, p, "0");
    }
    else if(0 == strcmp(p->dat_key, "HT_BW"))
    {
        if(0 == strcmp(p->value, "0"))
            FPRINT(fp, p, "0");
        else
            FPRINT(fp, p, "1");
    }
    else if(0 == strcmp(p->dat_key, "VHT_BW"))
    {
        if(0 == strcmp(p->value, "2"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(p->value, "3"))
            FPRINT(fp, p, "2");
        else if (0 == strcmp(p->value, "4"))
            FPRINT(fp, p, "3");
        else
            FPRINT(fp, p, "0");
    }
    else if(0 == strmatch(p->dat_key, "WPAPSK?"))//(0 == strncmp(p->dat_key, "WPAPSK", 6))
    {
        i = atoi(p->dat_key+6)-1;
        if(i<0 || i >= MBSSID_MAX)
        {
            fprintf(stderr, "%s() array index error, L%d\n", __FUNCTION__, __LINE__);
            return;
        }
        if (!wifi_cfg[N].vifs[i].authmode.value) return;
        if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk")
            || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk2")
            || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk+psk2")
            || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "psk-mixed"))
            FPRINT(fp, p, wifi_cfg[N].vifs[i].key.value);
    }
    else if(0 == strcmp(p->dat_key, "RADIUS_Server"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            FPRINT(fp, p, wifi_cfg[N].vifs[i].auth_server.value);
        }
    }
    else if(0 == strcmp(p->dat_key, "RADIUS_Port"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            FPRINT(fp, p, wifi_cfg[N].vifs[i].auth_port.value);
        }
    }
    else if(0 == strmatch(p->dat_key, "RADIUS_Key?"))//(0 == strncmp(p->dat_key, "WPAPSK", 6))
    {
        i = atoi(p->dat_key+10)-1;
        if(i<0 || i >= MBSSID_MAX)
        {
            fprintf(stderr, "%s() array index error, L%d\n", __FUNCTION__, __LINE__);
            return;
        }
        FPRINT(fp, p, wifi_cfg[N].vifs[i].auth_secret.value);
    }
    else if(0 == strcmp(p->dat_key, "PreAuth"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            FPRINT(fp, p, wifi_cfg[N].vifs[i].preauth.value);
        }
    }
    else if(0 == strcmp(p->dat_key, "RekeyInterval"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            FPRINT(fp, p, wifi_cfg[N].vifs[i].rekeyinteval.value);
        }
    }
    else if(0 == strcmp(p->dat_key, "PMKCachePeriod"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            FPRINT(fp, p, wifi_cfg[N].vifs[i].pmkcacheperiod.value);
        }
    }

    else if(0 == strcmp(p->dat_key, "IEEE8021X"))
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if(i>0) FPRINT(fp, p, ";");
            if (!wifi_cfg[N].vifs[i].authmode.value) continue;
            if(0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "8021x"))
                FPRINT(fp, p, "1");
        }
    }
    else if(0 == strmatch(p->dat_key, "DefaultKeyID"))  /* WEP */
    {
        for(i=0; i<wifi_cfg[N].vifnum; i++)
        {
            if (!wifi_cfg[N].vifs[i].authmode.value) continue;
            if (0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-open")
                || 0 == strcasecmp(wifi_cfg[N].vifs[i].authmode.value, "wep-shared"))
            {
                FPRINT(fp, p, wifi_cfg[N].vifs[i].key.value);
                FPRINT(fp, p, ";");
            }
            else
                FPRINT(fp, p, "1;");//default value
        }
    }
    else if(0 == strmatch(p->dat_key, "Key?Type"))  /* WEP */
    {
        int j;
        i = atoi(p->dat_key+3)-1;
        for(j=0; j<wifi_cfg[N].vifnum; j++)
        {
            if (!wifi_cfg[N].vifs[j].wepkey[i].value)
            {
                FPRINT(fp, p, "1;");
                continue;
            }
            if(0 == strncmp("s:", wifi_cfg[N].vifs[j].wepkey[i].value, 2))
                strcpy(wifi_cfg[N].vifs[j].wepkey[i].value,wifi_cfg[N].vifs[j].wepkey[i].value+2);
            if( 5 == strlen(wifi_cfg[N].vifs[j].wepkey[i].value) || 13 == strlen(wifi_cfg[N].vifs[j].wepkey[i].value))
                FPRINT(fp, p, "1;");//wep key type is asic
            else
                FPRINT(fp, p, "0;");//wep key type is hex
        }
    }
    else if(0 == strmatch(p->dat_key, "Key?Str?"))  /* WEP */
    {
        int j;
        i = atoi(p->dat_key+3)-1;
        j = atoi(p->dat_key+7)-1;
        if (!wifi_cfg[N].vifs[j].wepkey[i].value) return;
        if(0 == strncmp("s:", wifi_cfg[N].vifs[j].wepkey[i].value, 2))
            FPRINT(fp, p, wifi_cfg[N].vifs[j].wepkey[i].value+2);
        else
            FPRINT(fp, p, wifi_cfg[N].vifs[j].wepkey[i].value);
    }
    else if (0 == strcmp(p->dat_key, "ETxBfIncapable"))
    {
        char * value =  __get_value_by_ucikey("txbf", &wifi_cfg[N]);
        if (!value)
            FPRINT(fp, p, "0");
        else if (0 == strcmp(value, "3"))
            FPRINT(fp, p, "0");
        else if (0 == strcmp(value, "2"))
            FPRINT(fp, p, "0");
        else if (0 == strcmp(value, "1"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "0"))
            FPRINT(fp, p, "1");
        else
            FPRINT(fp, p, "0");
    }
    else if (0 == strcmp(p->dat_key, "ITxBfEn"))
    {
        char * value =  __get_value_by_ucikey("txbf", &wifi_cfg[N]);
        if (!value)
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "3"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "2"))
            FPRINT(fp, p, "0");
        else if (0 == strcmp(value, "1"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "0"))
            FPRINT(fp, p, "0");
        else
            FPRINT(fp, p, "1");
    }
    else if (0 == strcmp(p->dat_key, "ETxBfEnCond"))
    {
        char * value =  __get_value_by_ucikey("txbf", &wifi_cfg[N]);
        if (!value)
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "3"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "2"))
            FPRINT(fp, p, "1");
        else if (0 == strcmp(value, "1"))
            FPRINT(fp, p, "0");
        else if (0 == strcmp(value, "0"))
            FPRINT(fp, p, "0");
        else
            FPRINT(fp, p, "1");
    }
    /* the rest part is quite simple! */
    else
    {
        FPRINT(fp, p, p->value);
    }

}


void gen_dat(char * devname, char * datpath)
{
    FILE       * fp = NULL;
    char buffer[1024] = {0};
    int           i = 0;
    param       * p = NULL;
    wifi_params * cfg = NULL;
    extra_cfg   * e = __extra_cfgs__;

    for (i=0; i<DEVNUM_MAX; i++)
    {
        if(0 == strcmp(devname, wifi_cfg[i].devname))
            cfg = &wifi_cfg[i];
    }
    if(!cfg)
    {
        fprintf(stderr, "%s(), device (%s) not found!\n", __FUNCTION__, devname);
        return;
    }

    if (datpath)
    {
        fp = fopen(datpath, "wb");
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "mkdir -p /etc/wireless/%s", cfg->devname);
        system(buffer);

        snprintf(buffer, sizeof(buffer), "/etc/wireless/%s/%s.dat", cfg->devname, cfg->devname);
        fp = fopen(buffer, "wb");
    }

    if(!fp)
    {
        fprintf(stderr, "Failed to open %s, %s!\n", buffer, strerror(errno));
        return;
    }

    fprintf(fp, "# Generated by uci2dat\n");
    fprintf(fp, "# The word of \"Default\" must not be removed\n");
    fprintf(fp, "Default\n");


    for(i=0; i<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); i++)
    {
        p = &cfg->params[i];
        if (!p->dat_key) continue;
        printf("%s(), %s\n", __FUNCTION__, p->dat_key);
        /* either get value from dat or uci */
        if(p->value)
        {
            fprintf(fp, "%s=", p->dat_key);
            if(p->hook)
                p->hook(fp, p, cfg->devname);
            else if(strlen(p->value) > 0)
                fprintf(fp, p->value);
            /*
            else if(p->defvalue)
                fprintf(fp, p->defvalue);
            */
            fprintf(fp, "\n");
        }
    }

    while (e)
    {
        fprintf(fp, "%s=", e->cfg.dat_key);
        if(strlen(e->cfg.value) > 0)
            fprintf(fp, e->cfg.value);
        else if(e->cfg.defvalue)
            fprintf(fp, e->cfg.defvalue);
        fprintf(fp, "\n");
        e = e->next;
    }

    fclose(fp);

    return;
}

void init_wifi_cfg(void)
{
    struct uci_element *e   = NULL;
    int i,j;

    for(i=0; i<DEVNUM_MAX; i++)
    {
        memset(&wifi_cfg[i], 0, sizeof(wifi_params));
        wifi_cfg[i].params = (param *)malloc(sizeof(CFG_ELEMENTS));
        memcpy(wifi_cfg[i].params, CFG_ELEMENTS, sizeof(CFG_ELEMENTS));

        for(j=0; j<MBSSID_MAX; j++)
            memcpy(&wifi_cfg[i].vifs[j], &VIF, sizeof(VIF));
    }

    uci_foreach_element(&uci_wireless->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        if(0 == strcmp(s->type, "wifi-device"))
        {
            for(i=0; i<DEVNUM_MAX; i++)
            {
                if(0 == strlen(wifi_cfg[i].devname))
                {
                    strncpy(wifi_cfg[i].devname, s->e.name, sizeof(wifi_cfg[i].devname));
                    break;
                }
            }

            if(i>=DEVNUM_MAX)
            {
                fprintf(stderr, "%s(), too many devices!\n", __FUNCTION__);
                break;
            }
        }
    }
}

void usage(void)
{
    int i, j;
    param * p = NULL;

    printf("uci2dat  -- a tool to translate uci config (/etc/config/wireless)\n");
    printf("            into ralink driver dat.\n");
    printf("\nUsage:\n");
    printf("    uci2dat -d <dev-name> -f <dat-path>\n");

    printf("\nArguments:\n");
    printf("    -d <dev-name>   device name, mt7620, eg.\n");
    printf("    -f <dat-path>   dat file path.\n");

    printf("\nSupported keywords:\n");
    printf("     %-16s\t%-16s\t%s\n", "[uci-key]", "[dat-key]", "[default]");
    for(i=0, j=0; i<sizeof(CFG_ELEMENTS)/sizeof(CFG_ELEMENTS[0]); i++)
    {
        p = &CFG_ELEMENTS[i];
        if(p->uci_key)
        {
            printf("%3d. %-16s\t%-16s\t%s\n",j, p->uci_key, p->dat_key, p->defvalue);
            j++;
        }
    }

    printf("%2d. %s\n", j++, VIF.ssid.uci_key);
    printf("%2d. %s\n", j++, VIF.authmode.uci_key);
    printf("%2d. %s\n", j++, VIF.hidden.uci_key);
    printf("%2d. %s\n", j++, VIF.cipher.uci_key);
    printf("%2d. %s\n", j++, VIF.key.uci_key);

}


int main(int argc, char ** argv)
{
    int opt = 0;
    char * dev = NULL;
    char * dat = NULL;
    int test = 0;

    while ((opt = getopt (argc, argv, "htf:d:")) != -1)
    {
        switch (opt)
        {
            case 'f':
                dat = optarg;
                break;
            case 'd':
                dev = optarg;
                break;
            case 't':
                test = 1;
                break;
            case 'h':
                usage();
                return OK;
            default:
                usage();
                return NG;
        }
    }

    if (!uci_ctx)
    {
        uci_ctx = uci_alloc_context();
    }
    else
    {
        uci_wireless = uci_lookup_package(uci_ctx, "wireless");
        if (uci_wireless)
            uci_unload(uci_ctx, uci_wireless);
    }

    if (uci_load(uci_ctx, "wireless", &uci_wireless))
    {
        return NG;
    }

    init_wifi_cfg();

    if(dev && dat)
    {
        parse_dat(dev, dat);
        parse_uci(NULL);
    }

    if(test)
    {
        FILE * fp = NULL;
        char * p = NULL;
        char device[32] = {0};
        char profile[128] = {0};
        fp = popen("cat /etc/config/wireless"
                   " | grep \"wifi-device\""
                   " | sed  -n \"s/config[ \t]*wifi-device[\t]*//gp\"",
                   "r");
        if(!fp)
        {
            fprintf(stderr, "%s(), error L%d\n", __FUNCTION__, __LINE__);
            return NG;
        }
        while(fgets(device, sizeof(device), fp))
        {
            if (strlen(device) > 0)
            {
                p = device+strlen(device)-1;
                while(*p < ' ')
                {
                    *p=0;    // trim newline
                    p++;
                }
                snprintf(profile, sizeof(profile), "/etc/wireless/%s/%s.dat", device, device);
                parse_dat(device, profile);
            }
            else
                printf("%s(), error L%d\n", __FUNCTION__, __LINE__);
        }
        pclose(fp);
        parse_uci(NULL);
        __dump_all();

    }
    else if(dev && dat)
        gen_dat(dev, dat);
    else
        usage();

    uci_unload(uci_ctx, uci_wireless);
    return OK;
}




