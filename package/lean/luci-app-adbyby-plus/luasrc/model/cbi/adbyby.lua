
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"

local m,s,o
local Status

if SYS.call("pidof adbyby >/dev/null") == 0 then
	Status = translate("<strong><font color=\"green\">广告屏蔽大师 Plus + 正在运行</font></strong>")
else
	Status = translate("<strong><font color=\"red\">广告屏蔽大师 Plus + 没有运行</font></strong>")
end

m = Map("adbyby")
m.title	= translate("广告屏蔽大师 Plus+")
m.description = translate("广告屏蔽大师 Plus + 可以全面过滤各种横幅、弹窗、视频广告，同时阻止跟踪、隐私窃取及各种恶意网站<br /><font color=\"red\">Plus + 版本可以和 Adblock Plus Hosts 结合方式运行，过滤广告不损失带宽</font>")

s = m:section(TypedSection, "adbyby")
s.anonymous = true
s.description = translate(string.format("%s<br /><br />", Status))


s:tab("basic",  translate("基本设置"))

o = s:taboption("basic", Flag, "enable")
o.title = translate("Enable")
o.default = 0
o.rmempty = false

o = s:taboption("basic", ListValue, "wan_mode")
o.title = translate("运行模式")
o:value("0", translate("全局模式（最慢, 效果最好）"))
o:value("1", translate("Plus + 模式（只过滤列表内域名结合ABP名单。推荐！）"))
o:value("2", translate("手动代理模式（必须手动设置浏览器代理，或者客户端过滤模式设置）"))
o.default = 1
o.rmempty = false

o = s:taboption("basic", Button, "proxy")
o.title = translate("透明代理")
if SYS.call("iptables-save | grep ADBYBY >/dev/null") == 0 then
	o.inputtitle = translate("点击关闭")
	o.inputstyle = "reset"
	o.write = function()
		SYS.call("/etc/init.d/adbyby del_rule")
		HTTP.redirect(DISP.build_url("admin", "services", "adbyby"))
	end
else
	o.inputtitle = translate("点击开启")
	o.inputstyle = "apply"
	o.write = function()
		SYS.call('[ -n "$(pgrep adbyby)" ] && /etc/init.d/adbyby add_rule')
		HTTP.redirect(DISP.build_url("admin", "services", "adbyby"))
	end
end


local DL = SYS.exec("head -1 /usr/share/adbyby/data/lazy.txt | awk -F' ' '{print $3}'")
local NL = SYS.exec("cat /usr/share/adbyby/data/lazy.txt | wc -l")
local DV = SYS.exec("head -1 /usr/share/adbyby/data/video.txt | awk -F' ' '{print $3}'")
local NV = SYS.exec("cat /usr/share/adbyby/data/video.txt | wc -l")
local NR = SYS.exec("grep -v '^!' /usr/share/adbyby/data/rules.txt | wc -l")
local NU = SYS.exec("cat /usr/share/adbyby/data/user.txt | wc -l")
--local NW = SYS.exec("uci get adbyby.@adbyby[-1].domain 2>/dev/null | wc -l")
local ND = SYS.exec("cat /usr/share/adbyby/dnsmasq.adblock | wc -l")

o = s:taboption("basic", Button, "restart")
o.title = translate("手动执行")
o.inputtitle = translate("重启Adbyby")
o.inputstyle = "reload"
o.write = function()
	SYS.call("nohup sh /usr/share/adbyby/adupdate.sh > /tmp/adupdate.log 2>&1 &")
	SYS.call("sleep 4")
	HTTP.redirect(DISP.build_url("admin", "services", "adbyby"))
end

