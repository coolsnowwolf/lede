local m,s,o
local bypass="bypass"

m=Map(bypass)
m:section(SimpleSection).template="bypass/status"

local server_table={}
luci.model.uci.cursor():foreach(bypass,"servers",function(s)
	if s.alias then
		server_table[s[".name"]]="[%s]:%s"%{string.upper(s.type),s.alias}
	elseif s.server and s.server_port then
		server_table[s[".name"]]="[%s]:%s:%s"%{string.upper(s.type),s.server,s.server_port}
	end
end)

local key_table={}
for key in pairs(server_table) do
	table.insert(key_table,key)
end
table.sort(key_table)

s = m:section(TypedSection, 'global')
s.anonymous=true

o=s:option(ListValue,"global_server",translate("Main Server"))
o:value("",translate("Disable"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end

o=s:option(ListValue,"udp_relay_server",translate("Game Mode UDP Server"))
o:value("",translate("Disable"))
o:value("same",translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end

o=s:option(ListValue,"nf_server",translate("Netflix Server"))
o:value("",translate("Same as Global Server"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o:depends("run_mode","gfw")
o:depends("run_mode","router")
o:depends("run_mode","all")

o=s:option(Flag,"nf_proxy",translate("External Proxy Mode"),
translate("Forward Netflix Proxy through Main Proxy"))
for _,key in pairs(key_table) do o:depends("nf_server",key) end

o=s:option(ListValue,"threads",translate("Multi Threads Option"))
o:value("0",translate("Auto Threads"))
o:value("1",translate("1 Thread"))
o:value("2",translate("2 Threads"))
o:value("4",translate("4 Threads"))
o:value("8",translate("8 Threads"))
o:value("16",translate("16 Threads"))
o:value("32",translate("32 Threads"))
o:value("64",translate("64 Threads"))
o:value("128",translate("128 Threads"))

o=s:option(ListValue,"run_mode",translate("Running Mode"))
o:value("router",translate("Smart Mode"))
o:value("gfw",translate("GFW List Mode"))
o:value("all",translate("Global Mode"))
o:value("oversea",translate("Oversea Mode"))

if luci.sys.call("test `grep MemTotal /proc/meminfo | awk '{print $2}'` -le 500000") == 0 then
o=s:option(Flag,"gfw_mode",translate("Load GFW List"),
translate("If the domestic DNS does not hijack foreign domain name to domestic IP, No need to be enabled"))
o:depends("run_mode","router")
o.default=1
end

if luci.sys.call("test `grep MemTotal /proc/meminfo | awk '{print $2}'` -gt 500000") == 0 then
o=s:option(Flag,"adguardhome",translate("Used with AdGuardHome"),
translate("Luci-app-adguardhome require"))
if luci.sys.call("test `which AdGuardHome` && test -r /etc/init.d/AdGuardHome") == 0 then
o.default=1
else
o.default=0
end
end

o=s:option(ListValue,"dports",translate("Proxy Ports"))
o:value("1",translate("All Ports"))
o:value("2",translate("Only Common Ports"))

o=s:option(ListValue,"dns_mode_o",translate("Foreign Resolve Dns Mode"))
o:value("doh",translate("Use SmartDNS DoH query"))
o:value("tcp",translate("Use SmartDNS TCP query"))
o.default="doh"

o=s:option(Value,"doh_dns_o",translate("Foreign DoH"),
translate("Custom DNS format is https://cloudflare-dns.com/dns-query or https://104.16.249.249/dns-query -http-host cloudflare-dns.com"))
o:value("cloudflare","Cloudflare DoH")
o:value("google",""..translate("Google").." DoH")
o:value("quad9","Quad9 DoH")
o:value("opendns","OpenDNS DoH")
o.default="cloudflare"
o:depends("dns_mode_o","doh")

o=s:option(Value,"tcp_dns_o",translate("Foreign DNS"),
translate("Custom DNS format is 1.1.1.1:53,1.0.0.1 ,Port optional"))
o:value("1.1.1.1,1.0.0.1","1.1.1.1,1.0.0.1 (Cloudflare DNS)")
o:value("8.8.8.8,8.8.4.4","8.8.8.8,8.8.4.4 (Google DNS)")
o:value("9.9.9.9,149.112.112.112","9.9.9.9,149.112.112.112 (Quad9 DNS)")
o:value("208.67.222.222,208.67.220.220","208.67.222.222,208.67.220.220 (OpenDNS)")
o.default="1.1.1.1,1.0.0.1"
o:depends("dns_mode_o","tcp")

o=s:option(ListValue,"dns_mode_d",translate("Domestic Resolve Dns Mode"),
translate("If DoH resolution is not normal,use UDP mode and select ISP DNS"))
o:value("doh",translate("Use SmartDNS DoH query"))
o:value("udp",translate("Use SmartDNS UDP query"))
o.default="udp"

o=s:option(Value,"doh_dns_d",translate("Domestic DoH"),
translate("Custom DNS format is https://dns.alidns.com/dns-query or https://223.5.5.5/dns-query"))
o:value("alidns",""..translate("Ali").." DoH")
o:value("dnspod","Dnspod DoH")
o.default="alidns"
o:depends("dns_mode_d","doh")

o=s:option(Value,"udp_dns_d",translate("Domestic DNS"),
translate("Custom DNS format is 223.5.5.5:53,223.6.6.6 ,Port optional"))
o:value("isp",translate("ISP DNS"))
o:value("223.5.5.5,223.6.6.6","223.5.5.5,223.6.6.6 ("..translate("Ali").." DNS)")
o:value("119.29.29.29,119.28.28.28","119.29.29.29,119.28.28.28 (Dnspod DNS)")
o:value("114.114.114.114,114.114.115.115","114.114.114.114,114.114.115.115 (114 DNS)")
o.default="isp"
o:depends("dns_mode_d","udp")
m:section(SimpleSection).template = 'bypass/status_bottom'

return m
