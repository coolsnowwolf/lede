#!/bin/sh
alias echo_date='echo $(date +%Y年%m月%d日\ %X):'

if [ ! -f openssl.cnf ]; then
	echo_date "Cannot found openssl.cnf"
	exit 1
fi
if [ -f /usr/share/koolproxy/data/private/ca.key.pem ]; then
	echo_date "已经有证书了！"
else
	echo_date "生成证书中..."

	#step 1, root ca
	mkdir -p certs private
	rm -f serial private/ca.key.pem
	chmod 700 private
	echo 1000 > serial
	openssl genrsa -aes256 -passout pass:koolshare -out private/ca.key.pem 2048
	chmod 400 private/ca.key.pem
	openssl req -config openssl.cnf -passin pass:koolshare \
		-subj "/C=CN/ST=Beijing/L=KP/O=KoolProxy inc/CN=koolproxy.com" \
		-key private/ca.key.pem \
		-new -x509 -days 7300 -sha256 -extensions v3_ca \
		-out certs/ca.crt

	#step 2, domain rsa key
	openssl genrsa -aes256 -passout pass:koolshare -out private/base.key.pem 2048
	echo_date "证书生成完毕..."
fi
