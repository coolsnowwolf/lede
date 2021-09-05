-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.

require("nixio.fs")

m = Map("syncthing", translate("SyncThing Synchronization Tool"))

m:section(SimpleSection).template  = "syncthing/syncthing_status"

s = m:section(TypedSection, "setting", translate("Settings"))
s.anonymous = true

s:option(Flag, "enabled", translate("Enable"))

s:option(Value, "port", translate("port")).default = 8384
s.rmempty = true


return m
