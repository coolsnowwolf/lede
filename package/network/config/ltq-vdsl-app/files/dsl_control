#!/bin/sh /etc/rc.common
# Copyright (C) 2012 OpenWrt.org

START=97
USE_PROCD=1

dslstat() {
	ubus call dsl metrics
}

EXTRA_COMMANDS="dslstat"
EXTRA_HELP="	dslstat Get DSL status information"

#
# ITU-T G.997.1 (06/2012) - Section 7.3.1.1.1 (xTU transmission system enabling (XTSE))
# ITU-T G.997.1 Amendment 2 (04/2013) - Section 2.1 - (Vectoring mode enable (VECTORMODE_ENABLE))
#
# G.992.1 Annex A
# G.992.2 Annex A
# G.992.3 Annex A / L-US1 / L_US-2 / M
# G.992.5 Annex A / M
# G.993.2 Annex A/B/C
# G.993.5 Annex A/B/C
xtse_xdsl_a="05_01_04_00_4C_01_04_07"

# G.992.1 Annex B
# G.992.3 Annex B
# G.992.5 Annex B
# G.993.2 Annex A/B/C
# G.993.5 Annex A/B/C
xtse_xdsl_b="10_00_10_00_00_04_00_07"

# G.992.1 Annex B
# G.992.3 Annex B
# G.992.3 Annex J
# G.992.5 Annex B
# G.992.5 Annex J
# G.993.2 Annex A/B/C
# G.993.5 Annex A/B/C
xtse_xdsl_j="10_00_10_40_00_04_01_07"

# G.992.1 Annex B
xtse_xdsl_bdmt="10_00_00_00_00_00_00_00"

# G.992.3 Annex B
xtse_xdsl_b2="00_00_10_00_00_00_00_00"

# G.992.5 Annex B
xtse_xdsl_b2p="00_00_00_00_00_04_00_00"

# ANSI T1.413
xtse_xdsl_at1="01_00_00_00_00_00_00_00"

# G.992.2 Annex A
xtse_xdsl_alite="00_01_00_00_00_00_00_00"

# G.992.1 Annex A
xtse_xdsl_admt="04_00_00_00_00_00_00_00"

# G.992.3 Annex A
xtse_xdsl_a2="00_00_04_00_00_00_00_00"

# G.992.5 Annex A
xtse_xdsl_a2p="00_00_00_00_00_01_00_00"

# G.992.3 Annex L
xtse_xdsl_l="00_00_00_00_0C_00_00_00"

# G.992.3 Annex M
# G.992.5 Annex M
xtse_xdsl_m="00_00_00_00_40_00_04_00"

# G.992.3 Annex M
xtse_xdsl_m2="00_00_00_00_40_00_00_00"

# G.992.5 Annex M
xtse_xdsl_m2p="00_00_00_00_00_00_04_00"

#
# ITU-T G.994.1 (06/2012) - Table 2 (Mandatory carrier sets)
#

# A43
tone_adsl_a="0x142" # A43C + J43 + A43
tone_vdsl_a="0x142" # A43C + J43 + A43

# A43 + V43
tone_adsl_av="0x142" # A43C + J43 + A43
tone_vdsl_av="0x146" # A43C + J43 + A43 + V43

# B43
tone_adsl_b="0x81" # B43 + B43c
tone_vdsl_b="0x1" # B43

# B43 + V43
tone_adsl_bv="0x81" # B43 + B43c
tone_vdsl_bv="0x5" # B43 + V43

# create ADSL autoboot script. Used for SNR margin tweak
autoboot_script() {
    echo "[WaitForConfiguration]={
locs 0 $1
}

[WaitForLinkActivate]={
}

[WaitForRestart]={
}

[Common]={
}" > /tmp/dsl.scr
}

lowlevel_cfg() {
	echo "# VRX Low Level Configuration File
#
# Parameters must be separated by tabs or spaces.
# Empty lines and comments will be ignored.
#

# nFilter
#
# NA     = -1
# OFF    = 0
# ISDN   = 1
# POTS   = 2
# POTS_2 = 3
# POTS_3 = 4
#
#  (dec)
    -1

# nHsToneGroupMode nHsToneGroup_A       nHsToneGroup_V    nHsToneGroup_AV
#
# NA     = -1      NA         = -1      see               see
# AUTO   = 0       VDSL2_B43  = 0x0001  nHsToneGroup_A    nHsToneGroup_A
# MANUAL = 1       VDSL2_A43  = 0x0002
#                  VDSL2_V43  = 0x0004
#                  VDSL1_V43P = 0x0008
#                  VDSL1_V43I = 0x0010
#                  ADSL1_C43  = 0x0020
#                  ADSL2_J43  = 0x0040
#                  ADSL2_B43C = 0x0080
#                  ADSL2_A43C = 0x0100
#
#  (dec)           (hex)                (hex)             (hex)
     1             $1			$2		 0x0

#   nBaseAddr     nIrqNum
#
#     (hex)        (dec)
    0x1e116000      63

# nUtopiaPhyAdr   nUtopiaBusWidth      nPosPhyParity
#                 default(16b) = 0     NA   = -1
#                 8-bit        = 1     ODD  = 0
#                 16-bit       = 2
#
#
#    (hex)            (dec)                (dec)
      0xFF              0                    0

# bNtrEnable
#
#  (dec)
    0" > /tmp/lowlevel.cfg
}

