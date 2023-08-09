#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh
. /usr/share/openclash/log.sh

CORE_TYPE="$1"
C_CORE_TYPE=$(uci -q get openclash.config.core_type)
[ -z "$CORE_TYPE" ] || [ "$1" = "one_key_update" ] && CORE_TYPE="Dev"
small_flash_memory=$(uci -q get openclash.config.small_flash_memory)
CPU_MODEL=$(uci -q get openclash.config.core_version)
RELEASE_BRANCH=$(uci -q get openclash.config.release_branch || echo "master")
github_address_mod=$(uci -q get openclash.config.github_address_mod || echo 0)
LOG_FILE="/tmp/openclash.log"

[ ! -f "/tmp/clash_last_version" ] && /usr/share/openclash/clash_version.sh 2>/dev/null
if [ ! -f "/tmp/clash_last_version" ]; then
   LOG_OUT "Error: 【"$CORE_TYPE"】Core Version Check Error, Please Try Again Later..."
   SLOG_CLEAN
   exit 0
fi

if [ "$small_flash_memory" != "1" ]; then
   dev_core_path="/etc/openclash/core/clash"
   tun_core_path="/etc/openclash/core/clash_tun"
   meta_core_path="/etc/openclash/core/clash_meta"
   mkdir -p /etc/openclash/core
else
   dev_core_path="/tmp/etc/openclash/core/clash"
   tun_core_path="/tmp/etc/openclash/core/clash_tun"
   meta_core_path="/tmp/etc/openclash/core/clash_meta"
   mkdir -p /tmp/etc/openclash/core
fi

case $CORE_TYPE in
	"TUN")
   CORE_CV=$($tun_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 2p /tmp/clash_last_version 2>/dev/null)
   if [ -z "$CORE_LV" ]; then
      LOG_OUT "Error: 【"$CORE_TYPE"】Core Version Check Error, Please Try Again Later..."
      SLOG_CLEAN
      exit 0
   fi
   ;;
   "Meta")
   CORE_CV=$($meta_core_path -v 2>/dev/null |awk -F ' ' '{print $3}' |head -1)
   CORE_LV=$(sed -n 3p /tmp/clash_last_version 2>/dev/null)
   ;;
   *)
   CORE_CV=$($dev_core_path -v 2>/dev/null |awk -F ' ' '{print $2}')
   CORE_LV=$(sed -n 1p /tmp/clash_last_version 2>/dev/null)
esac
   
[ "$C_CORE_TYPE" = "$CORE_TYPE" ] || [ -z "$C_CORE_TYPE" ] && if_restart=1