updatead = s:taboption("basic", Button, "updatead")
updatead.title = translate("规则状态")
updatead.inputtitle = translate("强制更新规则")
updatead.description = translate(string.format("注意：下载并转换规则，后台进程可能需要60-120秒更新，完成后会自动运行，请不要重复点击！<br /><strong>Lazy &nbsp;&nbsp;规则：</strong>%s / %d 条<br /><strong>Video 规则：</strong>%s / %d 条<br /><strong>Hosts 列表：</strong>%d 条<br /><strong>第三方订阅 规则：</strong>%d 条<br /><strong>用户自定义 规则：</strong>%d 条", DL, NL, DV, NV, ND, math.abs(NR-NU), NR))
updatead.inputstyle = "apply"
updatead.write = function()
	SYS.call("nohup sh /usr/share/adbyby/adblock.sh > /tmp/adupdate.log 2>&1 &")
end

s:tab("advanced", translate("高级设置"))

o = s:taboption("advanced", Flag, "cron_mode")
o.title = translate("每天凌晨6点更新规则并重启")
o.default = 0
o.rmempty = false

o = s:taboption("advanced", Flag, "block_ios")
o.title = translate("拦截 Apple iOS 的OTA更新")
o.default = 0
o.rmempty = false

--o = s:taboption("advanced", Flag, "update_source")
--o.title = translate("优先从官方网站更新规则")
--o.default = 1
--o.rmempty = false

mem = s:taboption("advanced", Flag, "mem_mode")
mem.title = translate("内存运行模式")
mem.default = 1
mem.rmempty = false
--mem.description = translate("在内存中运行Adbyby。更快的速度，更少的存储空间损耗")


s:tab("help",  translate("Plus+ 模式过滤的域名"))

local conf = "/usr/share/adbyby/adhost.conf"
o = s:taboption("help", TextValue, "conf")
o.description = translate("这些域名在 Plus 模式中会被过滤。你需要要填写域名即可，例如  http://www.baidu.com，你写 baidu.com 即可。每行一个域名")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("esc",  translate("域名白名单"))

local escconf = "/usr/share/adbyby/adesc.conf"
o = s:taboption("esc", TextValue, "escconf")
o.description = translate("永不过滤白名单内的域名（所有模式中生效）")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(escconf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(escconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("black",  translate("域名黑名单"))

local blackconf = "/usr/share/adbyby/adblack.conf"
o = s:taboption("black", TextValue, "blackconf")
o.description = translate("拦截黑名单内的域名（所有模式中生效）")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(blackconf) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(blackconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("block",  translate("IP黑名单"))

local blockconf = "/usr/share/adbyby/blockip.conf"
o = s:taboption("block", TextValue, "blockconf")
o.description = translate("拦截黑名单内的IP地址（所有模式中生效）")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(blockconf) or " "
end
o.write = function(self, section, value)
	NXFS.writefile(blockconf, value:gsub("\r\n", "\n"))
	--SYS.call("/etc/init.d/adbyby restart")
end

s:tab("user", translate("用户自定义规则"))

local file = "/usr/share/adbyby/rules.txt"
o = s:taboption("user", TextValue, "rules")
o.description = translate("每行一条规则，感叹号开头的被认为是注释")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
	return NXFS.readfile(file) or ""
end
o.write = function(self, section, value)
	NXFS.writefile(file, value:gsub("\r\n", "\n"))
end

t=m:section(TypedSection,"acl_rule",translate("<strong>客户端过滤模式设置</strong>"),
translate("可以为局域网客户端分别设置不同的过滤模式 ( <font color=blue> 不过滤 , 全局过滤 </font> ) 。默认无需设置"))
t.template="cbi/tblsection"
t.sortable=true
t.anonymous=true
t.addremove=true

e=t:option(Value,"ipaddr",translate("IP地址"))
e.width="40%"
e.datatype="ip4addr"
e.placeholder="0.0.0.0/0"
luci.ip.neighbors({ family = 4 }, function(entry)
	if entry.reachable then
		e:value(entry.dest:string())
	end
end)

e=t:option(ListValue,"filter_mode",translate("过滤模式"))
e.width="40%"
e.default="disable"
e.rmempty=false
e:value("disable",translate("不过滤"))
e:value("global",translate("全局过滤"))

return m
