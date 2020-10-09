#!/bin/bash
. /lib/functions.sh
. /usr/share/openclash/openclash_ps.sh

status=$(unify_ps_status "openclash_debug.sh")
[ "$status" -gt "3" ] && exit 0

DEBUG_LOG="/tmp/openclash_debug.log"
LOGTIME=$(date "+%Y-%m-%d %H:%M:%S")
CHANGE_FILE="/tmp/yaml_change.yaml"
DNS_FILE="/tmp/yaml_dns.yaml"
uci commit openclash

enable_custom_dns=$(uci get openclash.config.enable_custom_dns 2>/dev/null)
rule_source=$(uci get openclash.config.rule_source 2>/dev/null)
enable_custom_clash_rules=$(uci get openclash.config.enable_custom_clash_rules 2>/dev/null) 
ipv6_enable=$(uci get openclash.config.ipv6_enable 2>/dev/null)
enable_redirect_dns=$(uci get openclash.config.enable_redirect_dns 2>/dev/null)
disable_masq_cache=$(uci get openclash.config.disable_masq_cache 2>/dev/null)
proxy_mode=$(uci get openclash.config.proxy_mode 2>/dev/null)
intranet_allowed=$(uci get openclash.config.intranet_allowed 2>/dev/null)
enable_udp_proxy=$(uci get openclash.config.enable_udp_proxy 2>/dev/null)
enable_rule_proxy=$(uci get openclash.config.enable_rule_proxy 2>/dev/null)
en_mode=$(uci get openclash.config.en_mode 2>/dev/null)
CONFIG_FILE=$(uci get openclash.config.config_path 2>/dev/null)
core_type=$(uci get openclash.config.core_version 2>/dev/null)
cpu_model=$(opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null)
core_version=$(/etc/openclash/core/clash -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_tun_version=$(/etc/openclash/core/clash_tun -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
core_game_version=$(/etc/openclash/core/clash_game -v 2>/dev/null |awk -F ' ' '{print $2}' 2>/dev/null)
servers_update=$(uci get openclash.config.servers_update 2>/dev/null)
op_version=$(sed -n 1p /usr/share/openclash/res/openclash_version 2>/dev/null)
china_ip_route=$(uci get openclash.config.china_ip_route 2>/dev/null)

if [ -z "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ]; then
	CONFIG_NAME=$(ls -lt /etc/openclash/config/ | grep -E '.yaml|.yml' | head -n 1 |awk '{print $9}')
	if [ ! -z "$CONFIG_NAME" ]; then
      CONFIG_FILE="/etc/openclash/config/$CONFIG_NAME"
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

\`\`\`
EOF

cat >> "$DEBUG_LOG" <<-EOF

#===================== 系统信息 =====================#
主机型号: $(cat /tmp/sysinfo/model 2>/dev/null)
固件版本: $(cat /usr/lib/os-release 2>/dev/null |grep OPENWRT_RELEASE 2>/dev/null |awk -F '"' '{print $2}' 2>/dev/null)
LuCI版本: $(opkg status luci 2>/dev/null |grep 'Version' |awk -F ': ' '{print $2}' 2>/dev/null)
内核版本: $(uname -r 2>/dev/null)
处理器架构: $cpu_model

#此项在使用Tun模式时应为ACCEPT
防火墙转发: $(uci get firewall.@defaults[0].forward 2>/dev/null)

#此项有值时建议到网络-接口-lan的设置中禁用IPV6的DHCP
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
jsonfilter: $(ts_re "$(opkg status jsonfilter 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ca-certificates: $(ts_re "$(opkg status ca-certificates 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ipset: $(ts_re "$(opkg status ipset 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
ip-full: $(ts_re "$(opkg status ip-full 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
iptables-mod-tproxy: $(ts_re "$(opkg status iptables-mod-tproxy 2>/dev/null |grep 'Status' |awk -F ': ' '{print $2}' 2>/dev/null)")
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
当前配置文件: $CONFIG_FILE
运行模式: $en_mode
默认代理模式: $proxy_mode
UDP流量转发: $(ts_cf "$enable_udp_proxy")
DNS劫持: $(ts_cf "$enable_redirect_dns")
自定义DNS: $(ts_cf "$enable_custom_dns")
IPV6-DNS解析: $(ts_cf "$ipv6_enable")
禁用Dnsmasq缓存: $(ts_cf "$disable_masq_cache")
自定义规则: $(ts_cf "$enable_custom_clash_rules")
仅允许内网: $(ts_cf "$intranet_allowed")
仅代理命中规则流量: $(ts_cf "$enable_rule_proxy")
绕过中国大陆IP: $(ts_cf "$china_ip_route")

#启动异常时建议关闭此项后重试
保留配置: $(ts_cf "$servers_update")
EOF
if [ "$rule_source" != "0" ]; then
cat >> "$DEBUG_LOG" <<-EOF

#启动异常时建议关闭此项后重试
第三方规则: $rule_source
EOF
cat >> "$DEBUG_LOG" <<-EOF
第三方规则策略组设置:
GlobalTV: $(uci get openclash.config.GlobalTV 2>/dev/null)
AsianTV: $(uci get openclash.config.AsianTV 2>/dev/null)
Proxy: $(uci get openclash.config.Proxy 2>/dev/null)
Apple: $(uci get openclash.config.Apple 2>/dev/null)
Netflix: $(uci get openclash.config.Netflix 2>/dev/null)
Spotify: $(uci get openclash.config.Spotify 2>/dev/null)
Steam: $(uci get openclash.config.Steam 2>/dev/null)
AdBlock: $(uci get openclash.config.AdBlock 2>/dev/null)
Netease Music: $(uci get openclash.config.Netease_Music 2>/dev/null)
Speedtest: $(uci get openclash.config.Speedtest 2>/dev/null)
Telegram: $(uci get openclash.config.Telegram 2>/dev/null)
Microsoft: $(uci get openclash.config.Microsoft 2>/dev/null)
PayPal: $(uci get openclash.config.PayPal 2>/dev/null)
Domestic: $(uci get openclash.config.Domestic 2>/dev/null)
Others: $(uci get openclash.config.Others 2>/dev/null)

读取的配置文件策略组:
EOF
cat /tmp/Proxy_Group  >> "$DEBUG_LOG"
else
cat >> "$DEBUG_LOG" <<-EOF
第三方规则: 停用
EOF
fi

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
if [ -n "$(grep OpenClash-General-Settings "$CONFIG_FILE")" ]; then
   sed '/OpenClash-General-Settings/,$d' "$CONFIG_FILE" >> "$DEBUG_LOG" 2>/dev/null
else
   /usr/share/openclash/yml_field_name_ch.sh "$CONFIG_FILE" 2>/dev/null
   #取出general部分
   /usr/share/openclash/yml_field_cut.sh "general" "$CHANGE_FILE" "$CONFIG_FILE"
   
   #取出dns部分
   nameserver_len=$(sed -n '/^ \{0,\}nameserver:/=' "$CONFIG_FILE" 2>/dev/null)
   if [ -n "$nameserver_len" ]; then
      /usr/share/openclash/yml_field_cut.sh "$nameserver_len" "$DNS_FILE" "$CONFIG_FILE"
   fi 2>/dev/null
   
   rm -rf /tmp/yaml_general 2>/dev/null
   cat "$CHANGE_FILE" "$DNS_FILE" >> "$DEBUG_LOG"
fi
sed -i '/^ \{0,\}secret:/d' "$DEBUG_LOG" 2>/dev/null

#firewall
cat >> "$DEBUG_LOG" <<-EOF

#===================== 防火墙设置 =====================#

#NAT chain

EOF
iptables -t nat -nL PREROUTING --line-number >> "$DEBUG_LOG"
iptables -t nat -nL OUTPUT --line-number >> "$DEBUG_LOG"

cat >> "$DEBUG_LOG" <<-EOF

#Mangle chain

EOF
iptables -t mangle -nL PREROUTING --line-number >> "$DEBUG_LOG"
iptables -t mangle -nL OUTPUT --line-number >> "$DEBUG_LOG"

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
   HTTP_PORT=$(uci get openclash.config.http_port 2>/dev/null)
   PROXY_ADDR=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
   if [ -s "/tmp/openclash.auth" ]; then
      PROXY_AUTH=$(cat /tmp/openclash.auth |awk -F '- ' '{print $2}' |sed -n '1p' 2>/dev/null)
   fi
   curl -IL -m 3 --retry 2 -x http://$PROXY_ADDR:$HTTP_PORT -U "$PROXY_AUTH" "$VERSION_URL" >> "$DEBUG_LOG"
else
   curl -IL -m 3 --retry 2 "$VERSION_URL" >> "$DEBUG_LOG"
fi

cat >> "$DEBUG_LOG" <<-EOF

#===================== 最近运行日志 =====================#
EOF
tail -n 50 "/tmp/openclash.log" >> "$DEBUG_LOG" 2>/dev/null

cat >> "$DEBUG_LOG" <<-EOF

\`\`\`
EOF