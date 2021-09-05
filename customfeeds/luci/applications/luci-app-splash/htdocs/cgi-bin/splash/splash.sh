#!/bin/sh

$(uci -q get luci_splash.general.redirect_url) || {
	touch /var/state/luci_splash_locations
	touch /etc/config/luci_splash_locations
	MAC=$(grep "$REMOTE_HOST" /proc/net/arp | awk '{print $4}')
	uci -P /var/state set luci_splash_locations.${MAC//:/}=redirect
	uci -P /var/state set luci_splash_locations.${MAC//:/}.location="http://${HTTP_HOST}${REQUEST_URI}"
}

echo -en "Cache-Control: no-cache, max-age=0, no-store, must-revalidate\r\n"
echo -en "Pragma: no-cache\r\n"
echo -en "Expires: -1\r\n"
echo -en "Status: 307 Temporary Redirect\r\n"
echo -en "Location: http://$SERVER_ADDR/cgi-bin/luci/splash\r\n" 
echo -en "\r\n"

cat <<EOT
<?xml version="1.0" encoding="UTF-8"?>
<WISPAccessGatewayParam xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://www.wballiance.net/wispr_2_0.xsd">
	<Redirect>
		<MessageType>100</MessageType>
		<ResponseCode>0</ResponseCode>
		<AccessProcedure>1.0</AccessProcedure>
		<AccessLocation>12</AccessLocation>
		<LocationName>$SERVER_ADDR</LocationName>
		<LoginURL>http://$SERVER_ADDR/cgi-bin/luci/splash?wispr=1</LoginURL>
		<AbortLoginURL>http://$SERVER_ADDR/</AbortLoginURL>
	</Redirect>
</WISPAccessGatewayParam>
EOT


