--[[
LuCI - Lua Configuration Interface

Copyright (C) 2014, QA Cafe, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local fs = require "nixio.fs"

m = Map("cshark", translate("CloudShark"))

if fs.access("/etc/config/cshark") then
	m:section(SimpleSection).template = "cshark"

	s = m:section(TypedSection, "cshark", translate("Options"))
	s.anonymous = true
	s.addremove = false

	s:option(Value, "url", translate("CloudShark URL"))
	s:option(Value, "token", translate("CloudShark API token"))
end

return m
