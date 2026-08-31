#!/bin/sh
. /lib/netifd/netifd-wireless.sh
. /lib/netifd/hostapd.sh
. /lib/functions/system.sh

init_wireless_driver "$@"

MP_CONFIG_INT="mesh_retry_timeout mesh_confirm_timeout mesh_holding_timeout mesh_max_peer_links
	       mesh_max_retries mesh_ttl mesh_element_ttl mesh_hwmp_max_preq_retries
	       mesh_path_refresh_time mesh_min_discovery_timeout mesh_hwmp_active_path_timeout
	       mesh_hwmp_preq_min_interval mesh_hwmp_net_diameter_traversal_time mesh_hwmp_rootmode
	       mesh_hwmp_rann_interval mesh_gate_announcements mesh_sync_offset_max_neighor
	       mesh_rssi_threshold mesh_hwmp_active_path_to_root_timeout mesh_hwmp_root_interval
	       mesh_hwmp_confirmation_interval mesh_awake_window mesh_plink_timeout"
MP_CONFIG_BOOL="mesh_auto_open_plinks mesh_fwding"
MP_CONFIG_STRING="mesh_power_mode"

wdev_tool() {
	ucode /usr/share/hostap/wdev.uc "$@"
}

ubus_call() {
	flock /var/run/hostapd.lock ubus call "$@"
}

drv_mac80211_init_device_config() {
	hostapd_common_add_device_config

	config_add_string path phy 'macaddr:macaddr'
	config_add_string tx_burst
	config_add_string distance
	config_add_string ifname_prefix
	config_add_string macaddr_base
	config_add_int radio beacon_int chanbw frag rts
	config_add_int rxantenna txantenna txpower min_tx_power
	config_add_int num_global_macaddr multiple_bssid
	config_add_boolean noscan ht_coex acs_exclude_dfs background_radar
	config_add_array ht_capab
	config_add_array channels
	config_add_array scan_list
	config_add_boolean \
		rxldpc \
		short_gi_80 \
		short_gi_160 \
		tx_stbc_2by1 \
		su_beamformer \
		su_beamformee \
		mu_beamformer \
		mu_beamformee \
		he_su_beamformer \
		he_su_beamformee \
		he_mu_beamformer \
		vht_txop_ps \
		htc_vht \
		rx_antenna_pattern \
		tx_antenna_pattern \
		he_spr_sr_control \
		he_spr_psr_enabled \
		he_bss_color_enabled \
		he_twt_required
	config_add_int \
		beamformer_antennas \
		beamformee_antennas \
		vht_max_a_mpdu_len_exp \
		vht_max_mpdu \
		vht_link_adapt \
		vht160 \
		rx_stbc \
		tx_stbc \
		he_bss_color \
		he_spr_non_srg_obss_pd_max_offset
	config_add_boolean \
		ldpc \
		greenfield \
		short_gi_20 \
		short_gi_40 \
		max_amsdu \
		dsss_cck_40
}

drv_mac80211_init_iface_config() {
	hostapd_common_add_bss_config

	config_add_string 'macaddr:macaddr' ifname

	config_add_boolean wds powersave enable
	config_add_string wds_bridge
	config_add_int maxassoc
	config_add_int max_listen_int
	config_add_int dtim_period
	config_add_int start_disabled

	# mesh
	config_add_string mesh_id
	config_add_int $MP_CONFIG_INT
	config_add_boolean $MP_CONFIG_BOOL
	config_add_string $MP_CONFIG_STRING
}

mac80211_add_capabilities() {
	local __var="$1"; shift
	local __mask="$1"; shift
	local __out= oifs

	oifs="$IFS"
	IFS=:
	for capab in "$@"; do
		set -- $capab

		[ "$(($4))" -gt 0 ] || continue
		[ "$(($__mask & $2))" -eq "$((${3:-$2}))" ] || continue
		__out="$__out[$1]"
	done
	IFS="$oifs"

	export -n -- "$__var=$__out"
}

mac80211_add_he_capabilities() {
	local __out= oifs

	oifs="$IFS"
	IFS=:
	for capab in "$@"; do
		set -- $capab
		[ "$(($4))" -gt 0 ] || continue
		[ "$(((0x$2) & $3))" -gt 0 ] || {
			eval "$1=0"
			continue
		}
		append base_cfg "$1=1" "$N"
	done
	IFS="$oifs"
}

