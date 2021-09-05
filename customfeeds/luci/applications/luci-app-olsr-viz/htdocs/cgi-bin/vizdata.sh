#!/bin/sh
echo Content-type: text/html
echo

cat<<EOF
<HTML>
<HEAD>
	<TITLE>OLSR-VIZ Data</TITLE>
	<META CONTENT="text/html; charset=iso-8859-1" HTTP-EQUIV="Content-Type">
	<META CONTENT="no-cache" HTTP-EQUIV="cache-control">
</HEAD>
<BODY>

<script langauge='JavaScript1.2' type='text/javascript'>
EOF

# sed + txtinfo plugin
re_ip='[0-9]\{1,\}\.[0-9]\{1,\}\.[0-9]\{1,\}\.[0-9]\{1,\}'
re_sep='[[:space:]]\{1,\}'
re_nosep='[^[:space:]]\{1,\}'
wget http://127.0.0.1:2006/all -qO - | sed -n "
/^Table: Links$/,/^$/ {
s#		#	-	#g
s#\($re_ip\)$re_sep\($re_ip\)\($re_sep$re_nosep\)\{3\}$re_sep\($re_nosep\)#parent.touch_edge(parent.touch_node('\1').set_metric(1).update(),parent.touch_node('\2').set_metric(1).update(),'\4');#p
}
/^Table: Topology$/,/^$/ {
s#\($re_ip\)$re_sep\($re_ip\)\($re_sep$re_nosep\)\{2\}$re_sep\($re_nosep\)#parent.touch_edge(parent.touch_node('\1').update(),parent.touch_node('\2').update(),'\4');#p
}
/^Table: HNA$/,/^$/ {
s#\($re_ip\)/\([0-9]\{1,\}\)$re_sep\($re_ip\)#parent.touch_hna(parent.touch_node('\3'),'\1','\2');#p
}
/^Table: Routes$/,/^$/ {
s#\($re_ip\)/32$re_sep$re_nosep$re_sep\($re_nosep\).*#parent.touch_node('\1').set_metric('\2').update();#p
}
"

hosts=$(uci show olsrd|grep hosts_file|cut -d "=" -f 2)
if [ -n $hosts ]; then
sed -n "
s#\($re_ip\)$re_sep\($re_nosep\)$re_sep.*#parent.touch_node('\1').set_desc('\2');#p
" < $hosts
fi

cat<<EOF
	parent.viz_callback();
</script>
</BODY></HTML>
EOF
