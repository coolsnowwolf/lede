-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.
--mod by wulishui 20191205

require("luci.model.ipkg")
local fs  = require "nixio.fs"
require("nixio.fs")
local uci = require "luci.model.uci".cursor()
local cport = uci:get_first("cupsd", "cupsd", "port") or 631

local m, s, o

local running=(luci.sys.call("pidof cupsd > /dev/null") == 0)

local button = ""
local state_msg = ""

if running then
        state_msg = "<b><font color=\"green\">" .. translate("～正在运行～") .. "</font></b>"
else
        state_msg = "<b><font color=\"red\">" .. translate("CUPS在睡觉觉zZZ") .. "</font></b>"
end

if running  then
	button = "<br/><br/>---<input class=\"cbi-button cbi-button-apply\" type=\"submit\" value=\" "..translate("打开管理界面").." \" onclick=\"window.open('http://'+window.location.hostname+':"..cport.."')\"/>---"
end

m = Map("cupsd", translate("CUPS打印服务器"))
m.description = translate("<font color=\"green\">CUPS是苹果公司为MacOS和其他类似UNIX的操作系统开发的基于标准的开源打印系统。</font>".. button  .. "<br/><br/>" .. translate("运行状态").. " : "  .. state_msg .. "<br />")

s = m:section(TypedSection, "cupsd", translate(""))
s.anonymous = true

s:option(Flag, "enabled", translate("Enable"))

s:option(Value, "port", translate("WEB管理端口"),translate("可随意设定为无冲突的端口，对程序运行无影响。")).default = 631
s.rmempty = true


return m


