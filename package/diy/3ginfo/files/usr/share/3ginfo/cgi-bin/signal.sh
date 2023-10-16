#!/bin/sh
D=$(uci -q get 3ginfo.@3ginfo[0].device)
echo -e "Content-type: text/html\n\n"
if [ -e $D ]; then
	gcom -d $D sig | sed -e 's/ /,/g'
fi
