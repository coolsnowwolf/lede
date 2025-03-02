# 确保 etc/config 目录存在
mkdir -p package/base-files/files/etc/config
# 1️⃣ **拉取第三方插件**
git clone --depth=1 https://github.com/fw876/helloworld package/helloworld
git clone --depth=1 https://github.com/xiaorouji/openwrt-passwall package/passwall
git clone --depth=1 https://github.com/kenzok8/openwrt-packages package/kenzok8

# 2️⃣ **替换默认 Netgear 主题**
rm -rf package/lean/luci-theme-argon
git clone --depth=1 https://github.com/i028/luci-theme-netgear package/luci-theme-netgear

# 3️⃣ 修改默认 IP（避免和旁路由 10.0.0.1 冲突）
sed -i 's/192.168.1.1/10.0.0.3/g' package/base-files/files/bin/config_generate

# 4️⃣ 设置默认网关和 DNS（指向旁路由 10.0.0.1）
sed -i 's/gateway=.*/gateway="10.0.0.1"/g' package/base-files/files/bin/config_generate
sed -i 's/dns=.*/dns="10.0.0.1"/g' package/base-files/files/bin/config_generate

# 5️⃣ 修改 OpenWRT 主机名
sed -i 's/hostname=.*/hostname="OpenWRT-Test"/g' package/base-files/files/bin/config_generate

# 6️⃣ **修改默认 WiFi 配置**
cat <<EOF > package/base-files/files/etc/config/wireless
config wifi-device 'radio0'
	option type 'mac80211'
	option channel 'auto'
	option hwmode '11g'
	option path 'platform/qca953x_wmac'
	option htmode 'HT20'
	option disabled '0'

config wifi-iface 'default_radio0'
	option device 'radio0'
	option mode 'ap'
	option ssid 'OpenWRT-Test'
	option encryption 'psk2'
	option key '12345678'
	option network 'lan'

config wifi-device 'radio1'
	option type 'mac80211'
	option channel 'auto'
	option hwmode '11a'
	option path 'platform/qca953x_wmac'
	option htmode 'VHT40'
	option disabled '0'

config wifi-iface 'default_radio1'
	option device 'radio1'
	option mode 'ap'
	option ssid 'OpenWRT-Test-5G'
	option encryption 'psk2'
	option key '12345678'
	option network 'lan'
EOF

# 8️⃣ **关闭 DHCP（避免和旁路由冲突）**
cat <<EOF > package/base-files/files/etc/config/dhcp
config dnsmasq
	option domainneeded '1'
	option boguspriv '1'
	option filterwin2k '0'
	option localise_queries '1'
	option rebind_protection '1'
	option rebind_localhost '1'
	option local '/lan/'
	option domain 'lan'
	option expandhosts '1'
	option nonegcache '0'
	option authoritative '1'
	option readethers '1'
	option leasefile '/tmp/dhcp.leases'
	option resolvfile '/tmp/resolv.conf.auto'
	option nonwildcard '1'
	option localservice '1'

config dhcp 'lan'
	option interface 'lan'
	option ignore '1'  # 关闭DHCP

config dhcp 'wan'
	option interface 'wan'
	option ignore '1'
EOF
