-- Copyright 2020 Lean <coolsnowwolf@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("forked-daapd", translate("Music Remote Center"), translate("Music Remote Center is a DAAP (iTunes Remote), MPD (Music Player Daemon) and RSP (Roku) media server."))

m:section(SimpleSection).template  = "forked-daapd/forked-daapd_status"

s = m:section(TypedSection, "forked-daapd")
s.addremove = false
s.anonymous = true

enable=s:option(Flag, "enabled", translate("Enabled"))
enable.default = "0"
enable.rmempty = false

port=s:option(Value, "port", translate("Port"))
port.rmempty = false
port.datatype = "port"

db_path=s:option(Value, "db_path", translate("Database File Path"))
db_path.default = "/opt/forked-daapd-songs3.db"
db_path.rmempty = false

directories=s:option(Value, "directories", translate("Music Directorie Path"))
directories.default = "/opt/music"
directories.rmempty = false

readme=s:option(DummyValue,"readme",translate("Readme"))
readme.description=translate("About iOS Remote Pairing: <br />1. Open the web interface <br /> 2. Start iPhone Remote APP, go to Settings, Add Library<br />3. Enter the pair code in the web interface")

return m