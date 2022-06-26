#!/bin/sh
#
# Copyright (c) 2013-2015 D-Team Technology Co.,Ltd. ShenZhen
# Copyright (c) 2005-2015, lintel <lintel.huang@gmail.com>
# Copyright (c) 2013, Hoowa <hoowa.sun@gmail.com>
# Copyright (c) 2015-2017, GuoGuo <gch981213@gmail.com>
# Copyright (c) 2020, jjm2473 <jjm2473@gmail.com>
# Copyright (c) 2022, nanchuci <nanchuci023@gmail.com>
#
# 	netifd config script for MT7615/MT7915 DBDC mode.
#
# 	嘿，对着屏幕的哥们,为了表示对原作者辛苦工作的尊重，任何引用跟借用都不允许你抹去所有作者的信息,请保留这段话。
#
. /lib/netifd/netifd-wireless.sh

init_wireless_driver "$@"

#Default configurations
RTWIFI_PROFILE_DIR="/tmp/profiles/"
RTWIFI_PROFILE_PATH=""
RTWIFI_CMD_PATH=""
RTWIFI_CMD_OPATH=""
APCLI_IF=""
APCLI_APCTRL=""
WIFI_OP_LOCK=$RTWIFI_PROFILE_DIR"mt_dbdc.lock"
RTWIFI_IFPREFIX=""
RTWIFI_DEF_BAND=""
RTWIFI_FORCE_HT=0
RTWIFI_DEF_MAX_BSSID=4

mt_cmd() {
	echo "$@" >> $RTWIFI_CMD_PATH
}

#读取device相关设置项并写入json
drv_mt_dbdc_init_device_config() { 
	config_add_string channel hwmode htmode country macaddr
	config_add_int beacon_int chanbw frag rts txburst
	config_add_int rxantenna txantenna antenna_gain txpower distance wmm
	config_add_boolean greenap diversity noscan ht_coex smart
	config_add_int powersave
	config_add_int maxassoc
	config_add_boolean hidessid bndstrg
	
	config_add_boolean \
		rxldpc \
		short_gi_80 \
		short_gi_160 \
		tx_stbc_2by1 \
		su_beamformer \
		su_beamformee \
		mu_beamformer \
		mu_beamformee \
		vht_txop_ps \
		htc_vht \
		rx_antenna_pattern \
		tx_antenna_pattern
	config_add_int vht_max_a_mpdu_len_exp vht_max_mpdu vht_link_adapt vht160 rx_stbc tx_stbc
	
	config_add_boolean \
		ldpc \
		greenfield \
		short_gi_20 \
		short_gi_40 \
		dsss_cck_40
}

#读取iface相关设置项并写入json
drv_mt_dbdc_init_iface_config() { 
	config_add_boolean disabled
	config_add_string mode bssid ssid encryption
	config_add_boolean hidden isolate doth ieee80211k
	config_add_boolean hidden isolate doth ieee80211v
	config_add_boolean hidden isolate doth ieee80211r
	config_add_boolean hidden isolate doth ieee80211w
	config_add_string key key1 key2 key3 key4
	config_add_string wps
	config_add_string pin
	config_add_string macfilter
	config_add_array maclist
	
	config_add_boolean wds
	config_add_int max_listen_int
	config_add_int dtim_period
	config_add_int disassoc_low_ack rssiassoc
	config_add_string wdsenctype wdskey wdsphymode
	config_add_int wdswepid wdstxmcs
}

get_wep_key_type() {
	local KeyLen=$(expr length "$1")
	if [ $KeyLen -eq 10 ] || [ $KeyLen -eq 26 ]
	then
		echo 0
	else
		echo 1
	fi	
}

