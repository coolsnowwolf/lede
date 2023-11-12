local fs   = require "nixio.fs"
local sys  = require "luci.sys"

if fs.access("/usr/bin/mosdns") then
    mosdns_version = sys.exec("/usr/share/mosdns/mosdns.sh version")
else
    mosdns_version = "Unknown Version"
end
m = Map("mosdns")
m.title = translate("MosDNS") .. " " .. mosdns_version
m.description = translate("MosDNS is a plugin-based DNS forwarder/traffic splitter.")

m:section(SimpleSection).template = "mosdns/mosdns_status"

s = m:section(TypedSection, "mosdns")
s.addremove = false
s.anonymous = true

s:tab("basic", translate("Basic Options"))

o = s:taboption("basic", Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:taboption("basic", ListValue, "configfile", translate("Config File"))
o:value("/etc/mosdns/config.yaml", translate("Default Config"))
o:value("/etc/mosdns/config_custom.yaml", translate("Custom Config"))
o.default = "/etc/mosdns/config.yaml"

o = s:taboption("basic", Value, "listen_port", translate("Listen port"))
o.datatype = "and(port,min(1))"
o.default = 5335
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("basic", ListValue, "log_level", translate("Log Level"))
o:value("debug", translate("Debug"))
o:value("info", translate("Info"))
o:value("warn", translate("Warning"))
o:value("error", translate("Error"))
o.default = "info"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("basic", Value, "logfile", translate("Log File"))
o.placeholder = "/var/log/mosdns.log"
o.default = "/var/log/mosdns.log"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("basic", Flag, "redirect", translate("DNS Forward"), translate("Forward Dnsmasq Domain Name resolution requests to MosDNS"))
o.default = true

o = s:taboption("basic", Flag, "prefer_ipv4", translate("Remote DNS prefer IPv4"), translate("IPv4 is preferred for remote DNS resolution of dual-stack addresses, and is not affected when the destination is IPv6 only"))
o:depends( "configfile", "/etc/mosdns/config.yaml")
o.default = true

o = s:taboption("basic", Flag, "custom_local_dns", translate("Custom China DNS"), translate("Follow WAN interface DNS if not enabled"))
o:depends( "configfile", "/etc/mosdns/config.yaml")
o.default = false

o = s:taboption("basic", Flag, "apple_optimization", translate("Apple domains optimization"), translate("For Apple domains equipped with Chinese mainland CDN, always responsive to Chinese CDN IP addresses"))
o:depends("custom_local_dns", "1")
o.default = false

o = s:taboption("basic", DynamicList, "local_dns", translate("China DNS server"))
o:value("119.29.29.29", translate("Tencent Public DNS (119.29.29.29)"))
o:value("119.28.28.28", translate("Tencent Public DNS (119.28.28.28)"))
o:value("223.5.5.5", translate("Aliyun Public DNS (223.5.5.5)"))
o:value("223.6.6.6", translate("Aliyun Public DNS (223.6.6.6)"))
o:value("114.114.114.114", translate("Xinfeng Public DNS (114.114.114.114)"))
o:value("114.114.115.115", translate("Xinfeng Public DNS (114.114.115.115)"))
o:value("180.76.76.76", translate("Baidu Public DNS (180.76.76.76)"))
o:value("https://doh.pub/dns-query", translate("Tencent Public DNS (DNS over HTTPS)"))
o:value("quic://dns.alidns.com", translate("Aliyun Public DNS (DNS over QUIC)"))
o:value("https://dns.alidns.com/dns-query", translate("Aliyun Public DNS (DNS over HTTPS)"))
o:value("h3://dns.alidns.com/dns-query", translate("Aliyun Public DNS (DNS over HTTPS/3)"))
o:value("https://doh.360.cn/dns-query", translate("360 Public DNS (DNS over HTTPS)"))
o:depends("custom_local_dns", "1")

o = s:taboption("basic", DynamicList, "remote_dns", translate("Remote DNS server"))
o:value("tls://1.1.1.1", translate("CloudFlare Public DNS (1.1.1.1)"))
o:value("tls://1.0.0.1", translate("CloudFlare Public DNS (1.0.0.1)"))
o:value("tls://8.8.8.8", translate("Google Public DNS (8.8.8.8)"))
o:value("tls://8.8.4.4", translate("Google Public DNS (8.8.4.4)"))
o:value("tls://9.9.9.9", translate("Quad9 Public DNS (9.9.9.9)"))
o:value("tls://149.112.112.112", translate("Quad9 Public DNS (149.112.112.112)"))
o:value("tls://208.67.222.222", translate("Cisco Public DNS (208.67.222.222)"))
o:value("tls://208.67.220.220", translate("Cisco Public DNS (208.67.220.220)"))
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("basic", ListValue, "bootstrap_dns", translate("Bootstrap DNS servers"), translate("Bootstrap DNS servers are used to resolve IP addresses of the DoH/DoT resolvers you specify as upstreams"))
o:value("119.29.29.29", translate("Tencent Public DNS (119.29.29.29)"))
o:value("119.28.28.28", translate("Tencent Public DNS (119.28.28.28)"))
o:value("223.5.5.5", translate("Aliyun Public DNS (223.5.5.5)"))
o:value("223.6.6.6", translate("Aliyun Public DNS (223.6.6.6)"))
o:value("114.114.114.114", translate("Xinfeng Public DNS (114.114.114.114)"))
o:value("114.114.115.115", translate("Xinfeng Public DNS (114.114.115.115)"))
o:value("180.76.76.76", translate("Baidu Public DNS (180.76.76.76)"))
o:value("8.8.8.8", translate("Google Public DNS (8.8.8.8)"))
o:value("1.1.1.1", translate("CloudFlare Public DNS (1.1.1.1)"))
o.default = "119.29.29.29"
o:depends("configfile", "/etc/mosdns/config.yaml")

s:tab("advanced", translate("Advanced Options"))

o = s:taboption("advanced", Value, "concurrent", translate("Concurrent"), translate("DNS query request concurrency, The number of upstream DNS servers that are allowed to initiate requests at the same time"))
o.datatype = "and(uinteger,min(1),max(3))"
o.default = "2"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "idle_timeout", translate("Idle Timeout"), translate("DoH/TCP/DoT Connection Multiplexing idle timeout (default 30 seconds)"))
o.datatype = "and(uinteger,min(1))"
o.default = "30"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Flag, "enable_pipeline", translate("TCP/DoT Connection Multiplexing"), translate("Enable TCP/DoT RFC 7766 new Query Pipelining connection multiplexing mode"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Flag, "insecure_skip_verify", translate("Disable TLS Certificate"), translate("Disable TLS Servers certificate validation, Can be useful if system CA certificate expires or the system time is out of order"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Flag, "enable_ecs_remote", translate("Enable EDNS client subnet"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "remote_ecs_ip", translate("IP Address"), translate("Please provide the IP address you use when accessing foreign websites. This IP subnet (0/24) will be used as the ECS address for Remote DNS requests") .. '<br />' .. translate("This feature is typically used when using a self-built DNS server as an Remote DNS upstream (requires support from the upstream server)"))
o.datatype = "ipaddr"
o:depends("enable_ecs_remote", "1")

o = s:taboption("advanced", Flag, "dns_leak", translate("Prevent DNS Leaks"), translate("Enable this option fallback policy forces forwarding to remote DNS"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "cache_size", translate("DNS Cache Size"), translate("DNS cache size (in piece). To disable caching, please set to 0."))
o.datatype = "and(uinteger,min(0))"
o.default = "8000"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "cache_survival_time", translate("Lazy Cache TTL"), translate("Lazy cache survival time (in second). To disable Lazy Cache, please set to 0."))
o.datatype = "and(uinteger,min(0))"
o.default = "86400"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Flag, "dump_file", translate("Cache Dump"), translate("Save the cache locally and reload the cache dump on the next startup"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "dump_interval", translate("Auto Save Cache Interval"))
o.datatype = "and(uinteger,min(0))"
o.default = "3600"
o:depends("dump_file", "1")

o = s:taboption("advanced", Value, "minimal_ttl", translate("Minimum TTL"), translate("Modify the Minimum TTL value (seconds) for DNS answer results, 0 indicating no modification"))
o.datatype = "and(uinteger,min(0),max(604800))"
o.default = "0"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Value, "maximum_ttl", translate("Maximum TTL"), translate("Modify the Maximum TTL value (seconds) for DNS answer results, 0 indicating no modification"))
o.datatype = "and(uinteger,min(0),max(604800))"
o.default = "0"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("advanced", Flag, "adblock", translate("Enable DNS ADblock"))
o:depends("configfile", "/etc/mosdns/config.yaml")
o.default = false

o = s:taboption("advanced", DynamicList, "ad_source", translate("ADblock Source"), translate("When using custom rule sources, please use rule types supported by MosDNS (domain lists).") .. '<br />' .. translate("Support for local files, such as: file:///var/mosdns/example.txt"))
o:depends("adblock", "1")
o.default = "geosite.dat"
o:value("geosite.dat", "v2ray-geosite")
o:value("https://raw.githubusercontent.com/privacy-protection-tools/anti-AD/master/anti-ad-domains.txt", "anti-AD")
o:value("https://raw.githubusercontent.com/Cats-Team/AdRules/main/mosdns_adrules.txt", "Cats-Team/AdRules")
o:value("https://raw.githubusercontent.com/ookangzheng/dbl-oisd-nl/master/dbl_light.txt", "oisd (small)")
o:value("https://raw.githubusercontent.com/ookangzheng/dbl-oisd-nl/master/dbl.txt", "oisd (big)")

o = s:taboption("basic",  Button, "_reload", translate("Restart-Service"), translate("Restart the MosDNS process to take effect of new configuration"))
o.write = function()
    sys.exec("/etc/init.d/mosdns reload")
end
o:depends("configfile", "/etc/mosdns/config_custom.yaml")

o = s:taboption("basic", TextValue, "config_custom", translate("Configuration Editor"))
o.template = "cbi/tvalue"
o.rows = 25
o:depends("configfile", "/etc/mosdns/config_custom.yaml")
function o.cfgvalue(self, section)
    return fs.readfile("/etc/mosdns/config_custom.yaml")
end
function o.write(self, section, value)
    value = value:gsub("\r\n?", "\n")
    fs.writefile("/etc/mosdns/config_custom.yaml", value)
end
-- codemirror
o = s:taboption("basic", DummyValue, "")
o.template = "mosdns/mosdns_editor"

s:tab("cloudflare", translate("Cloudflare Options"))
o = s:taboption("cloudflare", Flag, "cloudflare", translate("Enabled"), translate("Match the parsing result with the Cloudflare IP ranges, and when there is a successful match, use the 'Custom IP' as the parsing result (experimental feature)"))
o.rmempty = false
o.default = false
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("cloudflare", DynamicList, "cloudflare_ip", translate("Custom IP"))
o.datatype = "ipaddr"
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("cloudflare", TextValue, "cloudflare_cidr", translate("Cloudflare IP Ranges"))
o.description = translate("IPv4 CIDR：") .. [[<a href="https://www.cloudflare.com/ips-v4" target="_blank">https://www.cloudflare.com/ips-v4</a>]] .. '<br />' .. translate("IPv6 CIDR：") .. [[<a href="https://www.cloudflare.com/ips-v6" target="_blank">https://www.cloudflare.com/ips-v6</a>]]
o.template = "cbi/tvalue"
o.rows = 15
o:depends("configfile", "/etc/mosdns/config.yaml")
function o.cfgvalue(self, section)
    return fs.readfile("/etc/mosdns/rule/cloudflare-cidr.txt")
end
function o.write(self, section, value)
    value = value:gsub("\r\n?", "\n")
    fs.writefile("/etc/mosdns/rule/cloudflare-cidr.txt", value)
end

s:tab("api", translate("API Options"))

o = s:taboption("api", Value, "listen_port_api", translate("API Listen port"))
o.datatype = "and(port,min(1))"
o.default = 9091
o:depends("configfile", "/etc/mosdns/config.yaml")

o = s:taboption("api", Button, "flush_cache", translate("Flush Cache"), translate("Flushing Cache will clear any IP addresses or DNS records from MosDNS cache"))
o.rawhtml = true
o.template = "mosdns/mosdns_flush_cache"
o:depends("configfile", "/etc/mosdns/config.yaml")

s:tab("geodata", translate("GeoData Export"))

o = s:taboption("geodata", DynamicList, "geosite_tags", translate("GeoSite Tags"), translate("Enter the GeoSite.dat category to be exported, Allow add multiple tags") .. '<br />' .. translate("Export directory: /var/mosdns"))
o:depends("configfile", "/etc/mosdns/config_custom.yaml")

o = s:taboption("geodata", DynamicList, "geoip_tags", translate("GeoIP Tags"), translate("Enter the GeoIP.dat category to be exported, Allow add multiple tags") .. '<br />' .. translate("Export directory: /var/mosdns"))
o:depends("configfile", "/etc/mosdns/config_custom.yaml")

return m
