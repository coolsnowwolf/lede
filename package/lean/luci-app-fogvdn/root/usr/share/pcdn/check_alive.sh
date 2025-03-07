#!/bin/sh
pid_file="/run/pear_restart.pid"
while true; do
    if [ -f $pid_file ]; then
        pid=$(cat $pid_file)
        if [ -n "$pid" ]; then
            if [ ! -d /proc/$pid ]; then
                logger -t "fogvdn" "fogvdn is not running, restart it"
                /etc/init.d/openfog.sh start
            fi
        fi
    else
        logger -t "fogvdn" "pid file not found, restart it"
        /etc/init.d/openfog.sh start
    fi
    sleep 10
done