mac80211_hostapd_setup_base() {
	local phy="$1"

	json_select config

	[ "$auto_channel" -gt 0 ] && channel=acs_survey

	[ "$auto_channel" -gt 0 ] && json_get_vars acs_exclude_dfs
	[ -n "$acs_exclude_dfs" ] && [ "$acs_exclude_dfs" -gt 0 ] &&
		append base_cfg "acs_exclude_dfs=1" "$N"

	json_get_vars noscan ht_coex min_tx_power:0 tx_burst
	json_get_values ht_capab_list ht_capab
	json_get_values channel_list channels

	[ "$auto_channel" = 0 ] && [ -z "$channel_list" ] && \
		channel_list="$channel"

	[ "$min_tx_power" -gt 0 ] && append base_cfg "min_tx_power=$min_tx_power" "$N"

	set_default noscan 0

	[ "$noscan" -gt 0 ] && hostapd_noscan=1
	[ "$tx_burst" = 0 ] && tx_burst=

	chan_ofs=0
	[ "$band" = "6g" ] && chan_ofs=1

	if [ "$band" != "6g" ]; then
		ieee80211n=1
		ht_capab=
		case "$htmode" in
			VHT20|HT20|HE20|EHT20) ;;
			HT40*|VHT40|VHT80|VHT160|HE40|HE80|HE160|EHT40|EHT80|EHT160)
				case "$hwmode" in
					a)
						case "$(( (($channel / 4) + $chan_ofs) % 2 ))" in
							1) ht_capab="[HT40+]";;
							0) ht_capab="[HT40-]";;
						esac
					;;
					*)
						case "$htmode" in
							HT40+) ht_capab="[HT40+]";;
							HT40-) ht_capab="[HT40-]";;
							*)
								if [ "$channel" -lt 7 ]; then
									ht_capab="[HT40+]"
								else
									ht_capab="[HT40-]"
								fi
							;;
						esac
					;;
				esac
				[ "$auto_channel" -gt 0 ] && ht_capab="[HT40+]"
			;;
			*) ieee80211n= ;;
		esac

		[ -n "$ieee80211n" ] && {
			append base_cfg "ieee80211n=1" "$N"

			set_default ht_coex 0
			append base_cfg "ht_coex=$ht_coex" "$N"

			json_get_vars \
				ldpc:1 \
				greenfield:0 \
				short_gi_20:1 \
				short_gi_40:1 \
				tx_stbc:1 \
				rx_stbc:3 \
				max_amsdu:1 \
				dsss_cck_40:1

			ht_cap_mask=0
			for cap in $(iw phy "$phy" info | grep -E '^\s*Capabilities:' | cut -d: -f2); do
				ht_cap_mask="$(($ht_cap_mask | $cap))"
			done

			cap_rx_stbc=$((($ht_cap_mask >> 8) & 3))
			[ "$rx_stbc" -lt "$cap_rx_stbc" ] && cap_rx_stbc="$rx_stbc"
			ht_cap_mask="$(( ($ht_cap_mask & ~(0x300)) | ($cap_rx_stbc << 8) ))"

			mac80211_add_capabilities ht_capab_flags $ht_cap_mask \
				LDPC:0x1::$ldpc \
				GF:0x10::$greenfield \
				SHORT-GI-20:0x20::$short_gi_20 \
				SHORT-GI-40:0x40::$short_gi_40 \
				TX-STBC:0x80::$tx_stbc \
				RX-STBC1:0x300:0x100:1 \
				RX-STBC12:0x300:0x200:1 \
				RX-STBC123:0x300:0x300:1 \
				MAX-AMSDU-7935:0x800::$max_amsdu \
				DSSS_CCK-40:0x1000::$dsss_cck_40

			ht_capab="$ht_capab$ht_capab_flags"
			[ -n "$ht_capab" ] && append base_cfg "ht_capab=$ht_capab" "$N"
		}
	fi

	# 802.11ac
	enable_ac=0
	vht_oper_chwidth=0
	vht_center_seg0=

	idx="$channel"
	case "$htmode" in
		VHT20|HE20|EHT20) enable_ac=1;;
		VHT40|HE40|EHT40)
			case "$(( (($channel / 4) + $chan_ofs) % 2 ))" in
				1) idx=$(($channel + 2));;
				0) idx=$(($channel - 2));;
			esac
			enable_ac=1
			vht_center_seg0=$idx
		;;
		VHT80|HE80|EHT80)
			case "$(( (($channel / 4) + $chan_ofs) % 4 ))" in
				1) idx=$(($channel + 6));;
				2) idx=$(($channel + 2));;
				3) idx=$(($channel - 2));;
				0) idx=$(($channel - 6));;
			esac
			enable_ac=1
			vht_oper_chwidth=1
			vht_center_seg0=$idx
		;;
		VHT160|HE160|EHT160|EHT320)
			if [ "$band" = "6g" ]; then
				case "$channel" in
					1|5|9|13|17|21|25|29) idx=15;;
					33|37|41|45|49|53|57|61) idx=47;;
					65|69|73|77|81|85|89|93) idx=79;;
					97|101|105|109|113|117|121|125) idx=111;;
					129|133|137|141|145|149|153|157) idx=143;;
					161|165|169|173|177|181|185|189) idx=175;;
					193|197|201|205|209|213|217|221) idx=207;;
				esac
			else
				case "$channel" in
					36|40|44|48|52|56|60|64) idx=50;;
					100|104|108|112|116|120|124|128) idx=114;;
					149|153|157|161|165|169|173|177) idx=163;;
				esac
			fi
			enable_ac=1
			vht_oper_chwidth=2
			vht_center_seg0=$idx
		;;
	esac
	[ "$band" = "5g" ] && {
		json_get_vars background_radar:0

		[ "$background_radar" -eq 1 ] && append base_cfg "enable_background_radar=1" "$N"
	}

	eht_oper_chwidth=$vht_oper_chwidth
	eht_center_seg0=$vht_center_seg0

	[ "$band" = "6g" ] && {
		op_class=
		case "$htmode" in
			HE20|EHT20) op_class=131;;
			EHT320)
				case "$channel" in
					1|5|9|13|17|21|25|29|33|37|41|45|49|53|57|61) idx=31;;
					65|69|73|77|81|85|89|93|97|101|105|109|113|117|121|125) idx=95;;
					129|133|137|141|145|149|153|157|161|165|169|173|177|181|185|189) idx=159;;
					193|197|201|205|209|213|217|221) idx=191;;
				esac

				op_class=137
				eht_center_seg0=$idx
				eht_oper_chwidth=9
			;;
			HE*|EHT*) op_class=$((132 + $vht_oper_chwidth));;
		esac
		[ -n "$op_class" ] && append base_cfg "op_class=$op_class" "$N"
	}
	[ "$hwmode" = "a" ] || enable_ac=0
	[ "$band" = "6g" ] && enable_ac=0

	if [ "$enable_ac" != "0" ]; then
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
			beamformee_antennas:4 \
			beamformer_antennas:4 \
			rx_antenna_pattern:1 \
			tx_antenna_pattern:1 \
			vht_max_a_mpdu_len_exp:7 \
			vht_max_mpdu:11454 \
			rx_stbc:4 \
			vht_link_adapt:3 \
			vht160:2

		set_default tx_burst 2.0
		append base_cfg "ieee80211ac=1" "$N"
		vht_cap=0
		for cap in $(iw phy "$phy" info | awk -F "[()]" '/VHT Capabilities/ { print $2 }'); do
			vht_cap="$(($vht_cap | $cap))"
		done

		append base_cfg "vht_oper_chwidth=$vht_oper_chwidth" "$N"
		append base_cfg "vht_oper_centr_freq_seg0_idx=$vht_center_seg0" "$N"

		cap_rx_stbc=$((($vht_cap >> 8) & 7))
		[ "$rx_stbc" -lt "$cap_rx_stbc" ] && cap_rx_stbc="$rx_stbc"
		vht_cap="$(( ($vht_cap & ~(0x700)) | ($cap_rx_stbc << 8) ))"

		[ "$vht_oper_chwidth" -lt 2 ] && {
			vht160=0
			short_gi_160=0
		}

		mac80211_add_capabilities vht_capab $vht_cap \
			RXLDPC:0x10::$rxldpc \
			SHORT-GI-80:0x20::$short_gi_80 \
			SHORT-GI-160:0x40::$short_gi_160 \
			TX-STBC-2BY1:0x80::$tx_stbc_2by1 \
			SU-BEAMFORMER:0x800::$su_beamformer \
			SU-BEAMFORMEE:0x1000::$su_beamformee \
			MU-BEAMFORMER:0x80000::$mu_beamformer \
			MU-BEAMFORMEE:0x100000::$mu_beamformee \
			VHT-TXOP-PS:0x200000::$vht_txop_ps \
			HTC-VHT:0x400000::$htc_vht \
			RX-ANTENNA-PATTERN:0x10000000::$rx_antenna_pattern \
			TX-ANTENNA-PATTERN:0x20000000::$tx_antenna_pattern \
			RX-STBC-1:0x700:0x100:1 \
			RX-STBC-12:0x700:0x200:1 \
			RX-STBC-123:0x700:0x300:1 \
			RX-STBC-1234:0x700:0x400:1 \

		[ "$(($vht_cap & 0x800))" -gt 0 -a "$su_beamformer" -gt 0 ] && {
			cap_ant="$(( ( ($vht_cap >> 16) & 3 ) + 1 ))"
			[ "$cap_ant" -gt "$beamformer_antennas" ] && cap_ant="$beamformer_antennas"
			[ "$cap_ant" -gt 1 ] && vht_capab="$vht_capab[SOUNDING-DIMENSION-$cap_ant]"
		}

		[ "$(($vht_cap & 0x1000))" -gt 0 -a "$su_beamformee" -gt 0 ] && {
			cap_ant="$(( ( ($vht_cap >> 13) & 3 ) + 1 ))"
			[ "$cap_ant" -gt "$beamformee_antennas" ] && cap_ant="$beamformee_antennas"
			[ "$cap_ant" -gt 1 ] && vht_capab="$vht_capab[BF-ANTENNA-$cap_ant]"
		}

		# supported Channel widths
		vht160_hw=0
		[ "$(($vht_cap & 12))" -eq 4 -a 1 -le "$vht160" ] && \
			vht160_hw=1
		[ "$(($vht_cap & 12))" -eq 8 -a 2 -le "$vht160" ] && \
			vht160_hw=2
		[ "$vht160_hw" = 1 ] && vht_capab="$vht_capab[VHT160]"
		[ "$vht160_hw" = 2 ] && vht_capab="$vht_capab[VHT160-80PLUS80]"

		# maximum MPDU length
		vht_max_mpdu_hw=3895
		[ "$(($vht_cap & 3))" -ge 1 -a 7991 -le "$vht_max_mpdu" ] && \
			vht_max_mpdu_hw=7991
		[ "$(($vht_cap & 3))" -ge 2 -a 11454 -le "$vht_max_mpdu" ] && \
			vht_max_mpdu_hw=11454
		[ "$vht_max_mpdu_hw" != 3895 ] && \
			vht_capab="$vht_capab[MAX-MPDU-$vht_max_mpdu_hw]"

		# maximum A-MPDU length exponent
		vht_max_a_mpdu_len_exp_hw=0
		[ "$(($vht_cap & 58720256))" -ge 8388608 -a 1 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=1
		[ "$(($vht_cap & 58720256))" -ge 16777216 -a 2 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=2
		[ "$(($vht_cap & 58720256))" -ge 25165824 -a 3 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=3
		[ "$(($vht_cap & 58720256))" -ge 33554432 -a 4 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=4
		[ "$(($vht_cap & 58720256))" -ge 41943040 -a 5 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=5
		[ "$(($vht_cap & 58720256))" -ge 50331648 -a 6 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=6
		[ "$(($vht_cap & 58720256))" -ge 58720256 -a 7 -le "$vht_max_a_mpdu_len_exp" ] && \
			vht_max_a_mpdu_len_exp_hw=7
		vht_capab="$vht_capab[MAX-A-MPDU-LEN-EXP$vht_max_a_mpdu_len_exp_hw]"

		# whether or not the STA supports link adaptation using VHT variant
		vht_link_adapt_hw=0
		[ "$(($vht_cap & 201326592))" -ge 134217728 -a 2 -le "$vht_link_adapt" ] && \
			vht_link_adapt_hw=2
		[ "$(($vht_cap & 201326592))" -ge 201326592 -a 3 -le "$vht_link_adapt" ] && \
			vht_link_adapt_hw=3
		[ "$vht_link_adapt_hw" != 0 ] && \
			vht_capab="$vht_capab[VHT-LINK-ADAPT-$vht_link_adapt_hw]"

		[ -n "$vht_capab" ] && append base_cfg "vht_capab=$vht_capab" "$N"
	fi

	# 802.11ax
	enable_ax=0
	enable_be=0
	case "$htmode" in
		HE*) enable_ax=1 ;;
		EHT*) enable_ax=1; enable_be=1 ;;
	esac

	if [ "$enable_ax" != "0" ]; then
		json_get_vars \
			he_su_beamformer:1 \
			he_su_beamformee:1 \
			he_mu_beamformer:1 \
			he_twt_required:0 \
			he_spr_sr_control:3 \
			he_spr_psr_enabled:0 \
			he_spr_non_srg_obss_pd_max_offset:0 \
			he_bss_color:128 \
			he_bss_color_enabled:1

		he_phy_cap=$(iw phy "$phy" info | sed -n '/HE Iftypes: .*AP/,$p' | awk -F "[()]" '/HE PHY Capabilities/ { print $2 }' | head -1)
		he_phy_cap=${he_phy_cap:2}
		he_mac_cap=$(iw phy "$phy" info | sed -n '/HE Iftypes: .*AP/,$p' | awk -F "[()]" '/HE MAC Capabilities/ { print $2 }' | head -1)
		he_mac_cap=${he_mac_cap:2}

		append base_cfg "ieee80211ax=1" "$N"
		[ "$hwmode" = "a" ] && {
			append base_cfg "he_oper_chwidth=$vht_oper_chwidth" "$N"
			append base_cfg "he_oper_centr_freq_seg0_idx=$vht_center_seg0" "$N"
		}

		mac80211_add_he_capabilities \
			he_su_beamformer:${he_phy_cap:6:2}:0x80:$he_su_beamformer \
			he_su_beamformee:${he_phy_cap:8:2}:0x1:$he_su_beamformee \
			he_mu_beamformer:${he_phy_cap:8:2}:0x2:$he_mu_beamformer \
			he_twt_required:${he_mac_cap:0:2}:0x6:$he_twt_required

		if [ "$he_bss_color_enabled" -gt 0 ]; then
			append base_cfg "he_bss_color=$he_bss_color" "$N"
			[ "$he_spr_non_srg_obss_pd_max_offset" -gt 0 ] && { \
				append base_cfg "he_spr_non_srg_obss_pd_max_offset=$he_spr_non_srg_obss_pd_max_offset" "$N"
				he_spr_sr_control=$((he_spr_sr_control | (1 << 2)))
			}
			[ "$he_spr_psr_enabled" -gt 0 ] && he_spr_psr_enabled=$((0x${he_phy_cap:14:2} & 0x1))
			[ "$he_spr_psr_enabled" -gt 0 ] || he_spr_sr_control=$((he_spr_sr_control | (1 << 0)))
			append base_cfg "he_spr_sr_control=$he_spr_sr_control" "$N"
		else
			append base_cfg "he_bss_color_disabled=1" "$N"
		fi


		append base_cfg "he_default_pe_duration=4" "$N"
		append base_cfg "he_rts_threshold=1023" "$N"
		append base_cfg "he_mu_edca_qos_info_param_count=0" "$N"
		append base_cfg "he_mu_edca_qos_info_q_ack=0" "$N"
		append base_cfg "he_mu_edca_qos_info_queue_request=0" "$N"
		append base_cfg "he_mu_edca_qos_info_txop_request=0" "$N"
		append base_cfg "he_mu_edca_ac_be_aifsn=8" "$N"
		append base_cfg "he_mu_edca_ac_be_aci=0" "$N"
		append base_cfg "he_mu_edca_ac_be_ecwmin=9" "$N"
		append base_cfg "he_mu_edca_ac_be_ecwmax=10" "$N"
		append base_cfg "he_mu_edca_ac_be_timer=255" "$N"
		append base_cfg "he_mu_edca_ac_bk_aifsn=15" "$N"
		append base_cfg "he_mu_edca_ac_bk_aci=1" "$N"
		append base_cfg "he_mu_edca_ac_bk_ecwmin=9" "$N"
		append base_cfg "he_mu_edca_ac_bk_ecwmax=10" "$N"
		append base_cfg "he_mu_edca_ac_bk_timer=255" "$N"
		append base_cfg "he_mu_edca_ac_vi_ecwmin=5" "$N"
		append base_cfg "he_mu_edca_ac_vi_ecwmax=7" "$N"
		append base_cfg "he_mu_edca_ac_vi_aifsn=5" "$N"
		append base_cfg "he_mu_edca_ac_vi_aci=2" "$N"
		append base_cfg "he_mu_edca_ac_vi_timer=255" "$N"
		append base_cfg "he_mu_edca_ac_vo_aifsn=5" "$N"
		append base_cfg "he_mu_edca_ac_vo_aci=3" "$N"
		append base_cfg "he_mu_edca_ac_vo_ecwmin=5" "$N"
		append base_cfg "he_mu_edca_ac_vo_ecwmax=7" "$N"
		append base_cfg "he_mu_edca_ac_vo_timer=255" "$N"
	fi

	if [ "$enable_be" != "0" ]; then
		append base_cfg "ieee80211be=1" "$N"
		[ "$hwmode" = "a" ] && {
			append base_cfg "eht_oper_chwidth=$eht_oper_chwidth" "$N"
			append base_cfg "eht_oper_centr_freq_seg0_idx=$eht_center_seg0" "$N"
		}
	fi

	hostapd_prepare_device_config "$hostapd_conf_file" nl80211
	cat >> "$hostapd_conf_file" <<EOF
${channel:+channel=$channel}
${channel_list:+chanlist=$channel_list}
${hostapd_noscan:+noscan=1}
${tx_burst:+tx_queue_data2_burst=$tx_burst}
${multiple_bssid:+mbssid=$multiple_bssid}
#num_global_macaddr=$num_global_macaddr
#macaddr_base=$macaddr_base
$base_cfg

EOF
	json_select ..
}

mac80211_hostapd_setup_bss() {
	local phy="$1"
	local ifname="$2"
	local macaddr="$3"
	local type="$4"

	hostapd_cfg=
	append hostapd_cfg "$type=$ifname" "$N"

	hostapd_set_bss_options hostapd_cfg "$phy" "$vif" || return 1
	json_get_vars wds wds_bridge dtim_period max_listen_int start_disabled

	set_default wds 0
	set_default start_disabled 0

	[ "$wds" -gt 0 ] && {
		append hostapd_cfg "wds_sta=1" "$N"
		[ -n "$wds_bridge" ] && append hostapd_cfg "wds_bridge=$wds_bridge" "$N"
	}
	[ "$staidx" -gt 0 -o "$start_disabled" -eq 1 ] && append hostapd_cfg "start_disabled=1" "$N"

	cat >> /var/run/hostapd-$phy$vif_phy_suffix.conf <<EOF
$hostapd_cfg
bssid=$macaddr
${default_macaddr:+#default_macaddr}
${random_macaddr:+#random_macaddr}
${dtim_period:+dtim_period=$dtim_period}
${max_listen_int:+max_listen_interval=$max_listen_int}
EOF
}

mac80211_get_addr() {
	local phy="$1"
	local idx="$(($2 + 1))"

	head -n $idx /sys/class/ieee80211/${phy}/addresses | tail -n1
}

mac80211_generate_mac() {
	local phy="$1"
	local id="${macidx:-0}"

	wdev_tool "$phy$phy_suffix" get_macaddr id=$id num_global=$num_global_macaddr mbssid=${multiple_bssid:-0} macaddr_base=${macaddr_base}
}

get_board_phy_name() (
	local path="$1"
	local fallback_phy=""

	__check_phy() {
		local val="$1"
		local key="$2"
		local ref_path="$3"
		local type paths path

		json_select "$key"
		json_get_type type path
		if [ "$type" = array ]; then
			json_get_values paths path
		else
			json_get_vars path
			paths="$path"
		fi
		json_select ..

		for path in $paths; do
			[ "${ref_path%+*}" = "${path%+*}" ] && fallback_phy=$key
			[ "$ref_path" = "$path" ] || continue

			echo "$key"
			exit
		done
	}

	json_load_file /etc/board.json
	json_for_each_item __check_phy wlan "$path"
	[ -n "$fallback_phy" ] && echo "${fallback_phy}.${path##*+}"
)

rename_board_phy_by_path() {
	local path="$1"

	local new_phy="$(get_board_phy_name "$path")"
	[ -z "$new_phy" -o "$new_phy" = "$phy" ] && return

	iw "$phy" set name "$new_phy" && phy="$new_phy"
}

rename_board_phy_by_name() (
	local phy="$1"
	local suffix="${phy##*.}"
	[ "$suffix" = "$phy" ] && suffix=

	json_load_file /etc/board.json
	json_select wlan
	json_select "${phy%.*}" || return 0

	local type paths path
	json_get_type type path
	if [ "$type" = array ]; then
		json_get_values paths path
	else
		json_get_vars path
		paths="$path"
	fi

	local prev_phy=
	for path in $paths; do
		prev_phy="$(iwinfo nl80211 phyname "path=$path${suffix:++$suffix}")"
		[ -n "$prev_phy" ] && break
	done
	[ -n "$prev_phy" ] || return 0

	[ "$prev_phy" = "$phy" ] && return 0

	iw "$prev_phy" set name "$phy"
)

find_phy() {
	[ -n "$phy" ] && {
		rename_board_phy_by_name "$phy"
		[ -d /sys/class/ieee80211/$phy ] && return 0
	}
	[ -n "$path" ] && {
		phy="$(iwinfo nl80211 phyname "path=$path")"
		[ -n "$phy" ] && {
			rename_board_phy_by_path "$path"
			return 0
		}
	}
	[ -n "$macaddr" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			grep -i -q "$macaddr" "/sys/class/ieee80211/${phy}/macaddress" && {
				path="$(iwinfo nl80211 path "$phy")"
				rename_board_phy_by_path "$path"
				return 0
			}
		done
	}
	return 1
}

mac80211_check_ap() {
	has_ap=1
}

mac80211_set_ifname() {
	local prefix="$1"
	local type="$2"
	eval "ifname=\"$prefix$type\${idx_$type:-0}\"; idx_$type=\$((\${idx_$type:-0 } + 1))"
}

mac80211_prepare_vif() {
	json_select config

	json_get_vars ifname mode ssid wds powersave macaddr enable wpa_psk_file sae_password_file vlan_file

	[ -n "$ifname" ] || {
		local prefix;

		case "$mode" in
		ap|sta|mesh) prefix=$mode;;
		adhoc) prefix=ibss;;
		monitor) prefix=mon;;
		esac

		mac80211_set_ifname "$ifname_prefix" "$prefix"
	}

	append active_ifnames "$ifname"
	set_default wds 0
	set_default powersave 0
	json_add_string _ifname "$ifname"

	default_macaddr=
	random_macaddr=
	if [ -z "$macaddr" ]; then
		macaddr="$(mac80211_generate_mac $phy)"
		macidx="$(($macidx + 1))"
		default_macaddr=1
	elif [ "$macaddr" = 'random' ]; then
		macaddr="$(macaddr_random)"
		random_macaddr=1
	fi
	json_add_string _macaddr "$macaddr"
	json_add_string _default_macaddr "$default_macaddr"
	json_select ..


	[ "$mode" == "ap" ] && {
		json_select config
		wireless_vif_parse_encryption
		json_select ..

		[ -z "$wpa_psk_file" ] && hostapd_set_psk "$ifname"
		[ -z "$sae_password_file" ] && hostapd_set_sae "$ifname"
		[ -z "$vlan_file" ] && hostapd_set_vlan "$ifname"
	}

	json_select config

	# It is far easier to delete and create the desired interface
	case "$mode" in
		ap)
			# Hostapd will handle recreating the interface and
			# subsequent virtual APs belonging to the same PHY
			if [ -n "$hostapd_ctrl" ]; then
				type=bss
			else
				type=interface
			fi

			mac80211_hostapd_setup_bss "$phy" "$ifname" "$macaddr" "$type" || return

			[ -n "$hostapd_ctrl" ] || {
				ap_ifname="${ifname}"
				hostapd_ctrl="${hostapd_ctrl:-/var/run/hostapd/$ifname}"
			}
		;;
	esac

	json_select ..
}

mac80211_prepare_iw_htmode() {
	case "$htmode" in
		HT20|VHT20|HE20|EHT20)
			iw_htmode=HT20
		;;
		HT40*|VHT40|HE40|EHT40)
			case "$band" in
				2g)
					case "$htmode" in
						HT40+) iw_htmode="HT40+";;
						HT40-) iw_htmode="HT40-";;
						*)
							if [ "$channel" -lt 7 ]; then
								iw_htmode="HT40+"
							else
								iw_htmode="HT40-"
							fi
						;;
					esac
				;;
				*)
					case "$(( ($channel / 4) % 2 ))" in
						1) iw_htmode="HT40+" ;;
						0) iw_htmode="HT40-";;
					esac
				;;
			esac
			[ "$auto_channel" -gt 0 ] && iw_htmode="HT40+"
		;;
		VHT80|HE80|EHT80)
			iw_htmode="80MHz"
		;;
		VHT160|HE160|EHT160)
			iw_htmode="160MHz"
		;;
		NONE|NOHT)
			iw_htmode="NOHT"
		;;
		*) iw_htmode="" ;;
	esac
}

