--[[
LuCI - Lua Configuration Interface

Copyright 2016 Weijie Gao <hackpascal@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

m = Map("vsftpd")
m.title = translate("FTP Server - Log Settings")

sl = m:section(NamedSection, "log", "log", translate("Log Settings"))

o = sl:option(Flag, "syslog", translate("Enable syslog"))
o.default = false

o = sl:option(Flag, "xreflog", translate("Enable file log"))
o.default = true

o = sl:option(Value, "file", translate("Log file"))
o.default = "/var/log/vsftpd.log"

return m
