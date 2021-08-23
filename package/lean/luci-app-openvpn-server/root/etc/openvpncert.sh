#!/bin/sh

clean-all
pkitool --initca
build-dh
pkitool --server server
pkitool client1
openvpn --genkey --secret ta.key
cp /etc/easy-rsa/keys/ca.crt /etc/openvpn/
cp /etc/easy-rsa/keys/ca.key /etc/openvpn/
cp /etc/easy-rsa/keys/server.crt /etc/openvpn/
cp /etc/easy-rsa/keys/server.key /etc/openvpn/
cp /etc/easy-rsa/keys/dh1024.pem /etc/openvpn/
cp /etc/easy-rsa/keys/client1.crt /etc/openvpn/
cp /etc/easy-rsa/keys/client1.key /etc/openvpn/
echo "OpenVPN Cert renew successfully"