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
uci commit openclash
set_lock

enable_custom_dns=$(uci get openclash.config.enable_custom_dns 2>/dev/null)
rule_source=$(uci get openclash.config.rule_source 2>/dev/null)
enable_custom_clash_rules=$(uci get openclash.config.enable_custom_clash_rules 2>/dev/null) 
ipv6_enable=$(uci get openclash.config.ipv6_enable 2>/dev/null)
ipv6_dns=$(uci get openclash.config.ipv6_dns 2>/dev/null)
enable_redirect_dns=$(uci get openclash.config.enable_redirect_dns 2>/dev/null)
disable_masq_cache=$(uci get openclash.config.disable_masq_cache 2>/dev/null)
proxy_mode=$(uci get openclash.config.proxy_mode 2>/dev/null)
intranet_allowed=$(uci get openclash.config.intranet_allowed 2>/dev/null)
enable_udp_proxy=$(uci get openclash.config.enable_udp_proxy 2>/dev/null)
enable_rule_proxy=$(uci get openclash.config.enable_rule_proxy 2>/dev/null)
en_mode=$(uci get openclash.config.en_mode 2>/dev/null)
RAW_CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
CONFIG_FILE="/etc/openclash/$(uci get openclash.config.config_path 2>/dev/null |awk -F '/' '{print $5}' 2>/dev/null)"
core_type=$(uci get openclash.config.core_version 2>/dev/null)
cpu_model=$(opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null)
core_version=$(/etc/openclash/core/clash -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_tun_version=$(/etc/openclash/core/clash_tun -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_game_version=$(/etc/openclash/core/clash_game -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
mix_proxies=$(uci get openclash.config.mix_proxies 2>/dev/null)
op_version=$(sed -n 1p /usr/share/openclash/res/openclash_version 2>/dev/null)
china_ip_route=$(uci get openclash.config.china_ip_route 2>/dev/null)
common_ports=$(uci get openclash.config.common_ports 2>/dev/null)

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
IPV6-DHCP: $(uci get dhcp.lan.dhcpv6 2>/dev/null)

#此项结果应仅有配置文件的DNS监听地址
Dnsmasq转发设置: $(uci get dhcp.@dnsmasq[0].server 2>/dev/null)
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
iptables-mod-tproxy: $(ts_re "$(opkg status iptables-mod-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-ipt-tproxy: $(ts_re "$(opkg status kmod-ipt-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
iptables-mod-extra: $(ts_re "$(opkg status iptables-mod-extra 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-ipt-extra: $(ts_re "$(opkg status kmod-ipt-extra 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
libcap: $(ts_re "$(opkg status libcap 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
libcap-bin: $(ts_re "$(opkg status libcap-bin 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby: $(ts_re "$(opkg status ruby 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-yaml: $(ts_re "$(opkg status ruby-yaml 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-psych: $(ts_re "$(opkg status ruby-psych 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-pstore: $(ts_re "$(opkg status ruby-pstore 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ruby-dbm: $(ts_re "$(opkg status ruby-dbm 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
kmod-tun(TUN模式): $(ts_re "$(opkg status kmod-tun 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
luci-compat(Luci-19.07): $(ts_re "$(opkg status luci-compat 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
EOF

#core
cat >> "$DEBUG_LOG" <<-EOF

#===================== 内核检查 =====================#

EOF
if pidof clash >/dev/null; then
cat >> "$DEBUG_LOG" <<-EOF
运行状态: 运行中
进程pid: $(pidof clash)
运行权限: `getpcaps $(pidof clash)`
运行用户: $(ps |grep "/etc/openclash/clash" |grep -v grep |awk '{print $2}' 2>/dev/null)
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
运行状态: 未运行
EOF
fi
if [ "$core_type" = "0" ]; then
   core_type="未选择架构"
fi
cat >> "$DEBUG_LOG" <<-EOF
已选择的架构: $core_type

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

Game内核版本: $core_game_version
EOF

if [ ! -f "/etc/openclash/core/clash_game" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Game内核文件: 不存在
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Game内核文件: 存在
EOF
fi
if [ ! -x "/etc/openclash/core/clash_game" ]; then
cat >> "$DEBUG_LOG" <<-EOF
Game内核运行权限: 否
EOF
else
cat >> "$DEBUG_LOG" <<-EOF
Game内核运行权限: 正常
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

#===================== 插件设置 =====================#

当前配置文件: $RAW_CONFIG_FILE
启动配置文件: $CONFIG_FILE
运行模式: $en_mode
默认代理模式: $proxy_mode
UDP流量转发(tproxy): $(ts_cf "$enable_udp_proxy")
DNS劫持: $(ts_cf "$enable_redirect_dns")
自定义DNS: $(ts_cf "$enable_custom_dns")
IPV6代理: $(ts_cf "$ipv6_enable")
IPV6-DNS解析: $(ts_cf "$ipv6_dns")
禁用Dnsmasq缓存: $(ts_cf "$disable_masq_cache")
自定义规则: $(ts_cf "$enable_custom_clash_rules")
仅允许内网: $(ts_cf "$intranet_allowed")
仅代理命中规则流量: $(ts_cf "$enable_rule_proxy")
仅允许常用端口流量: $(ts_cf "$common_ports")
绕过中国大陆IP: $(ts_cf "$china_ip_route")

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

#firewall
cat >> "$DEBUG_LOG" <<-EOF

#===================== 防火墙设置 =====================#

#IPv4 NAT chain

EOF
iptables-save -t nat >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv4 Mangle chain

EOF
iptables-save -t mangle >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv6 NAT chain

EOF
ip6tables-save -t nat >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#IPv6 Mangle chain

EOF
ip6tables-save -t mangle >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== IPSET状态 =====================#

EOF
ipset list |grep "Name:" >> "$DEBUG_LOG"

cat >> "$DEBUG_LOG" <<-EOF

#===================== 路由表状态 =====================#

EOF
echo "#route -n" >> "$DEBUG_LOG"
route -n >> "$DEBUG_LOG" 2>/dev/null
echo "#ip route list" >> "$DEBUG_LOG"
ip route list >> "$DEBUG_LOG" 2>/dev/null
echo "#ip rule show" >> "$DEBUG_LOG"
ip rule show >> "$DEBUG_LOG" 2>/dev/null

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

#===================== 测试本机DNS查询 =====================#

EOF
nslookup www.baidu.com >> "$DEBUG_LOG" 2>/dev/null

if [ -s "/tmp/resolv.conf.auto" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== resolv.conf.auto =====================#

EOF
cat /tmp/resolv.conf.auto >> "$DEBUG_LOG"
fi

if [ -s "/tmp/resolv.conf.d/resolv.conf.auto" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#===================== resolv.conf.d =====================#

EOF
cat /tmp/resolv.conf.d/resolv.conf.auto >> "$DEBUG_LOG"
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试本机网络连接 =====================#

EOF
curl -I -m 5 www.baidu.com >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== 测试本机网络下载 =====================#

EOF
VERSION_URL="https://raw.githubusercontent.com/vernesong/OpenClash/master/version"
if pidof clash >/dev/null; then
   curl -IL -m 3 --retry 2 "$VERSION_URL" >> "$DEBUG_LOG" 2>/dev/null
else
   curl -IL -m 3 --retry 2 "$VERSION_URL" >> "$DEBUG_LOG" 2>/dev/null
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 最近运行日志 =====================#

EOF
tail -n 50 "/tmp/openclash.log" >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

#===================== 活动连接信息 =====================#

EOF
/usr/share/openclash/openclash_debug_getcon.lua

cat >> "$DEBUG_LOG" <<-EOF

\`\`\`
EOF

del_lock