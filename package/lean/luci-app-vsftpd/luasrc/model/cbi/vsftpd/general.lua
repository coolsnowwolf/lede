--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

m = Map("vsftpd", translate("FTP Server - General Settings"))

sl = m:section(NamedSection, "listen", "listen", translate("Listening Settings"))

o = sl:option(Flag, "enable4", translate("Enable IPv4"))
o.rmempty = false
o.default = true

o = sl:option(Value, "ipv4", translate("IPv4 Address"))
o.datatype = "ip4addr"
o.default = "0.0.0.0"

o = sl:option(Flag, "enable6", translate("Enable IPv6"))
o.rmempty = false

o = sl:option(Value, "ipv6", translate("IPv6 Address"))
o.datatype = "ip6addr"
o.default = "::"

o = sl:option(Value, "port", translate("Listen Port"))
o.datatype = "uinteger"
o.default = "21"

o = sl:option(Value, "dataport", translate("Data Port"))
o.datatype = "uinteger"
o.default = "20"

o = sl:option(Value, "pasv_min_port", translate("Pasv Min Port"))
o.datatype = "uinteger"
o.default = "50000"

o = sl:option(Value, "pasv_max_port", translate("Pasv Max Port"))
o.datatype = "uinteger"
o.default = "51000"


sg = m:section(NamedSection, "global", "global", translate("Global Settings"))

o = sg:option(Flag, "write", translate("Enable write"), translate("When disabled, all write request will give permission denied."));
o.default = true

o = sg:option(Flag, "download", translate("Enable download"), translate("When disabled, all download request will give permission denied."));
o.default = true

o = sg:option(Flag, "dirlist", translate("Enable directory list"), translate("When disabled, list commands will give permission denied."))
o.default = true

o = sg:option(Flag, "lsrecurse", translate("Allow directory recursely list"))

o = sg:option(Flag, "dotfile", translate("Show dot files"), translate(". and .. are excluded."));
o.default = true

o = sg:option(Value, "umask", translate("File mode umask"), translate("Uploaded file mode will be 666 - &lt;umask&gt;; directory mode will be 777 - &lt;umask&gt;."))
o.default = "022"

o = sg:option(Value, "banner", translate("FTP Banner"))

o = sg:option(Flag, "dirmessage", translate("Enable directory message"), translate("A message will be displayed when entering a directory."))

o = sg:option(Value, "dirmsgfile", translate("Directory message filename"))
o.default = ".message"


sl = m:section(NamedSection, "local", "local", translate("Local Users"))

o = sl:option(Flag, "enabled", translate("Enable local user"))
o.rmempty = false

o = sl:option(Value, "root", translate("Root directory"), translate("Leave empty will use user's home directory"))
o.default = ""


sc = m:section(NamedSection, "connection", "connection", translate("Connection Settings"))

o = sc:option(Flag, "portmode", translate("Enable PORT mode"))
o = sc:option(Flag, "pasvmode", translate("Enable PASV mode"))

o = sc:option(ListValue, "ascii", translate("ASCII mode"))
o:value("disabled", translate("Disabled"))
o:value("download", translate("Download only"))
o:value("upload", translate("Upload only"))
o:value("both", translate("Both download and upload"))
o.default = "both"

o = sc:option(Value, "idletimeout", translate("Idle session timeout"), translate("in seconds"))
o.datatype = "uinteger"
o.default = "1800"
o = sc:option(Value, "conntimeout", translate("Connection timeout"), translate("in seconds"))
o.datatype = "uinteger"
o.default = "120"
o = sc:option(Value, "dataconntimeout", translate("Data connection timeout"), translate("in seconds"))
o.datatype = "uinteger"
o.default = "120"
o = sc:option(Value, "maxclient", translate("Max clients"), translate("0 means no limitation"))
o.datatype = "uinteger"
o.default = "0"
o = sc:option(Value, "maxperip", translate("Max clients per IP"), translate("0 means no limitation"))
o.datatype = "uinteger"
o.default = "0"
o = sc:option(Value, "maxrate", translate("Max transmit rate"), translate("in KB/s, 0 means no limitation"))
o.datatype = "uinteger"
o.default = "0"
o = sc:option(Value, "maxretry", translate("Max login fail count"), translate("Can not be zero, default is 3"))
o.datatype = "uinteger"
o.default = "3"


return m
