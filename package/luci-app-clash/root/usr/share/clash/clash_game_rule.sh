#!/bin/bash /etc/rc.common
. /lib/functions.sh

   RULE_FILE_NAME="$1" 
   RULE_FILE_ENNAME=$(grep -F $RULE_FILE_NAME /usr/share/clash/rules/rules.list |awk -F ',' '{print $3}' 2>/dev/null)
   if [ ! -z "$RULE_FILE_ENNAME" ]; then
      DOWNLOAD_PATH=$(grep -F $RULE_FILE_NAME /usr/share/clash/rules/rules.list |awk -F ',' '{print $2}' 2>/dev/null)
   else
      DOWNLOAD_PATH=$RULE_FILE_NAME
   fi
   RULE_FILE_DIR="/usr/share/clash/rules/g_rules/$RULE_FILE_NAME"
   TMP_RULE_DIR="/tmp/$RULE_FILE_NAME"
   LOG_FILE="/usr/share/clash/clash.txt"
   REAL_LOG="/usr/share/clash/clash_real.txt"
   
   
	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating 【$RULE_FILE_NAME】 Rule..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始下载【$RULE_FILE_NAME】规则..." >$REAL_LOG
	fi 
	
   wget --no-check-certificate -c4 https://raw.githubusercontent.com/FQrabbit/SSTap-Rule/master/rules/"$DOWNLOAD_PATH" -O 2>&1 >1 "$TMP_RULE_DIR"
   
   if [ "$?" -eq "0" ] && [ "$(ls -l $TMP_RULE_DIR |awk '{print $5}')" -ne 0 ]; then
   
	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "【$RULE_FILE_NAME】 Downloaded successfully. Checking whether the rule version is updated.." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "【$RULE_FILE_NAME】规则下载成功，检查规则版本是否更新..." >$REAL_LOG
	fi
	
      cmp -s $TMP_RULE_DIR $RULE_FILE_DIR
         if [ "$?" -ne "0" ]; then
			
			if [ $lang == "en" ] || [ $lang == "auto" ];then
						echo "Rule version has been updated. Start to replace the old rule version.." >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
						echo "规则版本有更新，开始替换旧规则版本..." >$REAL_LOG
			fi				
            mv $TMP_RULE_DIR $RULE_FILE_DIR >/dev/null 2>&1
			
			if [ $lang == "en" ] || [ $lang == "auto" ];then
						echo "Delete download cache" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
						echo "删除下载缓存..." >$REAL_LOG
			fi  

            rm -rf $TMP_RULE_DIR >/dev/null 2>&1
			
			if [ $lang == "en" ] || [ $lang == "auto" ];then
						echo "Rule File【$RULE_FILE_NAME】 Download Successful" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
						echo "【$RULE_FILE_NAME】规则更新成功！" >$REAL_LOG
			fi 
		    sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
         else
			if [ $lang == "en" ] || [ $lang == "auto" ];then
						echo "Updated Rule File【$RULE_FILE_NAME】 No Change, Do Nothing" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
						echo "【$RULE_FILE_NAME】规则版本没有更新，停止继续操作..." >$REAL_LOG
			fi 

            rm -rf $TMP_RULE_DIR >/dev/null 2>&1
			  sleep 2
			  echo "Clash for OpenWRT" >$REAL_LOG
         fi
   else
			if [ $lang == "en" ] || [ $lang == "auto" ];then
						echo "Rule File【$RULE_FILE_NAME】 Download Error" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
						echo "【$RULE_FILE_NAME】规则下载失败，请检查网络或稍后再试！" >$REAL_LOG
			fi 
			  rm -rf $TMP_RULE_DIR >/dev/null 2>&1	  
			  sleep 2
			  echo "Clash for OpenWRT" >$REAL_LOG
   fi
