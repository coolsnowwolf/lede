#!/bin/sh

f_file=$1
test -z "$f_file" && return
test -d /tmp/appfilter && return
cur_class=""
cur_class_file=""
mkdir /tmp/appfilter
while read line
do
    echo "$line"| grep "^#class"
    if [ $? -eq 0 ];then
        class=`echo $line| grep '#class' | awk '{print $2}'`
	if ! test -z "$class";then
		cur_class=$class
		cur_class_file="/tmp/appfilter/${cur_class}.class"
		if [ -e "$cur_class_file" ];then
			rm $cur_class_file 
		fi
		touch $cur_class_file
	fi
    fi
    test -z "$cur_class" && continue
    appid=`echo "$line" |awk '{print $1}'`
    appname=`echo "$line" | awk '{print $2}' | awk -F: '{print $1}'`

    echo "$appid $appname" >> $cur_class_file
done  < $f_file
echo "ok"