mac80211_add_mesh_params() {
	for var in $MP_CONFIG_INT $MP_CONFIG_BOOL $MP_CONFIG_STRING; do
		eval "mp_val=\"\$$var\""
		[ -n "$mp_val" ] && json_add_string "$var" "$mp_val"
	done
}

mac80211_setup_adhoc() {
	local enable=$1
	json_get_vars bssid ssid key mcast_rate

	NEWUMLIST="${NEWUMLIST}$ifname "

	[ "$enable" = 0 ] && {
		ip link set dev "$ifname" down
		return 0
	}

	keyspec=
	[ "$auth_type" = "wep" ] && {
		set_default key 1
		case "$key" in
			[1234])
				local idx
				for idx in 1 2 3 4; do
					json_get_var ikey "key$idx"

					[ -n "$ikey" ] && {
						ikey="$(($idx - 1)):$(prepare_key_wep "$ikey")"
						[ $idx -eq $key ] && ikey="d:$ikey"
						append keyspec "$ikey"
					}
				done
			;;
			*)
				append keyspec "d:0:$(prepare_key_wep "$key")"
			;;
		esac
	}

	brstr=
	for br in $basic_rate_list; do
		wpa_supplicant_add_rate brstr "$br"
	done

	mcval=
	[ -n "$mcast_rate" ] && wpa_supplicant_add_rate mcval "$mcast_rate"

	local prev
	json_set_namespace wdev_uc prev

	json_add_object "$ifname"
	json_add_string mode adhoc
	[ -n "$default_macaddr" ] || json_add_string macaddr "$macaddr"
	json_add_string ssid "$ssid"
	json_add_string freq "$freq"
	json_add_string htmode "$iw_htmode"
	[ -n "$bssid" ] && json_add_string bssid "$bssid"
	json_add_int beacon-interval "$beacon_int"
	[ -n "$brstr" ] && json_add_string basic-rates "$brstr"
	[ -n "$mcval" ] && json_add_string mcast-rate "$mcval"
	[ -n "$keyspec" ] && json_add_string keys "$keyspec"
	json_close_object

	json_set_namespace "$prev"
}

