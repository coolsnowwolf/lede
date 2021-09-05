-- Copyright 2017-2018 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs       = require("nixio.fs")
local uci      = require("luci.model.uci").cursor()
local sys      = require("luci.sys")
local util     = require("luci.util")
local dump     = util.ubus("network.interface", "dump", {})
local json     = require("luci.jsonc")
local adbinput = uci:get("adblock", "global", "adb_rtfile") or "/tmp/adb_runtime.json"

m = Map("adblock", translate("Adblock"),
	translate("Configuration of the adblock package to block ad/abuse domains by using DNS. ")
	.. translatef("For further information "
	.. "<a href=\"%s\" target=\"_blank\">"
	.. "check the online documentation</a>", "https://github.com/openwrt/packages/blob/master/net/adblock/files/README.md"))
m.apply_on_parse = true

function m.on_apply(self)
	luci.sys.call("/etc/init.d/adblock reload >/dev/null 2>&1")
	luci.http.redirect(luci.dispatcher.build_url("admin", "services", "adblock"))
end

-- Main adblock options

s = m:section(NamedSection, "global", "adblock")

local parse = json.parse(fs.readfile(adbinput) or "")
if parse then
	status  = parse.data.adblock_status
	version = parse.data.adblock_version
	domains = parse.data.overall_domains
	fetch   = parse.data.fetch_utility
	backend = parse.data.dns_backend
	rundate = parse.data.last_rundate
end

o1 = s:option(Flag, "adb_enabled", translate("Enable Adblock"))
o1.default = o1.disabled
o1.rmempty = false

btn = s:option(Button, "", translate("Suspend / Resume Adblock"))
if parse and status == "enabled" then
	btn.inputtitle = translate("Suspend")
	btn.inputstyle = "reset"
	btn.disabled = false
	function btn.write()
		luci.sys.call("/etc/init.d/adblock suspend >/dev/null 2>&1")
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "adblock"))
	end
elseif parse and status == "paused" then
	btn.inputtitle = translate("Resume")
	btn.inputstyle = "apply"
	btn.disabled = false
	function btn.write()
		luci.sys.call("/etc/init.d/adblock resume >/dev/null 2>&1")
		luci.http.redirect(luci.dispatcher.build_url("admin", "services", "adblock"))
	end
else
	btn.inputtitle = translate("-------")
	btn.inputstyle = "button"
	btn.disabled = true
end

o2 = s:option(ListValue, "adb_dns", translate("DNS Backend (DNS Directory)"),
	translate("List of supported DNS backends with their default list export directory.<br />")
	.. translate("To overwrite the default path use the 'DNS Directory' option in the extra section below."))
o2:value("dnsmasq", "dnsmasq (/tmp)")
o2:value("unbound", "unbound (/var/lib/unbound)")
o2:value("named", "named (/var/lib/bind)")
o2:value("kresd", "kresd (/etc/kresd)")
o2:value("dnscrypt-proxy","dnscrypt-proxy (/tmp)")
o2.default = "dnsmasq (/tmp)"
o2.rmempty = false

o3 = s:option(ListValue, "adb_fetchutil", translate("Download Utility"),
translate("List of supported and fully pre-configured download utilities."))
o3:value("uclient-fetch")
o3:value("wget")
o3:value("curl")
o3:value("aria2c")
o3:value("wget-nossl", "wget-nossl (noSSL)")
o3:value("busybox", "wget-busybox (noSSL)")
o3.default = "uclient-fetch"
o3.rmempty = false

o4 = s:option(ListValue, "adb_trigger", translate("Startup Trigger"),
	translate("List of available network interfaces. Usually the startup will be triggered by the 'wan' interface.<br />")
	.. translate("Choose 'none' to disable automatic startups, 'timed' to use a classic timeout (default 30 sec.) or select another trigger interface."))
o4:value("none")
o4:value("timed")
if dump then
	local i, v
	for i, v in ipairs(dump.interface) do
		if v.interface ~= "loopback" then
			o4:value(v.interface)
		end
	end
end
o4.rmempty = false

-- Runtime information

ds = m:section(NamedSection, "global", "adblock", translate("Runtime Information"))

dv1 = ds:option(DummyValue, "", translate("Adblock Status"))
dv1.template = "adblock/runtime"
if parse == nil then
	dv1.value = translate("n/a")
else
	if status == "error" then
		dv1.value = translate("error")
	elseif status == "disabled" then
		dv1.value = translate("disabled")
	elseif status == "paused" then
		dv1.value = translate("paused")
	elseif status == "running" then
		dv1.value = translate("running")
	else
		dv1.value = translate("enabled")
	end
end

dv2 = ds:option(DummyValue, "", translate("Adblock Version"))
dv2.template = "adblock/runtime"
if parse == nil then
	dv2.value = translate("n/a")
else
	dv2.value = version
end

dv3 = ds:option(DummyValue, "", translate("Download Utility (SSL Library)"),
	translate("For SSL protected blocklist sources you need a suitable SSL library, e.g. 'libustream-ssl' or 'built-in'."))
dv3.template = "adblock/runtime"
if parse == nil then
	dv3.value = translate("n/a")
else
	dv3.value = fetch
end

dv4 = ds:option(DummyValue, "", translate("DNS Backend (DNS Directory)"))
dv4.template = "adblock/runtime"
if parse == nil then
	dv4.value = translate("n/a")
