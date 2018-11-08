--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

m = Map("vsftpd", translate("FTP Server - Anonymous Settings"))

sa = m:section(NamedSection, "anonymous", "anonymous", translate("Anonymous Settings"))

o = sa:option(Flag, "enabled", translate("Enabled"))
o.default = false

o = sa:option(Value, "username", translate("Username"), translate("An actual local user to handle anonymous user"))
o.default = "ftp"

o = sa:option(Value, "root", translate("Root directory"))
o.default = "/home/ftp"

o = sa:option(Value, "umask", translate("File mode umask"))
o.default = "022"

o = sa:option(Value, "maxrate", translate("Max transmit rate"), translate("0 means no limitation"))
o.default = "0"

o = sa:option(Flag, "writemkdir", translate("Enable write/mkdir"))
o.default = false

o = sa:option(Flag, "upload", translate("Enable upload"))
o.default = false

o = sa:option(Flag, "others", translate("Enable other rights"), translate("Include rename, deletion ..."))
o.default = false


return m
