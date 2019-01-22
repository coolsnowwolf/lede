-- Copyright 2018 Rosy Song <rosysong@rosinson.com>
-- Licensed to the public under the Apache License 2.0.

local uci = require("luci.model.uci").cursor()

local def_rate_dl = uci:get("nft-qos", "@default[0]", "def_rate_dl")
local def_rate_ul = uci:get("nft-qos", "@default[0]", "def_rate_ul")
local def_unit_dl = uci:get("nft-qos", "@default[0]", "def_unit_dl")
local def_unit_ul = uci:get("nft-qos", "@default[0]", "def_unit_ul")
local def_enabled = uci:get("nft-qos", "@default[0]", "enabled")

m = Map("nft-qos", translate("Qos over Nftables"))

--
-- Default Settings
--
s = m:section(TypedSection, "default", translate("Default Settings"))
s.addremove = false
s.anonymous = true

grd = s:option(Value, "def_rate_dl", translate("Default Download Rate"), translate("Default value for download rate"))
grd.default = def_rate_dl or '512'

gdu = s:option(ListValue, "def_unit_dl", translate("Default Download Unit"), translate("Default unit for download rate"))
gdu.default = def_unit_dl or "kbytes"
gdu:value("bytes", "Bytes/s")
gdu:value("kbytes", "KBytes/s")
gdu:value("mbytes", "MBytes/s")

gru = s:option(Value, "def_rate_ul", translate("Default Upload Rate"), translate("Default value for upload rate"))
gru.default = def_rate_ul or '512'

guu = s:option(ListValue, "def_unit_ul", translate("Default Upload Unit"), translate("Default unit for upload rate"))
guu.default = def_unit_ul or "kbytes"
guu:value("bytes", "Bytes/s")
guu:value("kbytes", "KBytes/s")
guu:value("mbytes", "MBytes/s")

ge = s:option(Flag, "enabled", translate("Enabled Flag"), translate("Enable this package"))
ge.default = def_enabled or ge.enabled
ge.rmempty = false

--
-- Download Rate IPv4
--
dr = m:section(TypedSection, "download", translate("Download Rate IPv4"))
dr.anonymous = true
dr.addremove = true
dr.template = "cbi/tblsection"

hn = dr:option(Value, "hostname", translate("Hostname"))
hn.default = '?'

ip = dr:option(Value, "ipaddr", translate("IPv4"))
ip.default = '?'
ip.size = 18
if nixio.fs.access("/tmp/dhcp.leases") then
        ip.titleref = luci.dispatcher.build_url("admin", "status", "overview")
end

mc = dr:option(Value, "macaddr", translate("MAC (deprecated)"))
mc.rmempty = true
mc.size = 17

rt = dr:option(Value, "rate", translate("Rate"))
rt.default = def_rate_dl or '512'
rt.size = 4

un = dr:option(ListValue, "unit", translate("Unit"))
un.default = def_unit_dl or "kbytes"
un:value("bytes", "Bytes/s")
un:value("kbytes", "KBytes/s")
un:value("mbytes", "MBytes/s")

--
-- Upload Rate IPv4
--
--
ur = m:section(TypedSection, "upload", translate("Upload Rate IPv4"))
ur.anonymous = true
ur.addremove = true
ur.template = "cbi/tblsection"

uhn = ur:option(Value, "hostname", translate("Hostname"))
uhn.default = '?'

uip = ur:option(Value, "ipaddr", translate("IPv4"))
uip.default = '?'
uip.size = 18
if nixio.fs.access("/tmp/dhcp.leases") then
        uip.titleref = luci.dispatcher.build_url("admin", "status", "overview")
end

umc = ur:option(Value, "macaddr", translate("MAC (optional)"))
umc.rmempty = true
umc.size = 17

urt = ur:option(Value, "rate", translate("Rate"))
urt.default = def_rate_ul or '512'
urt.size = 4

uun = ur:option(ListValue, "unit", translate("Unit"))
uun.default = def_unit_ul or "kbytes"
uun:value("bytes", "Bytes/s")
uun:value("kbytes", "KBytes/s")
uun:value("mbytes", "MBytes/s")

return m
