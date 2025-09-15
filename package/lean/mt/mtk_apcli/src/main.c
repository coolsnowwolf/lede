#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if.h>
#include <ctype.h>
#include <unistd.h>

#include "iwinfo.h" 

#include "wext.h"
#include "mtk.h"

int iwpriv_set(char *ifname, char *option, char *value)
{
    int s;
    size_t len;
    char buf[64];
    struct iwreq wrq;

    snprintf(buf, sizeof(buf), "%s=%s", option, value);
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(wrq.ifr_name, ifname);
    len = strlen(buf);
    wrq.u.data.pointer = buf;
    wrq.u.data.length = len;
    ioctl(s, RTPRIV_IOCTL_SET, &wrq);
    return close(s);
}

void print_usage(){
    printf("Usage: apcli [ifname] connect [params]\n");
    printf("\t-s ssid\n");
    printf("\t-b bssid\n");
    printf("\t-k key\n");
    printf("\t-c PATH_TO_CONNECT_NOTIFY_SCRIPT\n");
    printf("\t-d PATH_TO_DISCONNECT_NOTIFY_SCRIPT\n");
}

char* mac2str(unsigned char *mac_bin)
{
    static char mac_str_buf[18];
  snprintf(
    mac_str_buf,
    sizeof(mac_str_buf),
    "%02X:%02X:%02X:%02X:%02X:%02X",
    mac_bin[0],
    mac_bin[1],
    mac_bin[2],
    mac_bin[3],
    mac_bin[4],
    mac_bin[5]);
  return mac_str_buf;
}

char * format_channel(int channel)
{
    static char buf[8];
    if ( channel > 0 )
        snprintf(buf, sizeof(buf), "%d", channel);
    else
        strcpy(buf, "unknown");
    return buf;
}

char * get_sec_str(int sec)
{
    char *cur; // $s0
    static char sec_str_buf[120];

    if ( (sec & 2) != 0 )
    {
        strcpy(sec_str_buf, "WEP-40, ");
        cur = sec_str_buf + 8;
    }
    else
    {
        cur = sec_str_buf;
    }
    if ( (sec & IWINFO_CIPHER_WEP104) != 0 )
    {
        strcpy(cur, "WEP-104, ");
        cur += 9;
    }
    if ( (sec & IWINFO_CIPHER_TKIP) != 0 )
    {
        strcpy(cur, "TKIP, ");
        cur += 6;
    }
    if ( (sec & IWINFO_CIPHER_CCMP) != 0 )
    {
        strcpy(cur, "CCMP, ");
        cur += 6;
    }
    if ( (sec & IWINFO_CIPHER_WRAP) != 0 )
    {
        strcpy(cur, "WRAP, ");
        cur += 6;
    }
    if ( (sec & IWINFO_CIPHER_AESOCB) != 0 )
    {
        strcpy(cur, "AES-OCB, ");
        cur += 9;
    }
    if ( (sec & IWINFO_CIPHER_CKIP) != 0 )
    {
        strcpy(cur, "CKIP, ");
        cur += 6;
    }
    if ( !sec || (sec & IWINFO_CIPHER_NONE) != 0)
    {
        strcpy(cur, "NONE, ");
        cur += 6;
    }
    *(cur - 2) = 0;
    return sec_str_buf;
}

char * fill_enc_str(int enc)
{
    char *cur;
    static char enc_str_buf[64];

    if ( (enc & IWINFO_KMGMT_PSK) != 0 )
    {
        strcpy(enc_str_buf, "PSK/");
        cur = enc_str_buf+4;
    }
    else
    {
        cur = enc_str_buf;
    }
    if ( (enc & IWINFO_KMGMT_8021x) != 0 )
    {
        strcpy(cur, "802.1X/");
        cur += 7;
    }
    else if ( !enc || (enc & IWINFO_KMGMT_NONE) != 0 )
    {
        strcpy(cur, "NONE/");
        cur += 5;
    }
    *(cur - 1) = 0;
    return enc_str_buf;
}

