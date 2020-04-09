#!/bin/sh

alias separate_front="awk -F \":\" '{print \$1}'"
alias separate_back="awk -F \":\" '{print \$2}'"
alias DATE="date '+%A %Y-%m-%d %H:%M:%S'"
VACATIONFILE="/etc/oafControl/vacation.conf"
OVERTIMEFILE="/etc/oafControl/overtime.conf"
LOGFILE="/tmp/oafControl/oafControl.log"
VACATIONBAK="/etc/oafControl/overtime.bak"
OVERIIMEBAK="/etc/oafControl/vacation.bak"
NAME="oafControl"
mode="$1"
fun="$2"
line=`wc -l $LOGFILE | awk -F " " '{print $1}'`
[ $line -ge 1000 ] && echo "日志超过一千行，清除日志" > $LOGFILE

# 重新定义config_get
config_get(){
	local var
	local num="$3"
	var=$(uci get $NAME.@$1[$num].$2 2>/dev/null)
	echo ${var}
}

uci_get_by_type() {
	local total=`uci show 2>/dev/null | grep $NAME.@"$1" | grep -w "=$1" | sed '/^$/!h;$!d;g' | awk -F "[" '{print $2}' | awk -F "]" '{print $1}'`
	echo "$total"
}

#########################USAGE###########################
# config_get_global(){									#
# 	total=`uci_get_by_type $cfg`						#
# 	while [[ $total -ge 0 ]]							#
# 	do													#		
#	 	enable=`config_get "$cfg" enable $total`		#
# 		comments=`config_get "$cfg" comment $total`		#
# 		total=$((total-1))								#
# 	done												#
# }														#				
# config_get_global										#
#########################################################

uci_get_weekday(){
	echo `uci get "$NAME".@weekday[0]."$1" 2>/dev/null`
}

uci_get_workday(){
	echo `uci get "$NAME".@workday[0]."$1" 2>/dev/null`
}

add_cron(){
	sed -i "/#$4/d" /etc/crontabs/root
	if [ -n "$5" ]; then
		echo "$1 $2 * * $3 /usr/bin/oafControl.sh global_start #$4" >> /etc/crontabs/root
	else
		echo "$1 $2 * * $3 /usr/bin/oafControl.sh global_stop #$4" >> /etc/crontabs/root
	fi
}

overtime_add_cron(){
	sed -i "/#$5/d" /etc/crontabs/root
	if [ -n "$6" ]; then
		echo "$1 $2 $3 $4 * /usr/bin/oafControl.sh global_start #$5" >> /etc/crontabs/root
	else
		echo "$1 $2 $3 $4 * /usr/bin/oafControl.sh global_stop #$5" >> /etc/crontabs/root
	fi
}

isDate(){ ##判断2020-02-31日期是否为有效日期
	local val="$1"
	yearstr=`echo $val | awk -F "-" '{print $1}'` 
	monthstr=$(expr `echo $val | awk -F "-" '{print $2}'` + 0)
	daystr=$(expr `echo $val | awk -F "-" '{print $3}'` + 0)

	function get_days_in_month(){ ##返回每个月的最大天数
		local year="$yearstr"
		local month="$monthstr"
		leap4=`expr $year % 4`
		leap100=`expr $year % 100`
		leap400=`expr $year % 400`
		local day=30
		local days=30
		Even=`expr $month % 2`
		[ "$leap4" -eq 0 -a "$leap100" -ne 0 -o "$leap400" -eq 0 ] && days=`expr $days + 1`
		[ $month = 7 ] && echo $((++day)) && return
		[ $month = 2 ] && echo $((days - 2)) && return
		[ "$month" -lt 7 ] && ([ "$Even" -eq 0 ] && echo $day  || echo $((++day))) && return
		[ "$month" -gt 6 ] && ([ "$Even" -eq 0 ] && echo $((++day)) || echo $day)
	}
	
	isdate=`get_days_in_month $yearstr $ monthstr`
	[ "$daystr" -le "$isdate" ] && echo "1" || echo "0"
}

