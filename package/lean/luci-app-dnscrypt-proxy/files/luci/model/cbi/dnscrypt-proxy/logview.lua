-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local NXFS = require "nixio.fs"
local DISP = require "luci.dispatcher"
local HTTP = require "luci.http"
local UCI = luci.model.uci.cursor()

local m = Map("dnscrypt-proxy")

-- log directory
log_dir = UCI:get_first(m.config, "global", "log_dir") or "/tmp"
run_dir = UCI:get_first(m.config, "global", "run_dir") or "/var/etc"
dnsmasq_dir = UCI:get_first(m.config, "global", "dnsmasq_dir") or "/tmp/dnsmasq"
local logfile_list = {}

local logfs = {
"%s/dnscrypt*" % run_dir,
"%s.*/*.*" % dnsmasq_dir,
"%s/*.*" % "/usr/share/dnscrypt-proxy",
"%s/dnscrypt_*" % log_dir,
}
local _, dir, ns, lv, lfile
for _, dir in pairs(logfs) do
for path in (NXFS.glob(dir) or function() end) do
	logfile_list[#logfile_list+1] = path
end
end

ns = m:section(TypedSection, "_dummy", translate("File Viewer"))
ns.addremove = false
ns.anonymous = true
function ns.cfgsections()
	return{"_exrules"}
end

lv = ns:option(DynamicList, "logfiles")
lv.template = "dnscrypt-proxy/logsview"
lv.inputtitle = translate("Read / Reread log file")
lv.rows = 25
lv.default = ""
for _, lfile in ipairs(logfile_list) do lv:value(lfile, lfile) end
function lv.cfgvalue(self, section)
	if logfile_list[1] then
		local lfile=logfile_list[1]
		if NXFS.access(lfile) then
			return lfile .. "\n" .. translate("Please press [Read] button")
		end
		return lfile .. "\n" .. translate("File not found or empty")
	else
		return log_dir .. "\/\n" .. translate("No files found")
	end
end

return m