mac80211_setup_mesh() {
	json_get_vars ssid mesh_id mcast_rate
	json_get_values iface_basic_rate_list basic_rate

	mcval=
	[ -n "$mcast_rate" ] && wpa_supplicant_add_rate mcval "$mcast_rate"
	[ -n "$mesh_id" ] && ssid="$mesh_id"

	br_list="$basic_rate_list"
	if [ -n "$iface_basic_rate_list" ]; then
		br_list="$iface_basic_rate_list"
	fi

	brstr=
	for br in $br_list; do
		wpa_supplicant_add_rate brstr "$br"
	done

	local prev
	json_set_namespace wdev_uc prev

	json_add_object "$ifname"
	json_add_string mode mesh
	[ -n "$default_macaddr" ] || json_add_string macaddr "$macaddr"
	json_add_string ssid "$ssid"
	json_add_string freq "$freq"
	json_add_string htmode "$iw_htmode"
	[ -n "$mcval" ] && json_add_string mcast-rate "$mcval"
	[ -n "$brstr" ] && json_add_string basic-rates "$brstr"
	json_add_int beacon-interval "$beacon_int"
	mac80211_add_mesh_params

	json_close_object

	json_set_namespace "$prev"
}

mac80211_setup_monitor() {
	local prev
	json_set_namespace wdev_uc prev

	json_add_object "$ifname"
	json_add_string mode monitor
	[ -n "$freq" ] && json_add_string freq "$freq"
	json_add_string htmode "$iw_htmode"
	json_close_object

	json_set_namespace "$prev"
}