newDate(){ ##生成区间日期到指定文件
	local val_1="$1"
	local val_2="$2"
	local first_day=1
	local first_month
	local file="$3"
	year1=`echo $val_1 | awk -F "-" '{print $1}'` 
	month1=$(expr `echo $val_1 | awk -F "-" '{print $2}'` + 0)
	day1=$(expr `echo $val_1 | awk -F "-" '{print $3}'` + 0)
	year2=`echo $val_2 | awk -F "-" '{print $1}'` 
#	month2=`echo $val_2 | awk -F "-" '{print $2}'`
#	day2=`echo $val_2 | awk -F "-" '{print $3}'`

	for i in $(seq $year1 $year2)
	do
		[ "$i" -eq "$year1" ] && first_month="$month1"
		for j in $(seq $first_month 12)
		do
			[ "$i" -eq "$year1" -a "$j" -eq "$month1" ] && first_day="$day1" || first_day=1
			for k in $(seq $first_day 31)
			do
				tmpDate=`printf "%-4d-%0.2d-%0.2d" "$i" "$j" "$k"` ##格式化输出
				if [ $k -ge 29 ]; then
					judge=`isDate "$i-$j-$k"`
					[ $judge = 1 ] && echo $tmpDate >> $file
				else
					echo $tmpDate >> $file 
				fi
				[ "$val_2" = "$tmpDate" ] && return
			done
		done
	done

}

isHour(){
	local hour="$1"
	[ $hour -ge 0 -a $hour -le 23 ] && echo $hour || echo ""
}

isMinu(){
	local minu="$1"
	[ $minu -ge 0 -a $minu -le 59 ] && echo $minu || echo ""
}

auto_update(){
	local url
	echo "`DATE`：正在检查特征库版本" >> $LOGFILE
	url="https://raw.githubusercontent.com/destan19/OpenAppFilter/master/open-app-filter/files/feature.cfg"
	remote_update=$(uci get oafControl.@global[0].auto_update 2>/dev/null)
	[ "$remote_update" -eq 1 ] && return
	curl "$url" > /tmp/oafControl/feature.cfg
	tmp_md5=`md5sum /tmp/oafControl/feature.cfg | awk -F " " '{print $1}'`
	cp -f /tmp/oafControl/feature.cfg /tmp/oafControl/feature1.cfg
	cp -f /etc/appfilter/feature.cfg /tmp/oafControl/feature.cfg
	etc_md5=`md5sum /tmp/oafControl/feature.cfg | awk -F " " '{print $1}'`
	if [ "$tmp_md5" = "$etc_md5" ]; then
		echo "`DATE`：特征库已是最新，无需更新" >> $LOGFILE
		return
	else
		echo "`DATE`：特征库有更新，正在更新" >> $LOGFILE
		cp -f /tmp/oafControl/feature1.cfg /etc/appfilter/feature.cfg
		echo "`DATE`：特征库更新完成" >> $LOGFILE
	fi
}

remote_control(){ #远程控制
	global_enable=$(uci get oafControl.@global[0].global_enabled 2>/dev/null)
#	global_run=$(uci get oafControl.global.global_run 2>/dev/null)
	remote_url=$(uci get oafControl.@global[0].global_wget 2>/dev/null)
	curl "$remote_url" > /tmp/oafControl/oafControl.Status
	echo "`DATE`：正在获取远程控制状态" >> $LOGFILE
	status=$(cat /tmp/oafControl/oafControl.Status 2>/dev/null)
	if [ "$status" -eq "1" ]; then
		if [ "$status" -eq "$global_enable" ]; then #均为1则无操作
			echo "`DATE`：远程无命令" >> $LOGFILE
			return
		else #不同则start并改变启动状态为已启动
			echo "`DATE`：远程启动中" >> $LOGFILE
			uci set oafControl.@global[0].global_enabled="1"
			/etc/init.d/oafControl restart
		fi
	elif [ "$status" -eq "0" ]; then
		if [ "$status" -ne "$global_enable" ]; then #不同则stop并改变启动状态为已禁止
			echo "`DATE`：远程关闭中" >> $LOGFILE
			uci set oafControl.@global[0].global_enabled="0"
			/etc/init.d/oafControl restart
		else #均为1则无操作
			echo "`DATE`：远程无命令" >> $LOGFILE
			return
		fi
	else
		echo "`DATE`：远程无命令" >> $LOGFILE
	fi
}

init_vacation(){
	local cfg="vacation"
	echo "" > $VACATIONFILE
	echo "" > $VACATIONBAK
	config_vacation(){
		total=`uci_get_by_type $cfg`
		while [[ $total -ge 0 ]]
		do
			enable=`config_get "$cfg" enable $total`
			restStart=`config_get "$cfg" rest_start $total`
			restEnd=`config_get "$cfg" rest_end $total`
			comments=`config_get "$cfg" comment $total`
			[ "$enable" -eq "1" ] && newDate $restStart $restEnd $VACATIONFILE
			[ "$enable" -eq "1" ] && echo "${comments} 已启用：从 $restStart 到 $restEnd" >> $VACATIONBAK || echo "${comments} 未启用：从 $restStart 到 $restEnd" >> $VACATIONBAK 
			total=$((total-1))
		done
	}
	config_vacation
	sed -i '/^$/d' $VACATIONFILE
	sed -i '/^$/d' $VACATIONBAK
}