mt_dbdc_ap_vif_pre_config() {
	local name="$1"

	json_select config
	json_get_vars disabled encryption key key1 key2 key3 key4 ssid mode wps pin isolate doth hidden disassoc_low_ack rssiassoc ieee80211k ieee80211v ieee80211r ieee80211w macfilter
	json_get_values maclist maclist
	json_select ..
	[ "$disabled" == "1" ] && return
	echo "Generating ap config for interface ra${RTWIFI_IFPREFIX}${ApBssidNum}"
	ifname="ra${RTWIFI_IFPREFIX}${ApBssidNum}"

	#MAC过滤方式相关设定 由于编号问题......我扔在这了......
	ra_maclist="${maclist// /;};"
	case "$macfilter" in
	allow)
		echo "Interface ${ifname} has MAC Policy.Allow list:${ra_maclist}"
		echo "AccessPolicy${ApBssidNum}=1" >> $RTWIFI_PROFILE_PATH
		echo "AccessControlList$ApBssidNum=${ra_maclist}" >> $RTWIFI_PROFILE_PATH
	;;
	deny)
		echo "Interface ${ifname} has MAC Policy.Deny list:${ra_maclist}"
		echo "AccessPolicy${ApBssidNum}=2" >> $RTWIFI_PROFILE_PATH
		echo "AccessControlList${ApBssidNum}=${ra_maclist}" >> $RTWIFI_PROFILE_PATH
	;;
	esac

	let ApBssidNum+=1
	echo "SSID$ApBssidNum=${ssid}" >> $RTWIFI_PROFILE_PATH #SSID
	case "$encryption" in #加密方式
	wpa*|psk*|WPA*|Mixed|mixed)
		local enc
		local crypto
		case "$encryption" in
			Mixed|mixed|psk+psk2|psk-mixed*)
				enc=WPAPSKWPA2PSK
			;;
			WPA2*|wpa2*|psk2*)
				enc=WPA2PSK
			;;
			WPA*|WPA1*|wpa*|wpa1*|psk*)
				enc=WPAPSK
			;;
			esac
			crypto="AES"
		case "$encryption" in
			*tkip+aes*|*tkip+ccmp*|*aes+tkip*|*ccmp+tkip*)
				crypto="TKIPAES"
			;;
			*aes*|*ccmp*)
				crypto="AES"
			;;
			*tkip*) 
				crypto="TKIP"
				echo "Warning!!! TKIP is not support in 802.11n 40Mhz!!!"
			;;
			esac
				ApAuthMode="${ApAuthMode}${enc};"
				ApEncrypType="${ApEncrypType}${crypto};"
				ApDefKId="${ApDefKId}2;"
			echo "WPAPSK$ApBssidNum=${key}" >> $RTWIFI_PROFILE_PATH
	;;
	WEP|wep|wep-open|wep-shared)
		if [ "$encryption" == "wep-shared" ]; then
			ApAuthMode="${ApAuthMode}SHARED;"
		else  
			ApAuthMode="${ApAuthMode}OPEN;"
		fi
		ApEncrypType="${ApEncrypType}WEP;"
		K1Tp=$(get_wep_key_type "$key1")
		K2Tp=$(get_wep_key_type "$key2")
		K3Tp=$(get_wep_key_type "$key3")
		K4Tp=$(get_wep_key_type "$key4")

		[ $K1Tp -eq 1 ] && key1=$(echo $key1 | cut -d ':' -f 2- )
		[ $K2Tp -eq 1 ] && key2=$(echo $key2 | cut -d ':' -f 2- )
		[ $K3Tp -eq 1 ] && key3=$(echo $key3 | cut -d ':' -f 2- )
		[ $K4Tp -eq 1 ] && key4=$(echo $key4 | cut -d ':' -f 2- )
		echo "Key1Str${ApBssidNum}=${key1}" >> $RTWIFI_PROFILE_PATH
		echo "Key2Str${ApBssidNum}=${key2}" >> $RTWIFI_PROFILE_PATH
		echo "Key3Str${ApBssidNum}=${key3}" >> $RTWIFI_PROFILE_PATH
		echo "Key4Str${ApBssidNum}=${key4}" >> $RTWIFI_PROFILE_PATH
		ApDefKId="${ApDefKId}${key};"
		;;
	none|open)
		ApAuthMode="${ApAuthMode}OPEN;"
		ApEncrypType="${ApEncrypType}NONE;"
		ApDefKId="${ApDefKId}1;"
		;;
	esac
	ApHideESSID="${ApHideESSID}${hidden:-0};"
	ApK1Tp="${ApK1Tp}${K1Tp:-0};"
	ApK2Tp="${ApK2Tp}${K2Tp:-0};"
	ApK3Tp="${ApK3Tp}${K3Tp:-0};"
	ApK4Tp="${ApK4Tp}${K4Tp:-0};"

	mt_cmd ifconfig $ifname up
	mt_cmd echo "Interface $ifname now up."
	mt_cmd iwpriv $ifname set NoForwarding=${isolate:-0}
	mt_cmd iwpriv $ifname set IEEE80211H=${doth:-0}
	if [ "$wps" == "pbc" ] || [ "$wps" == "pin" ]  && [ "$encryption" != "none" ]; then
		echo "Enable WPS for ${ifname}."
		mt_cmd iwpriv $ifname set WscConfMode=4
		mt_cmd iwpriv $ifname set WscConfStatus=2
		mt_cmd iwpriv $ifname set WscMode=2
		mt_cmd iwpriv $ifname set WscV2Support=0
	else
		mt_cmd iwpriv $ifname set WscConfMode=0
	fi
	[ -n "$disassoc_low_ack" ]  && [ "$disassoc_low_ack" != "0" ] && mt_cmd iwpriv $ifname set KickStaRssiLow=$disassoc_low_ack
	[ -n "$rssiassoc" ]  && [ "$rssiassoc" != "0" ] && mt_cmd iwpriv $ifname set AssocReqRssiThres=$rssiassoc
	[ -n "$ieee80211k" ]  && [ "$ieee80211k" != "0" ] && mt_cmd iwpriv $ifname set rrmenable=1
	[ -n "$ieee80211v" ]  && [ "$ieee80211v" != "0" ] && mt_cmd iwpriv $ifname set wnmenable=1
	[ -n "$ieee80211r" ]  && [ "$ieee80211r" != "0" ] && mt_cmd iwpriv $ifname set ftenable=1
	[ -n "$ieee80211w" ]  && [ "$ieee80211w" != "0" ] && mt_cmd iwpriv $ifname set pmfenable=1
}

