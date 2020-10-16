#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh

START_LOG="/tmp/openclash_start.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE="/tmp/openclash.log"
CORE_TYPE="$1"
[ -z "$CORE_TYPE" ] || [ "$1" = "one_key_update" ] && CORE_TYPE="Dev"
en_mode=$(uci get openclash.config.en_mode 2>/dev/null)
small_flash_memory=$(uci get openclash.config.small_flash_memory 2>/dev/null)
CPU_MODEL=$(uci get openclash.config.core_version 2>/dev/null)

[ ! -f "/tmp/clash_last_version" ] && /usr/share/openclash/clash_version.sh 2>/dev/null
if [ "$small_flash_memory" != "1" ]; then
   dev_core_path="/etc/openclash/core/clash"
   tun_core_path="/etc/openclash/core/clash_tun"
   game_core_path="/etc/openclash/core/clash_game"
   mkdir -p /etc/openclash/core
else
   dev_core_path="/tmp/etc/openclash/core/clash"
   tun_core_path="/tmp/etc/openclash/core/clash_tun"
   game_core_path="/tmp/etc/openclash/core/clash_game"
   mkdir -p /tmp/etc/openclash/core
fi

case $CORE_TYPE in
	"Tun")
   CORE_CV=$($tun_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 2p /tmp/clash_last_version 2>/dev/null)
   if [ -z "$CORE_LV" ]; then
      echo "获取【Tun】内核最新版本信息失败，请稍后再试..." >$START_LOG
      echo "${LOGTIME} Error: 【Tun】Core Version Check Error, Please Try Again After A few Seconds" >>$LOG_FILE
      sleep 5
      echo "" >$START_LOG
      exit 0
   fi
   if [ "$en_mode" = "fake-ip-tun" ] || [ "$en_mode" = "redir-host-tun" ] || [ "$en_mode" = "redir-host-mix" ] || [ "$en_mode" = "fake-ip-mix" ]; then
      if_restart=1
   fi
   ;;
	"Game")
   CORE_CV=$($game_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 3p /tmp/clash_last_version 2>/dev/null)
   if [ "$en_mode" = "fake-ip-vpn" ] || [ "$en_mode" = "redir-host-vpn" ]; then
      if_restart=1
   fi
   ;;
   *)
   CORE_CV=$($dev_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 1p /tmp/clash_last_version 2>/dev/null)
   if [ "$en_mode" = "fake-ip" ] || [ "$en_mode" = "redir-host" ]; then
      if_restart=1
   fi
esac

[ -n "$2" ] || [ "$1" = "one_key_update" ] && if_restart=0