init_overtime(){
	local cfg="overtime"
	echo "" > $OVERTIMEFILE
	echo "" > $OVERIIMEBAK
	config_overtime(){
		total=`uci_get_by_type $cfg`
		while [[ $total -ge 0 ]]
		do
			enable=`config_get "$cfg" enable $total`
			overtimeStart=`config_get "$cfg" overtime_start $total`
			overtimeEnd=`config_get "$cfg" overtime_end $total`
			comments=`config_get "$cfg" comment $total`
			[ "$enable" -eq "1" ] && newDate $overtimeStart $overtimeEnd $OVERTIMEFILE
			[ "$enable" -eq "1" ] && echo "${comments} 已启用：从 $overtimeStart 到 $overtimeEnd" >> $OVERIIMEBAK || echo "${comments} 未启用：从 $restStart 到 $restEnd" >> $OVERIIMEBAK 
			total=$((total-1))
		done
	}
	config_overtime
	sed -i '/^$/d' $OVERTIMEFILE
	sed -i '/^$/d' $OVERIIMEBAK
}

init_workday(){
	enable=`uci_get_workday enabled`
	[ "$enable" != "1" ] && sed -i '/#oafControl_workday/d' /etc/crontabs/root && return
	workday=$(uci_get_workday daysofweek | sed "s/ /,/g")
    morning_start=$(uci_get_workday workday_morning_start)
    morning_stop=$(uci_get_workday workday_morning_stop)
    afternoon_start=$(uci_get_workday workday_afternoon_start)
    afternoon_stop=$(uci_get_workday workday_afternoon_stop)
    night_start=$(uci_get_workday workday_night_start)
    night_stop=$(uci_get_workday workday_night_stop)
	morning_start_hour=$(isHour `echo $morning_start | separate_front` 2>/dev/null)
	morning_start_minu=$(isMinu `echo $morning_start | separate_back` 2>/dev/null)
	morning_stop_hour=$(isHour `echo $morning_stop | separate_front` 2>/dev/null)
	morning_stop_minu=$(isMinu `echo $morning_stop | separate_back` 2>/dev/null)
	afternoon_start_hour=$(isHour `echo $afternoon_start | separate_front` 2>/dev/null)
	afternoon_start_minu=$(isMinu `echo $afternoon_start | separate_back` 2>/dev/null)
	afternoon_stop_hour=$(isHour `echo $afternoon_stop | separate_front` 2>/dev/null)
	afternoon_stop_minu=$(isMinu `echo $afternoon_stop | separate_back` 2>/dev/null)
	night_start_hour=$(isHour `echo $night_start | separate_front` 2>/dev/null)
	night_start_minu=$(isMinu `echo $night_start | separate_back` 2>/dev/null)
	night_stop_hour=$(isHour `echo $night_stop | separate_front` 2>/dev/null)
	night_stop_minu=$(isMinu `echo $night_stop | separate_back` 2>/dev/null)
	[ "$morning_start_minu" -a "$morning_start_hour" ] && add_cron $morning_start_minu $morning_start_hour $workday "oafControl_workday_morning_start" "start"
	[ "$morning_start_minu" -a "$morning_start_hour" -a "$morning_stop_minu" -a "$morning_stop_hour" ] && add_cron $morning_stop_minu $morning_stop_hour $workday "oafControl_workday_morning_stop"
	[ "$afternoon_start_minu" -a "$afternoon_stop_hour" ] && add_cron $afternoon_start_minu $afternoon_start_hour $workday "oafControl_workday_afternoon_start" "start"
	[ "$afternoon_start_minu" -a "$afternoon_stop_hour" -a "$afternoon_stop_minu" -a "afternoon_stop_hour$" ] && add_cron $afternoon_stop_minu $afternoon_stop_hour $workday "oafControl_workday_afternoon_stop"
	[ "$night_start_minu" -a "$night_start_hour" ] && add_cron $night_start_minu $night_start_hour $workday "oafControl_workday_night_start" "start"
	[ "$night_start_minu" -a "$night_start_hour" -a "$night_stop_minu" -a "$night_stop_hour" ] && add_cron $night_stop_minu $night_stop_hour $workday "oafControl_workday_night_stop"
	killall crond
	/usr/sbin/crond
	echo "`DATE`：已添加工作日定时任务" >> $LOGFILE
}