mt_dbdc_wds_vif_pre_config() {
	local name="$1"

	json_select config
	json_get_vars disabled bssid wdsenctype wdskey wdswepid wdsphymode wdstxmcs
	set_default wdswepid 1
	set_default wdstxmcs 33
	set_default wdsphymode "GREENFIELD"
	json_select ..
	[ "$disabled" == "1" ] && return
	[ $WDSBssidNum -gt 3 ] && return
	ifname="wds${RTWIFI_IFPREFIX}${WDSBssidNum}"
	echo "Generating WDS config for interface $ifname"
	WDSEN=1
	WDSList="${WDSList}${bssid};"
	WDSEncType="${WDSEncType}${wdsenctype};"
	WDSDefKeyID="${WDSDefKeyID}${wdswepid};"
	WDSPhyMode="${WDSPhyMode}${wdsphymode};"
	WDSTxMCS="${WDSTxMCS}${wdstxmcs};"
	echo "Wds${ApBssidNum}Key=${wdskey}" >> $RTWIFI_PROFILE_PATH #WDS Key
	let WDSBssidNum+=1

	mt_cmd ifconfig $ifname up
	mt_cmd echo "WDS interface $ifname now up."
}

mt_dbdc_sta_vif_pre_config() {
	local name="$1"

	json_select config
	json_get_vars disabled encryption key key1 key2 key3 key4 ssid mode bssid
	json_select ..

	[ $stacount -gt 1 ] && {
		return
	}

	[ "$disabled" == "1" ] && return
	let stacount+=1

	mt_cmd ifconfig $APCLI_IF up
	killall  $APCLI_APCTRL
	[ ! -z "$key" ] && APCTRL_KEY_ARG="-k"
	[ ! -z "$bssid" ] && APCTRL_BSS_ARG="-b $(echo $bssid | tr 'A-Z' 'a-z')"
	mt_cmd $APCLI_APCTRL ra${RTWIFI_IFPREFIX}0 connect -s "$ssid" $APCTRL_BSS_ARG $APCTRL_KEY_ARG "$key"
}

mt_dbdc_wds_vif_post_config() {
	local name="$1"
	json_select config
	json_get_vars disabled
	json_select ..

	[ "$disabled" == "1" ] && return
	[ $WDSBssidNum -gt 3 ] && return

	ifname="wds${RTWIFI_IFPREFIX}${WDSBssidNum}"
	let WDSBssidNum+=1

	wireless_add_vif "$name" "$ifname"
}

mt_dbdc_ap_vif_post_config() {
	local name="$1"

	json_select config
	json_get_vars disabled encryption key key1 key2 key3 key4 ssid mode wps pin isolate doth hidden disassoc_low_ack rssiassoc ieee80211k ieee80211v ieee80211r ieee80211w
	json_select ..

	[ "$disabled" == "1" ] && return

	[ $ApIfCNT -gt $RTWIFI_DEF_MAX_BSSID ] && return 

	ifname="ra${RTWIFI_IFPREFIX}${ApIfCNT}"
	let ApIfCNT+=1

	wireless_add_vif "$name" "$ifname"
}

