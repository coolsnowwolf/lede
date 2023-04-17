#!/bin/bash /etc/rc.common
. /lib/functions.sh

GROUP_FILE="/tmp/yaml_group.yaml"
GAME_RULE_FILE="/tmp/yaml_game_rule_group.yaml"
game_rules=$(uci get clash.config.g_rules 2>/dev/null)

CUSTOM_RULE_FILE="/tmp/ipadd.conf"
RULE="/tmp/rules_conf.yaml"
CLASH="/tmp/conf.yaml"
CONFIG_YAML="/etc/clash/config.yaml"
CLASH_CONFIG="/tmp/config.yaml"


if [ "${game_rules}" -eq 1 ];then

if [ -f $CLASH_CONFIG ];then
	rm -rf $CLASH_CONFIG 2>/dev/null
fi

cp $CONFIG_YAML $CLASH_CONFIG 2>/dev/null
if [ ! -z "$(grep "^Rule:" "$CLASH_CONFIG")" ]; then
	sed -i "/^Rule:/i\#RULESTART#" $CLASH_CONFIG 2>/dev/null
elif [ ! -z "$(grep "^rules:" "$CLASH_CONFIG")" ]; then
	sed -i "/^rules:/i\#RULESTART#" $CLASH_CONFIG 2>/dev/null
fi
sed -i -e "\$a#RULEEND#" $CLASH_CONFIG 2>/dev/null

awk '/#RULESTART#/,/#RULEEND#/{print}' "$CLASH_CONFIG" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep - |awk -F '- ' '{print "- "$2}' | sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null  >$RULE 2>&1

sed -i '/#RULESTART#/,/#RULEEND#/d' "$CLASH_CONFIG" 2>/dev/null

sed -i -e "\$a " $CLASH_CONFIG 2>/dev/null
sed -i "1i\rules:" $RULE 2>/dev/null
cat $CLASH_CONFIG $RULE  >$CLASH 2>/dev/null
mv $CLASH $CLASH_CONFIG 2>/dev/null
rm -rf $RULE 2>/dev/null




get_rule_file()
{
   if [ -z "$1" ]; then
      return
   fi

   GAME_RULE_FILE_NAME=$(grep -F $1 /usr/share/clash/rules/rules.list |awk -F ',' '{print $3}' 2>/dev/null)

   if [ -z "$GAME_RULE_FILE_NAME" ]; then
      GAME_RULE_FILE_NAME=$(grep -F $1 /usr/share/clash/rules/rules.list |awk -F ',' '{print $2}' 2>/dev/null)
   fi

   GAME_RULE_PATH="/usr/share/clash/rules/g_rules/$GAME_RULE_FILE_NAME"
   sed '/^#/d' $GAME_RULE_PATH 2>/dev/null |sed '/^ *$/d' |awk '{print "- IP-CIDR,"$0}' |awk -v tag="$2" '{print $0","'tag'""}' >> $GAME_RULE_FILE 2>/dev/null
   set_rule_file=1
}


yml_game_rule_get()
{
   local section="$1"
   config_get "group" "$section" "group" ""

   if [ -f $GAME_RULE_FILE ];then
	rm -rf $GAME_RULE_FILE 2>/dev/null
   fi

   if [ -z "$group" ]; then
      return
   fi

   config_list_foreach "$section" "rule_name" get_rule_file "$group"
}



config_load "clash"
config_foreach yml_game_rule_get "game"

if [ -f $GAME_RULE_FILE ];then

sed -i -e "\$a#GAMERULEEND#" $GAME_RULE_FILE 2>/dev/null
sed -i '/#GAMERULESTART#/,/#GAMERULEEND#/d' "$CLASH_CONFIG" 2>/dev/null

if [ ! -z "$(grep "^ \{0,\}- GEOIP" "$CLASH_CONFIG")" ]; then
   sed -i '1,/^ \{0,\}- GEOIP,/{/^ \{0,\}- GEOIP,/s/^ \{0,\}- GEOIP,/#GAMERULESTART#\n&/}' "$CLASH_CONFIG" 2>/dev/null
elif [ ! -z "$(grep "^ \{0,\}- MATCH," "$CLASH_CONFIG")" ]; then
   sed -i '1,/^ \{0,\}- MATCH,/{/^ \{0,\}- MATCH,/s/^ \{0,\}- MATCH,/#GAMERULESTART#\n&/}' "$CLASH_CONFIG" 2>/dev/null
else
   echo "#GAMERULESTART#" >> "$CLASH_CONFIG" 2>/dev/null
fi

sed -i '/GAMERULESTART/r/tmp/yaml_game_rule_group.yaml' "$CLASH_CONFIG" 2>/dev/null
mv 	$CLASH_CONFIG $CONFIG_YAML 2>/dev/null
fi
rm -rf $GAME_RULE_FILE 2>/dev/null
else
sed -i '/#GAMERULESTART#/,/#GAMERULEEND#/d' "$CONFIG_YAML" 2>/dev/null
rm -rf $GAME_RULE_FILE 2>/dev/null
fi


