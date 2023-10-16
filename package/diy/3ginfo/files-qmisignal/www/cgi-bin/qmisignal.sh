#!/bin/sh
echo "Content-type: application/json"
echo ""
if [ -e /dev/cdc-wdm0 ]; then
	uqmi -s -d /dev/cdc-wdm0 --get-signal-info | sed 's/"type":"hdr".*,"type"/"type"/g'
else
	echo "{}"
fi
exit 0