init_weekday(){
	enable=`uci_get_weekday enabled`
	[ "$enable" != "1" ] && sed -i '/#oafControl_weekday/d' /etc/crontabs/root && return
	weekday=$(uci_get_weekday daysofweek | sed "s/ /,/g")
    morning_start=$(uci_get_weekday weekday_morning_start)
    morning_stop=$(uci_get_weekday weekday_morning_stop)
    afternoon_start=$(uci_get_weekday weekday_afternoon_start)
    afternoon_stop=$(uci_get_weekday weekday_afternoon_stop)
    night_start=$(uci_get_weekday weekday_night_start)
    night_stop=$(uci_get_weekday weekday_night_stop)
	morning_start_hour=$(isHour `echo $morning_start | separate_front` 2>/dev/null)
	morning_start_minu=$(isMinu `echo $morning_start | separate_back` 2>/dev/null)
	morning_stop_hour=$(isHour `echo $morning_stop | separate_front` 2>/dev/null)
	morning_stop_minu=$(isMinu `echo $morning_stop | separate_back` 2>/dev/null)
	afternoon_start_hour=$(isHour `echo $afternoon_start | separate_front` 2>/dev/null)
	afternoon_start_minu=$(isMinu `echo $afternoon_start | separate_back` 2>/dev/null)
	afternoon_stop_hour=$(isHour `echo $afternoon_stop | separate_front` 2>/dev/null)
	afternoon_stop_minu=$(isMinu `echo $afternoon_stop | separate_back` 2>/dev/null)
	night_start_hour=$(isHour `echo $night_start | separate_front` 2>/dev/null)
	night_start_minu=$(isMinu `echo $night_start | separate_back` 2>/dev/null)
	night_stop_hour=$(isHour `echo $night_stop | separate_front` 2>/dev/null)
	night_stop_minu=$(isMinu `echo $night_stop | separate_back` 2>/dev/null)
	[ "$morning_start_minu" -a "$morning_start_hour" ] && add_cron $morning_start_minu $morning_start_hour $weekday "oafControl_weekday_morning_start" "start"
	[ "$morning_start_minu" -a "$morning_start_hour" -a "$morning_stop_minu" -a "$morning_stop_hour" ] && add_cron $morning_stop_minu $morning_stop_hour $weekday "oafControl_weekday_morning_stop"
	[ "$afternoon_start_minu" -a "$afternoon_stop_hour" ] && add_cron $afternoon_start_minu $afternoon_start_hour $weekday "oafControl_weekday_afternoon_start" "start"
	[ "$afternoon_start_minu" -a "$afternoon_stop_hour" -a "$afternoon_stop_minu" -a "afternoon_stop_hour$" ] && add_cron $afternoon_stop_minu $afternoon_stop_hour $weekday "oafControl_weekday_afternoon_stop"
	[ "$night_start_minu" -a "$night_start_hour" ] && add_cron $night_start_minu $night_start_hour $weekday "oafControl_weekday_night_start" "start"
	[ "$night_start_minu" -a "$night_start_hour" -a "$night_stop_minu" -a "$night_stop_hour" ] && add_cron $night_stop_minu $night_stop_hour $weekday "oafControl_weekday_night_stop"
	killall crond
	/usr/sbin/crond
	echo "`DATE`：已添加非工作日定时任务" >> $LOGFILE
}

control_appfilter(){
	uci set appfilter.global.enable=$1
	uci commit appfilter
	/etc/init.d/appfilter restart
	[ $1 = 1 ] && echo "`DATE`：应用过滤已开启" >> $LOGFILE
	[ $1 = 0 ] && echo "`DATE`：应用过滤已关闭" >> $LOGFILE
}

isVacation(){
	today=`date +%Y-%m-%d`
	if [ -s "$VACATIONFILE" ];then
		for line in `cat $VACATIONFILE` 
		do 
			if [ "$line" = "$today" ]; then
#				control_appfilter 0
				echo "`DATE`：今天放假" >> $LOGFILE
				echo "1"
				return
			fi
		done
	else
#		control_appfilter 1
		echo "`DATE`：今天正常上班" >> $LOGFILE
		echo "0"
	fi
}

isOvertime(){
	today=`date +%Y-%m-%d`
	if [ -s "$OVERTIMEFILE" ];then
		for line in `cat $VACATIONFILE` 
		do 
			if [ "$line" = "$today" ]; then
#				control_appfilter 1
				echo "`DATE`：今天加班" >> $LOGFILE
				echo "1"
				return
			fi
		done	
	else
#		control_appfilter 0
		echo "`DATE`：今天放假" >> $LOGFILE
		echo "1"
	fi
}
	
