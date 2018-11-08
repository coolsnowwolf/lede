--[[
Sync Dial Luci configuration page.
Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]]--

local fs = require "nixio.fs"

local cmd = "mwan3 status | grep -c \"is online and tracking is active\""
local shellpipe = io.popen(cmd,"r")
local ifnum = shellpipe:read("*a")
shellpipe:close()


m = Map("syncdial", translate("创建虚拟WAN接口"),
        translatef("使用macvlan驱动创建多个虚拟WAN口。<br />当前在线接口数量：")..ifnum)

s = m:section(TypedSection, "syncdial", translate(" "))
s.anonymous = true

switch = s:option(Flag, "enabled", "启用")
switch.rmempty = false

--s:option(Flag, "force_redial", "强制全部重拨", "如果有接口掉线则强制所有接口下线重拨。").rmempty = false

wannum = s:option(Value, "wannum", "虚拟WAN接口数量")
wannum.datatype = "range(0,20)"
wannum.optional = false

s:option(Flag, "old_frame", "使用旧的macvlan创建方式").rmempty = false

o = s:option(DummyValue, "_redial", "重新并发拨号")
o.template = "syncdial/redial_button"
o.width    = "10%"

return m


