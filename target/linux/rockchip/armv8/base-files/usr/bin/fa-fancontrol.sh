#!/bin/bash

# determine fan controller
if [ -d /sys/devices/platform/pwm-fan ]; then
    (cd /sys/devices/virtual/thermal/thermal_zone0 && {        
        logger -p user.info -t "pwmfan" "set the conditions for fan"
        [ -f trip_point_3_temp ] && {
            # select fan level 1
            echo 50000 > trip_point_3_temp
        }
        [ -f trip_point_4_temp ] && {
            # select fan level 2-4
            echo 55000 > trip_point_4_temp
        }
    })

    (cd /sys/devices/virtual/thermal/cooling_device0 && {
        TYPE=`cat type`
        if [ $TYPE = 'pwm-fan' ]; then
            # run 5s
            for i in `seq 1 5`; do
                logger -p user.info -t "pwmfan" "start to spin ${i}/5"
                echo 3 > cur_state
                sleep 1
            done
            logger -p user.info -t "pwmfan" "set to auto"
            echo 0 > cur_state
        fi
    })
else
    logger -p user.info -t "pwmfan" "not found cooling device"
    if [ -d /sys/class/pwm/pwmchip1 ]; then
        nohup /usr/bin/fa-fancontrol-direct.sh&
    fi
fi