global_control(){
	global_enable=$(uci get oafControl.@global[0].global_enabled 2>/dev/null)
	[ $global_enable != 1 ] && return
	rest=`isVacation`
	over=`isOvertime`
	if [ "$rest" = "0" ]; then
		sed -i '/#oafControl_overtime_/d' /etc/crontabs/root
		killall crond
		/usr/sbin/crond
	fi
	if [ "$over" = "0" ]; then
		today=`date +%Y-%m-%d`
		month=$(expr `echo $val_1 | awk -F "-" '{print $2}'` + 0)
		day=$(expr `echo $val_1 | awk -F "-" '{print $3}'` + 0)
		morning_start=$(uci_get_weekday weekday_morning_start)
		morning_stop=$(uci_get_weekday weekday_morning_stop)
		afternoon_start=$(uci_get_weekday weekday_afternoon_start)
		afternoon_stop=$(uci_get_weekday weekday_afternoon_stop)
		night_start=$(uci_get_weekday weekday_night_start)
		night_stop=$(uci_get_weekday weekday_night_stop)
		morning_start_hour=$(isHour `echo $morning_start | separate_front` 2>/dev/null)
		morning_start_minu=$(isMinu `echo $morning_start | separate_back` 2>/dev/null)
		morning_stop_hour=$(isHour `echo $morning_stop | separate_front` 2>/dev/null)
		morning_stop_minu=$(isMinu `echo $morning_stop | separate_back` 2>/dev/null)
		afternoon_start_hour=$(isHour `echo $afternoon_start | separate_front` 2>/dev/null)
		afternoon_start_minu=$(isMinu `echo $afternoon_start | separate_back` 2>/dev/null)
		afternoon_stop_hour=$(isHour `echo $afternoon_stop | separate_front` 2>/dev/null)
		afternoon_stop_minu=$(isMinu `echo $afternoon_stop | separate_back` 2>/dev/null)
		night_start_hour=$(isHour `echo $night_start | separate_front` 2>/dev/null)
		night_start_minu=$(isMinu `echo $night_start | separate_back` 2>/dev/null)
		night_stop_hour=$(isHour `echo $night_stop | separate_front` 2>/dev/null)
		night_stop_minu=$(isMinu `echo $night_stop | separate_back` 2>/dev/null)
		[ "$morning_start_minu" -a "$morning_start_hour" ] && overtime_add_cron $morning_start_minu $morning_start_hour $day $month "oafControl_overtime_morning_start" "start"
		[ "$morning_start_minu" -a "$morning_start_hour" -a "$morning_stop_minu" -a "$morning_stop_hour" ] && overtime_add_cron $morning_stop_minu $morning_stop_hour $day $month "oafControl_overtime_morning_stop"
		[ "$afternoon_start_minu" -a "$afternoon_stop_hour" ] && overtime_add_cron $afternoon_start_minu $afternoon_start_hour $day $month "oafControl_overtime_afternoon_start" "start"
		[ "$afternoon_start_minu" -a "$afternoon_stop_hour" -a "$afternoon_stop_minu" -a "afternoon_stop_hour$" ] && overtime_add_cron $afternoon_stop_minu $afternoon_stop_hour $day $month "oafControl_overtime_afternoon_stop"
		[ "$night_start_minu" -a "$night_start_hour" ] && overtime_add_cron $night_start_minu $night_start_hour $day $month "oafControl_overtime_night_start" "start"
		[ "$night_start_minu" -a "$night_start_hour" -a "$night_stop_minu" -a "$night_stop_hour" ] && overtime_add_cron $night_stop_minu $night_stop_hour $day $month "oafControl_overtime_night_stop"
		killall crond
		/usr/sbin/crond
	fi
}
global_start(){
	control_appfilter 1
	echo "`DATE`：上班了" >> $LOGFILE
}

global_stop(){
	control_appfilter 0
	echo "`DATE`：下班了" >> $LOGFILE
	[ -n "$1" ] && 	(sed -i '/#oafControl/d' /etc/crontabs/root && echo "`DATE`：关闭应用过滤控制器" >> $LOGFILE)
}

[ "$mode" = "global_stop" -a "$fun" = "all" ] && global_stop all
[ "$mode" = "global_stop" -a "$fun" != "all" ] && global_stop
[ "$mode" = "init_overtime" ] && init_overtime
[ "$mode" = "init_vacation" ] && init_vacation
[ "$mode" = "init_weekday" ] && init_weekday
[ "$mode" = "init_workday" ] && init_workday
[ "$mode" = "global_start" ] && global_start
[ "$mode" = "auto_update" ] && auto_update
[ "$mode" = "remote_control" ] && remote_control
[ "$mode" = "global_control" ] && global_control

