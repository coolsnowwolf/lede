-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.

require("luci.model.ipkg")
local fs  = require "nixio.fs"
require("nixio.fs")

local uci = require "luci.model.uci".cursor()
local vport = uci:get_first("verysync", "setting", "port") or 8886

local m, s

local running=(luci.sys.call("pidof verysync > /dev/null") == 0)

local b = ""
local s = ""


if running  then
b = "<br/><br/><input type=\"submit\" value=\" "..translate("Open Web Interface").." \" onclick=\"window.open('http://'+window.location.hostname+':"..vport.."')\"/>"
end

if running then
s = "<b><font color=\"green\">" .. translate("Verysync 运行中") .. "</font></b>"
else
s = "<b><font color=\"red\">" .. translate("Verysync 未运行") .. "</font></b>"
end

m = Map("verysync", translate("微力同步 一款高效的数据传输工具"))

m.description = translate("简单易用的多平台文件同步软件，惊人的传输速度是不同于其他产品的最大优势， 微力同步 的智能 P2P 技术加速同步，会将文件分割成若干份仅 KB 的数据同步，而文件都会进行 AES 加密处理。".. b .. "<br/><br/>" .. s .. "<br />")

s = m:section(TypedSection, "setting", translate("Settings"))
s.anonymous = true

s:option(Flag, "enabled", translate("Enable"))

s:option(Value, "port", translate("port")).default = 8886
s.rmempty = true

s:option(Value,"home",translate("config_dir")).default="/opt/verysync"
s.rmempty=true

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/verysync start")
end

return m