char * format_sec_enc(struct iwinfo_crypto_entry *crypto)
{
  static char sec_ens_str_buf[512];

  if ( !crypto )
  {
    strcpy(sec_ens_str_buf, "unknown");
    return sec_ens_str_buf;
  }
  if ( !crypto->enabled ){
    strcpy(sec_ens_str_buf, "none");
    return sec_ens_str_buf;
  }

  if ( crypto->auth_algs && !crypto->wpa_version )
  {
    if ( (crypto->auth_algs & 3) == 3 )
    {
      snprintf(sec_ens_str_buf, 512, "WEP Open/Shared (%s)", get_sec_str(crypto->pair_ciphers));
    }
    else
    {
      if ( crypto->auth_algs & 1 )
      {
        snprintf(sec_ens_str_buf, 512, "WEP Open System (%s)", get_sec_str(crypto->pair_ciphers));
      }
      else if ( crypto->auth_algs & 2 )
      {
        snprintf(sec_ens_str_buf, 512, "WEP Shared Auth (%s)", get_sec_str(crypto->pair_ciphers));
      }
    }
    return sec_ens_str_buf;
  }
  if ( !crypto->wpa_version )
  {
    strcpy(sec_ens_str_buf, "none");
    return sec_ens_str_buf;
  }
  switch ( crypto->wpa_version )
  {
    case 2:
      snprintf(sec_ens_str_buf, 512, "WPA2 %s (%s)", fill_enc_str(crypto->auth_suites), get_sec_str((crypto->pair_ciphers | crypto->group_ciphers)));
      break;
    case 3:
      snprintf(sec_ens_str_buf, 512, "mixed WPA/WPA2 %s (%s)", fill_enc_str(crypto->auth_suites), get_sec_str((crypto->pair_ciphers | crypto->group_ciphers)));
      break;
    case 1:
      snprintf(sec_ens_str_buf, 512, "WPA %s (%s)", fill_enc_str(crypto->auth_suites), get_sec_str((crypto->pair_ciphers | crypto->group_ciphers)));
      break;
  }
  return sec_ens_str_buf;
}

char * format_signal(int value)
{
    static char signal_buf[10];
    if ( value )
        snprintf(signal_buf, sizeof(signal_buf), "%d dBm", value);
    else
        strcpy(signal_buf, "unknown");
    return signal_buf;
}

char * format_quality1(int value)
{
    static char quality1_buf[8];
    if ( value >= 0 )
        snprintf(quality1_buf, sizeof(quality1_buf), "%d", value);
    else
        strcpy(quality1_buf, "unknown");
    return quality1_buf;
}

char * format_quality2(int value)
{
    static char quality2_buf[8];
    if ( value >= 0 )
        snprintf(quality2_buf, sizeof(quality2_buf), "%d", value);
    else
        strcpy(quality2_buf, "unknown");
    return quality2_buf;
}

int get_scanlist_entry(char *phyname, struct iwinfo_scanlist_entry *out, char *ssid, char *bssid){
    char buf[IWINFO_BUFSIZE];
    int len,i;
    struct iwinfo_scanlist_entry *e;
    const struct iwinfo_ops *iw;
    char *tmp_mac_str;
    int ssid_len;

    memset(out,0,sizeof(struct iwinfo_scanlist_entry));

    if(bssid==NULL && ssid==NULL){
        return -1;
    }
    iw = iwinfo_backend(phyname);
    if ( !iw )
    {
        fprintf(stderr, "No such wireless device: %s\n", phyname);
        return -1;
    }
    if ( iw->scanlist(phyname, buf, &len) )
    {
        printf("Scanning not possible\n");
        goto finish;
    }
    if(len==0){
        printf("No scan results\n");
        return -1;
    }

    for (i = 0; i < len; i += sizeof(struct iwinfo_scanlist_entry)) {
        e = (struct iwinfo_scanlist_entry *) &buf[i];
        if(out->channel > 0){
            break;
        }
        if(bssid){
            tmp_mac_str=mac2str(e->mac);
            if(strncmp(bssid, tmp_mac_str, 17)==0){
                if(ssid){
                    ssid_len=strlen(ssid);
                    if (strncmp(ssid, e->ssid, ssid_len) == 0 && ssid_len == strlen(e->ssid) ){
                        printf("Bundling ESSID:%s BSSID:%s\n", e->ssid, tmp_mac_str);
                        memcpy(out,e,sizeof(struct iwinfo_scanlist_entry));
                        goto finish;
                    }
                    if(e->ssid[0]=='\0'){
                        printf("Connect Hiden BSSID:%s\n", tmp_mac_str);
                        memcpy(out,e,sizeof(struct iwinfo_scanlist_entry));
                        memcpy(e->ssid,ssid,ssid_len);
                        goto finish;
                    }
                    printf("Connect Fixed by BSSID:%s\n", tmp_mac_str);
                    memcpy(out,e,sizeof(struct iwinfo_scanlist_entry));
                    goto finish;
                }
                printf("Connect by BSSID:%s\n", tmp_mac_str);
                memcpy(out,e,sizeof(struct iwinfo_scanlist_entry));
                goto finish;
            }
        }else{
            //只有essid，就只根据essid来选
            ssid_len=strlen(ssid);
            if (strncmp(ssid, e->ssid, ssid_len) == 0 && ssid_len == strlen(e->ssid) ){
                printf("Connect Fixed by ESSID:%s\n", e->ssid);
                memcpy(out,e,sizeof(struct iwinfo_scanlist_entry));
                goto finish;
            }
        }
    }
finish:
    iwinfo_finish();
    return -(out->channel==0);
}