mt_dbdc_sta_vif_post_config() {
	local name="$1"

	json_select config
	json_get_vars disabled
	json_select ..

	[ $stacount -gt 1 ] && {
		return
	}

	[ "$disabled" == "1" ] && return
	let stacount+=1

	wireless_add_vif "$name" "$APCLI_IF"
}

get_if_stat() {
	[ ! -z "$1" ] && [ -d "/sys/class/net/$1" ] && cat /sys/class/net/$1/operstate
}

mt_dbdc_vif_down() {
	phy_name=${1}
	killall -9 -q apcli_2g
	killall -9 -q apcli_5g
	case "$phy_name" in
		rax0)
			for vif in ra0 ra1 ra2 ra3 ra4 ra5 ra6 ra7 wds0 wds1 wds2 wds3 apcli0; do
				[ "$(get_if_stat $vif)" != "down" ] && ifconfig $vif down && echo $vif
			done
		;;
		ra0)
			for vif in rax0 rax1 rax2 rax3 rax4 rax5 rax6 rax7 wdsx0 wdsx1 wdsx2 wdsx3 apclix0; do
				[ "$(get_if_stat $vif)" != "down" ] && ifconfig $vif down && echo $vif
			done
		;;
	esac
}

drv_mt_dbdc_cleanup() {
	return
}

drv_mt_dbdc_teardown() {
	phy_name=${1}
	case "$phy_name" in
		ra0)
			killall -9 -q apcli_2g
			for vif in ra0 ra1 ra2 ra3 ra4 ra5 ra6 ra7 wds0 wds1 wds2 wds3 apcli0; do
				# iwpriv $vif set DisConnectAllSta=1
				[ -d "/sys/class/net/$vif" ] && ifconfig $vif down
			done
		;;
		rax0)
			killall -9 -q apcli_5g
			for vif in rax0 rax1 rax2 rax3 rax4 rax5 rax6 rax7 wdsx0 wdsx1 wdsx2 wdsx3 apclix0; do
				# iwpriv $vif set DisConnectAllSta=1
				[ -d "/sys/class/net/$vif" ] && ifconfig $vif down
			done
		;;
	esac
}

#接口启动
drv_mt_dbdc_setup() {
	json_select config
	json_get_vars main_if macaddr channel mode hwmode wmm htmode \
		txpower country macfilter maclist greenap \
		diversity frag rts txburst distance hidden \
		disabled maxassoc noscan ht_coex smart #device所有配置项
		
	json_get_vars \
			ldpc:1 \
			greenfield:0 \
			short_gi_20:1 \
			short_gi_40:1 \
			tx_stbc:1 \
			rx_stbc:3 \
			max_amsdu:1 \
			dsss_cck_40:1
			
	json_get_vars \
			rxldpc:1 \
			short_gi_80:1 \
			short_gi_160:1 \
			tx_stbc_2by1:1 \
			su_beamformer:1 \
			su_beamformee:1 \
			mu_beamformer:1 \
			mu_beamformee:1 \
			vht_txop_ps:1 \
			htc_vht:1 \
			rx_antenna_pattern:1 \
			tx_antenna_pattern:1 \
			vht_max_a_mpdu_len_exp:7 \
			vht_max_mpdu:11454 \
			rx_stbc:4 \
			vht_link_adapt:3 

	json_select ..

	phy_name=${1}
	wireless_set_data phy=${phy_name}
	case "$phy_name" in
		ra0)
			WirelessMode=9
			APCLI_IF="apcli0"
			APCLI_APCTRL="apcli_2g"
			RTWIFI_IFPREFIX=""
			RTWIFI_DEF_BAND="g"
			RTWIFI_PROFILE_PATH="${RTWIFI_PROFILE_DIR}mt_dbdc_2g.dat"
			RTWIFI_CMD_PATH="${RTWIFI_PROFILE_DIR}mt_dbdc_cmd_2g.sh"
			RTWIFI_CMD_OPATH="${RTWIFI_PROFILE_DIR}mt_dbdc_cmd_5g.sh"
		;;
		rax0)
			WirelessMode=14
			APCLI_IF="apclix0"
			APCLI_APCTRL="apcli_5g"
			RTWIFI_IFPREFIX="x"
			RTWIFI_DEF_BAND="a"
			RTWIFI_PROFILE_PATH="${RTWIFI_PROFILE_DIR}mt_dbdc_5g.dat"
			RTWIFI_CMD_PATH="${RTWIFI_PROFILE_DIR}mt_dbdc_cmd_5g.sh"
			RTWIFI_CMD_OPATH="${RTWIFI_PROFILE_DIR}mt_dbdc_cmd_2g.sh"
		;;
		*)
			echo "Unknown phy:$phy_name"
			return 1
	esac

