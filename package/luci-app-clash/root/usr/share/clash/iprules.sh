#!/bin/bash /etc/rc.common
. /lib/functions.sh


CUSTOM_RULE_FILE="/tmp/ipadd.conf"
RULE="/tmp/rules_conf.yaml"
CLASH="/tmp/conf.yaml"
CONFIG_YAML="/etc/clash/config.yaml"
CLASH_CONFIG="/tmp/config.yaml"



append=$(uci get clash.config.append_rules 2>/dev/null)
if [ "${append}" -eq 1 ];then

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

awk '/#RULESTART#/,/#RULEEND#/{print}' "$CLASH_CONFIG" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep '^ \{0,\}- '|awk -F '- ' '{print "- "$2}' | sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null  >$RULE 2>&1

sed -i '/#RULESTART#/,/#RULEEND#/d' "$CLASH_CONFIG" 2>/dev/null

sed -i -e "\$a " $CLASH_CONFIG 2>/dev/null
sed -i "1i\rules:" $RULE 2>/dev/null
cat $CLASH_CONFIG $RULE  >$CLASH 2>/dev/null
mv $CLASH $CLASH_CONFIG 2>/dev/null
rm -rf $RULE 2>/dev/null



if [ -f $CUSTOM_RULE_FILE ];then
	rm -rf $CUSTOM_RULE_FILE 2>/dev/null
fi
	   
	   
ipadd()
{


	   local section="$1"
	   config_get "pgroup" "$section" "pgroup" ""
	   config_get "ipaaddr" "$section" "ipaaddr" ""
	   config_get "type" "$section" "type" ""
	   config_get "res" "$section" "res" ""
	   
	   if [ -z "$type" ]; then
		  return
	   fi
 
	   if [ -z "$pgroup" ]; then
		  return
	   fi
	   
	   if [ "${res}" -eq 1 ];then
		echo "- $type,$ipaaddr,$pgroup,no-resolve">>/tmp/ipadd.conf
	   else
		echo "- $type,$ipaaddr,$pgroup">>/tmp/ipadd.conf
	   fi
}

	
 config_load clash
 config_foreach ipadd "addtype"


if [ -f $CUSTOM_RULE_FILE ];then

sed -i -e "\$a#CUSTOMRULEEND#" $CUSTOM_RULE_FILE 2>/dev/null
sed -i '/#CUSTOMRULESTART#/,/#CUSTOMRULEEND#/d' "$CLASH_CONFIG" 2>/dev/null

if [ ! -z "$(grep "^ \{0,\}rules:" "$CLASH_CONFIG")" ]; then
	sed -i '/rules:/a\#CUSTOMRULESTART#' "$CLASH_CONFIG" 2>/dev/null
else
   echo "#CUSTOMRULESTART#" >> "$CLASH_CONFIG" 2>/dev/null
fi

sed -i '/CUSTOMRULESTART/r/tmp/ipadd.conf' "$CLASH_CONFIG" 2>/dev/null
mv 	$CLASH_CONFIG $CONFIG_YAML 2>/dev/null

fi

else
sed -i '/#CUSTOMRULESTART#/,/#CUSTOMRULEEND#/d' "$CONFIG_YAML" 2>/dev/null
fi