mac80211_set_vif_txpower() {
	local name="$1"

	json_select config
	json_get_var ifname _ifname
	json_get_vars vif_txpower
	json_select ..

	set_default vif_txpower "$txpower"
	if [ -n "$vif_txpower" ]; then
		iw dev "$ifname" set txpower fixed "${vif_txpower%%.*}00"
	else
		iw dev "$ifname" set txpower auto
	fi
}

wpa_supplicant_init_config() {
	json_set_namespace wpa_supp prev

	json_init
	json_add_array config

	json_set_namespace "$prev"
}

wpa_supplicant_add_interface() {
	local ifname="$1"
	local mode="$2"
	local prev

	_wpa_supplicant_common "$ifname"

	json_set_namespace wpa_supp prev

	json_add_object
	json_add_string ctrl "$_rpath"
	json_add_string iface "$ifname"
	json_add_string mode "$mode"
	json_add_string config "$_config"
	[ -n "$default_macaddr" ] || json_add_string macaddr "$macaddr"
	[ -n "$network_bridge" ] && json_add_string bridge "$network_bridge"
	[ -n "$wds" ] && json_add_boolean 4addr "$wds"
	json_add_boolean powersave "$powersave"
	[ "$mode" = "mesh" ] && mac80211_add_mesh_params
	json_close_object

	json_set_namespace "$prev"

	wpa_supp_init=1
}