#检查配置文件目录是否存在，否则创建目录
	[ ! -d $RTWIFI_PROFILE_DIR ] && mkdir $RTWIFI_PROFILE_DIR
	echo > $RTWIFI_CMD_PATH

	hwmode=${hwmode##11}
	case "$hwmode" in
		a)
			WirelessMode=14
			ITxBfEn=1
			HT_HTC=1
		;;
		g)
			WirelessMode=9
			ITxBfEn=0
			HT_HTC=1
		;;
		*) 
			echo "Unknown wireless mode.Use default value:${WirelessMode}"
			hwmode=${RTWIFI_DEF_BAND}
		;;
	esac
	
#HT默认模式设定
	HT_BW=1  #允许HT40
	HT_CE=1  #允许HT20/40共存
	HT_DisallowTKIP=0 #是否允许TKIP加密
	HT_GI=1 #HT_SHORT_GI
	VHT_SGI=1 #VHT_SHORT_GI
	#HT_MIMOPSMode用于省电模式设置
	#HT_MIMOPSMode=3
	
#VHT默认模式设定
	VHT_BW=1 #允许VHT
	VHT_DisallowNonVHT=0 #是否禁止非VHT客户端连接，VHT80 only
	
	[ "$short_gi_20" == "0" -o "$short_gi_40" == "0" ] && HT_GI=0
	[ "$short_gi_80" == "0" -o "$short_gi_160" == "0" ] && VHT_SGI=0
	
	case "$htmode" in
		HT20 |\
		VHT20) 
			HT_BW=0
			VHT_BW=0
		;;
		HT40 |\
		VHT40)
			HT_BW=1
			VHT_BW=0
			VHT_DisallowNonVHT=0
		;;
		HT80 |\
		VHT80)
			HT_BW=1
			VHT_BW=1
		;;
		
		VHT160)
			echo "only VHT80 support!!"
			HT_BW=1
			VHT_BW=1
		;;
		*) 
		echo "Unknown HT Mode."
		;;
	esac
	
#仅HT20以外才需要设置的参数
     [ "$htmode" != "HT20" ] && {
#强制HT40/VHT80
		[ "$noscan" == "1" ] && HT_CE=0 && RTWIFI_FORCE_HT=1
#HT HTC
		[ "$ht_htc" == "1" ] && HT_HTC=1
    }

#自动处理CountryRegion:指定信道的时候支持全频段
      [ "$channel" != "auto" ] && {
		#CountryRegion CN
		countryregion=5
		countryregion_a=7
      }

#信道相关
	case "$hwmode" in
		a)
			EXTCHA=1
			[ "$channel" != "auto" ] && [ "$channel" != "0" ] && [ "$(( ($channel / 4) % 2 ))" == "0" ] && EXTCHA=0
			[ "$channel" == "165" ] && EXTCHA=0
			[ "$channel" == "auto" -o "$channel" == "0" ] && {
				#CountryRegion CN
				countryregion=1
				countryregion_a=0
				channel=149
				AutoChannelSelect=2
			}
			ACSSKIP="36;38;40;42;44;46;48;52;56;60;64;100;104;108;112;116;120;124;128;132;136;140;165"
		;;
		g)
			EXTCHA=0
			[ "$channel" != "auto" ] && [ "$channel" != "0" ] && [ "$channel" -lt "7" ] && EXTCHA=1
			[ "$channel" == "auto" -o "$channel" == "0" ] && {
				channel=6
				AutoChannelSelect=2
				countryregion=1
			}
			ACSSKIP="12;13;14"
		;;
	esac

#设备配置文件生成
	cat > $RTWIFI_PROFILE_PATH <<EOF