if [ "$CORE_CV" != "$CORE_LV" ] || [ -z "$CORE_CV" ]; then
   if [ "$CPU_MODEL" != 0 ]; then
      case $CORE_TYPE in
         "TUN")
            LOG_OUT "【TUN】Core Downloading, Please Try to Download and Upload Manually If Fails"
            if [ "$github_address_mod" != "0" ]; then
               if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/vernesong/OpenClash@core/"$RELEASE_BRANCH"/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_tun.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/vernesong/OpenClash/core/"$RELEASE_BRANCH"/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_tun.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               else
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_tun.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               fi
            else
			      curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_tun.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
			   fi
            if [ "${PIPESTATUS[0]}" -ne 0 ]; then
               curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/o/op/openclash/"$RELEASE_BRANCH"/core-lateset/premium/clash-"$CPU_MODEL"-"$CORE_LV".gz -o /tmp/clash_tun.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_tun.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               curl_status=${PIPESTATUS[0]}
            else
               curl_status=0
            fi
            if [ "$curl_status" -eq 0 ]; then
               gzip -t /tmp/clash_tun.gz >/dev/null 2>&1
            fi
			   ;;
         "Meta")
            LOG_OUT "【Meta】Core Downloading, Please Try to Download and Upload Manually If Fails"
            if [ "$github_address_mod" != "0" ]; then
               if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/vernesong/OpenClash@core/"$RELEASE_BRANCH"/meta/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_meta.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_meta.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/vernesong/OpenClash/core/"$RELEASE_BRANCH"/meta/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_meta.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_meta.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               else
                  curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/meta/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_meta.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_meta.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               fi
            else
               curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/meta/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_meta.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_meta.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
            fi
            if [ "${PIPESTATUS[0]}" -ne 0 ]; then
               curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/o/op/openclash/"$RELEASE_BRANCH"/core-lateset/meta/clash-"$CPU_MODEL".tar.gz -o /tmp/clash_meta.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash_meta.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
               curl_status=${PIPESTATUS[0]}
            else
               curl_status=0
            fi
            if [ "$curl_status" -eq 0 ]; then
			      gzip -t /tmp/clash_meta.tar.gz >/dev/null 2>&1
			   fi
			   ;;
			   *)
			      LOG_OUT "【Dev】Core Downloading, Please Try to Download and Upload Manually If Fails"
			      if [ "$github_address_mod" != "0" ]; then
                  if [ "$github_address_mod" == "https://cdn.jsdelivr.net/" ] || [ "$github_address_mod" == "https://fastly.jsdelivr.net/" ] || [ "$github_address_mod" == "https://testingcf.jsdelivr.net/" ]; then
                     curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"gh/vernesong/OpenClash@core/"$RELEASE_BRANCH"/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
                  elif [ "$github_address_mod" == "https://raw.fastgit.org/" ]; then
                     curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.fastgit.org/vernesong/OpenClash/core/"$RELEASE_BRANCH"/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
                  else
                     curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 "$github_address_mod"https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
                  fi
               else
			         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://raw.githubusercontent.com/vernesong/OpenClash/core/"$RELEASE_BRANCH"/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
			      fi
			      if [ "${PIPESTATUS[0]}" -ne 0 ]; then
			         curl -SsL --connect-timeout 30 -m 60 --speed-time 30 --speed-limit 1 --retry 2 https://ftp.jaist.ac.jp/pub/sourceforge.jp/storage/g/o/op/openclash/"$RELEASE_BRANCH"/core-lateset/dev/clash-"$CPU_MODEL".tar.gz -o /tmp/clash.tar.gz 2>&1 | awk -v time="$(date "+%Y-%m-%d %H:%M:%S")" -v file="/tmp/clash.tar.gz" '{print time "【" file "】Download Failed:【"$0"】"}' >> "$LOG_FILE"
                  curl_status=${PIPESTATUS[0]}
               else
                  curl_status=0
               fi
               if [ "$curl_status" -eq 0 ]; then
			         gzip -t /tmp/clash.tar.gz >/dev/null 2>&1
			      fi
			esac

      if [ "$?" == "0" ]; then
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
         "Meta")
            [ -s "/tmp/clash_meta.tar.gz" ] && {
               rm -rf "$meta_core_path" >/dev/null 2>&1
               tar zxvf /tmp/clash_meta.tar.gz -C /tmp
               mv /tmp/clash /tmp/clash_meta >/dev/null 2>&1
               rm -rf /tmp/clash_meta.tar.gz >/dev/null 2>&1
               chmod 4755 /tmp/clash_meta >/dev/null 2>&1
            }
			   ;;
			   *)
            [ -s "/tmp/clash.tar.gz" ] && {
               rm -rf "$dev_core_path" >/dev/null 2>&1
               tar zxvf /tmp/clash.tar.gz -C /tmp
               rm -rf /tmp/clash.tar.gz >/dev/null 2>&1
               chmod 4755 /tmp/clash >/dev/null 2>&1
            }
         esac
         if [ "$?" != "0" ]; then
            LOG_OUT "【"$CORE_TYPE"】Core Update Failed. Please Make Sure Enough Flash Memory Space And Try Again!"
            case $CORE_TYPE in
            "TUN")
               rm -rf /tmp/clash_tun >/dev/null 2>&1
            ;;
            "Meta")
               rm -rf /tmp/clash_meta >/dev/null 2>&1
            ;;
            *)
               rm -rf /tmp/clash >/dev/null 2>&1
            esac
            SLOG_CLEAN
            exit 0
         fi

			   case $CORE_TYPE in
         "TUN")
			      mv /tmp/clash_tun "$tun_core_path" >/dev/null 2>&1
			   ;;
         "Meta")
               mv /tmp/clash_meta "$meta_core_path" >/dev/null 2>&1
			   ;;
         *)
               mv /tmp/clash "$dev_core_path" >/dev/null 2>&1
			   esac
			   
         if [ "$?" == "0" ]; then
            LOG_OUT "【"$CORE_TYPE"】Core Update Successful!"
            if [ "$if_restart" -eq 1 ]; then
                  uci -q set openclash.config.config_reload=0
         	      uci -q commit openclash
               if [ -z "$2" ] && [ "$1" != "one_key_update" ] && [ "$(find /tmp/lock/ |grep -v "openclash.lock" |grep -c "openclash")" -le 1 ] && [ "$(unify_ps_prevent)" -eq 0 ]; then
                  /etc/init.d/openclash restart >/dev/null 2>&1 &
               fi
            else
               SLOG_CLEAN
            fi
         else
            LOG_OUT "【"$CORE_TYPE"】Core Update Failed. Please Make Sure Enough Flash Memory Space And Try Again!"
            SLOG_CLEAN
         fi
      else
         LOG_OUT "【"$CORE_TYPE"】Core Update Failed, Please Check The Network or Try Again Later!"
         SLOG_CLEAN
      fi
   else
      LOG_OUT "No Compiled Version Selected, Please Select In Global Settings And Try Again!"
      SLOG_CLEAN
   fi
else
   LOG_OUT "【"$CORE_TYPE"】Core Has Not Been Updated, Stop Continuing Operation!"
   SLOG_CLEAN
fi

case $CORE_TYPE in
"TUN")
   rm -rf /tmp/clash_tun >/dev/null 2>&1
;;
"Meta")
   rm -rf /tmp/clash_meta >/dev/null 2>&1
;;
*)
   rm -rf /tmp/clash >/dev/null 2>&1
esac
