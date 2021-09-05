#!/bin/sh

case "$1" in
	"openvpn-mbedtls")
		openvpn --version | grep "$2.*SSL (mbed TLS)"
		;;
	"openvpn-openssl"|"openvpn-wolfssl")
		openvpn --version | grep "$2.*SSL (OpenSSL)"
		;;
esac
