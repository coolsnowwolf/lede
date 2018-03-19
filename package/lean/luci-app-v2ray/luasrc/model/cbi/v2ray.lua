local fs = require "nixio.fs"
local NXFS = require "nixio.fs"
local WLFS = require "nixio.fs"
local SYS  = require "luci.sys"
local ND = SYS.exec("cat /etc/gfwlist/china-banned | wc -l")
local conf = "/etc/v2ray/base-gfwlist.txt"
local watch = "/tmp/v2ray_watchdog.log"
local dog = "/tmp/v2raypro.log"
local http = luci.http
local ucursor = require "luci.model.uci".cursor()

local Status

if SYS.call("pidof v2ray > /dev/null") == 0 then
	Status = translate("<strong><font color=\"green\">V2Ray is Running</font></strong>")
else
	Status = translate("<strong><font color=\"red\">V2Ray is Not Running</font></strong>")
end

m = Map("v2ray")
m.title	= translate("V2Ray Transparent Proxy")
m.description = translate("A fast secure tunnel proxy that help you get through firewalls on your router")

s = m:section(TypedSection, "v2ray")
s.anonymous = true
s.description = translate(string.format("%s<br /><br />", Status))

-- ---------------------------------------------------

s:tab("basic",  translate("Base Setting"))


switch = s:taboption("basic",Flag, "enabled", translate("Enable"))
switch.rmempty = false

proxy_mode = s:taboption("basic",ListValue, "proxy_mode", translate("Proxy Mode"))
proxy_mode:value("M", translate("Base on GFW-List Auto Proxy Mode(Recommend)"))
proxy_mode:value("S", translate("Bypassing China Manland IP Mode(Be caution when using P2P download！)"))
proxy_mode:value("G", translate("Global Mode"))
proxy_mode:value("V", translate("Overseas users watch China video website Mode"))

cronup = s:taboption("basic", Flag, "cron_mode", translate("Auto Update GFW-List"),
	translate(string.format("GFW-List Lines： <strong><font color=\"blue\">%s</font></strong> Lines", ND)))
cronup.default = 0
cronup.rmempty = false

updatead = s:taboption("basic", Button, "updatead", translate("Manually force update GFW-List"), translate("Note: It needs to download and convert the rules. The background process may takes 60-120 seconds to run. <br / > After completed it would automatically refresh, please do not duplicate click!"))
updatead.inputtitle = translate("Manually force update GFW-List")
updatead.inputstyle = "apply"
updatead.write = function()
	SYS.call("nohup sh /etc/v2ray/up-gfwlist.sh > /tmp/gfwupdate.log 2>&1 &")
end

safe_dns_tcp = s:taboption("basic",Flag, "safe_dns_tcp", translate("DNS uses TCP"),
	translate("Through the server transfer mode inquires DNS pollution prevention (Safer and recommended)"))
safe_dns_tcp.rmempty = false
-- safe_dns_tcp:depends("more", "1")

-- more_opt = s:taboption("basic",Flag, "more", translate("More Options"),
-- 	translate("Options for advanced users"))

-- timeout = s:taboption("basic",Value, "timeout", translate("Timeout"))
-- timeout.datatype = "range(0,10000)"
-- timeout.placeholder = "60"
-- timeout.optional = false
-- timeout:depends("more", "1")

-- safe_dns = s:taboption("basic",Value, "safe_dns", translate("Safe DNS"),
-- 	translate("8.8.8.8 or 8.8.4.4 is recommended"))
-- safe_dns.datatype = "ip4addr"
-- safe_dns.optional = false
-- safe_dns:depends("more", "1")

-- safe_dns_port = s:taboption("basic",Value, "safe_dns_port", translate("Safe DNS Port"),
-- 	translate("Foreign DNS on UDP port 53 might be polluted"))
-- safe_dns_port.datatype = "range(1,65535)"
-- safe_dns_port.placeholder = "53"
-- safe_dns_port.optional = false
-- safe_dns_port:depends("more", "1")

--fast_open =s:taboption("basic",Flag, "fast_open", translate("TCP Fast Open"),
--	translate("Enable TCP fast open, only available on kernel > 3.7.0"))



s:tab("main",  translate("Server Setting"))

