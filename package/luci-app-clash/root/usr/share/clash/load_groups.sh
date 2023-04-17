#!/bin/bash /etc/rc.common
. /lib/functions.sh

lang=$(uci get luci.main.lang 2>/dev/null) 
load="/tmp/config.yaml"
CONFIG_YAML_PATH=$(uci get clash.config.use_config 2>/dev/null)


if [  -f $CONFIG_YAML_PATH ] && [ "$(ls -l $CONFIG_YAML_PATH|awk '{print int($5)}')" -ne 0 ];then
	cp $CONFIG_YAML_PATH $load 2>/dev/null		
fi

if [ ! -f $load ] || [ "$(ls -l $load|awk '{print int($5)}')" -eq 0 ]; then 
  exit 0
fi 

CFG_FILE="/etc/config/clash"
REAL_LOG="/usr/share/clash/clash_real.txt"

rm -rf /tmp/Proxy_Group /tmp/group_*.yaml /tmp/yaml_group.yaml 2>/dev/null


	if [ $lang == "zh_cn" ];then
		echo "开始更新策略组配置..." >$REAL_LOG 
	elif [ $lang == "en" ] || [ $lang == "auto" ];then
    	echo "Start updating policy group config" >$REAL_LOG
	fi

	   sed -i 's/^Proxy Group:/proxy-groups:/g' "$load"
	   sed -i 's/^proxy-provider:/proxy-providers:/g' "$load"
	   sed -i 's/^Proxy:/proxies:/g' "$load"
	   sed -i 's/^Rule:/rules:/g' "$load"
	   sed -i 's/^rule-provider:/rule-providers:/g' "$load"
	   

   group_len=$(sed -n '/^ \{0,\}proxy-groups:/=' "$load" 2>/dev/null)
   provider_len=$(sed -n '/^ \{0,\}proxy-providers:/=' "$load" 2>/dev/null)
   if [ "$provider_len" -ge "$group_len" ]; then
       awk '/proxies:/,/proxy-providers:/{print}' "$load" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
       sed -i "s/proxy-providers://g" /tmp/Proxy_Group 2>&1
   else
       awk '/proxies:/,/rules:/{print}' "$load" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   fi
   
   
   if [ "$?" -eq "0" ]; then
      echo 'DIRECT' >>/tmp/Proxy_Group
      echo 'REJECT' >>/tmp/Proxy_Group
   else
      
	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Read error, configuration file exception!" >/tmp/Proxy_Group
		elif [ $lang == "zh_cn" ];then
			echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
		fi
   fi



group_len=$(sed -n '/^ \{0,\}proxy-groups:/=' "$load" 2>/dev/null)
provider_len=$(sed -n '/^ \{0,\}proxy-providers:/=' "$load" 2>/dev/null)
ruleprovider_len=$(sed -n '/^ \{0,\}rule-providers:/=' "$load" 2>/dev/null)
if [ "$provider_len" -ge "$group_len" ]; then
   awk '/proxy-groups:/,/proxy-providers:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1
   sed -i "s/proxy-providers://g" /tmp/yaml_group.yaml 2>&1
elif [ "$ruleprovider_len" -ge "$group_len" ]; then
   awk '/proxy-groups:/,/rule-providers:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1
   sed -i "s/rule-providers://g" /tmp/yaml_group.yaml 2>&1
else
   awk '/proxy-groups:/,/Rule:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1
fi



#######READ GROUPS START


if [ -f /tmp/yaml_group.yaml ];then
	while [[ "$( grep -c "config conf_groups" $CFG_FILE )" -ne 0 ]] 
	do
      uci delete clash.@conf_groups[0] && uci commit clash >/dev/null 2>&1
	done



count=1
file_count=1
match_group_file="/tmp/Proxy_Group"
group_file="/tmp/yaml_group.yaml"
line=$(sed -n '/name:/=' $group_file)
num=$(grep -c "name:" $group_file)
   
cfg_get()
{
	echo "$(grep "$1" "$2" 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null)"
}



for n in $line
do
   single_group="/tmp/group_$file_count.yaml"
   
   [ "$count" -eq 1 ] && {
      startLine="$n"
  }

   count=$(expr "$count" + 1)
   if [ "$count" -gt "$num" ]; then
      endLine=$(sed -n '$=' $group_file)
   else
      endLine=$(expr $(echo "$line" | sed -n "${count}p") - 1)
   fi
  
   sed -n "${startLine},${endLine}p" $group_file >$single_group
   startLine=$(expr "$endLine" + 1)
   
   #type
   group_type="$(cfg_get "type:" "$single_group")"
   #name
   group_name="$(cfg_get "name:" "$single_group")"
   #test_url
   
	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Now Reading 【$group_type】-【$group_name】 Policy Group..." >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "正在读取【$group_type】-【$group_name】策略组配置..." >$REAL_LOG
		fi
		
   name=clash
   uci_name_tmp=$(uci add $name conf_groups)
   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."
   ${uci_set}name="$group_name"
   ${uci_set}type="$group_type"


   file_count=$(( $file_count + 1))
    
done

uci commit clash

 	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Reading Policy Group Completed" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "读取策略组配置完成" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG			
		fi

rm -rf /tmp/Proxy_Group /tmp/group_*.yaml /tmp/yaml_group.yaml $load 2>/dev/null
fi
#######READ GROUPS END