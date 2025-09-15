#!/bin/sh

uci_write_config() {
	uci -q set "cpufreq.cpufreq.governor$1"="$2"
	uci -q set "cpufreq.cpufreq.minfreq$1"="$3"
	uci -q set "cpufreq.cpufreq.maxfreq$1"="$4"
	[ -n "$5" ] && uci -q set "cpufreq.cpufreq.sdfactor$1"="$5"
	[ -n "$6" ] && uci -q set "cpufreq.cpufreq.upthreshold$1"="$6"
	uci -q commit cpufreq
}

[ "$(uci -q get cpufreq.global.set)" -eq "1" ] && exit 0

CPU_FREQS="$(cat '/sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies')"
CPU_MIN_FREQ="$(cat '/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq')"
CPU_MAX_FREQ="$(cat '/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq')"
CPU_POLICYS="$(find '/sys/devices/system/cpu/cpufreq/policy'* -maxdepth 0 | grep -Eo '[0-9]+')"

source "/etc/openwrt_release"
case "$DISTRIB_TARGET" in
	"bcm27xx/bcm2710"|\
	"bcm27xx/bcm2711")
		uci_write_config 0 ondemand 600000 "$CPU_MAX_FREQ" 10 50
		;;
	"ipq40xx/generic")
		uci_write_config 0 performance 200000 "$CPU_MAX_FREQ"
		;;
	"ipq806x/generic")
		uci_write_config 0 performance 600000 "$CPU_MAX_FREQ"
		# IPQ8064/5
		echo "$CPU_POLICYS" | grep -q "1" && uci_write_config 1 performance 600000 1200000
		;;
	"mediatek/mt7622")
		uci_write_config 0 ondemand 600000 1350000 10 50
		;;
	"qualcommax/ipq60xx"|\
	"qualcommax/ipq807x")
		uci_write_config 0 schedutil "$CPU_MIN_FREQ" "$CPU_MAX_FREQ"
		;;
	"rockchip/armv8")
		if echo "$CPU_POLICYS" | grep -q "6"; then
			# RK3588/S
			uci_write_config 0 schedutil 1008000 1800000
			uci_write_config 4 schedutil 816000 2208000
			uci_write_config 6 schedutil 816000 2208000
		elif echo "$CPU_POLICYS" | grep -q "4"; then
			# RK3399
			uci_write_config 0 schedutil 600000 1608000
			uci_write_config 4 schedutil 600000 2016000
		else
			if ! echo "$CPU_FREQS" | grep -q "1992000"; then
				# RK3328
				CPU_MAX_FREQ="1512000"
			fi
			uci_write_config 0 schedutil 816000 "$CPU_MAX_FREQ"
		fi
		;;
	"sunxi/cortexa53")
		if echo "$CPU_FREQS" | grep -q "1800000"; then
			# H6
			uci_write_config 0 schedutil "888000" "$CPU_MAX_FREQ"
		elif echo "$CPU_FREQS" | grep -q "1512000"; then
			# H616/8
			uci_write_config 0 schedutil "936000" "1512000"
		elif echo "$CPU_FREQS" | grep -q "1296000"; then
			# H5
			uci_write_config 0 ondemand "$CPU_MIN_FREQ" "1296000" 10 50
		else
			# A64
			uci_write_config 0 ondemand "$CPU_MIN_FREQ" "$CPU_MAX_FREQ" 10 50
		fi
		;;
esac

exit 0
