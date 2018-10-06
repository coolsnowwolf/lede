#!/bin/sh
FW="/tmp/firmware-speedport-w921v-1.44.000.bin"
URL="https://www.telekom.de/hilfe/downloads/firmware-speedport-w921v-1.44.000.bin"
FW_TAPI="vr9_tapi_fw.bin"
FW_DSL="vr9_dsl_fw_annex_b.bin"
MD5_FW="cefbeb7073e02e0fa4ddb6b31ecb3d1e"
MD5_TAPI="57f2d07f59e11250ce1219bad99c1eda"
MD5_DSL="655442e31deaa42c9c68944869361ec0"

[ -f /lib/firmware/vdsl.bin ] && exit 0

[ -z "$1" ] || URL=$1

[ -f "${FW}" ] || {
	echo "${FW} does not exist. Try to Download it ? (y/N)"
	read -n 1 R
	echo ""
	[ "$R" = "y" ] || {
		echo "Please manually download the firmware from ${URL} and copy the file to ${FW}"
		echo "See also https://xdarklight.github.io/lantiq-xdsl-firmware-info/ for alternatives"
		exit 1
	}
	echo "Download w921v Firmware"
	wget "${URL}" -O "${FW}"
	[ $? -eq 0 -a -f "${FW}" ] || exit 1
}

F=`md5sum -b ${FW} | cut -d" " -f1`
[ "$F" = "${MD5_FW}" ] || {
	echo "Failed to verify Firmware MD5"
	exit 1
}

cd /tmp
echo "Unpack and decompress w921v Firmware"

w921v_fw_cutter
[ $? -eq 0 ] || exit 1

T=`md5sum -b ${FW_TAPI} | cut -d" " -f1`
D=`md5sum -b ${FW_DSL} | cut -d" " -f1`

[ "$T" = "${MD5_TAPI}" -a "$D" = "${MD5_DSL}" ] || {
	echo "Failed to verify MD5"
	exit 1
}

cp ${FW_TAPI} ${FW_DSL} /lib/firmware/
