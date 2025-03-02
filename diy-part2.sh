#!/bin/bash
# ================================
#   diy-part2.sh - OpenWRT 自定义 .config 生成
# ================================
cd lede || { echo "❌ 进入 lede 目录失败"; exit 1; }

echo "🔧 [diy-part2] 生成 .config 配置..."

# 1️⃣ **清除旧配置**
rm -f .config
touch .config

# 2️⃣ **目标架构（x86_64）**
cat <<EOF >> .config
CONFIG_TARGET_x86=y
CONFIG_TARGET_x86_64=y
CONFIG_TARGET_ROOTFS_EXT4FS=y
CONFIG_TARGET_ROOTFS_SQUASHFS=y
CONFIG_GRUB_IMAGES=y
CONFIG_GRUB_EFI_IMAGES=y
CONFIG_TARGET_ROOTFS_PARTSIZE=1024
EOF

# 3️⃣ **启用 Swap（512MB 防止 OOM）**
cat <<EOF >> .config
CONFIG_PACKAGE_block-mount=y
CONFIG_PACKAGE_f2fs-tools=y
CONFIG_PACKAGE_kmod-fs-f2fs=y
EOF

# 4️⃣ **启用 CPU 频率管理（优化 Ryzen 5 3600 省电 & 性能）**
cat <<EOF >> .config
CONFIG_CPU_FREQ_GOV_POWERSAVE=y
CONFIG_CPU_FREQ_GOV_ONDEMAND=y
CONFIG_CPU_FREQ_GOV_CONSERVATIVE=y
CONFIG_CPU_FREQ_GOV_PERFORMANCE=y
CONFIG_CPU_FREQ_GOV_SCHEDUTIL=y
EOF

# 5️⃣ **启用 LuCI 界面 & Netgear 主题**
cat <<EOF >> .config
CONFIG_PACKAGE_luci=y
CONFIG_PACKAGE_luci-base=y
CONFIG_PACKAGE_luci-theme-netgear=y
EOF

# 6️⃣ **科学上网（Passwall / WireGuard）**
cat <<EOF >> .config
CONFIG_PACKAGE_luci-app-passwall=y
CONFIG_PACKAGE_v2ray-core=y
CONFIG_PACKAGE_xray-core=y
CONFIG_PACKAGE_trojan=y
CONFIG_PACKAGE_naiveproxy=y
CONFIG_PACKAGE_luci-app-wireguard=y
CONFIG_PACKAGE_wireguard-tools=y
CONFIG_PACKAGE_kmod-wireguard=y
EOF

# 7️⃣ **启用 ZeroTier / Frp / Tailscale（内网穿透）**
cat <<EOF >> .config
CONFIG_PACKAGE_luci-app-zerotier=y
CONFIG_PACKAGE_luci-app-upnp=y
CONFIG_PACKAGE_luci-app-vpn-policy-routing=y
CONFIG_PACKAGE_tailscale=y
CONFIG_PACKAGE_frpc=y
EOF

# 8️⃣ **启用 NAT 加速（CTF / FullCone NAT）**
cat <<EOF >> .config
CONFIG_PACKAGE_kmod-fast-classifier=y
CONFIG_PACKAGE_kmod-shortcut-fe=y
CONFIG_PACKAGE_iptables-mod-fullconenat=y
EOF

# 9️⃣ **启用 SQM QoS（流量优化 & Ping 低延迟）**
cat <<EOF >> .config
CONFIG_PACKAGE_luci-app-sqm=y
CONFIG_PACKAGE_sqm-scripts=y
CONFIG_PACKAGE_bbr=y
EOF

# 🔟 **启用 AdGuardHome + SmartDNS（DNS 加速 & 去广告）**
cat <<EOF >> .config
CONFIG_PACKAGE_adguardhome=y
CONFIG_PACKAGE_luci-app-adguardhome=y
CONFIG_PACKAGE_luci-app-smartdns=y
CONFIG_PACKAGE_smartdns=y
EOF

# 1️⃣1️⃣ **启用存储共享（Samba / NFS / FTP）**
cat <<EOF >> .config
CONFIG_PACKAGE_samba4-server=y
CONFIG_PACKAGE_nfs-utils=y
CONFIG_PACKAGE_vsftpd=y
EOF

# 1️⃣2️⃣ **启用流量监控 & 端口转发**
cat <<EOF >> .config
CONFIG_PACKAGE_luci-app-nlbwmon=y
CONFIG_PACKAGE_luci-app-vnstat=y
CONFIG_PACKAGE_luci-app-upnp=y
EOF

# 1️⃣3️⃣ **启用 IPv6 支持**
cat <<EOF >> .config
CONFIG_PACKAGE_ipv6helper=y
CONFIG_PACKAGE_kmod-ipv6=y
CONFIG_PACKAGE_kmod-sit=y
CONFIG_PACKAGE_odhcp6c=y
CONFIG_PACKAGE_odhcpd-ipv6only=y
EOF

# 1️⃣4️⃣ **启用硬件 AES 加密（加速 Shadowsocks / WireGuard）**
cat <<EOF >> .config
CONFIG_PACKAGE_kmod-crypto-hw=y
CONFIG_PACKAGE_kmod-crypto-aes=y
CONFIG_PACKAGE_kmod-crypto-ecb=y
EOF

# 1️⃣5️⃣ **设定默认主题**
echo "CONFIG_PACKAGE_luci-theme-netgear=y" >> .config

# 1️⃣6️⃣ **防止 "Error opening terminal: unknown" 错误**
export TERM=dumb
export LC_ALL=C
export LANG=C

# 1️⃣7️⃣ **保存配置**
make defconfig

echo "✅ [diy-part2] 配置完成！"