int check_Associated(const char *ifname, int visable)
{
    int s;
    struct iwreq wrq;
    char tmp_mac[6]={0};
    int ret;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(wrq.ifr_name, ifname);
    ioctl(s, SIOCGIWAP, &wrq);
    close(s);
    if(memcmp(tmp_mac,&wrq.u.ap_addr.sa_data,6) == 0){
        if(visable){
            printf("%s:Not Associated.\n", ifname);
        }
        ret=0;
    }else{
        ret=1;
        if(visable){
            printf(
                "%s:Associated %02X:%02X:%02X:%02X:%02X:%02X.\n",
                ifname,
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[0],
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[1],
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[2],
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[3],
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[4],
                ((unsigned char*)&wrq.u.ap_addr.sa_data)[5]);
        }
    }
    return ret;
}

int scan_and_connect(char *phy_name, char *ssid, char *bssid, char *key) {
    char *apcli_name=NULL;
    int scan_retry_count;
    struct iwinfo_scanlist_entry e;
    const struct iwinfo_ops *iw;
    int ret;
    char cmd_buf[128];
    char *ApCliAuthMode=NULL;
    int retry_count;

    if(!strcmp(phy_name,"ra0")) {
        apcli_name="apcli0";
    } else if(!strcmp(phy_name,"rai0")) {
        apcli_name="apclii0";
    } else if(!strcmp(phy_name,"rax0")) {
        apcli_name="apclix0";
    } else {
        return -1;
    }
    scan_retry_count = 3;
    while(1) {
        if(get_scanlist_entry(phy_name, &e, ssid, bssid)==0) {
            break;
        }
        if ( !--scan_retry_count ) {
            printf("\nNo AP Matched!!\n");
            sleep(2);
            return -1;
        }
    }
    printf(
        "\nMatched AP:\n\tBSSID:\"%s\"\n\tESSID:\"%s\"\n\tChannel:%s\n\tEncryption:%s\n\tSignal:%s\n\tQuality:%s/%s\n",
        mac2str(e.mac),
        e.ssid,
        format_channel(e.channel),
        format_sec_enc(&e.crypto),
        format_signal(e.signal - 256),
        format_quality1(e.quality),
        format_quality2(e.quality_max));
    memset(cmd_buf, 0, sizeof(cmd_buf));
    iwpriv_set(apcli_name, "ApCliEnable", "0");
    iwpriv_set(apcli_name, "Channel", format_channel(e.channel));
    iwpriv_set(apcli_name, "ApCliSsid", e.ssid);
    iwpriv_set(apcli_name, "ApCliBssid", mac2str(e.mac));
    if ( !e.crypto.enabled || !key ) {
        iwpriv_set(apcli_name, "ApCliAuthMode", "NONE");
        iwpriv_set(apcli_name, "ApCliEncrypType", "NONE");
    } else {
        if ( !e.crypto.auth_algs || e.crypto.wpa_version ) {
            if ( e.crypto.wpa_version ) {
                if ( (e.crypto.auth_suites & IWINFO_KMGMT_PSK) != 0 || (ApCliAuthMode = "WPANONE", (e.crypto.auth_suites & IWINFO_KMGMT_8021x) != 0) ) {
                    if ( e.crypto.wpa_version == 2 ) {
                        ApCliAuthMode = "WPA2PSK";
                    } else if ( e.crypto.wpa_version == 3 ) {
                        ApCliAuthMode = "WPAPSKWPA2PSK";
                    } else if ( e.crypto.wpa_version == 1 ) {
                        ApCliAuthMode = "WPAPSK";
                    }
                }
            } else {
                ApCliAuthMode = "WPANONE";
            }
        } else if ( (e.crypto.auth_algs & 3) == 3 ) {
            ApCliAuthMode = "WEPAUTO";
        } else if ( (e.crypto.auth_algs & 1) != 0 ) {
            ApCliAuthMode = "OPEN";
        } else if ( (e.crypto.auth_algs & 2) != 0 ) {
            ApCliAuthMode = "SHARED";
        }
        if(ApCliAuthMode) {
            iwpriv_set(apcli_name, "ApCliAuthMode", ApCliAuthMode);
        }
        if ( e.crypto.wpa_version ) {
            if ( ((e.crypto.pair_ciphers | e.crypto.group_ciphers) & IWINFO_CIPHER_CCMP) != 0 ) {
                iwpriv_set(apcli_name, "ApCliEncrypType", "AES");
            } else if ( ((e.crypto.pair_ciphers | e.crypto.group_ciphers) & IWINFO_CIPHER_TKIP) != 0 ) {
                iwpriv_set(apcli_name, "ApCliEncrypType", "TKIP");
            } else {
                iwpriv_set(apcli_name, "ApCliEncrypType", "TKIPAES");
            }
            iwpriv_set(apcli_name, "ApCliWPAPSK", key);
        } else {
            iwpriv_set(apcli_name, "ApCliEncrypType", "WEP");
            iwpriv_set(apcli_name, "ApCliDefaultKeyID", "1");
            iwpriv_set(apcli_name, "ApCliKey1", key);
        }
    }
    iwpriv_set(apcli_name, "ApCliEnable", "1");
    snprintf(cmd_buf, sizeof(cmd_buf), "ifconfig %s up", apcli_name);
    system(cmd_buf);
    sleep(2);
    retry_count = 10;
    do
    {
        if ( check_Associated(apcli_name, 0) )
          return 0;
        --retry_count;
        sleep(2);
    }
    while ( retry_count );
    return 0;
}

