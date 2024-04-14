#!/bin/sh /etc/rc.common
# Copyright (C) 2006-2014 OpenWrt.org

START=99
STOP=16
USE_PROCD=1
#使用procd启动

run_5g()
{
    local enabled
    config_get_bool enabled $1 enabled

    echo "run 4G" >> /tmp/log4g

    if [ "$enabled" = "1" ]; then
        local user
        local password
        local apn
        local auth
        local pincode
        local tool

	# echo "enable 5G" >> /tmp/log5g
        config_get user $1 user
        config_get password $1 password
        config_get apn $1 apn
        config_get auth $1 auth
        config_get pincode $1 pincode
        config_get tool $1 tool
        config_get tty $1 tty
        config_get atcmd $1 atcmd

        if [ "$tool" = "at" ];then
        at_tool "$atcmd" -d $tty
        else
            procd_open_instance
        #创建一个实例， 在procd看来一个应用程序可以多个实\E4\BE?
        #ubus call service list 可以查看实例
            procd_set_param command $tool -i rmnet_mhi0 -s $apn
            if [ "$password" != "" ];then
                procd_append_param command $user $password $auth
            fi
            if [ "$pincode" != "" ]; then
                procd_append_param command -p $pincode
            fi
            # procd_append_param command -f /tmp/4g.log
            procd_set_param respawn
            echo "quectel-CM has started."
            procd_close_instance
            #关闭实例
        fi

    fi
}


service_triggers()
{
	procd_add_reload_trigger "pcimodem"
}

start_service() {
	config_load pcimodem
	config_foreach run_5g service
}

stop_service()
{
    echo "5G stop" >> /tmp/log5g
    killall quectel-CM
    echo "quectel-CM has stoped."
}


