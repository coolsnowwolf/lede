#!/bin/sh
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

CORE_TYPE="$1"
C_CORE_TYPE=$(uci get openclash.config.core_type 2>/dev/null)
[ -z "$CORE_TYPE" ] || [ "$1" = "one_key_update" ] && CORE_TYPE="Dev"
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
	"TUN")
   CORE_CV=$($tun_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 2p /tmp/clash_last_version 2>/dev/null)
   if [ -z "$CORE_LV" ]; then
      LOG_OUT "Error: 【"$CORE_TYPE"】Core Version Check Error, Please Try Again Later..."
      sleep 5
      SLOG_CLEAN
      exit 0
   fi
   ;;
	"Game")
   CORE_CV=$($game_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 3p /tmp/clash_last_version 2>/dev/null)
   ;;
   *)
   CORE_CV=$($dev_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 1p /tmp/clash_last_version 2>/dev/null)
esac
   
[ "$C_CORE_TYPE" = "$CORE_TYPE" ] || [ -z "$C_CORE_TYPE" ] && if_restart=1
[ -n "$2" ] || [ "$1" = "one_key_update" ] && if_restart=0

if [ "$CORE_CV" != "$CORE_LV" ] || [ -z "$CORE_CV" ]; then
   if [ "$CPU_MODEL" != 0 ]; then
   if pidof clash >/dev/null; then
			case $CORE_TYPE in
      	"TUN")
      	LOG_OUT "【Tun】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://github.com/vernesong/OpenClash/releases/download/TUN-Premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz >/dev/null 2>&1
				;;
				"Game")
				LOG_OUT "【Game】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://github.com/vernesong/OpenClash/releases/download/TUN/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_game.tar.gz >/dev/null 2>&1
				;;
				*)
				LOG_OUT "【Dev】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://github.com/vernesong/OpenClash/releases/download/Clash/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz >/dev/null 2>&1
			esac
   fi
   if [ "$?" -ne "0" ] || ! pidof clash >/dev/null; then
			case $CORE_TYPE in
      	"TUN")
      	LOG_OUT "【Tun】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz >/dev/null 2>&1
				;;
				"Game")
				LOG_OUT "【Game】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/game/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_game.tar.gz >/dev/null 2>&1
				;;
				*)
				LOG_OUT "【Dev】Core Downloading, Please Try to Download and Upload Manually If Fails"
				curl -sL -m 10 --retry 2 https://cdn.jsdelivr.net/gh/vernesong/OpenClash@master/core-lateset/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz >/dev/null 2>&1
			esac
   fi
   if [ "$?" -eq "0" ]; then
   	  LOG_OUT "【"$CORE_TYPE"】Core Download Successful, Start Update..."
			case $CORE_TYPE in
      	"TUN")
				[ -s "/tmp/clash_tun.gz" ] && {
					gzip -d /tmp/clash_tun.gz >/dev/null 2>&1
					rm -rf /tmp/clash_tun.gz >/dev/null 2>&1
					rm -rf "$tun_core_path" >/dev/null 2>&1
					chmod 4755 /tmp/clash_tun >/dev/null 2>&1
				}
				;;
				"Game")
				[ -s "/tmp/clash_game.tar.gz" ] && {
					tar zxvf /tmp/clash_game.tar.gz -C /tmp >/dev/null 2>&1
					mv /tmp/clash /tmp/clash_game >/dev/null 2>&1
					rm -rf /tmp/clash_game.tar.gz >/dev/null 2>&1
					rm -rf "$game_core_path" >/dev/null 2>&1
					chmod 4755 /tmp/clash_game >/dev/null 2>&1
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
				}
			esac
      
      if [ "$?" -ne "0" ]; then
      	LOG_OUT "【"$CORE_TYPE"】Core Update Failed, Please Check The Network or Try Again Later!"
        case $CORE_TYPE in
            "TUN")
				    rm -rf /tmp/clash_tun >/dev/null 2>&1
				    ;;
				    "Game")
				    rm -rf /tmp/clash_game >/dev/null 2>&1
				    ;;
				    *)
			   esac
         sleep 5
         SLOG_CLEAN
         exit 0
      fi
      
      if [ "$if_restart" -eq 1 ]; then
      	 clash_pids=$(pidof clash|sed 's/$//g')
         for clash_pid in $clash_pids; do
            kill -9 "$clash_pid" 2>/dev/null
         done
      fi
      
			case $CORE_TYPE in
      	"TUN")
				mv /tmp/clash_tun "$tun_core_path" >/dev/null 2>&1
				;;
				"Game")
				mv /tmp/clash_game "$game_core_path" >/dev/null 2>&1
				;;
				*)
			esac
      if [ "$?" -eq "0" ]; then
         LOG_OUT "【"$CORE_TYPE"】Core Update Successful!"
         sleep 3
         if [ -n "$2" ] || [ "$1" = "one_key_update" ]; then
         	 uci set openclash.config.config_reload=0
         	 uci commit openclash
         fi
         [ "$if_restart" -eq 1 ] && [ "$(unify_ps_prevent)" -eq 0 ] && /etc/init.d/openclash restart
         SLOG_CLEAN
      else
         LOG_OUT "【"$CORE_TYPE"】Core Update Failed. Please Make Sure Enough Flash Memory Space And Try Again!"
         case $CORE_TYPE in
            "TUN")
				    rm -rf /tmp/clash_tun >/dev/null 2>&1
				    ;;
				    "Game")
				    rm -rf /tmp/clash_game >/dev/null 2>&1
				    ;;
				    *)
			   esac
         sleep 5
         SLOG_CLEAN
      fi
   else
      LOG_OUT "【"$CORE_TYPE"】Core Update Failed, Please Check The Network or Try Again Later!"
      case $CORE_TYPE in
         "TUN")
			   rm -rf /tmp/clash_tun >/dev/null 2>&1
			   ;;
			   "Game")
			   rm -rf /tmp/clash_game >/dev/null 2>&1
			   ;;
			   *)
			   rm -rf /tmp/clash >/dev/null 2>&1
		  esac
      sleep 10
      SLOG_CLEAN
   fi
   else
      LOG_OUT "No Compiled Version Selected, Please Select In Global Settings And Try Again!"
      sleep 10
      SLOG_CLEAN
   fi
else
      LOG_OUT "【"$CORE_TYPE"】Core Has Not Been Updated, Stop Continuing Operation!"
      sleep 5
      SLOG_CLEAN
fi
