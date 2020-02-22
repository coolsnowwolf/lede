
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"

local DL = SYS.exec("head -1 /tmp/adbyby/data/lazy.txt | awk -F' ' '{print $3,$4}'") or ""
local DV = SYS.exec("head -1 /tmp/adbyby/data/video.txt | awk -F' ' '{print $3,$4}'") or ""
local NR = SYS.exec("grep -v '^!' /usr/share/adbyby/data/rules.txt | wc -l")
local NU = SYS.exec("cat /usr/share/adbyby/data/user.txt | wc -l")
local UD = NXFS.readfile("/tmp/adbyby.updated") or "1970-01-01 00:00:00"
local ND = SYS.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l")

m = Map("adbyby")
m.title	= translate("Adbyby Plus +")
m.description = translate("Adbyby Plus + can filter all kinds of banners, popups, video ads, and prevent tracking, privacy theft and a variety of malicious websites<br /><font color=\"red\">Plus + version combination mode can operation with Adblock Plus Host，filtering ads without losing bandwidth</font>")

m:section(SimpleSection).template  = "adbyby/adbyby_status"

s = m:section(TypedSection, "adbyby")
s.anonymous = true

s:tab("basic",  translate("Base Setting"))

o = s:taboption("basic", Flag, "enable")
o.title = translate("Enable")
o.default = 0
o.rmempty = false

o = s:taboption("basic", ListValue, "wan_mode")
o.title = translate("Running Mode")
o:value("0", translate("Global Mode（The slowest, the best effects）"))
o:value("1", translate("Plus + Mode（Filter domain name list and blacklist website.Recommended）"))
o:value("2", translate("No filter Mode (Must set in Client Filter Mode Settings manually)"))
o.default = 1
o.rmempty = false

o = s:taboption("basic", Button, "restart")
o.title = translate("Adbyby and Rule state")
o.inputtitle = translate("Restart Adbyby")
o.description = string.format("<strong>Last Update Checked：</strong> %s<br /><strong>Lazy Rule：</strong>%s <br /><strong>Video Rule：</strong>%s", UD, DL, DV) 
o.inputstyle = "reload"
o.write = function()
	SYS.call("rm -rf /tmp/adbyby.updated && /usr/share/adbyby/admem.sh > /tmp/adupdate.log 2>&1 &")
  SYS.call("sleep 5")
	HTTP.redirect(DISP.build_url("admin", "services", "adbyby"))
end

s:tab("advanced", translate("Advance Setting"))

o = s:taboption("advanced", Flag, "cron_mode")
o.title = translate("Update the rule at 6 a.m. every morning and restart adbyby")
o.default = 0
o.rmempty = false
o.description = translate(string.format("<strong><font color=blue>Adblock Plus Host List：</font></strong> %s Lines<br /><br />", ND))

updatead = s:taboption("advanced", Button, "updatead", translate("Manually force update<br />Adblock Plus Host List"), translate("Note: It needs to download and convert the rules. The background process may takes 60-120 seconds to run. <br / > After completed it would automatically refresh, please do not duplicate click!"))
updatead.inputtitle = translate("Manually force update")
updatead.inputstyle = "apply"
updatead.write = function()
	SYS.call("nohup sh /usr/share/adbyby/adblock.sh > /tmp/adupdate.log 2>&1 &")
end

o = s:taboption("advanced", Flag, "block_ios")
o.title = translate("Block Apple iOS OTA update")
o.default = 0
o.rmempty = false

s:tab("help",  translate("Plus+ Domain List"))

local conf = "/usr/share/adbyby/adhost.conf"
o = s:taboption("help", TextValue, "conf")
o.description = translate("（!）Note that you should fill to the domain name ONLY. For example, http://www.baidu.com only needs to write to baidu.com. One line for each")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("esc",  translate("Bypass Domain List"))

local escconf = "/usr/share/adbyby/adesc.conf"
o = s:taboption("esc", TextValue, "escconf")
o.description = translate("（!）Will Never filter these Domain")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(escconf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(escconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("black",  translate("Black Domain List"))

local blackconf = "/usr/share/adbyby/adblack.conf"
o = s:taboption("black", TextValue, "blackconf")
o.description = translate("（!）Will Always block these Domain")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(blackconf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(blackconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("block",  translate("Black IP List"))

local blockconf = "/usr/share/adbyby/blockip.conf"
o = s:taboption("block", TextValue, "blockconf")
o.description = translate("（!）Will Always block these IP")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(blockconf) or " "
end
o.write = function(self, section, value)
	NXFS.writefile(blockconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("user", translate("User-defined Rule"))

local file = "/usr/share/adbyby/rules.txt"
o = s:taboption("user", TextValue, "rules")
o.description = translate("Each line of the beginning exclamation mark is considered an annotation.")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(file) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(file, value:gsub("\r\n", "\n"))
end

t=m:section(TypedSection,"acl_rule",translate("<strong>Client Filter Mode Settings</strong>"),
translate("Filter mode settings can be set to specific LAN clients ( <font color=blue> No filter , Global filter </font> ) . Does not need to be set by default."))
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

e=t:option(ListValue,"filter_mode",translate("Filter Mode"))
e.width="40%"
e.default="disable"
e.rmempty=false
e:value("disable",translate("No filter"))
e:value("global",translate("Global filter"))

return m