use_conf_file = s:taboption("main",Flag, "use_conf_file", translate("Use Config File"))
use_conf_file.rmempty = false

if nixio.fs.access("/usr/bin/v2ray/v2ctl") then
	conf_file_type = s:taboption("main",ListValue, "conf_file_type", translate("Config File Type"))
	conf_file_type:value("pb","Protobuf")
else 
	conf_file_type = s:taboption("main",ListValue, "conf_file_type", translate("Config File Type"), translate("Warning: Can't find v2ctl. You can only choose Protobuf."))
end
conf_file_type:value("json","JSON")
conf_file_type:depends("use_conf_file", 1)

conf_file_path = s:taboption("main",Value, "conf_file_path", translate("Config File Path"),
	translate("If you choose to upload a new file, please do not modify and this configuration will be overwritten automatically."))
conf_file_path:depends("use_conf_file", 1)

upload_conf = s:taboption("main",FileUpload, "")
upload_conf.template = "cbi/other_upload2"
upload_conf:depends("use_conf_file", 1)

um = s:taboption("main",DummyValue, "", nil)
um.template = "cbi/other_dvalue"
um:depends("use_conf_file", 1)



local conf_dir, fd
conf_dir = "/etc/v2ray/"
nixio.fs.mkdir(conf_dir)
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(conf_dir .. meta.file, "w") end

			if not fd then
				um.value = translate("Create upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			um.value = translate("File saved to") .. ' "/etc/v2ray/' .. meta.file .. '"'
			ucursor:set("v2ray","v2ray","conf_file_path","/etc/v2ray/" .. meta.file)
			ucursor:commit("v2ray")
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end




server = s:taboption("main",Value, "address", translate("Server Address"))
server.datatype = "host"
server:depends("use_conf_file", 0)

server_port = s:taboption("main",Value, "port", translate("Server Port"))
server_port.datatype = "range(0,65535)"
server_port:depends("use_conf_file", 0)

id = s:taboption("main",Value, "id", translate("ID"))
id.password = true
id:depends("use_conf_file", 0)

alterId = s:taboption("main",Value, "alterId", translate("Alter ID"))
alterId.datatype = "range(1,65535)"
alterId:depends("use_conf_file", 0)

security = s:taboption("main",ListValue, "security", translate("Security"))
security:value("none")
security:value("auto")
security:value("aes-128-cfb")
security:value("aes-128-gcm")
security:value("chacha20-poly1305")
security:depends("use_conf_file", 0)

network_type = s:taboption("main",ListValue, "network_type", translate("Network Type"))
network_type:value("tcp")
network_type:value("kcp")
network_type:value("ws")
network_type:depends("use_conf_file", 0)

-- tcp settings
tcp_obfs = s:taboption("main",ListValue, "tcp_obfs", translate("TCP Obfs"))
tcp_obfs:value("none")
tcp_obfs:value("http")
tcp_obfs:depends("network_type", "tcp")

tcp_path = s:taboption("main",DynamicList, "tcp_path", translate("TCP Obfs Path"))
tcp_path:depends("tcp_obfs", "http")

tcp_host = s:taboption("main",DynamicList, "tcp_host", translate("TCP Obfs Header"))
tcp_host:depends("tcp_obfs", "http")
tcp_host.datatype = "host"

-- kcp settings
kcp_obfs = s:taboption("main",ListValue, "kcp_obfs", translate("KCP Obfs"))
kcp_obfs:value("none")
kcp_obfs:value("srtp")
kcp_obfs:value("utp")
kcp_obfs:value("wechat-video")
kcp_obfs:depends("network_type", "kcp")

kcp_mtu = s:taboption("main",Value, "kcp_mtu", translate("KCP MTU"))
kcp_mtu.datatype = "range(576,1460)"
kcp_mtu:depends("network_type", "kcp")

kcp_tti = s:taboption("main",Value, "kcp_tti", translate("KCP TTI"))
kcp_tti.datatype = "range(10,100)"
kcp_tti:depends("network_type", "kcp")

kcp_uplink = s:taboption("main",Value, "kcp_uplink", translate("KCP uplinkCapacity"))
kcp_uplink.datatype = "range(0,1000)"
kcp_uplink:depends("network_type", "kcp")

kcp_downlink = s:taboption("main",Value, "kcp_downlink", translate("KCP downlinkCapacity"))
kcp_downlink.datatype = "range(0,1000)"
kcp_downlink:depends("network_type", "kcp")

kcp_readbuf = s:taboption("main",Value, "kcp_readbuf", translate("KCP readBufferSize"))
kcp_readbuf.datatype = "range(0,100)"
kcp_readbuf:depends("network_type", "kcp")

kcp_writebuf = s:taboption("main",Value, "kcp_writebuf", translate("KCP writeBufferSize"))
kcp_writebuf.datatype = "range(0,100)"
kcp_writebuf:depends("network_type", "kcp")

kcp_congestion = s:taboption("main",Flag, "kcp_congestion", translate("KCP Congestion"))
kcp_congestion:depends("network_type", "kcp")

-- websocket settings
ws_path = s:taboption("main",Value, "ws_path", translate("WebSocket Path"))
ws_path:depends("network_type", "ws")

ws_headers = s:taboption("main",Value, "ws_headers", translate("WebSocket Header"))
ws_headers:depends("network_type", "ws")
ws_headers.datatype = "host"

-- others
tls = s:taboption("main",Flag, "tls", translate("TLS"))
tls.rmempty = false
tls:depends("use_conf_file", 0)

mux = s:taboption("main",Flag, "mux", translate("Mux"))
mux.rmempty = false
mux:depends("use_conf_file", 0)



s:tab("list",  translate("User-defined GFW-List"))
gfwlist = s:taboption("list", TextValue, "conf")
gfwlist.description = translate("<br />（!）Note: When the domain name is entered and will automatically merge with the online GFW-List. Please manually update the GFW-List list after applying.")
gfwlist.rows = 13
gfwlist.wrap = "off"
gfwlist.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
gfwlist.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

local addipconf = "/etc/v2ray/addinip.txt"

s:tab("addip",  translate("GFW-List Add-in IP"))
gfwaddin = s:taboption("addip", TextValue, "addipconf")
gfwaddin.description = translate("<br />（!）Note: IP add-in to GFW-List. Such as Telegram Messenger")
gfwaddin.rows = 13
gfwaddin.wrap = "off"
gfwaddin.cfgvalue = function(self, section)
	return NXFS.readfile(addipconf) or ""
end
gfwaddin.write = function(self, section, value)
	NXFS.writefile(addipconf, value:gsub("\r\n", "\n"))
end

s:tab("status",  translate("Status and Tools"))
s:taboption("status", DummyValue,"opennewwindow" , 
	translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"IP111.CN\" onclick=\"window.open('http://www.ip111.cn/')\" />"))


