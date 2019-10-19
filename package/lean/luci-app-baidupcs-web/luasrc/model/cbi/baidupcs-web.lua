require("luci.sys")
require("luci.util")

local fs  = require "nixio.fs"

local uci = require "luci.model.uci".cursor()

local m, s

local running=(luci.sys.call("pidof BaiduPCS-Web > /dev/null") == 0)

local button = ""
local state_msg = ""
local trport = uci:get("baidupcs-web", "config", "port")
if running  then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"button\" value=\" " .. translate("打开BaiduPCS-Web管理界面") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. trport .. "')\"/>"
end

if running then
        state_msg = "<b><font color=\"green\">" .. translate("BaiduPCS-Web 运行中") .. "</font></b>"
else
        state_msg = "<b><font color=\"red\">" .. translate("BaiduPCS-Web 未运行") .. "</font></b>"
end

m = Map("baidupcs-web", translate("百度网盘管理"), translate("基于BaiduPCS-Web，让你高效的使用百度云。") .. button
        .. "<br/><br/>" .. translate("BaiduPCS-Web运行状态").. " : "  .. state_msg .. "<br/>")

s = m:section(TypedSection, "baidupcs-web", "")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("启用"))
enable.rmempty = false

o = s:option(Value, "port", translate("监听端口"))
o.placeholder = 5299
o.default     = 5299
o.datatype    = "port"
o.rmempty     = false

o = s:option(Value, "dl_dir", translate("下载目录"))
o.placeholder = "/opt/baidupcsweb-download"
o.default     = "/opt/baidupcsweb-download"
o.rmempty     = false

return m
