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
 <title>USSD</title>
 <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />
</head>
<body>
"
if [ "$REQUEST_METHOD" = POST ]; then
	read -t 3 QUERY_STRING
	eval $(echo "$QUERY_STRING"|awk -F'&' '{for(i=1;i<=NF;i++){print $i}}')
	action=`uhttpd -d $action`
	ussd=`uhttpd -d $ussd`
else
	action="x"
fi

case "$action" in
	send)
		A=$(ussd159 -p $(uci get 3ginfo.@3ginfo[0].device) -t 30 -u "$ussd")
		echo "Odpowiedź: $A<br /><br />"
		;;
esac

echo "
<form method=\"post\">
  <input type=\"hidden\" name=\"action\" id=\"action\" value=\"send\">
  <div class=label>USSD:</div><input name=\"ussd\" class=text><br />
  <input type=\"submit\" name=\"submit\" value=\"Wyślij\" text-align=center>
</form>
</div>
</body></html>"
