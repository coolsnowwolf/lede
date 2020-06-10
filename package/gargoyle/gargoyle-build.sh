#!/bin/bash
#添加gargoyle软件之前需要先运行这个脚本
patches_dir="./package/gargoyle/netfilter-match-modules/patches"
target_name=$(cat .config | egrep  "CONFIG_TARGET_([^_]+)=y" | sed 's/^.*_//g' | sed 's/=y$//g' )
kernel_a=$(cat .config | grep CONFIG_LINUX | grep "=y" | awk -F "_" '{print $3}')
kernel_b=$(cat .config | grep CONFIG_LINUX | grep "=y" | sed 's/^.*_//g' | sed 's/=y$//g')
scripts/patch-kernel.sh . "$patches_dir/" >/dev/null 2>&1
# x86还需要一个patch
sh ./package/gargoyle/netfilter-match-modules/*.sh . ./package/gargoyle/netfilter-match-modules 1 1
#参考https://github.com/project-openwrt/openwrt/commit/47040d246daceff23739e44f4cbb38ebbf3bb593，为qos-gargoyle打补丁
ifExist=`cat ./package/network/utils/iproute2/Makefile | grep "Hack for qos-gargoyle"`
ifExist0=`cat ./package/gargoyle/qos-gargoyle/src/Makefile | grep "/usr/lib/iproute2"`
[ -z "$ifExist" ] && patch -p0<./package/gargoyle/patches/qos-gargoyle.patch
[ -z "$ifExist0" ] && patch -p0<./package/gargoyle/patches/qosmon.patch
echo "gargoyle patch complete"