wpa_supplicant_set_config() {
	local phy="$1"
	local radio="$2"
	local prev

	json_set_namespace wpa_supp prev
	json_close_array
	json_add_string phy "$phy"
	json_add_int radio "$radio"
	json_add_int num_global_macaddr "$num_global_macaddr"
	json_add_string macaddr_base "$macaddr_base"
	json_add_boolean defer 1
	local data="$(json_dump)"

	json_cleanup
	json_set_namespace "$prev"

	ubus -S -t 0 wait_for wpa_supplicant || {
		[ -n "$wpa_supp_init" ] || return 0

		ubus wait_for wpa_supplicant
	}

	local supplicant_res="$(ubus_call wpa_supplicant config_set "$data")"
	ret="$?"
	[ "$ret" != 0 -o -z "$supplicant_res" ] && wireless_setup_vif_failed WPA_SUPPLICANT_FAILED

	wireless_add_process "$(jsonfilter -s "$supplicant_res" -l 1 -e @.pid)" "/usr/sbin/wpa_supplicant" 1 1

}

hostapd_set_config() {
	local phy="$1"
	local radio="$2"

	[ -n "$hostapd_ctrl" ] || {
		ubus_call hostapd config_set '{ "phy": "'"$phy"'", "radio": '"$radio"', "config": "", "prev_config": "'"${hostapd_conf_file}.prev"'" }' > /dev/null
		return 0;
	}

	ubus wait_for hostapd
	local hostapd_res="$(ubus_call hostapd config_set "{ \"phy\": \"$phy\", \"radio\": $radio, \"config\":\"${hostapd_conf_file}\", \"prev_config\": \"${hostapd_conf_file}.prev\"}")"
	ret="$?"
	[ "$ret" != 0 -o -z "$hostapd_res" ] && {
		wireless_setup_failed HOSTAPD_START_FAILED
		return
	}
	wireless_add_process "$(jsonfilter -s "$hostapd_res" -l 1 -e @.pid)" "/usr/sbin/hostapd" 1 1
}