#The word of "Default" must not be removed
Default
DBDC_MODE=1
BssidNum=${RTWIFI_DEF_MAX_BSSID}
MacAddress=${macaddr}
CountryRegion=${countryregion:-5}
CountryRegionABand=${countryregion_a:-7}
CountryCode=${country:-CN}
RRMEnable=${RRMEnable:-0};${RRMEnable:-0};${RRMEnable:-0};${RRMEnable:-0}
WNMEnable=${WNMEnable:-0};${WNMEnable:-0};${WNMEnable:-0};${WNMEnable:-0}
FTEnable=${FTEnable:-0};${FTEnable:-0};${FTEnable:-0};${FTEnable:-0}
PMFenable=${PMFenable:-0};${PMFenable:-0};${PMFenable:-0};${PMFenable:-0}
WirelessMode=${WirelessMode}
G_BAND_256QAM=1
FixedTxMode=
TxRate=0
Channel=${channel}
BasicRate=15
BeaconPeriod=100
DtimPeriod=1
PERCENTAGEenable=1
TxPower=${txpower:-100}
SKUenable=1
BFBACKOFFenable=0
CalCacheApply=0
DisableOLBC=0
BGProtection=0
TxAntenna=${TxAntenna:-2};${TxAntenna:-2};${TxAntenna:-4};${TxAntenna:-4}
RxAntenna=${RxAntenna:-2};${RxAntenna:-2};${RxAntenna:-4};${RxAntenna:-4}
TxPreamble=1
RTSThreshold=${rts:-2347}
FragThreshold=${frag:-2346}
TxBurst=${txburst:-0}
PktAggregate=1
AutoProvisionEn=0
FreqDelta=0
TurboRate=0
WmmCapable=${wmm:-0}
APAifsn=3;7;1;1
APCwmin=4;4;3;2
APCwmax=6;10;4;3
APTxop=0;0;94;47
APACM=0;0;0;0
BSSAifsn=3;7;2;2
BSSCwmin=4;4;3;2
BSSCwmax=10;10;4;3
BSSTxop=0;0;94;47
BSSACM=0;0;0;0
AckPolicy=0;0;0;0
APSDCapable=0
DLSCapable=0
NoForwarding=0
NoForwardingBTNBSSID=0
ShortSlot=1
AutoChannelSelect=${AutoChannelSelect:-0}
IEEE8021X=0
IEEE80211H=0
CarrierDetect=0
ITxBfEn=${ITxBfEn}
PreAntSwitch=
PhyRateLimit=0
DebugFlags=0
ETxBfEnCond=${ITxBfEn}
ITxBfTimeout=0
ETxBfTimeout=0
ETxBfNoncompress=0
ETxBfIncapable=0
MUTxRxEnable=3
DfsEnable=0
DfsZeroWait=0
DfsZeroWaitCacTime=255
FineAGC=0
StreamMode=0
StreamModeMac0=
StreamModeMac1=
StreamModeMac2=
StreamModeMac3=
CSPeriod=6
RDRegion=
StationKeepAlive=0
DfsLowerLimit=0
DfsUpperLimit=0
DfsOutdoor=0
SymRoundFromCfg=0
BusyIdleFromCfg=0
DfsRssiHighFromCfg=0
DfsRssiLowFromCfg=0
DFSParamFromConfig=0
FCCParamCh0=
FCCParamCh1=
FCCParamCh2=
FCCParamCh3=
CEParamCh0=
CEParamCh1=
CEParamCh2=
CEParamCh3=
JAPParamCh0=
JAPParamCh1=
JAPParamCh2=
JAPParamCh3=
JAPW53ParamCh0=
JAPW53ParamCh1=
JAPW53ParamCh2=
JAPW53ParamCh3=
FixDfsLimit=0
LongPulseRadarTh=0
AvgRssiReq=0
DFS_R66=0
BlockCh=
PreAuth=0
WapiPsk1=0123456789
WapiPsk2=
WapiPsk3=
WapiPsk4=
WapiPsk5=
WapiPsk6=
WapiPsk7=
WapiPsk8=
WapiPskType=0
Wapiifname=
WapiAsCertPath=
WapiUserCertPath=
WapiAsIpAddr=
WapiAsPort=
RekeyMethod=DISABLE
RekeyInterval=3600
PMKCachePeriod=10
MeshAutoLink=0
MeshAuthMode=
MeshEncrypType=
MeshDefaultkey=0
MeshWEPKEY=
MeshWPAKEY=
MeshId=
HSCounter=0
HT_HTC=${HT_HTC}
HT_RDG=1
HT_LDPC=${ldpc:-1}
HT_LinkAdapt=0
HT_OpMode=${greenfield:-0}
HT_MpduDensity=4
HT_EXTCHA=${EXTCHA}
HT_BW=${HT_BW:-0}
HT_AutoBA=1
HT_BADecline=0
HT_AMSDU=1
HT_BAWinSize=64
HT_GI=${HT_GI:-1}
HT_STBC=${tx_stbc:-1}
HT_LDPC=${ldpc:-1}
HT_MCS=33
VHT_BW=${VHT_BW:-0}
VHT_SGI=1
VHT_STBC=${tx_stbc:-1}
VHT_BW_SIGNAL=0
VHT_DisallowNonVHT=${VHT_DisallowNonVHT:-0}
VHT_LDPC=${ldpc:-1}
HT_TxStream=2;2;2;2
HT_RxStream=2;2;2;2
HT_PROTECT=0
HT_DisallowTKIP=${HT_DisallowTKIP:-0}
HT_BSSCoexistence=${HT_CE:-1}
HT_BSSCoexApCntThr=10
GreenAP=${greenap:-0}
WscConfMode=0
WscConfStatus=1
WCNTest=0
RADIUS_Server=
RADIUS_Port=1812
RADIUS_Key1=
RADIUS_Key2=
RADIUS_Key3=
RADIUS_Key4=
RADIUS_Key5=
RADIUS_Key6=
RADIUS_Key7=
RADIUS_Key8=
RADIUS_Acct_Server=
RADIUS_Acct_Port=1813
RADIUS_Acct_Key=
own_ip_addr=
Ethifname=
EAPifname=
PreAuthifname=
session_timeout_interval=0
idle_timeout_interval=0
WiFiTest=0
TGnWifiTest=0
ApCliEnable=0
ApCliSsid=
ApCliBssid=
ApCliAuthMode=
ApCliEncrypType=
ApCliWPAPSK=
ApCliDefaultKeyID=0
ApCliKey1Type=0
ApCliKey1Str=
ApCliKey2Type=0
ApCliKey2Str=
ApCliKey3Type=0
ApCliKey3Str=
ApCliKey4Type=0
ApCliKey4Str=
RadioOn=1
WscManufacturer=PandoraBox
WscModelName=PandoraBox Wireless Router
WscDeviceName=PandoraBox WiFi
WscModelNumber=
WscSerialNumber=
PMFMFPC=0
PMFMFPR=0
PMFSHA256=0
LoadCodeMethod=0
AutoChannelSkipList=${ACSSKIP}
MaxStaNum=${maxassoc:-0}
WirelessEvent=1
AuthFloodThreshold=64
AssocReqFloodThreshold=64
ReassocReqFloodThreshold=64
ProbeReqFloodThreshold=64
DisassocFloodThreshold=64
DeauthFloodThreshold=64
EapReqFloodThreshold=64
Thermal=100
EnhanceMultiClient=1
IgmpSnEnable=0
#DetectPhy=1
BGMultiClient=1
EDCCA=0
HT_MIMOPSMode=3
PandoraBoxSmart=${smart:-1}
RED_Enable=1
VOW_Airtime_Fairness_En=1
CP_SUPPORT=2
BandSteering=0
BndStrgRssiDiff=15
BndStrgRssiLow=-86
BndStrgAge=600000
BndStrgHoldTime=3000
BndStrgCheckTime=6000
SCSEnable=1
DyncVgaEnable=1
SkipLongRangeVga=0
VgaClamp=0
FastRoaming=0
AutoRoaming=0
FtSupport=0
FtRic=1;1;1;1
FtOtd=0;0;0;0
FtMdId1=A1
FtMdId2=A2
FtMdId3=A3
FtMdId4=A4
FtR0khId1=4f577274
FtR0khId2=4f577276
FtR0khId3=4f577278
FtR0khId4=4f57727A
BandDeltaRssi=-12
ApProbeRspTimes=3
#AuthRspFail=0
#AuthRspRssi=0
#AssocReqRssiThres=-68
#AssocRspIgnor=0
#KickStaRssiLow=-75
KickStaRssiLowPSM=-77
#KickStaRssiLowDelay=6
#ProbeRspRssi=-72
VideoClassifierEnable=1
VideoHighTxMode=1
VideoTurbine=1
VideoTxLifeTimeMode=1
EOF

