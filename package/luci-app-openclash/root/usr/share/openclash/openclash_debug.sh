#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/ruby.sh

set_lock() {
   exec 885>"/tmp/lock/openclash_debug.lock" 2>/dev/null
   flock -x 885 2>/dev/null
}

del_lock() {
   flock -u 885 2>/dev/null
   rm -rf "/tmp/lock/openclash_debug.lock"
}

DEBUG_LOG="/tmp/openclash_debug.log"
LOGTIME=$(echo $(date "+%Y-%m-%d %H:%M:%S"))
uci -q commit openclash
set_lock

enable_custom_dns=$(uci -q get openclash.config.enable_custom_dns)
rule_source=$(uci -q get openclash.config.rule_source)
enable_custom_clash_rules=$(uci -q get openclash.config.enable_custom_clash_rules) 
ipv6_enable=$(uci -q get openclash.config.ipv6_enable)
ipv6_dns=$(uci -q get openclash.config.ipv6_dns)
enable_redirect_dns=$(uci -q get openclash.config.enable_redirect_dns)
disable_masq_cache=$(uci -q get openclash.config.disable_masq_cache)
proxy_mode=$(uci -q get openclash.config.proxy_mode)
intranet_allowed=$(uci -q get openclash.config.intranet_allowed)
enable_udp_proxy=$(uci -q get openclash.config.enable_udp_proxy)
enable_rule_proxy=$(uci -q get openclash.config.enable_rule_proxy)
en_mode=$(uci -q get openclash.config.en_mode)
RAW_CONFIG_FILE=$(uci -q get openclash.config.config_path)
CONFIG_FILE="/etc/openclash/$(uci -q get openclash.config.config_path |awk -F '/' '{print $5}' 2>/dev/null)"
core_model=$(uci -q get openclash.config.core_version)
cpu_model=$(opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null)
core_version=$(/etc/openclash/core/clash -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_tun_version=$(/etc/openclash/core/clash_tun -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_meta_version=$(/etc/openclash/core/clash_meta -v 2>/dev/null |awk -F ' ' '{print $3}' |head -1 2>/dev/null)
servers_update=$(uci -q get openclash.config.servers_update)
mix_proxies=$(uci -q get openclash.config.mix_proxies)
op_version=$(opkg status luci-app-openclash 2>/dev/null |grep 'Version' |awk -F 'Version: ' '{print "v"$2}')
china_ip_route=$(uci -q get openclash.config.china_ip_route)
common_ports=$(uci -q get openclash.config.common_ports)
router_self_proxy=$(uci -q get openclash.config.router_self_proxy)
core_type=$(uci -q get openclash.config.core_type || echo "Dev")
da_password=$(uci -q get openclash.config.dashboard_password)
cn_port=$(uci -q get openclash.config.cn_port)
lan_ip=$(uci -q get network.lan.ipaddr |awk -F '/' '{print $1}' 2>/dev/null || ip address show $(uci -q -p /tmp/state get network.lan.ifname || uci -q -p /tmp/state get network.lan.device) | grep -w "inet"  2>/dev/null |grep -Eo 'inet [0-9\.]+' | awk '{print $2}' || ip addr show 2>/dev/null | grep -w 'inet' | grep 'global' | grep 'brd' | grep -Eo 'inet [0-9\.]+' | awk '{print $2}' | head -n 1)
dnsmasq_default_resolvfile=$(uci -q get openclash.config.default_resolvfile)

if [ -z "$RAW_CONFIG_FILE" ] || [ ! -f "$RAW_CONFIG_FILE" ]; then
	CONFIG_NAME=$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')
	if [ ! -z "$CONFIG_NAME" ]; then
      RAW_CONFIG_FILE="/etc/openclash/config/$CONFIG_NAME"
      CONFIG_FILE="/etc/openclash/$CONFIG_NAME"
  fi
fi

ts_cf()
{
	if [ "$1" != 1 ]; then
	   echo "停用"
	else
	   echo "启用"
  fi
}

ts_re()
{
	if [ -z "$1" ]; then
	   echo "未安装"
	else
	   echo "已安装"
  fi
}

dns_re()
{
   if [ "$1" = "1" ]; then
	   echo "Dnsmasq 转发"
   elif [ "$1" = "2" ]; then
	   echo "Firewall 转发"
   else
      echo "停用"
   fi
}

echo "OpenClash 调试日志" > "$DEBUG_LOG"
cat >> "$DEBUG_LOG" <<-EOF

生成时间: $LOGTIME
插件版本: $op_version
隐私提示: 上传此日志前请注意检查、屏蔽公网IP、节点、密码等相关敏感信息

\`\`\`
EOF

cat >> "$DEBUG_LOG" <<-EOF

#===================== 系统信息 =====================#

主机型号: $(cat /tmp/sysinfo/model 2>/dev/null)
固件版本: $(cat /usr/lib/os-release 2>/dev/null |grep OPENWRT_RELEASE 2>/dev/null |awk -F '"' '{print $2}' 2>/dev/null)
LuCI版本: $(opkg status luci 2>/dev/null |grep 'Version' |awk -F ': ' '{print $2}' 2>/dev/null)
内核版本: $(uname -r 2>/dev/null)
处理器架构: $cpu_model

#此项有值时,如不使用IPv6,建议到网络-接口-lan的设置中禁用IPV6的DHCP
IPV6-DHCP: $(uci -q get dhcp.lan.dhcpv6)

DNS劫持: $(dns_re "$enable_redirect_dns")
#DNS劫持为Dnsmasq时，此项结果应仅有配置文件的DNS监听地址
Dnsmasq转发设置: $(uci -q get dhcp.@dnsmasq[0].server)
EOF

cat >> "$DEBUG_LOG" <<-EOF

#===================== 依赖检查 =====================#

dnsmasq-full: $(ts_re "$(opkg status dnsmasq-full 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
coreutils: $(ts_re "$(opkg status coreutils 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
coreutils-nohup: $(ts_re "$(opkg status coreutils-nohup 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
bash: $(ts_re "$(opkg status bash 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
curl: $(ts_re "$(opkg status curl 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ca-certificates: $(ts_re "$(opkg status ca-certificates 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ipset: $(ts_re "$(opkg status ipset 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ip-full: $(ts_re "$(opkg status ip-full 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
libcap: $(ts_re "$(opkg status libcap 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
libcap-bin: $(ts_re "$(opkg status libcap-bin 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby: $(ts_re "$(opkg status ruby 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-yaml: $(ts_re "$(opkg status ruby-yaml 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-psych: $(ts_re "$(opkg status ruby-psych 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-pstore: $(ts_re "$(opkg status ruby-pstore 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-tun(TUN模式): $(ts_re "$(opkg status kmod-tun 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
luci-compat(Luci >= 19.07): $(ts_re "$(opkg status luci-compat 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-inet-diag(PROCESS-NAME): $(ts_re "$(opkg status kmod-inet-diag 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
unzip: $(ts_re "$(opkg status unzip 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
EOF
if [ -n "$(command -v fw4)" ]; then
cat >> "$DEBUG_LOG" <<-EOF
kmod-nft-tproxy: $(ts_re "$(opkg status kmod-nft-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
iptables-mod-tproxy: $(ts_re "$(opkg status iptables-mod-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-ipt-tproxy: $(ts_re "$(opkg status kmod-ipt-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
iptables-mod-extra: $(ts_re "$(opkg status iptables-mod-extra 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-ipt-extra: $(ts_re "$(opkg status kmod-ipt-extra 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-ipt-nat: $(ts_re "$(opkg status kmod-ipt-nat 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
EOF
fi

#core
cat >> "$DEBUG_LOG" <<-EOF

#===================== 内核检查 =====================#

EOF
if pidof clash >/dev/null; then
cat >> "$DEBUG_LOG" <<-EOF
运行状态: 运行中
运行内核：$core_type
进程pid: $(pidof clash)
运行权限: `getpcaps $(pidof clash)`
运行用户: $(ps |grep "/etc/openclash/clash" |grep -v grep |awk '{print $2}' 2>/dev/null)
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
运行状态: 未运行
EOF
fi
if [ "$core_model" = "0" ]; then
   core_model="未选择架构"
fi
cat >> "$DEBUG_LOG" <<-EOF
已选择的架构: $core_model

#下方无法显示内核版本号时请确认您的内核版本是否正确或者有无权限
EOF

cat >> "$DEBUG_LOG" <<-EOF
Tun内核版本: $core_tun_version
EOF
if [ ! -f "/etc/openclash/core/clash_tun" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Tun内核文件: 不存在
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Tun内核文件: 存在
EOF
fi
if [ ! -x "/etc/openclash/core/clash_tun" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Tun内核运行权限: 否
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Tun内核运行权限: 正常
EOF
fi

cat >> "$DEBUG_LOG" <<-EOF

Dev内核版本: $core_version
EOF
if [ ! -f "/etc/openclash/core/clash" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Dev内核文件: 不存在
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Dev内核文件: 存在
EOF
fi
if [ ! -x "/etc/openclash/core/clash" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Dev内核运行权限: 否
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Dev内核运行权限: 正常
EOF
fi

cat >> "$DEBUG_LOG" <<-EOF

Meta内核版本: $core_meta_version
EOF

if [ ! -f "/etc/openclash/core/clash_meta" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Meta内核文件: 不存在
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Meta内核文件: 存在
EOF
fi
if [ ! -x "/etc/openclash/core/clash_meta" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Meta内核运行权限: 否
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Meta内核运行权限: 正常
EOF
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 插件设置 =====================#

当前配置文件: $RAW_CONFIG_FILE
启动配置文件: $CONFIG_FILE
运行模式: $en_mode
默认代理模式: $proxy_mode
UDP流量转发(tproxy): $(ts_cf "$enable_udp_proxy")
自定义DNS: $(ts_cf "$enable_custom_dns")
IPV6代理: $(ts_cf "$ipv6_enable")
IPV6-DNS解析: $(ts_cf "$ipv6_dns")
禁用Dnsmasq缓存: $(ts_cf "$disable_masq_cache")
自定义规则: $(ts_cf "$enable_custom_clash_rules")
仅允许内网: $(ts_cf "$intranet_allowed")
仅代理命中规则流量: $(ts_cf "$enable_rule_proxy")
仅允许常用端口流量: $(ts_cf "$common_ports")
绕过中国大陆IP: $(ts_cf "$china_ip_route")
路由本机代理: $(ts_cf "$router_self_proxy")

#启动异常时建议关闭此项后重试
混合节点: $(ts_cf "$mix_proxies")
保留配置: $(ts_cf "$servers_update")
EOF

cat >> "$DEBUG_LOG" <<-EOF

#启动异常时建议关闭此项后重试
第三方规则: $(ts_cf "$rule_source")
EOF


if [ "$enable_custom_clash_rules" -eq 1 ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== 自定义规则 一 =====================#
EOF
cat /etc/openclash/custom/openclash_custom_rules.list >> "$DEBUG_LOG"

cat >> "$DEBUG_LOG" <<-EOF

#===================== 自定义规则 二 =====================#
EOF
cat /etc/openclash/custom/openclash_custom_rules_2.list >> "$DEBUG_LOG"
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 配置文件 =====================#

EOF
if [ -f "$CONFIG_FILE" ]; then
   ruby_read "$CONFIG_FILE" ".select {|x| 'proxies' != x and 'proxy-providers' != x }.to_yaml" 2>/dev/null >> "$DEBUG_LOG"
else
   ruby_read "$RAW_CONFIG_FILE" ".select {|x| 'proxies' != x and 'proxy-providers' != x }.to_yaml" 2>/dev/null >> "$DEBUG_LOG"
fi

sed -i '/^ \{0,\}secret:/d' "$DEBUG_LOG" 2>/dev/null

#custom overwrite
cat >> "$DEBUG_LOG" <<-EOF

#===================== 自定义覆写设置 =====================#

EOF

cat /etc/openclash/custom/openclash_custom_overwrite.sh >> "$DEBUG_LOG" 2>/dev/null

#firewall
cat >> "$DEBUG_LOG" <<-EOF

#===================== 自定义防火墙设置 =====================#

EOF

cat /etc/openclash/custom/openclash_custom_firewall_rules.sh >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== IPTABLES 防火墙设置 =====================#

#IPv4 NAT chain

EOF
iptables-save -t nat >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv4 Mangle chain

EOF
iptables-save -t mangle >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv4 Filter chain

EOF
iptables-save -t filter >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv6 NAT chain

EOF
ip6tables-save -t nat >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv6 Mangle chain

EOF
ip6tables-save -t mangle >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv6 Filter chain

EOF
ip6tables-save -t filter >> "$DEBUG_LOG" 2>/dev/null

if [ -n "$(command -v fw4)" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== NFTABLES 防火墙设置 =====================#

EOF
   for nft in "input" "forward" "dstnat" "srcnat" "nat_output" "mangle_prerouting" "mangle_output"; do
      nft list chain inet fw4 "$nft" >> "$DEBUG_LOG" 2>/dev/null
   done >/dev/null 2>&1
   for nft in "openclash" "openclash_mangle" "openclash_mangle_output" "openclash_output" "openclash_post" "openclash_wan_input" "openclash_dns_hijack" "openclash_dns_redirect" "openclash_mangle_v6" "openclash_mangle_output_v6" "openclash_post_v6" "openclash_wan6_input"; do
      nft list chain inet fw4 "$nft" >> "$DEBUG_LOG" 2>/dev/null
   done >/dev/null 2>&1
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== IPSET状态 =====================#

EOF
ipset list -t >> "$DEBUG_LOG"

cat >> "$DEBUG_LOG" <<-EOF

#===================== 路由表状态 =====================#

EOF
echo "#IPv4" >> "$DEBUG_LOG"
echo "" >> "$DEBUG_LOG"
echo "#route -n" >> "$DEBUG_LOG"
route -n >> "$DEBUG_LOG" 2>/dev/null
echo "" >> "$DEBUG_LOG"
echo "#ip route list" >> "$DEBUG_LOG"
ip route list >> "$DEBUG_LOG" 2>/dev/null
echo "" >> "$DEBUG_LOG"
echo "#ip rule show" >> "$DEBUG_LOG"
ip rule show >> "$DEBUG_LOG" 2>/dev/null
echo "" >> "$DEBUG_LOG"
echo "#IPv6" >> "$DEBUG_LOG"
echo "" >> "$DEBUG_LOG"
echo "#route -A inet6" >> "$DEBUG_LOG"
route -A inet6 >> "$DEBUG_LOG" 2>/dev/null
echo "" >> "$DEBUG_LOG"
echo "#ip -6 route list" >> "$DEBUG_LOG"
ip -6 route list >> "$DEBUG_LOG" 2>/dev/null
echo "" >> "$DEBUG_LOG"
echo "#ip -6 rule show" >> "$DEBUG_LOG"
ip -6 rule show >> "$DEBUG_LOG" 2>/dev/null

if [ "$en_mode" != "fake-ip" ] && [ "$en_mode" != "redir-host" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== Tun设备状态 =====================#

EOF
ip tuntap list >> "$DEBUG_LOG" 2>/dev/null
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 端口占用状态 =====================#

EOF
netstat -nlp |grep clash >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试本机DNS查询(www.baidu.com) =====================#

EOF
nslookup www.baidu.com >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试内核DNS查询(www.instagram.com) =====================#

EOF
/usr/share/openclash/openclash_debug_dns.lua "www.instagram.com" >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

Dnsmasq 当前默认 resolv 文件：$dnsmasq_default_resolvfile
EOF

if [ -s "/tmp/resolv.conf.auto" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== /tmp/resolv.conf.auto =====================#

EOF
cat /tmp/resolv.conf.auto >> "$DEBUG_LOG"
fi

if [ -s "/tmp/resolv.conf.d/resolv.conf.auto" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== /tmp/resolv.conf.d/resolv.conf.auto =====================#

EOF
cat /tmp/resolv.conf.d/resolv.conf.auto >> "$DEBUG_LOG"
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试本机网络连接(www.baidu.com) =====================#

EOF
curl -SsI -m 5 www.baidu.com >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试本机网络下载(raw.githubusercontent.com) =====================#

EOF
VERSION_URL="https://raw.githubusercontent.com/vernesong/OpenClash/master/version"
if pidof clash >/dev/null; then
   curl -SsIL -m 3 --retry 2 "$VERSION_URL" >> "$DEBUG_LOG" 2>/dev/null
else
   curl -SsIL -m 3 --retry 2 "$VERSION_URL" >> "$DEBUG_LOG" 2>/dev/null
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 最近运行日志(自动切换为Debug模式) =====================#

EOF

if pidof clash >/dev/null; then
   curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer ${da_password}" -XPATCH http://${lan_ip}:${cn_port}/configs -d '{"log-level": "debug"}'
   sleep 10
fi
tail -n 100 "/tmp/openclash.log" >> "$DEBUG_LOG" 2>/dev/null
cat >> "$DEBUG_LOG" <<-EOF

#===================== 最近运行日志获取完成(自动切换为silent模式) =====================#

EOF
if pidof clash >/dev/null; then
   curl -sL -m 3 -H "Content-Type: application/json" -H "Authorization: Bearer ${da_password}" -XPATCH http://${lan_ip}:${cn_port}/configs -d '{"log-level": "silent"}'
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 活动连接信息 =====================#

EOF
/usr/share/openclash/openclash_debug_getcon.lua

cat >> "$DEBUG_LOG" <<-EOF

\`\`\`
EOF

wan_ip=$(/usr/share/openclash/openclash_get_network.lua "wanip")
wan_ip6=$(/usr/share/openclash/openclash_get_network.lua "wanip6")

if [ -n "$wan_ip" ]; then
	for i in $wan_ip; do
      wanip=$(echo "$i" |awk -F '.' '{print $1"."$2"."$3}')
      sed -i "s/${wanip}/*WAN IP*/g" "$DEBUG_LOG" 2>/dev/null
  done
fi

if [ -n "$wan_ip6" ]; then
	for i in $wan_ip6; do
      wanip=$(echo "$i" |awk -F: 'OFS=":",NF-=1')
      sed -i "s/${wanip}/*WAN IP*/g" "$DEBUG_LOG" 2>/dev/null
  done
fi

del_lock