int main(int argc, char*argv[]){

    char *apcli_name=NULL;
    char *ssid=NULL;
    char *bssid=NULL;
    char *key=NULL;
    char *notify_connect_path=NULL;
    char *notify_disconnect_path=NULL;
    char *phy_name=NULL;
    int i;
    int first_time=1;
    int ch;
    int pid;

    if(argc<=3){
        print_usage();
        return -1;
    }

    if(strcmp(argv[1],"ra0")==0 || strcmp(argv[1],"apcli0")==0){
        phy_name="ra0";
        apcli_name="apcli0";
    }else if(strcmp(argv[1],"rai0")==0 || strcmp(argv[1],"apclii0")==0){
        phy_name="rai0";
        apcli_name="apclii0";
    }else if(strcmp(argv[1],"rax0")==0 || strcmp(argv[1],"apclix0")==0){
        phy_name="rax0";
        apcli_name="apclix0";
    }else{
        printf("dev name=%s error!!!\n", argv[1]);
        goto finish;
    }

    if(strcmp(argv[2],"connect")){
        printf("only support connect command!\n");
        goto finish;
    }

    opterr = 0;
   
    while((ch = getopt(argc-2,argv+2,"s:b:k:c:d")) != -1)
    {
        switch(ch)
        {
            case 's':
                if(strlen(optarg)>32 || strlen(optarg)==0){
                    printf("Error, ssid length should <= 32 !!!!\n");
                    goto finish;
                }
                ssid=strdup(optarg);
                break;
            case 'b':
                if(strlen(optarg)!=17){
                    printf("Error, bssid length should == 17 !!!!\n");
                    goto finish;
                }
                bssid=strdup(optarg);
                break;
            case 'k':
                if(strlen(optarg)>63){
                    printf("Error, key len should <= 63 !!!!\n");
                    goto finish;
                }
                key=strdup(optarg);
                break;
            case 'd':
                notify_disconnect_path=optarg;
                break;
            case 'c':
                notify_connect_path=optarg;
                break;
            default: 
                break;
        }
    }
    if(ssid==NULL && bssid==NULL){
        printf("You must specify a SSID or BSSID to connect to a wireless network.\n");
        goto finish;
    }

    if(bssid){
        for(i=0;i<17;i++){
            bssid[i]=toupper(bssid[i]);
        }
    }

    pid=fork();
    if(pid==-1){
        printf("Fail to fork a monitor process.Exit now!\n");
        goto finish;
    }else if(pid>0){
        printf("Monitor process [%d] started.Exit now!\n", pid);
        goto finish;
    }

    printf("Device [%s] connect SSID:[%s] BSSID:[%s] KEY:[%s]\n", phy_name, (ssid==NULL?"NONE":ssid), (bssid==NULL?"NONE":bssid), key);

    scan_and_connect(phy_name,ssid,bssid,key);
    while(1){
        if ( check_Associated(apcli_name, 0) ){
            if(first_time){
                printf("%s up\n", apcli_name);
                first_time=0;
                if(notify_connect_path)
                    system(notify_connect_path);
            }
            sleep(2);
        }else{
            if(!first_time){
                first_time=1;
                printf("%s down\n", apcli_name);
                if(notify_disconnect_path)
                    system(notify_disconnect_path);
            }
            scan_and_connect(phy_name,ssid,bssid,key);
        }
    }

finish:
    return 0;
}
