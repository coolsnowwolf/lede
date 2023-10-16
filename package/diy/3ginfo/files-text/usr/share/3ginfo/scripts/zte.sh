#!/bin/sh

IP=$1
[ -z "$IP" ] && exit 0

T=$(mktemp)
wget -q -O $T "http://$IP/goform/goform_get_cmd_process?multi_data=1&cmd=manufacturer_name,model_name,network_provider,network_type,lte_rsrp,lte_rsrq,lte_rssi,lte_snr,cell_id,lac_code,hmcc,hmnc,rmcc,rmnc,rssi,rscp,ecio"

. /usr/share/libubox/jshn.sh
json_load "$(cat $T)"

json_get_vars manufacturer_name model_name network_provider network_type lte_rsrp lte_rsrq lte_rssi lte_snr cell_id lac_code hmcc hmnc rmcc rmnc rssi rscp ecio

if [ -n "$lte_rssi" ]; then
	rssi=$lte_rssi
fi
if [ -n "$rssi" ]; then
	CSQ=$(((rssi+113)/2))
else
	CSQ=0
fi
echo "+CSQ: $CSQ,99"

echo "DEVICE:$manufacturer_name $model_name"

echo "^SYSINFOEX:x,x,x,x,,x,\"$network_type\",x,\"$network_type\""

if [ -n "$hmcc" ]; then
	mcc=$(printf "%03d" $hmcc)
else
	[ -n "$rmcc" ] && mcc=$(printf "%03d" $rmcc)
fi

if [ -n "$hmnc" ]; then
	mnc=$(printf "%02d" $hmnc)
else
	[ -n "$rmnc" ] && mnc=$(printf "%02d" $rmnc)
fi
echo "+COPS: 0,2,\"$mcc$mnc\",x"

if [ "x$network_type" = "xLTE" ]; then
	echo "^LTERSRP: $lte_rsrp,$lte_rsrq"
else
	echo "^CSNR: $rscp,$ecio"
fi

echo "+CREG: 2,1,\"$lac_code\",\"$cell_id\""

rm $T
exit 0
