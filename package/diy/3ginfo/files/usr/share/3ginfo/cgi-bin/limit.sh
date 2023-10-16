#!/bin/sh

DEVICE=$(uci get 3ginfo.@3ginfo[0].device)
SCRIPT=$(uci get 3ginfo.@3ginfo[0].script)

echo -e "Content-type: text/html\n\n"
if [ -e scripts/$SCRIPT.sh ]; then
	sh scripts/$SCRIPT.sh
elif [ -e scripts/$SCRIPT.gcom ]; then
	if ! pidof gcom > /dev/null; then
		O=$(gcom -d $DEVICE -s scripts/$SCRIPT.gcom)
		echo "<div class=\"c\">$O</div>"
	else
		echo "<div class=\"c\">Brak możliwości sprawdzenia limitu. Spróbuj za chwilę.</div>"
	fi
else
	echo "<div class=\"c\"><font color=red>Brak skryptu $SCRIPT</font></div>"
fi
echo "<div class=\"c\"><input type=\"button\" class=\"button\" value=\"Wróć\" onclick=\"trzyginfo('cgi-bin/3ginfo.sh');\"></div>"