wpa_supplicant_start() {
	local phy="$1"
	local radio="$2"

	[ -n "$wpa_supp_init" ] || return 0

	ubus_call wpa_supplicant config_set '{ "phy": "'"$phy"'", "radio": '"$radio"', "num_global_macaddr": '"$num_global_macaddr"', "macaddr_base": "'"$macaddr_base"'" }' > /dev/null
}

mac80211_setup_supplicant() {
	local enable=$1
	local add_sp=0

	wpa_supplicant_prepare_interface "$ifname" nl80211 || return 1

	if [ "$mode" = "sta" ]; then
		wpa_supplicant_add_network "$ifname" "" "$htmode"
	else
		wpa_supplicant_add_network "$ifname" "$freq" "$htmode" "$hostapd_noscan"
	fi

	wpa_supplicant_add_interface "$ifname" "$mode"

	return 0
}

mac80211_setup_vif() {
	local name="$1"
	local failed

	json_select config
	json_get_var ifname _ifname
	json_get_var macaddr _macaddr
	json_get_var default_macaddr _default_macaddr
	json_get_vars mode wds powersave

	set_default powersave 0
	set_default wds 0

	case "$mode" in
		mesh)
			json_get_vars $MP_CONFIG_INT $MP_CONFIG_BOOL $MP_CONFIG_STRING
			wireless_vif_parse_encryption
			[ -z "$htmode" ] && htmode="NOHT";
			if [ -x /usr/sbin/wpa_supplicant ] && wpa_supplicant -vmesh; then
				mac80211_setup_supplicant || failed=1
			else
				mac80211_setup_mesh
			fi
		;;
		adhoc)
			wireless_vif_parse_encryption
			if [ "$wpa" -gt 0 -o "$auto_channel" -gt 0 ]; then
				mac80211_setup_supplicant || failed=1
			else
				mac80211_setup_adhoc
			fi
		;;
		sta)
			mac80211_setup_supplicant || failed=1
		;;
		monitor)
			mac80211_setup_monitor
		;;
	esac

	json_select ..
	[ -n "$failed" ] || wireless_add_vif "$name" "$ifname"
}

get_freq() {
	local phy="$1"
	local channel="$2"
	local band="$3"

	case "$band" in
		2g) band="1:";;
		5g) band="2:";;
		60g) band="3:";;
		6g) band="4:";;
	esac

	iw "$phy" info | awk -v band="$band" -v channel="[$channel]" '

$1 ~ /Band/ {
	band_match = band == $2
}

band_match && $3 == "MHz" && $4 == channel {
	print int($2)
	exit
}
'
}

chan_is_dfs() {
	local phy="$1"
	local chan="$2"
	iw "$phy" info | grep -E -m1 "(\* ${chan:-....} MHz${chan:+|\\[$chan\\]})" | grep -q "MHz.*radar detection"
	return $!
}

mac80211_set_noscan() {
	hostapd_noscan=1
}

