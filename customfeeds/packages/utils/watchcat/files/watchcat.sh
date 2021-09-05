#!/bin/sh
#
# Copyright (C) 2010 segal.di.ubi.pt
# Copyright (C) 2020 nbembedded.com
#
# This is free software, licensed under the GNU General Public License v2.
#

get_ping_size() {
    ps=$1
    case "$ps" in
    small)
        ps="1"
        ;;
    windows)
        ps="32"
        ;;
    standard)
        ps="56"
        ;;
    big)
        ps="248"
        ;;
    huge)
        ps="1492"
        ;;
    jumbo)
        ps="9000"
        ;;
    *)
        echo "Error: invalid ping_size. ping_size should be either: small, windows, standard, big, huge or jumbo"
        echo "Cooresponding ping packet sizes (bytes): small=1, windows=32, standard=56, big=248, huge=1492, jumbo=9000"
        ;;
    esac
    echo $ps
}

reboot_now() {
    reboot &

    [ "$1" -ge 1 ] && {
        sleep "$1"
        echo 1 >/proc/sys/kernel/sysrq
        echo b >/proc/sysrq-trigger # Will immediately reboot the system without syncing or unmounting your disks.
    }
}

watchcat_periodic() {
    failure_period="$1"
    force_reboot_delay="$2"

    sleep "$failure_period" && reboot_now "$force_reboot_delay"
}

watchcat_restart_modemmanager_iface() {
    [ "$2" -gt 0 ] && {
        logger -t INFO "Resetting current-bands to 'any' on modem: \"$1\" now."
        /usr/bin/mmcli -m any --set-current-bands=any
    }
    logger -t INFO "Reconnecting modem: \"$1\" now."
    /etc/init.d/modemmanager restart
    ifup "$1"
}

watchcat_restart_network_iface() {
    logger -t INFO "Restarting network interface: \"$1\"."
    ip link set "$1" down
    ip link set "$1" up
}

watchcat_restart_all_network() {
    logger -t INFO "Restarting networking now by running: /etc/init.d/network restart"
    /etc/init.d/network restart
}

watchcat_monitor_network() {
    failure_period="$1"
    ping_hosts="$2"
    ping_frequency_interval="$3"
    ping_size="$4"
    iface="$5"
    mm_iface_name="$6"
    mm_iface_unlock_bands="$7"

    time_now="$(cat /proc/uptime)"
    time_now="${time_now%%.*}"

    [ "$time_now" -lt "$failure_period" ] && sleep "$((failure_period - time_now))"

    time_now="$(cat /proc/uptime)"
    time_now="${time_now%%.*}"
    time_lastcheck="$time_now"
    time_lastcheck_withinternet="$time_now"

    ping_size="$(get_ping_size "$ping_size")"

    while true; do
        # account for the time ping took to return. With a ping time of 5s, ping might take more than that, so it is important to avoid even more delay.
        time_now="$(cat /proc/uptime)"
        time_now="${time_now%%.*}"
        time_diff="$((time_now - time_lastcheck))"

        [ "$time_diff" -lt "$ping_frequency_interval" ] && sleep "$((ping_frequency_interval - time_diff))"

        time_now="$(cat /proc/uptime)"
        time_now="${time_now%%.*}"
        time_lastcheck="$time_now"

        for host in $ping_hosts; do
            if [ "$iface" != "" ]; then
                ping_result="$(
                    ping -I "$iface" -s "$ping_size" -c 1 "$host" &>/dev/null
                    echo $?
                )"
            else
                ping_result="$(
                    ping -s "$ping_size" -c 1 "$host" &>/dev/null
                    echo $?
                )"
            fi

            if [ "$ping_result" -eq 0 ]; then
                time_lastcheck_withinternet="$time_now"
            else
                if [ "$iface" != "" ]; then
                    logger -p daemon.info -t "watchcat[$$]" "Could not reach $host via \"$iface\" for \"$((time_now - time_lastcheck_withinternet))\" seconds. Restarting \"$iface\" after reaching \"$failure_period\" seconds"
                else
                    logger -p daemon.info -t "watchcat[$$]" "Could not reach $host for \"$((time_now - time_lastcheck_withinternet))\" seconds. Restarting networking after reaching \"$failure_period\" seconds"
                fi
            fi
        done

        [ "$((time_now - time_lastcheck_withinternet))" -ge "$failure_period" ] && {
            if [ "$mm_iface_name" != "" ]; then
                watchcat_restart_modemmanager_iface "$mm_iface_name" "$mm_iface_unlock_bands"
            fi
            if [ "$iface" != "" ]; then
                watchcat_restart_network_iface "$iface"
            else
                watchcat_restart_all_network
            fi
            /etc/init.d/watchcat start
        }

    done
}

watchcat_ping() {
    failure_period="$1"
    force_reboot_delay="$2"
    ping_hosts="$3"
    ping_frequency_interval="$4"
    ping_size="$5"

    time_now="$(cat /proc/uptime)"
    time_now="${time_now%%.*}"

    [ "$time_now" -lt "$failure_period" ] && sleep "$((failure_period - time_now))"

    time_now="$(cat /proc/uptime)"
    time_now="${time_now%%.*}"
    time_lastcheck="$time_now"
    time_lastcheck_withinternet="$time_now"

    ping_size="$(get_ping_size "$ping_size")"

    while true; do
        # account for the time ping took to return. With a ping time of 5s, ping might take more than that, so it is important to avoid even more delay.
        time_now="$(cat /proc/uptime)"
        time_now="${time_now%%.*}"
        time_diff="$((time_now - time_lastcheck))"

        [ "$time_diff" -lt "$ping_frequency_interval" ] && sleep "$((ping_frequency_interval - time_diff))"

        time_now="$(cat /proc/uptime)"
        time_now="${time_now%%.*}"
        time_lastcheck="$time_now"

        for host in $ping_hosts; do
            if [ "$iface" != "" ]; then
                ping_result="$(
                    ping -I "$iface" -s "$ping_size" -c 1 "$host" &>/dev/null
                    echo $?
                )"
            else
                ping_result="$(
                    ping -s "$ping_size" -c 1 "$host" &>/dev/null
                    echo $?
                )"
            fi

            if [ "$ping_result" -eq 0 ]; then
                time_lastcheck_withinternet="$time_now"
            else
                logger -p daemon.info -t "watchcat[$$]" "Could not reach $host for $((time_now - time_lastcheck_withinternet)). Rebooting after reaching $failure_period"
            fi
        done

        [ "$((time_now - time_lastcheck_withinternet))" -ge "$failure_period" ] && reboot_now "$force_reboot_delay"
    done
}

mode="$1"

# Fix potential typo in mode and provide backward compatibility.
[ "$mode" = "allways" ] && mode="periodic_reboot"
[ "$mode" = "always" ] && mode="periodic_reboot"
[ "$mode" = "ping" ] && mode="ping_reboot"

case "$mode" in
periodic_reboot)
    watchcat_periodic "$2" "$3"
    ;;
ping_reboot)
    watchcat_ping "$2" "$3" "$4" "$5" "$6"
    ;;
restart_iface)
    watchcat_monitor_network "$2" "$3" "$4" "$5" "$6" "$7"
    ;;
*)
    echo "Error: invalid mode selected: $mode"
    ;;
esac
