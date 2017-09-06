--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

require("luci.sys")

module("luci.controller.vsftpd", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/vsftpd") then
		return
	end

	entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false
	entry({"admin", "nas", "vsftpd"},
		alias("admin", "nas", "vsftpd", "general"),
		_("FTP Server"))

	entry({"admin", "nas", "vsftpd", "general"},
		cbi("vsftpd/general"),
		_("General Settings"), 10).leaf = true

	entry({"admin", "nas", "vsftpd", "users"},
		cbi("vsftpd/users"),
		_("Virtual Users"), 20).leaf = true

	entry({"admin", "nas", "vsftpd", "anonymous"},
		cbi("vsftpd/anonymous"),
		_("Anonymous User"), 30).leaf = true

	entry({"admin", "nas", "vsftpd", "log"},
		cbi("vsftpd/log"),
		_("Log Settings"), 40).leaf = true

	entry({"admin", "nas", "vsftpd", "item"},
		cbi("vsftpd/item"), nil).leaf = true
end
