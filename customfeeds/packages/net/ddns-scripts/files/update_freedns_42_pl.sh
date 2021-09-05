#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#.2018 Michal Lipka <michal at sysadmin dot care>
local __DUMMY
local __URL="https://freedns.42.pl/xmlrpc.php"
[ -z "$username" ] && write_log 14 "Service section not configured correctly! Missing 'username'"
[ -z "$password" ] && write_log 14 "Service section not configured correctly! Missing 'password'"
[ $ip_dynamic -eq 1 ] && __IP='\&lt;dynamic\&gt;'
PROG="$(which curl) -sk"
write_log 7 "sending update to freedns.42.pl with ip $__IP"
XMLDATA="<?xml version='1.0'?><methodCall><methodName>xname.updateArecord</methodName><params><param><value><struct><member><name>name</name><value><string>[RECORDNAME]</string></value></member><member><name>zone</name><value><string>[ZONENAME]</string></value></member><member><name>oldaddress</name><value><string>*</string></value></member><member><name>updatereverse</name><value><string>0</string></value></member><member><name>user</name><value><string>[USERNAME]</string></value></member><member><name>ttl</name><value><string>600</string></value></member><member><name>newaddress</name><value><string>[IP]</string></value></member><member><name>password</name><value><string>[PASSWORD]</string></value></member></struct></value></param></params></methodCall>"
XMLDATA=$(echo $XMLDATA | sed -e "s#\[USERNAME\]#$URL_USER#g" -e "s#\[PASSWORD\]#$URL_PASS#g" \
-e "s#\[ZONENAME\]#$zone#g" -e "s#\[RECORDNAME\]#$record#g" -e "s#\[IP\]#$__IP#g")
$PROG -d "$XMLDATA" -o $DATFILE $__URL || return 1
write_log 7 "'freedns.42.pl' answered:${N}$(cat $DATFILE)"
grep "<fault>" $DATFILE > /dev/null 2>&1 && return 1
grep '<name>serial</name>' $DATFILE >/dev/null 2>&1
return $?
