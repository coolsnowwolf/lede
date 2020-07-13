#!/bin/sh

ddns=`uci get openvpn.myvpn.ddns`
port=`uci get openvpn.myvpn.port`
proto=`uci get openvpn.myvpn.proto`

cat > /tmp/my.ovpn  <<EOF
client
dev tun
proto $proto
remote $ddns $port
resolv-retry infinite
nobind
persist-key
persist-tun
verb 3"
EOF
echo '<ca>' >> /tmp/my.ovpn
cat /etc/openvpn/ca.crt >> /tmp/my.ovpn
echo '</ca>' >> /tmp/my.ovpn
echo '<cert>' >> /tmp/my.ovpn
cat /etc/openvpn/client1.crt >> /tmp/my.ovpn
echo '</cert>' >> /tmp/my.ovpn
echo '<key>' >> /tmp/my.ovpn
cat /etc/openvpn/client1.key >> /tmp/my.ovpn
echo '</key>' >> /tmp/my.ovpn
[ -f /etc/ovpnadd.conf ] && cat /etc/ovpnadd.conf >> /tmp/my.ovpn
