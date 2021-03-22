#!/bin/sh

if [[ ! -f /etc/easy-rsa/pki/ca.crt || "$1" == "renew" ]]; then
    echo yes|easyrsa init-pki
    echo CN|easyrsa build-ca nopass
    easyrsa gen-dh
    easyrsa build-server-full server nopass
    easyrsa build-client-full client1 nopass

    cp /etc/easy-rsa/pki/ca.crt /etc/openvpn/
    cp /etc/easy-rsa/pki/issued/server.crt /etc/openvpn/
    cp /etc/easy-rsa/pki/private/server.key /etc/openvpn/
    cp /etc/easy-rsa/pki/dh.pem /etc/openvpn/
    cp /etc/easy-rsa/pki/issued/client1.crt /etc/openvpn/
    cp /etc/easy-rsa/pki/private/client1.key /etc/openvpn/
    /etc/init.d/openvpn restart
    echo "OpenVPN Cert renew successfully"
else
    echo "Use the 'renew' option renew Cert"
fi
