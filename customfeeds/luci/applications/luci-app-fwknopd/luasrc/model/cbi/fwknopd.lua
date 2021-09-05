-- Copyright 2015 Jonathan Bennett <jbennett@incomsystems.biz>
-- Licensed to the public under the GNU General Public License v2.
tmp = 0
m = Map("fwknopd", translate("Firewall Knock Operator"))

s = m:section(TypedSection, "global", translate("Enable Uci/Luci control")) -- Set uci control on or off
s.anonymous=true
s:option(Flag, "uci_enabled", translate("Enable config overwrite"), translate("When unchecked, the config files in /etc/fwknopd will be used as is, ignoring any settings here."))

s = m:section(TypedSection, "access", translate("access.conf stanzas")) -- set the access.conf settings
s.anonymous=true
s.addremove=true
qr = s:option(DummyValue, "note0", "dummy")
qr.tmp = tmp
qr.template = "fwknopd-qr"
qr:depends("uci_enabled", "1")
s:option(Value, "SOURCE", "SOURCE", translate("Use ANY for any source ip"))
k1 = s:option(Value, "KEY", "KEY", translate("Define the symmetric key used for decrypting an incoming SPA packet that is encrypted by the fwknop client with Rijndael."))
k1:depends("keytype", translate("Normal Key"))
k2 = s:option(Value, "KEY_BASE64", "KEY_BASE64", translate("Define the symmetric key used for decrypting an incoming SPA \
						packet that is encrypted by the fwknop client with Rijndael."))
k2:depends("keytype", translate("Base 64 key"))
l1 = s:option(ListValue, "keytype", "Key type")
l1:value("Normal Key", "Normal Key")
l1:value("Base 64 key", "Base 64 key")
k3 = s:option(Value, "HMAC_KEY", "HMAC_KEY", "The hmac key")
k3:depends("hkeytype", "Normal Key")
k4 = s:option(Value, "HMAC_KEY_BASE64", "HMAC_KEY_BASE64", translate("The base64 hmac key"))
k4:depends("hkeytype", "Base 64 key")
l2 = s:option(ListValue, "hkeytype", "HMAC Key type")
l2:value("Normal Key", "Normal Key")
l2:value("Base 64 key", "Base 64 key")
s:option(Value, "OPEN_PORTS", "OPEN_PORTS", translate("Define a set of ports and protocols (tcp or udp) that will be opened if a valid knock sequence is seen. \
					If this entry is not set, fwknopd will attempt to honor any proto/port request specified in the SPA data \
					(unless of it matches any “RESTRICT_PORTS” entries). Multiple entries are comma-separated."))
s:option(Value, "FW_ACCESS_TIMEOUT", "FW_ACCESS_TIMEOUT", translate("Define the length of time access will be granted by fwknopd through the firewall after a \
					valid knock sequence from a source IP address. If “FW_ACCESS_TIMEOUT” is not set then the default \
					timeout of 30 seconds will automatically be set."))
s:option(Value, "REQUIRE_SOURCE_ADDRESS", "REQUIRE_SOURCE_ADDRESS", translate("Force all SPA packets to contain a real IP address within the encrypted data. \
					This makes it impossible to use the -s command line argument on the fwknop client command line, so either -R \
					has to be used to automatically resolve the external address (if the client behind a NAT) or the client must \
					know the external IP and set it via the -a argument."))

s = m:section(TypedSection, "config", translate("fwknopd.conf config options")) 
s.anonymous=true
s:option(Value, "MAX_SPA_PACKET_AGE", "MAX_SPA_PACKET_AGE", translate("Maximum age in seconds that an SPA packet will be accepted. defaults to 120 seconds"))
s:option(Value, "PCAP_INTF", "PCAP_INTF", translate("Specify the ethernet interface on which fwknopd will sniff packets."))
s:option(Value, "ENABLE_IPT_FORWARDING", "ENABLE_IPT_FORWARDING", translate("Allow SPA clients to request access to services through an iptables firewall instead of just to it."))
s:option(Value, "ENABLE_NAT_DNS", "ENABLE_NAT_DNS", translate("Allow SPA clients to request forwarding destination by DNS name."))

return m