service_triggers() {
	procd_add_reload_trigger network
}

start_service() {
	local annex
	local firmware
	local tone
	local tone_adsl
	local tone_vdsl
	local xtse
	local xfer_mode
	local line_mode
	local tc_layer
	local mode
	local lowlevel
	local snr

	config_load network
	config_get tone dsl tone
	config_get annex dsl annex
	config_get firmware dsl firmware
	config_get xfer_mode dsl xfer_mode
	config_get line_mode dsl line_mode
	config_get snr dsl ds_snr_offset

	eval "xtse=\"\${xtse_xdsl_$annex}\""

	case "${xfer_mode}" in
	atm)
		tc_layer="-T1:0x1:0x1_1:0x1:0x1"
		;;
	ptm)
		tc_layer="-T2:0x1:0x1_2:0x1:0x1"
		;;
	esac

	case "${line_mode}" in
	adsl)
		mode="-M1"

		# mask out VDSL bits when ADSL is requested
		xtse="${xtse%_*}_00"
		;;
	vdsl)
		mode="-M2"

		# mask out ADSL bits when VDSL is requested
		xtse="00_00_00_00_00_00_00_${xtse##*_}"
		;;
	esac

	local annexgpio="/sys/class/gpio/annex"
	if [ -d "${annexgpio}a" ] && [ -d "${annexgpio}b" ]; then
		case "${annex}" in
			a*|l*|m*)
				echo 1 > "${annexgpio}a/value"
				echo 0 > "${annexgpio}b/value"
				;;
			b*|j*)
				echo 0 > "${annexgpio}a/value"
				echo 1 > "${annexgpio}b/value"
				;;
		esac
	fi

	if [ -z "${firmware}" ]; then
		# search for the firmware provided by dsl-vrx200-firmware-xdsl-*
		if grep -qE "system type.*: (VR9|xRX200)" /proc/cpuinfo; then
			case "${annex}" in
			a*|l*|m*)
				if [ -f "/lib/firmware/lantiq-vrx200-a.bin" ]; then
					firmware="/lib/firmware/lantiq-vrx200-a.bin"
				elif [ -f "/tmp/lantiq-vrx200-a.bin" ]; then
					firmware="/tmp/lantiq-vrx200-a.bin"
				elif [ -f "/lib/firmware/lantiq-vrx200-b.bin" ] && [ -f "/lib/firmware/lantiq-vrx200-b-to-a.bspatch" ]; then
					bspatch /lib/firmware/lantiq-vrx200-b.bin \
						/tmp/lantiq-vrx200-a.bin \
						/lib/firmware/lantiq-vrx200-b-to-a.bspatch
					firmware="/tmp/lantiq-vrx200-a.bin"
				else
					echo "firmware for annex a not found"
					return 1
				fi
				;;
			b*|j*)
				if [ -f "/lib/firmware/vr9_dsl_fw_annex_b.bin" ]; then
					firmware="/lib/firmware/vr9_dsl_fw_annex_b.bin"
				elif [ -f "/lib/firmware/lantiq-vrx200-b.bin" ]; then
					firmware="/lib/firmware/lantiq-vrx200-b.bin"
				elif [ -f "/tmp/lantiq-vrx200-b.bin" ]; then
					firmware="/tmp/lantiq-vrx200-b.bin"
				elif [ -f "/lib/firmware/lantiq-vrx200-a.bin" ] && [ -f "/lib/firmware/lantiq-vrx200-a-to-b.bspatch" ]; then
					bspatch /lib/firmware/lantiq-vrx200-a.bin \
						/tmp/lantiq-vrx200-b.bin \
						/lib/firmware/lantiq-vrx200-a-to-b.bspatch
					firmware="/tmp/lantiq-vrx200-b.bin"
				else
					echo "firmware for annex b not found"
					return 1
				fi
				;;
			*)
				echo "annex type not supported use a or b"
				return 1
				;;
			esac
		fi
	fi

	[ -z "${firmware}" ] && firmware=/lib/firmware/vdsl.bin
	[ -f "${firmware}" ] || {
		echo failed to find $firmware
		return 1
	}

	eval "tone_adsl=\"\${tone_adsl_$tone}\""
	eval "tone_vdsl=\"\${tone_vdsl_$tone}\""
	[ -n "${tone_adsl}" ] && [ -n "${tone_vdsl}" ] && {
		lowlevel_cfg "${tone_adsl}" "${tone_vdsl}"
		lowlevel="-l /tmp/lowlevel.cfg"
	}

	[ -z "${snr}" ] || {
	    # for SNR offset setting
	    autoboot_script "$snr"
	    autoboot="-a /tmp/dsl.scr -A /tmp/dsl.scr"
	}

	procd_open_instance
	procd_set_param command /sbin/vdsl_cpe_control \
			-i$xtse \
			-n /sbin/dsl_notify.sh \
			-f ${firmware} \
			$lowlevel \
			${mode} \
			${tc_layer} \
			$autoboot
	procd_close_instance
}

stop_service() {
	DSL_NOTIFICATION_TYPE="DSL_INTERFACE_STATUS" \
	DSL_INTERFACE_STATUS="DOWN" \
		/sbin/dsl_notify.sh
}
