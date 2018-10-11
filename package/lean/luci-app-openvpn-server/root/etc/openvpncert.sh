#!/bin/sh

clean-all
echo -en "\n\n\n\n\n\n\n\n" | build-ca
build-dh
build-key-server server
build-key-pkcs12 client1
cp /etc/easy-rsa/keys/ca.crt /etc/openvpn/
cp /etc/easy-rsa/keys/server.crt /etc/openvpn/
cp /etc/easy-rsa/keys/server.key /etc/openvpn/
cp /etc/easy-rsa/keys/dh1024.pem /etc/openvpn/
cp /etc/easy-rsa/keys/client1.crt /etc/openvpn/
cp /etc/easy-rsa/keys/client1.key /etc/openvpn/
/etc/init.d/openvpn restart
echo "OpenVPN Cert renew successfully"