s:tab("watchdog",  translate("Watchdog Log"))
log = s:taboption("watchdog", TextValue, "sylogtext")
log.template = "cbi/tvalue"
log.rows = 13
log.wrap = "off"
log.readonly="readonly"

function log.cfgvalue(self, section)
  SYS.exec("[ -f /tmp/v2ray_watchdog.log ] && sed '1!G;h;$!d' /tmp/v2ray_watchdog.log > /tmp/v2raypro.log")
	return nixio.fs.readfile(dog)
end

function log.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile(dog, value)
end



t=m:section(TypedSection,"acl_rule",translate("<strong>Client Proxy Mode Settings</strong>"),
translate("Proxy mode settings can be set to specific LAN clients ( <font color=blue> No Proxy, Global Proxy, Game Mode</font>) . Does not need to be set by default."))
t.template="cbi/tblsection"
t.sortable=true
t.anonymous=true
t.addremove=true
e=t:option(Value,"ipaddr",translate("IP Address"))
e.width="40%"
e.datatype="ip4addr"
e.placeholder="0.0.0.0/0"
luci.ip.neighbors({ family = 4 }, function(entry)
	if entry.reachable then
		e:value(entry.dest:string())
	end
end)

e=t:option(ListValue,"filter_mode",translate("Proxy Mode"))
e.width="40%"
e.default="disable"
e.rmempty=false
e:value("disable",translate("No Proxy"))
e:value("global",translate("Global Proxy"))
e:value("game",translate("Game Mode"))

-- ---------------------------------------------------
local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("chmod +x /etc/init.d/v2raypro &")
	os.execute("/etc/init.d/v2raypro restart >/dev/null 2>&1 &")
end

return m
