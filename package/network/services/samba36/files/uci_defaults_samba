#!/bin/sh
# Copyright (c) 2013 OpenWrt
# Copyright (C) 2014 D-Team Technology Co.,Ltd. ShenZhen
# Copyright (c) 2005-2014, lintel <lintel.huang@gmail.com>
#
#
#     警告:对着屏幕的哥们,我们允许你使用此脚本，但不允许你抹去作者的信息,请保留这段话。
# 

samba_name=`uci get system.@system[0].hostname`
 
[ -f /etc/config/samba ] && {
uci set samba.@samba[0].name="$samba_name"
uci commit samba
}