#接口配置生成
#	STA模式
	stacount=0
	for_each_interface "sta" mt_dbdc_sta_vif_pre_config

#	AP模式
#	统一设置的内容:
	ApEncrypType=""
	ApAuthMode=""
	ApBssidNum=0
	ApHideESSID=""
	ApDefKId=""
	ApK1Tp=""
	ApK2Tp=""
	ApK3Tp=""
	ApK4Tp=""

	for_each_interface "ap" mt_dbdc_ap_vif_pre_config

	[ "$phy_name" == "ra0" ] && [ "$ApBssidNum" == "0" ] && mt_cmd ifconfig ra0 down
#For DBDC profile merging......
	while [ $ApBssidNum -lt $RTWIFI_DEF_MAX_BSSID ]
	do
		ApEncrypType="${ApEncrypType}NONE;"
		ApAuthMode="${ApAuthMode}OPEN;"
		ApHideESSID="${ApHideESSID}0;"
		ApDefKId="${ApDefKId}0;"
		ApK1Tp="${ApK1Tp}0;"
		ApK2Tp="${ApK2Tp}0;"
		ApK3Tp="${ApK3Tp}0;"
		ApK4Tp="${ApK4Tp}0;"
		let ApBssidNum+=1
	done

	echo "AuthMode=${ApAuthMode%?}" >> $RTWIFI_PROFILE_PATH
	echo "EncrypType=${ApEncrypType%?}" >> $RTWIFI_PROFILE_PATH
	echo "HideSSID=${ApHideESSID%?}" >> $RTWIFI_PROFILE_PATH
	echo "DefaultKeyID=${ApDefKId%?}" >> $RTWIFI_PROFILE_PATH
	echo "Key1Type=${ApK1Tp%?}" >> $RTWIFI_PROFILE_PATH
	echo "Key2Type=${ApK2Tp%?}" >> $RTWIFI_PROFILE_PATH
	echo "Key3Type=${ApK3Tp%?}" >> $RTWIFI_PROFILE_PATH
	echo "Key4Type=${ApK4Tp%?}" >> $RTWIFI_PROFILE_PATH

