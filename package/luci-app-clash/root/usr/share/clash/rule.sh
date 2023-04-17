#!/bin/bash
rule=$(uci get clash.config.rule_url 2>/dev/null)
RULE_YAML="/tmp/Rule"

wget -c4 --no-check-certificate --user-agent="Clash/OpenWRT"  "$rule" -O 2>&1 >1 $RULE_YAML

if [ -f  "$RULE_YAML" ]; then
status=$(egrep '^ {0,}Rule:' /tmp/Rule)

	if  [ $status ];then
		sed -i "/Rule:/i\===" $RULE_YAML   
		sed -i "1i\ " $RULE_YAML
		mv $RULE_YAML /tmp/test 
		sed -i '1,/===/d' /tmp/test
		mv /tmp/test /usr/share/clash/rule.yaml
	else
		sed -i "1i\Rule:" $RULE_YAML
		mv $RULE_YAML /tmp/test
		sed -i "1i\ " /tmp/test 
		mv /tmp/test /usr/share/clash/rule.yaml
	fi
fi
 