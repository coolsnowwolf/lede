#!/bin/sh

echo "Content-type: text/html"
echo ""
echo "
<html>
<style type=\"text/css\">
 body {
  font-family: Verdana, Arial, Helvetica, sans-serif;
 }
 #centered {
  width: 800px;
  margin: auto;
  text-align: center;
 }
 .label {
  text-align: left;
 }
 .text {
  width: 100%;
 }
 pre {
  text-align: left;
 }
</style>
<head>
 <title>SMS</title>
 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />
</head>
<body>
"
if [ "$REQUEST_METHOD" = POST ]; then
	read -t 3 QUERY_STRING
	eval $(echo "$QUERY_STRING"|awk -F'&' '{for(i=1;i<=NF;i++){print $i}}')
	action=`uhttpd -d $action 2>/dev/null`
	tel=`uhttpd -d $msisdn 2>/dev/null`
	msg=`uhttpd -d $msg 2>/dev/null`
	id=`uhttpd -d $id 2>/dev/null`
else
	action="x"
fi

case "$action" in
	send)
		echo "[global]" > /tmp/gnokiirc
		echo "model = AT" >> /tmp/gnokiirc
		echo "port = "$(uci get 3ginfo.@3ginfo[0].device) >> /tmp/gnokiirc
		echo "connection = serial" >> /tmp/gnokiirc

		echo "$msg" | tr '+' ' ' | gnokii --config /tmp/gnokiirc --sendsms $tel 2>/dev/null
		R=$?
		if [ $R -eq 0 ]; then
			echo "Wysłano wiadomość do $tel!<br />"
		else
			echo "<font color=red><strong>Wystąpił problem z wysłaniem wiadomości!</strong></font><br />"
		fi
		echo $tel": "$msg >> /tmp/sms.txt
		rm /tmp/gnokiirc
		;;
	delete)
		echo "[global]" > /tmp/gnokiirc
		echo "model = AT" >> /tmp/gnokiirc
		echo "port = "$(uci get 3ginfo.@3ginfo[0].device) >> /tmp/gnokiirc
		echo "connection = serial" >> /tmp/gnokiirc
		gnokii --config /tmp/gnokiirc --deletesms SM $id  2>/dev/null
		rm /tmp/gnokiirc
		;;
esac

echo "
<form method=\"post\">
  <input type=\"hidden\" name=\"action\" id=\"action\" value=\"send\">
  <div class=label>Numer telefonu:</div><input name=\"msisdn\" class=text><br />
  <div class=label>Wiadomość:</div><textarea name=\"msg\" class=text></textarea><br /><br />
  <input type=\"submit\" name=\"submit\" value=\"Wyślij\" text-align=center>
</form>
"

echo "[global]" > /tmp/gnokiirc
echo "model = AT" >> /tmp/gnokiirc
echo "port = "$(uci get 3ginfo.@3ginfo[0].device) >> /tmp/gnokiirc
echo "connection = serial" >> /tmp/gnokiirc
echo "<pre>"
gnokii --config /tmp/gnokiirc --getsms SM 0 end 2>/dev/null | \
sed -e ' \
s/ (unread)/<\/pre><font color=green><strong>Nowa wiadomość<\/strong><\/font><pre>/g; \
s/Msg Center.*//g; \
s/(read)//g; \
s/^Sender/Nadawca/g;s/Date\/time/Data\/czas/g; \
s/^Text:/Treść wiadomości:<\/pre><pre style="white-space:-moz-pre-wrap;white-space:-pre-wrap;white-space:-o-pre-wrap;white-space:pre-wrap;word-wrap:break-word;">/g; \
s/\(^[0-9]*\)..Inbox Message/<\/pre><hr \/><form style="text-align: left;" method="post"><input type="hidden" name="action" id="action" value="delete"><input type="hidden" name="id" id="id" value="\1"><input type="submit" name="submit" value="Usuń"><\/form><pre>/g'
rm /tmp/gnokiirc

echo "</pre>
</div>
</body></html>"
