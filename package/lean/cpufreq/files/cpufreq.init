#!/bin/sh /etc/rc.common

START=15
USE_PROCD=1

NAME="cpufreq"
CPUFREQ_PATH="/sys/devices/system/cpu/cpufreq"

extra_command "get_policies" "Get CPU scaling governors and frequencies"

get_policies() {
	json_init
	for policy in $(ls -d "$CPUFREQ_PATH"/policy[0-9]* 2>"/dev/null"); do
		[ -s "$policy/scaling_available_frequencies" ] || continue
		json_add_object "$(basename "$policy")"
		json_add_string "index" "$(basename "$policy" | grep -Eo "[0-9]*$")"
		json_add_string "cpus" "$(cat "$policy/affected_cpus")"
		json_add_array "freqs"
		for freq in $(cat "$policy/scaling_available_frequencies"); do
			json_add_string "" "$freq"
		done
		json_close_array
		json_add_array "governors"
		for governor in $(cat "$policy/scaling_available_governors"); do
			json_add_string "" "$governor"
		done
		json_close_array
		json_close_object
	done
	json_dump
}

write_cpufreq_config() {
	local value
	config_get value "$NAME" "$1"
	[ -z "$value" ] || echo -n "$value" > "$2"
}

start_service() {
	config_load "$NAME"

	for i in $(ls -d "$CPUFREQ_PATH"/policy[0-9]* 2>"/dev/null" | grep -Eo "[0-9]*$")
	do
		[ -z "$(config_get "$NAME" "governor$i")" ] && return

		write_cpufreq_config "governor$i" "$CPUFREQ_PATH/policy$i/scaling_governor"
		write_cpufreq_config "minfreq$i" "$CPUFREQ_PATH/policy$i/scaling_min_freq"
		write_cpufreq_config "maxfreq$i" "$CPUFREQ_PATH/policy$i/scaling_max_freq"
		if [ "$(config_get "$NAME" "governor$i")" = "ondemand" ]; then
			write_cpufreq_config "sdfactor$i" "$CPUFREQ_PATH/ondemand/sampling_down_factor"
			write_cpufreq_config "upthreshold$i" "$CPUFREQ_PATH/ondemand/up_threshold"
		fi
	done
}

service_triggers() {
	procd_add_reload_trigger "$NAME"
}
