#!/bin/bash

if [ ! -d /sys/class/pwm/pwmchip0 ]; then
    echo "this model does not support pwm."
    exit 1
fi

if [ ! -d /sys/class/pwm/pwmchip0/pwm0 ]; then
    echo -n 0 > /sys/class/pwm/pwmchip0/export
fi
sleep 1
while [ ! -d /sys/class/pwm/pwmchip0/pwm0 ];
do
    sleep 1
done
ISENABLE=`cat /sys/class/pwm/pwmchip0/pwm0/enable`
if [ $ISENABLE -eq 1 ]; then
    echo -n 0 > /sys/class/pwm/pwmchip0/pwm0/enable
fi
echo -n 50000 > /sys/class/pwm/pwmchip0/pwm0/period
echo -n 1 > /sys/class/pwm/pwmchip0/pwm0/enable

# max speed run 5s
echo -n 46990 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
sleep 5
echo -n 25000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle

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

	echo -n $DUTY > /sys/class/pwm/pwmchip0/pwm0/duty_cycle;

        echo "temp: $temp, duty: $DUTY, ${PERCENT}%"
        # cat /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_cur_freq

	sleep 2s;
done
