m = Map("mosdns")
m.title = translate("MosDNS")
m.description = translate("MosDNS is a 'programmable' DNS forwarder.")

m:section(SimpleSection).template = "mosdns/mosdns_status"

s = m:section(TypedSection, "mosdns")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false

configfile = s:option(ListValue, "configfile", translate("Config File"))
configfile:value("/etc/mosdns/config.yaml", translate("Default Config"))
configfile:value("/etc/mosdns/config_custom.yaml", translate("Custom Config"))
configfile.default = "/etc/mosdns/config.yaml"

listenport = s:option(Value, "listen_port", translate("Listen port"))
listenport.datatype = "and(port,min(1))"
listenport.default = 5335
listenport:depends( "configfile", "/etc/mosdns/config.yaml")

loglevel = s:option(ListValue, "log_level", translate("Log Level"))
loglevel:value("debug", translate("Debug"))
loglevel:value("info", translate("Info"))
loglevel:value("warn", translate("Warning"))
loglevel:value("error", translate("Error"))
loglevel.default = "info"
loglevel:depends( "configfile", "/etc/mosdns/config.yaml")

logfile = s:option(Value, "logfile", translate("Log File"))
logfile.placeholder = "/tmp/mosdns.log"
logfile.default = "/tmp/mosdns.log"
logfile:depends( "configfile", "/etc/mosdns/config.yaml")

remote_dns = s:option(DynamicList, "remote_dns", translate("Remote DNS"))
remote_dns:value("tls://1.1.1.1", "1.1.1.1 (CloudFlare DNS)")
remote_dns:value("tls://1.0.0.1", "1.0.0.1 (CloudFlare DNS)")
remote_dns:value("tls://8.8.8.8", "8.8.8.8 (Google DNS)")
remote_dns:value("tls://8.8.4.4", "8.8.4.4 (Google DNS)")
remote_dns:value("tls://9.9.9.9", "9.9.9.9 (Quad9 DNS)")
remote_dns:value("tls://149.112.112.112", "149.112.112.112 (Quad9 DNS)")
remote_dns:value("tls://45.11.45.11", "45.11.45.11 (DNS.SB)")
remote_dns:value("tls://208.67.222.222", "208.67.222.222 (Open DNS)")
remote_dns:value("tls://208.67.220.220", "208.67.220.220 (Open DNS)")
remote_dns:depends( "configfile", "/etc/mosdns/config.yaml")

remote_dns_pipeline = s:option(Flag, "enable_pipeline", translate("Remote DNS Connection Multiplexing"), translate("Enable TCP/DoT RFC 7766 new Query Pipelining connection multiplexing mode"))
remote_dns_pipeline.rmempty = false
remote_dns_pipeline.default = false
remote_dns_pipeline:depends( "configfile", "/etc/mosdns/config.yaml")

cache_size = s:option(Value, "cache_size", translate("DNS Cache Size"))
cache_size.datatype = "and(uinteger,min(0))"
cache_size.default = "200000"
cache_size:depends( "configfile", "/etc/mosdns/config.yaml")

cache_size = s:option(Value, "cache_survival_time", translate("Cache Survival Time"))
cache_size.datatype = "and(uinteger,min(0))"
cache_size.default = "259200"
cache_size:depends( "configfile", "/etc/mosdns/config.yaml")

minimal_ttl = s:option(Value, "minimal_ttl", translate("Minimum TTL"))
minimal_ttl.datatype = "and(uinteger,min(0))"
minimal_ttl.datatype = "and(uinteger,max(3600))"
minimal_ttl.default = "0"
minimal_ttl:depends( "configfile", "/etc/mosdns/config.yaml")

maximum_ttl = s:option(Value, "maximum_ttl", translate("Maximum TTL"))
maximum_ttl.datatype = "and(uinteger,min(0))"
maximum_ttl.default = "0"
maximum_ttl:depends( "configfile", "/etc/mosdns/config.yaml")

redirect = s:option(Flag, "redirect", translate("Enable DNS Forward"), translate("Forward Dnsmasq Domain Name resolution requests to MosDNS"))
redirect.default = true

adblock = s:option(Flag, "adblock", translate("Enable DNS ADblock"))
adblock:depends( "configfile", "/etc/mosdns/config.yaml")
adblock.default = false

adblock = s:option(Value, "ad_source", translate("ADblock Source"))
adblock:depends("adblock", "1")
adblock.default = "https://raw.githubusercontent.com/privacy-protection-tools/anti-AD/master/anti-ad-domains.txt"
adblock:value("geosite.dat", "v2ray-geosite")
adblock:value("https://raw.githubusercontent.com/privacy-protection-tools/anti-AD/master/anti-ad-domains.txt", "anti-AD")
adblock:value("https://raw.githubusercontent.com/sjhgvr/oisd/main/dbl_basic.txt", "oisd (basic)")
adblock:value("https://raw.githubusercontent.com/QiuSimons/openwrt-mos/master/dat/serverlist.txt", "QiuSimons/openwrt-mos")

reload_service = s:option( Button, "_reload", translate("Reload Service"), translate("Reload service to take effect of new configuration"))
reload_service.write = function()
  luci.sys.exec("/etc/init.d/mosdns reload")
end
reload_service:depends( "configfile", "/etc/mosdns/config_custom.yaml")

config = s:option(TextValue, "manual-config")
config.description = translate("<font color=\"ff0000\"><strong>View the Custom YAML Configuration file used by this MosDNS. You can edit it as you own need.</strong></font>")
config.template = "cbi/tvalue"
config.rows = 25
config:depends( "configfile", "/etc/mosdns/config_custom.yaml")

function config.cfgvalue(self, section)
  return nixio.fs.readfile("/etc/mosdns/config_custom.yaml")
end

function config.write(self, section, value)
  value = value:gsub("\r\n?", "\n")
  nixio.fs.writefile("/etc/mosdns/config_custom.yaml", value)
end

return m
