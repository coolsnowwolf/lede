#!/bin/bash

# determine fan controller
if [ -d /sys/devices/platform/pwm-fan ]; then
    echo "pls use /usr/bin/fa-fancontrol.sh."
    exit 1
fi

if [ ! -d /sys/class/pwm/pwmchip1 ]; then
    echo "this model does not support pwm."
    exit 1
fi
if [ ! -d /sys/class/pwm/pwmchip1/pwm0 ]; then
    echo 0 > /sys/class/pwm/pwmchip1/export
fi
sleep 1
while [ ! -d /sys/class/pwm/pwmchip1/pwm0 ];
do
    sleep 1
done
ISENABLE=`cat /sys/class/pwm/pwmchip1/pwm0/enable`
if [ $ISENABLE -eq 1 ]; then
    echo 0 > /sys/class/pwm/pwmchip1/pwm0/enable
fi
echo 50000 > /sys/class/pwm/pwmchip1/pwm0/period
echo 1 > /sys/class/pwm/pwmchip1/pwm0/enable

# max speed run 5s
echo 46990 > /sys/class/pwm/pwmchip1/pwm0/duty_cycle
sleep 5
echo 25000 > /sys/class/pwm/pwmchip1/pwm0/duty_cycle

# declare -a CpuTemps=(55000 43000 38000 32000)
# declare -a PwmDutyCycles=(1000 20000 30000 45000)

declare -a CpuTemps=(75000 63000 58000 52000)
declare -a PwmDutyCycles=(25000 35000 45000 46990)

declare -a Percents=(100 75 50 25)
DefaultDuty=49990
DefaultPercents=0

while true
do
	temp=$(cat /sys/class/thermal/thermal_zone0/temp)
	INDEX=0
	FOUNDTEMP=0
	DUTY=$DefaultDuty
	PERCENT=$DefaultPercents
	
	for i in 0 1 2 3; do
		if [ $temp -gt ${CpuTemps[$i]} ]; then
			INDEX=$i
			FOUNDTEMP=1
			break
		fi	
	done
	if [ ${FOUNDTEMP} == 1 ]; then
		DUTY=${PwmDutyCycles[$i]}
		PERCENT=${Percents[$i]}
	fi

	echo $DUTY > /sys/class/pwm/pwmchip1/pwm0/duty_cycle;

        # echo "temp: $temp, duty: $DUTY, ${PERCENT}%"
        # cat /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_cur_freq

	sleep 2s;
done