drv_mac80211_cleanup() {
	:
}

mac80211_reset_config() {
	hostapd_conf_file="/var/run/hostapd-$phy$vif_phy_suffix.conf"
	ubus_call hostapd config_set '{ "phy": "'"$phy"'", "radio": '"$radio"', "config": "", "prev_config": "'"$hostapd_conf_file"'" }' > /dev/null
	ubus_call wpa_supplicant config_set '{ "phy": "'"$phy"'", "radio": '"$radio"', "config": [] }' > /dev/null
	wdev_tool "$phy$phy_suffix" set_config '{}'
}

mac80211_set_suffix() {
	[ "$radio" = "-1" ] && radio=
	phy_suffix="${radio:+:$radio}"
	vif_phy_suffix="${radio:+.$radio}"
	set_default radio -1
}

drv_mac80211_setup() {
	json_select config
	json_get_vars \
		radio phy macaddr path \
		country chanbw distance \
		txpower \
		rxantenna txantenna \
		frag rts beacon_int:100 htmode \
		num_global_macaddr:1 multiple_bssid \
		ifname_prefix macaddr_base
	json_get_values basic_rate_list basic_rate
	json_get_values scan_list scan_list
	json_select ..

	mac80211_set_suffix

	json_select data && {
		json_get_var prev_rxantenna rxantenna
		json_get_var prev_txantenna txantenna
		json_select ..
	}

	find_phy || {
		echo "Could not find PHY for device '$1'"
		wireless_set_retry 0
		return 1
	}

	set_default ifname_prefix "$phy$vif_phy_suffix-"

	local wdev
	local cwdev
	local found

	# convert channel to frequency
	[ "$auto_channel" -gt 0 ] || freq="$(get_freq "$phy" "$channel" "$band")"

	[ -n "$country" ] && {
		iw reg get | grep -q "^country $country:" || {
			iw reg set "$country"
			sleep 1
		}
	}

	hostapd_conf_file="/var/run/hostapd-$phy$vif_phy_suffix.conf"

	macidx=0
	staidx=0

	[ -n "$chanbw" ] && {
		for file in /sys/kernel/debug/ieee80211/$phy/ath9k*/chanbw /sys/kernel/debug/ieee80211/$phy/ath5k/bwmode; do
			[ -f "$file" ] && echo "$chanbw" > "$file"
		done
	}

	set_default rxantenna 0xffffffff
	set_default txantenna 0xffffffff
	set_default distance 0

	[ "$txantenna" = "all" ] && txantenna=0xffffffff
	[ "$rxantenna" = "all" ] && rxantenna=0xffffffff

	[ "$rxantenna" = "$prev_rxantenna" -a "$txantenna" = "$prev_txantenna" ] || mac80211_reset_config "$phy"
	wireless_set_data phy="$phy" radio="$radio" txantenna="$txantenna" rxantenna="$rxantenna"

	iw phy "$phy" set antenna $txantenna $rxantenna >/dev/null 2>&1
	iw phy "$phy" set distance "$distance" >/dev/null 2>&1

	[ -n "$frag" ] && iw phy "$phy" set frag "${frag%%.*}"
	[ -n "$rts" ] && iw phy "$phy" set rts "${rts%%.*}"

	has_ap=
	hostapd_ctrl=
	ap_ifname=
	hostapd_noscan=
	wpa_supp_init=
	for_each_interface "ap" mac80211_check_ap

	[ -f "$hostapd_conf_file" ] && mv "$hostapd_conf_file" "$hostapd_conf_file.prev"

	for_each_interface "sta adhoc mesh" mac80211_set_noscan
	[ -n "$has_ap" ] && mac80211_hostapd_setup_base "$phy"

	local prev
	json_set_namespace wdev_uc prev
	json_init
	json_set_namespace "$prev"

	wpa_supplicant_init_config

	mac80211_prepare_iw_htmode
	active_ifnames=
	for_each_interface "ap sta adhoc mesh monitor" mac80211_prepare_vif
	for_each_interface "ap sta adhoc mesh monitor" mac80211_setup_vif

	[ -x /usr/sbin/wpa_supplicant ] && wpa_supplicant_set_config "$phy" "$radio"
	[ -x /usr/sbin/hostapd ] && hostapd_set_config "$phy" "$radio"

	[ -x /usr/sbin/wpa_supplicant ] && wpa_supplicant_start "$phy" "$radio"

	json_set_namespace wdev_uc prev
	wdev_tool "$phy$phy_suffix" set_config "$(json_dump)" $active_ifnames
	json_set_namespace "$prev"

	[ -z "$phy_suffix" ] && {
		if [ -n "$txpower" ]; then
			iw phy "$phy" set txpower fixed "${txpower%%.*}00"
		else
			iw phy "$phy" set txpower auto
		fi
	}

	for_each_interface "ap sta adhoc mesh monitor" mac80211_set_vif_txpower
	wireless_set_up
}

_list_phy_interfaces() {
	local phy="$1"
	if [ -d "/sys/class/ieee80211/${phy}/device/net" ]; then
		ls "/sys/class/ieee80211/${phy}/device/net" 2>/dev/null;
	else
		ls "/sys/class/ieee80211/${phy}/device" 2>/dev/null | grep net: | sed -e 's,net:,,g'
	fi
}

list_phy_interfaces() {
	local phy="$1"

	for dev in $(_list_phy_interfaces "$phy"); do
		readlink "/sys/class/net/${dev}/phy80211" | grep -q "/${phy}\$" || continue
		echo "$dev"
	done
}

drv_mac80211_teardown() {
	json_select data
	json_get_vars phy radio
	json_select ..
	[ -n "$phy" ] || {
		echo "Bug: PHY is undefined for device '$1'"
		return 1
	}

	mac80211_set_suffix
	mac80211_reset_config "$phy"
}

add_driver mac80211
