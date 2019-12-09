. /usr/share/libubox/jshn.sh
. /lib/functions.sh

config_apply()
{
    test -z "$1" && return 1
    
	if [ -e "/dev/appfilter" ];then
    	echo "config json str=$1"
    	echo "$1" >/dev/appfilter
	fi
}

clean_rule()
{
    json_init
    echo "clean appfilter rule..."

    json_add_int "op" 3
    json_add_object "data"
    json_str=`json_dump`

    config_apply "$json_str"

    json_cleanup
}

load_rule()
{
    json_init

    config_load appfilter
    config_get enable "global" enable
    echo "enable = $enable"	
    if [ x"$enable" != x"1" ];then
	echo "appfilter is disabled"
	return 0
    fi 
    echo "appfilter is enabled"
    json_add_int "op" 1

    json_add_object "data"
    json_add_array "apps"

    for file in `ls /etc/appfilter/*.class`
    do
	class_name=`echo "$file" | awk -F/ '{print $4}'| awk -F. '{print $1}'`
	config_get appid_list "appfilter" "${class_name}apps"
	echo "appid_list=$appid_list"

	if ! test -z "$appid_list";then
	    for appid in $appid_list:
	    do
	        json_add_int "" $appid
	    done
	fi
    done

    json_str=`json_dump`
    config_apply "$json_str"
    json_cleanup
}

clean_rule
load_rule
