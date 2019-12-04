#!/bin/sh
logfile="/tmp/ssrplus.log"
dir="/usr/share/v2ray/"
v2ray_new_version=$(wget -qO- "https://github.com/v2ray/v2ray-core/tags"| grep "/v2ray/v2ray-core/releases/tag/"| head -n 1| awk -F "/tag/v" '{print $2}'| sed 's/\">//')
echo "$v2ray_new_version" > ${dir}v2ray_new_version
if [ $? -eq 0 ];then
	edition=$(uci get shadowsocksr.@server_subscribe[0].edition 2>/dev/null)
	if [ "$edition" = "auto_detected" ];then
		v2ray_new_version=$(cat ${dir}v2ray_new_version|sed -n '1p')
	else
		v2ray_new_version=$edition
	fi
	echo "$(date "+%Y-%m-%d %H:%M:%S") v2ray自动更新启动，验证版本..." >> ${logfile}
	mem_mode=$(uci get shadowsocksr.@server_subscribe[0].mem_mode 2>/dev/null)

	echo "$(date "+%Y-%m-%d %H:%M:%S") 检测到v2ray版本为$v2ray_new_version..." >> ${logfile}

		UpdateApp() {
			for a in $(opkg print-architecture | awk '{print $2}'); do
				case "$a" in
					all|noarch)
						;;
					aarch64_armv8-a|arm_arm1176jzf-s_vfp|arm_arm926ej-s|arm_cortex-a15_neon-vfpv4|arm_cortex-a5|arm_cortex-a53_neon-vfpv4|arm_cortex-a7_neon-vfpv4|arm_cortex-a8_vfpv3|arm_cortex-a9|arm_cortex-a9_neon|arm_cortex-a9_vfpv3|arm_fa526|arm_mpcore|arm_mpcore_vfp|arm_xscale|armeb_xscale)
						ARCH="arm"
						;;
					i386_pentium|i386_pentium4)
						ARCH="32"
						;;
					ar71xx|mips_24kc|mips_mips32|mips64_octeon)
						ARCH="mips"
						;;
					mipsel_24kc|mipsel_24kec_dsp|mipsel_74kc|mipsel_mips32|mipsel_1004kc_dsp)
						ARCH="mipsle"
						;;
					x86_64)
						ARCH="64"
						;;
					*)
						exit 0
						;;
				esac
			done
		}

		download_binary(){
			echo "$(date "+%Y-%m-%d %H:%M:%S") 开始下载v2ray二进制文件..." >> ${logfile}
			rm -rf $bin_dir/v2ray*.zip
			bin_dir="/tmp"
			UpdateApp
			cd $bin_dir
			down_url=https://github.com/v2ray/v2ray-core/releases/download/v"$v2ray_new_version"/v2ray-linux-"$ARCH".zip

			local a=0
			while [ ! -f $bin_dir/v2ray-linux-"$ARCH"*.zip ]; do
				[ $a = 6 ] && exit
				/usr/bin/wget -T10 $down_url
				sleep 2
				let "a = a + 1"
			done
	
			if [ -f $bin_dir/v2ray-linux-"$ARCH"*.zip ]; then
				echo "$(date "+%Y-%m-%d %H:%M:%S") 成功下载v2ray二进制文件" >> ${logfile}
				killall -q -9 v2ray

				if [ ! -d "/usr/bin/v2ray/" ];  then
					mkdir -p /usr/bin/v2ray
				fi	
				unzip -o v2ray-linux-"$ARCH"*.zip -d $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/
				mem_mode=$(uci get shadowsocksr.@server_subscribe[0].mem_mode 2>/dev/null)
				if [ $mem_mode -eq 1 ]; then
					if [ -f /usr/bin/v2ray/v2ray ]; then
						rm -rf /usr/bin/v2ray/*
						rm -rf $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"
					fi
					chmod +x $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/v2*
					ln -s $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/v2ray /usr/bin/v2ray/
					ln -s $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/v2ctl /usr/bin/v2ray/
					ln -s $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/geoip.dat /usr/bin/v2ray/
					ln -s $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/geosite.dat /usr/bin/v2ray/
				else
					if [ -f /usr/bin/v2ray/v2ray ]; then
						rm -rf /usr/bin/v2ray/*
					fi
					mv $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/v2ray /usr/bin/v2ray/
					mv $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/v2ctl /usr/bin/v2ray/
					mv $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/geoip.dat /usr/bin/v2ray/
					mv $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"/geosite.dat /usr/bin/v2ray/
					rm -rf $bin_dir/v2ray-v"$v2ray_new_version"-linux-"$ARCH"
				fi
				rm -rf $bin_dir/v2ray*.zip
				if [ -f "/usr/bin/v2ray/v2ray" ]; then
					chmod +x /usr/bin/v2ray/v2*
					/etc/init.d/shadowsocksr restart
				fi
			else
				echo "$(date "+%Y-%m-%d %H:%M:%S") 下载v2ray二进制文件失败，请重试！" >> ${logfile}
			fi


		}

		download_binary
		echo "" > ${dir}v2ray_version
		echo "$v2ray_new_version" > ${dir}v2ray_version
		rm -rf ${dir}v2ray_new_version
fi
