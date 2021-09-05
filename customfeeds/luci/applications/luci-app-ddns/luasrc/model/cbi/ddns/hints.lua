-- Copyright 2014-2016 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
-- Licensed to the public under the Apache License 2.0.

local DISP = require "luci.dispatcher"
local SYS  = require "luci.sys"
local CTRL = require "luci.controller.ddns"	-- this application's controller
local DDNS = require "luci.tools.ddns"		-- ddns multiused functions

-- html constants
font_red = [[<font color="red">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

-- cbi-map definition -- #######################################################
m = Map("ddns")
m.title		= CTRL.app_title_back()
m.description	= CTRL.app_description()
m.redirect	= DISP.build_url("admin", "services", "ddns")

-- SimpleSection definition -- #################################################
-- show Hints to optimize installation and script usage
s = m:section( SimpleSection,
	translate("Hints"),
	translate("Below a list of configuration tips for your system to run Dynamic DNS updates without limitations") )

-- ddns-scripts needs to be updated for full functionality
if not CTRL.service_ok() then
	local so = s:option(DummyValue, "_update_needed")
	so.titleref = DISP.build_url("admin", "system", "packages")
	so.rawhtml  = true
	so.title = font_red .. bold_on ..
		translate("Software update required") .. bold_off .. font_off
	so.value = translate("The currently installed 'ddns-scripts' package did not support all available settings.") ..
			"<br />" ..
			translate("Please update to the current version!")
end

-- DDNS Service disabled
if not SYS.init.enabled("ddns") then
	local se = s:option(DummyValue, "_not_enabled")
	se.titleref = DISP.build_url("admin", "system", "startup")
	se.rawhtml  = true
	se.title = bold_on ..
		translate("DDNS Autostart disabled") .. bold_off
	se.value = translate("Currently DDNS updates are not started at boot or on interface events." .. "<br />" ..
			"This is the default if you run DDNS scripts by yourself (i.e. via cron with force_interval set to '0')" )
end

-- No IPv6 support
if not DDNS.env_info("has_ipv6") then
	local v6 = s:option(DummyValue, "_no_ipv6")
	v6.titleref = 'http://www.openwrt.org" target="_blank'
	v6.rawhtml  = true
	v6.title = bold_on ..
		translate("IPv6 not supported") .. bold_off
	v6.value = translate("IPv6 is currently not (fully) supported by this system" .. "<br />" ..
			"Please follow the instructions on OpenWrt's homepage to enable IPv6 support" .. "<br />" ..
			"or update your system to the latest OpenWrt Release")
end

-- No HTTPS support
if not DDNS.env_info("has_ssl") then
	local sl = s:option(DummyValue, "_no_https")
	sl.titleref = DISP.build_url("admin", "system", "packages")
	sl.rawhtml  = true
	sl.title = bold_on ..
		translate("HTTPS not supported") .. bold_off
	sl.value = translate("Neither GNU Wget with SSL nor cURL installed to support secure updates via HTTPS protocol.") ..
			"<br />- " ..
			translate("You should install 'wget' or 'curl' or 'uclient-fetch' with 'libustream-*ssl' package.") ..
			"<br />- " ..
			translate("In some versions cURL/libcurl in OpenWrt is compiled without proxy support.")
end

-- No bind_network
if not DDNS.env_info("has_bindnet") then
	local bn = s:option(DummyValue, "_no_bind_network")
	bn.titleref = DISP.build_url("admin", "system", "packages")
	bn.rawhtml  = true
	bn.title = bold_on ..
		translate("Binding to a specific network not supported") .. bold_off
	bn.value = translate("Neither GNU Wget with SSL nor cURL installed to select a network to use for communication.") ..
			"<br />- " ..
			translate("You should install 'wget' or 'curl' package.") ..
			"<br />- " ..
			translate("GNU Wget will use the IP of given network, cURL will use the physical interface.") ..
			"<br />- " ..
			translate("In some versions cURL/libcurl in OpenWrt is compiled without proxy support.")
end

-- currently only cURL possibly without proxy support
if not DDNS.env_info("has_proxy") then
	local px = s:option(DummyValue, "_no_proxy")
	px.titleref = DISP.build_url("admin", "system", "packages")
	px.rawhtml  = true
	px.title = bold_on ..
		translate("cURL without Proxy Support") .. bold_off
	px.value = translate("cURL is installed, but libcurl was compiled without proxy support.") ..
			"<br />- " ..
			translate("You should install 'wget' or 'uclient-fetch' package or replace libcurl.") ..
			"<br />- " ..
			translate("In some versions cURL/libcurl in OpenWrt is compiled without proxy support.")
end

-- "Force IP Version not supported"
if not DDNS.env_info("has_forceip") then
	local fi = s:option(DummyValue, "_no_force_ip")
	fi.titleref = DISP.build_url("admin", "system", "packages")
	fi.rawhtml  = true
	fi.title = bold_on ..
		translate("Force IP Version not supported") .. bold_off
	local value = translate("BusyBox's nslookup and Wget do not support to specify " ..
				"the IP version to use for communication with DDNS Provider!")
	if not (DDNS.env_info("has_wgetssl") or DDNS.env_info("has_curl") or DDNS.env_info("has_fetch")) then
		value = value .. "<br />- " ..
			translate("You should install 'wget' or 'curl' or 'uclient-fetch' package.")
	end
	if not DDNS.env_info("has_bindhost") then
		value = value .. "<br />- " ..
			translate("You should install 'bind-host' or 'knot-host' or 'drill' package for DNS requests.")
	end
	fi.value = value
end

-- "DNS requests via TCP not supported"
if not DDNS.env_info("has_bindhost") then
	local dt = s:option(DummyValue, "_no_dnstcp")
	dt.titleref = DISP.build_url("admin", "system", "packages")
	dt.rawhtml  = true
	dt.title = bold_on ..
		translate("DNS requests via TCP not supported") .. bold_off
	dt.value = translate("BusyBox's nslookup and hostip do not support to specify to use TCP " ..
				"instead of default UDP when requesting DNS server!") ..
			"<br />- " ..
			translate("You should install 'bind-host' or 'knot-host' or 'drill' package for DNS requests.")
end

-- nslookup compiled with musl produce problems when using
if not DDNS.env_info("has_dnsserver") then
	local ds = s:option(DummyValue, "_no_dnsserver")
	ds.titleref = DISP.build_url("admin", "system", "packages")
	ds.rawhtml  = true
	ds.title = bold_on ..
		translate("Using specific DNS Server not supported") .. bold_off
	ds.value = translate("BusyBox's nslookup in the current compiled version " ..
				"does not handle given DNS Servers correctly!") ..
			"<br />- " ..
			translate("You should install 'bind-host' or 'knot-host' or 'drill' or 'hostip' package, " ..
				"if you need to specify a DNS server to detect your registered IP.")
end

-- certificates installed
if DDNS.env_info("has_ssl") and not DDNS.env_info("has_cacerts") then
	local ca = s:option(DummyValue, "_no_certs")
	ca.titleref = DISP.build_url("admin", "system", "packages")
	ca.rawhtml  = true
	ca.title = bold_on ..
		translate("No certificates found") .. bold_off
	ca.value = translate("If using secure communication you should verify server certificates!") ..
			"<br />- " ..
			translate("Install 'ca-certificates' package or needed certificates " ..
				"by hand into /etc/ssl/certs default directory")
end

return m
