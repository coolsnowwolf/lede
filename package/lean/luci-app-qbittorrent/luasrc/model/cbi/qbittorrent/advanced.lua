m = Map("qbittorrent")

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "AnonymousMode", translate("Anonymous Mode"))
o.description = translate("When enabled, qBittorrent will take certain measures to try to mask its identity. Refer to the <a href=\"https://github.com/qbittorrent/qBittorrent/wiki/Anonymous-Mode\" target=\"_blank\">wiki</a>")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "SuperSeeding", translate("Super Seeding"))
o.description = translate("The super seeding mode.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Value, "configuration", translate("Profile Folder Suffix"))
o.description = translate("Suffix for profile folder, for example, <b>qBittorrent_[NAME]</b>.")

o = s:option(Flag, "IncludeOverhead", translate("Limit Overhead Usage"))
o.description = translate("The overhead usage is been limitted.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "IgnoreLimitsLAN", translate("Ignore LAN Limit"))
o.description = translate("Ignore the speed limit to LAN.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "osCache", translate("Use os Cache"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "OutgoingPortsMax", translate("Max Outgoing Port"))
o.description = translate("The max outgoing port.")
o.datatype = "port"

o = s:option(Value, "OutgoingPortsMin", translate("Min Outgoing Port"))
o.description = translate("The min outgoing port.")
o.datatype = "port"

o = s:option(ListValue, "SeedChokingAlgorithm", translate("Choking Algorithm"))
o.description = translate("The strategy of choking algorithm.")
o:value("RoundRobin", translate("Round Robin"))
o:value("FastestUpload", translate("Fastest Upload"))
o:value("AntiLeech", translate("Anti-Leech"))
o.default = "FastestUpload"

o = s:option(Flag, "AnnounceToAllTrackers", translate("Announce To All Trackers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "AnnounceToAllTiers", translate("Announce To All Tiers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "Enabled", translate("Enable Log"))
o.description = translate("Enable logger to log file.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "Path", translate("Log Path"))
o.description = translate("The path for qbittorrent log.")
o:depends("Enabled", "true")

o = s:option(Flag, "Backup", translate("Enable Backup"))
o.description = translate("Backup log file when oversize the given size.")
o:depends("Enabled", "true")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "DeleteOld", translate("Delete Old Backup"))
o.description = translate("Delete the old log file.")
o:depends("Enabled", "true")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "MaxSizeBytes", translate("Log Max Size"))
o.description = translate("The max size for qbittorrent log (Unit: Bytes).")
o:depends("Enabled", "true")
o.placeholder = "66560"

o = s:option(Value, "SaveTime", translate("Log Saving Period"))
o.description = translate("The log file will be deteted after given time. 1d -- 1 day, 1m -- 1 month, 1y -- 1 year.")
o:depends("Enabled", "true")
o.datatype = "string"

return m