#	WDS接口数目
	WDSBssidNum=0
	WDSEN=0
	WDSList=""
	WDSEncType=""
	WDSDefKeyID=""
	WDSPhyMode=""
	WDSTxMCS=""
	for_each_interface "wds" mt_dbdc_wds_vif_pre_config
	echo "WdsEnable=${WDSEN%?}" >> $RTWIFI_PROFILE_PATH
	echo "WdsList=${WDSList%?}" >> $RTWIFI_PROFILE_PATH
	echo "WdsEncrypType=${WdsEncType%?}" >> $RTWIFI_PROFILE_PATH
	echo "WdsDefaultKeyID=${WDSDefKeyID%?}" >> $RTWIFI_PROFILE_PATH
	echo "WdsPhyMode=${WDSPhyMode%?}" >> $RTWIFI_PROFILE_PATH
	echo "WdsTxMcs=${WDSTxMCS%?}" >> $RTWIFI_PROFILE_PATH

#接口上线
#加锁
	echo "Pending..."
	if lock -n $WIFI_OP_LOCK; then
		sleep 3
		RA_MAIN_UP=$(get_if_stat ra0)
		drv_mt_dbdc_teardown $phy_name
		RESET_IF=$(mt_dbdc_vif_down $phy_name)
		echo "MT_DBDC:ra0:$RA_MAIN_UP.Later we'll restart $(echo ${RESET_IF} | tr '\n' ' ')"
		sleep 1

#Start root device
		ifconfig ra0 up
#restore interfaces
		sh $RTWIFI_CMD_OPATH

		sh $RTWIFI_CMD_PATH
#重启HWNAT
		[ -d /sys/module/hw_nat ] && {
			/etc/init.d/hwacc restart
		}
	else
		echo "Wait other process"
		lock $WIFI_OP_LOCK
	fi
#AP模式
	ApIfCNT=0
	for_each_interface "ap" mt_dbdc_ap_vif_post_config
#WDS接口
	WDSBssidNum=0
	for_each_interface "wds" mt_dbdc_wds_vif_post_config
#STA模式
	stacount=0
	for_each_interface "sta" mt_dbdc_sta_vif_post_config

#设置无线上线
	wireless_set_up
#解锁
	lock -u $WIFI_OP_LOCK
}
add_driver mt_dbdc
