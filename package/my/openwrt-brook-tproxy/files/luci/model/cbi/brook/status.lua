-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Copyright (C) 2018 openwrt-brook-tproxy
-- Licensed to the public under the GNU General Public License v3.

local IPK_Version="1.0.0"
local m, s, o
local tproxy_run=0
local tunnel_run=0
local gfw_count=0
local ad_count=0
local ip_count=0
local gfwmode=0

local fs = require "nixio.fs"
local sys = require "luci.sys"
local brook = "brook"

if nixio.fs.access("/etc/dnsmasq.brook/gfw_list.conf") then
	gfwmode=1
end

-- html constants
font_blue = [[<font color="blue">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

local brook_version=translate("Unknown")
local brook_bin="/usr/bin/brook"
if not fs.access(brook_bin)  then
	brook_version=translate("Not exist")
else
	if not fs.access(brook_bin, "rwx", "rx", "rx") then
		fs.chmod(brook_bin, 755)
	end
	brook_version=sys.exec(brook_bin .. " -v | awk '{print $3}'")
	if not brook_version or brook_version == "" then
		brook_version = translate("Unknown")
	end
end

if gfwmode==1 then
	gfw_count = tonumber(sys.exec("cat /etc/dnsmasq.brook/gfw_list.conf | wc -l"))/2
	if nixio.fs.access("/etc/dnsmasq.brook/ad.conf") then
		ad_count=tonumber(sys.exec("cat /etc/dnsmasq.brook/ad.conf | wc -l"))
	end
end

if nixio.fs.access("/etc/china_ip.txt") then
	ip_count = sys.exec("cat /etc/china_ip.txt | wc -l")
end

local icount=sys.exec("ps -w | grep \"brook tproxy\" |grep -v grep| wc -l")
if tonumber(icount)>0 then
	tproxy_run=1
end

icount=sys.exec("ps -w | grep \"brook tunnel\" |grep -v grep| wc -l")
if tonumber(icount)>0 then
	tunnel_run=1
end

m = SimpleForm("Version", translate("Running Status"))
m.reset = false
m.submit = false

s=m:field(DummyValue,"tproxy_run",translate("Brook Tproxy"))
s.rawhtml  = true
if tproxy_run == 1 then
	s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

s=m:field(DummyValue,"tunnel_run",translate("DNS Tunnel"))
s.rawhtml  = true
if tunnel_run == 1 then
	s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
	s.value = translate("Not Running")
end

if gfwmode==1 then
s=m:field(DummyValue,"gfw_data",translate("GFW List Data"))
s.rawhtml  = true
s.template = "brook/refresh"
s.value =tostring(math.ceil(gfw_count)) .. " " .. translate("Records")

s=m:field(DummyValue,"ad_data",translate("Advertising Data"))
s.rawhtml  = true
s.template = "brook/refresh"
s.value =tostring(math.ceil(ad_count)) .. " " .. translate("Records")
end

s=m:field(DummyValue,"ip_data",translate("China IP Data"))
s.rawhtml  = true
s.template = "brook/refresh"
s.value =ip_count .. " " .. translate("Records")

s=m:field(DummyValue,"brook_version",translate("Brook Version"))
s.rawhtml  = true
s.value =brook_version

s=m:field(DummyValue,"brook_project",translate("Brook Project"))
s.rawhtml  = true
s.value =bold_on .. [[<a href="]] .. "https://github.com/txthinking/brook" .. [[" >]]
	.. "https://github.com/txthinking/brook" .. [[</a>]] .. bold_off

s=m:field(DummyValue,"ipk_version",translate("IPK Version"))
s.rawhtml  = true
s.value =IPK_Version

s=m:field(DummyValue,"ipk_project",translate("IPK Project"))
s.rawhtml  = true
s.value =bold_on .. [[<a href="]] .. "https://github.com/WouldChar/openwrt-brook-tproxy" .. [[" >]]
	.. "https://github.com/WouldChar/openwrt-brook-tproxy" .. [[</a>]] .. bold_off

return m