if [ "$CORE_CV" != "$CORE_LV" ] || [ -z "$CORE_CV" ]; then
   if [ "$CPU_MODEL" != 0 ]; then
   if pidof clash >/dev/null; then
			case $CORE_TYPE in
      	"Tun")
      	echo "正在下载【Tun】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://github.com/vernesong/OpenClash/releases/download/TUN-Premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz >/dev/null 2>&1
				;;
				"Game")
				echo "正在下载【Game】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://github.com/vernesong/OpenClash/releases/download/TUN/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_game.tar.gz >/dev/null 2>&1
				;;
				*)
				echo "正在下载【Dev】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://github.com/vernesong/OpenClash/releases/download/Clash/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz >/dev/null 2>&1
			esac
   else
			case $CORE_TYPE in
      	"Tun")
      	echo "正在下载【Tun】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz >/dev/null 2>&1
				;;
				"Game")
				echo "正在下载【Game】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/game/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_game.tar.gz >/dev/null 2>&1
				;;
				*)
				echo "正在下载【Dev】版本内核，如下载失败请尝试手动下载并上传..." >$START_LOG
				curl -sL -m 30 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz >/dev/null 2>&1
			esac
   fi
   if [ "$?" -eq "0" ]; then
   	  echo "【"$CORE_TYPE"】版本内核下载成功，开始更新..." >$START_LOG
			case $CORE_TYPE in
      	"Tun")
				[ -s "/tmp/clash_tun.gz" ] && {
					gzip -d /tmp/clash_tun.gz >/dev/null 2>&1
					rm -rf /tmp/clash_tun.gz >/dev/null 2>&1
					rm -rf "$tun_core_path" >/dev/null 2>&1
					chmod 4755 /tmp/clash_tun >/dev/null 2>&1
					chown root:root /tmp/clash_tun >/dev/null 2>&1
				}
				;;
				"Game")
				[ -s "/tmp/clash_game.tar.gz" ] && {
					tar zxvf /tmp/clash_game.tar.gz -C /tmp >/dev/null 2>&1
					mv /tmp/clash /tmp/clash_game >/dev/null 2>&1
          rm -rf /tmp/clash_game.tar.gz >/dev/null 2>&1
					rm -rf "$game_core_path" >/dev/null 2>&1
					chmod 4755 /tmp/clash_game >/dev/null 2>&1
					chown root:root /tmp/clash_game >/dev/null 2>&1
				}
				;;
				*)
				[ -s "/tmp/clash.tar.gz" ] && {
					rm -rf "$dev_core_path" >/dev/null 2>&1
					if [ "$small_flash_memory" != "1" ]; then
					   tar zxvf /tmp/clash.tar.gz -C /etc/openclash/core
					else
					   tar zxvf /tmp/clash.tar.gz -C /tmp/etc/openclash/core
				  fi
					rm -rf /tmp/clash.tar.gz >/dev/null 2>&1
					chmod 4755 "$dev_core_path" >/dev/null 2>&1
					chown root:root "$dev_core_path" >/dev/null 2>&1
				}
			esac
      
      if [ "$?" -ne "0" ]; then
      	echo "【"$CORE_TYPE"】版本内核更新失败，请检查网络或稍后再试！" >$START_LOG
        echo "${LOGTIME} Error: OpenClash 【"$CORE_TYPE"】 Core Update Error" >>$LOG_FILE
        case $CORE_TYPE in
            "Tun")
				    rm -rf /tmp/clash_tun >/dev/null 2>&1
				    ;;
				    "Game")
				    rm -rf /tmp/clash_game >/dev/null 2>&1
				    ;;
				    *)
			   esac
         sleep 5
         echo "" >$START_LOG
         exit 0
      fi
      
      if [ "$if_restart" -eq 1 ]; then
      	 clash_pids=$(pidof clash|sed 's/$//g')
         for clash_pid in $clash_pids; do
            kill -9 "$clash_pid" 2>/dev/null
         done
      fi
      
			case $CORE_TYPE in
      	"Tun")
				mv /tmp/clash_tun "$tun_core_path" >/dev/null 2>&1
				;;
				"Game")
				mv /tmp/clash_game "$game_core_path" >/dev/null 2>&1
				;;
				*)
			esac
      if [ "$?" -eq "0" ]; then
         echo "【"$CORE_TYPE"】版本内核更新成功！" >$START_LOG
         echo "${LOGTIME} OpenClash 【"$CORE_TYPE"】 Core Update Successful" >>$LOG_FILE
         sleep 3
         if [ -n "$2" ] || [ "$1" = "one_key_update" ]; then
         	 uci set openclash.config.config_reload=0
         	 uci commit openclash
         fi
         [ "$if_restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart
         echo "" >$START_LOG
      else
         echo "【"$CORE_TYPE"】版本内核更新失败，请确认设备闪存空间足够后再试！" >$START_LOG
         echo "${LOGTIME} Error: OpenClash 【"$CORE_TYPE"】 Core Update Error" >>$LOG_FILE
         case $CORE_TYPE in
            "Tun")
				    rm -rf /tmp/clash_tun >/dev/null 2>&1
				    ;;
				    "Game")
				    rm -rf /tmp/clash_game >/dev/null 2>&1
				    ;;
				    *)
			   esac
         sleep 5
         echo "" >$START_LOG
      fi
   else
      echo "【"$CORE_TYPE"】版本内核下载失败，请检查网络或稍后再试！" >$START_LOG
      echo "${LOGTIME} Error: OpenClash 【"$CORE_TYPE"】 Core Update Error" >>$LOG_FILE
      case $CORE_TYPE in
         "Tun")
			   rm -rf /tmp/clash_tun >/dev/null 2>&1
			   ;;
			   "Game")
			   rm -rf /tmp/clash_game >/dev/null 2>&1
			   ;;
			   *)
			   rm -rf /tmp/clash >/dev/null 2>&1
		  esac
      sleep 10
      echo "" >$START_LOG
   fi
   else
      echo "未选择编译版本，请到全局设置中选择后再试！" >$START_LOG
      sleep 10
      echo "" >$START_LOG
   fi
else
      echo "【"$CORE_TYPE"】版本内核没有更新，停止继续操作！" >$START_LOG
      echo "${LOGTIME} OpenClash 【"$CORE_TYPE"】 Core No Change, Do Nothing" >>$LOG_FILE
      sleep 5
      echo "" >$START_LOG
fi