else
	dv4.value = backend
end

dv5 = ds:option(DummyValue, "", translate("Overall Domains"))
dv5.template = "adblock/runtime"
if parse == nil then
	dv5.value = translate("n/a")
else
	dv5.value = domains
end

dv6 = ds:option(DummyValue, "", translate("Last Run"))
dv6.template = "adblock/runtime"
if parse == nil then
	dv6.value = translate("n/a")
else
	dv6.value = rundate
end

-- Blocklist table

bl = m:section(TypedSection, "source", translate("Blocklist Sources"),
	translate("<b>Caution:</b> To prevent OOM exceptions on low memory devices with less than 64 MB free RAM, please only select a few of them!"))
bl.template = "adblock/blocklist"

name = bl:option(Flag, "enabled", translate("Enabled"))
name.rmempty = false

ssl = bl:option(DummyValue, "adb_src", translate("SSL req."))
function ssl.cfgvalue(self, section)
	local source = self.map:get(section, "adb_src")
	if source and source:match("https://") then
		return translate("Yes")
	else
		return translate("No")
	end
end

des = bl:option(DummyValue, "adb_src_desc", translate("Description"))

cat = bl:option(DynamicList, "adb_src_cat", translate("Categories"))
cat.datatype = "uciname"
cat.optional = true

-- Extra options

e = m:section(NamedSection, "extra", "adblock", translate("Extra Options"),
	translate("Options for further tweaking in case the defaults are not suitable for you."))

e1 = e:option(Flag, "adb_debug", translate("Verbose Debug Logging"),
	translate("Enable verbose debug logging in case of any processing error."))
e1.default = e1.disabled
e1.rmempty = false

e2 = e:option(Flag, "adb_forcedns", translate("Force Local DNS"),
	translate("Redirect all DNS queries from 'lan' zone to the local resolver."))
e2.default = e2.disabled
e2.rmempty = false

e3 = e:option(Flag, "adb_forcesrt", translate("Force Overall Sort"),
	translate("Enable memory intense overall sort / duplicate removal on low memory devices (&lt; 64 MB free RAM)"))
e3.default = e3.disabled
e3.rmempty = false

e4 = e:option(Flag, "adb_backup", translate("Enable Blocklist Backup"),
	translate("Create compressed blocklist backups, they will be used in case of download errors or during startup in backup mode."))
e4.default = e4.disabled
e4.rmempty = false

e5 = e:option(Value, "adb_backupdir", translate("Backup Directory"),
	translate("Target directory for adblock backups. Please use only non-volatile disks, e.g. an external usb stick."))
e5:depends("adb_backup", 1)
e5.datatype = "directory"
e5.default = "/mnt"
e5.rmempty = true

e6 = e:option(Flag, "adb_backup_mode", translate("Backup Mode"),
	translate("Do not automatically update blocklists during startup, use blocklist backups instead."))
e6:depends("adb_backup", 1)
e6.default = e6.disabled
e6.rmempty = true

e7 = e:option(Value, "adb_maxqueue", translate("Max. Download Queue"),
	translate("Size of the download queue to handle downloads &amp; list processing in parallel (default '4').<br />")
	.. translate("For further performance improvements you can raise this value, e.g. '8' or '16' should be safe."))
e7.default = 4
e7.datatype = "range(1,32)"
e7.rmempty = false

e8 = e:option(Flag, "adb_jail", translate("'Jail' Blocklist Creation"),
	translate("Builds an additional 'Jail' list (/tmp/adb_list.jail) to block access to all domains except those listed in the whitelist file.<br />")
	.. translate("You can use this restrictive blocklist manually e.g. for guest wifi or kidsafe configurations."))
e8.default = e8.disabled
e8.rmempty = true

e9 = e:option(Flag, "adb_dnsflush", translate("Flush DNS Cache"),
	translate("Flush DNS Cache after adblock processing."))
e9.default = e9.disabled
e9.rmempty = true

e10 = e:option(Flag, "adb_notify", translate("Email Notification"),
	translate("Send notification emails in case of a processing error or if domain count is &le; 0.<br />")
	.. translate("Please note: this needs additional 'msmtp' package installation and setup."))
e10.default = e10.disabled
e10.rmempty = true

e11 = e:option(Value, "adb_notifycnt", translate("Email Notification Count"),
	translate("Raise the minimum email notification count, to get emails if the overall count is less or equal to the given limit (default 0),<br />")
	.. translate("e.g. to receive an email notification with every adblock update set this value to 150000."))
e11.default = 0
e11.datatype = "min(0)"
e11.optional = true

e12 = e:option(Value, "adb_dnsdir", translate("DNS Directory"),
	translate("Target directory for the generated blocklist 'adb_list.overall'."))
e12.datatype = "directory"
e12.optional = true

e13 = e:option(Value, "adb_whitelist", translate("Whitelist File"),
	translate("Full path to the whitelist file."))
e13.datatype = "file"
e13.default = "/etc/adblock/adblock.whitelist"
e13.optional = true

e14 = e:option(Value, "adb_triggerdelay", translate("Trigger Delay"),
	translate("Additional trigger delay in seconds before adblock processing begins."))
e14.datatype = "range(1,60)"
e14.optional = true

return m
