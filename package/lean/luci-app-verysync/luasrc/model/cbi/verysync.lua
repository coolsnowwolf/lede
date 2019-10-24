-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.

require("nixio.fs")

m = Map("verysync", translate("An Efficient Data Transfer Tool"),
	translate("Simple and easy-to-use multi-platform file synchronization software, astonishing transmission speed is different from the greatest advantage of other products, micro-force synchronization of intelligent P2P technology to accelerate synchronization, will split the file into several KB-only data synchronization, and the file will be AES encryption processing."))

m:section(SimpleSection).template  = "verysync/verysync_status"

s = m:section(TypedSection, "setting", translate("Settings"))
s.anonymous = true

s:option(Flag, "enabled", translate("Enable"))

s:option(Value, "port", translate("port")).default = 8886
s.rmempty = true


return m
