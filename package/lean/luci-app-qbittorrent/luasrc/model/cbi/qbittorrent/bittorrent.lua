m = Map("qbittorrent")

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "DHT", translate("Enable DHT"))
o.description = translate("Enable DHT (decentralized network) to find more peers.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "PeX", translate("Enable PeX"))
o.description = translate("Enable Peer Exchange (PeX) to find more peers.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "LSD", translate("Enable LSD"))
o.description = translate("Enable Local Peer Discovery to find more peers.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "uTP_rate_limited", translate("uTP Rate Limit"))
o.description = translate("Apply rate limit to uTP protocol.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(ListValue, "Encryption", translate("Encryption Mode"))
o.description = translate("Enable DHT (decentralized network) to find more peers.")
o:value("0", translate("Prefer Encryption"))
o:value("1", translate("Require Encryption"))
o:value("2", translate("Disable Encryption"))
o.default = "0"

o = s:option(Value, "MaxConnecs", translate("Max Connections"))
o.description = translate("The max number of connections.")
o.datatype = "integer"
o.placeholder = "500"

o = s:option(Value, "MaxConnecsPerTorrent", translate("Max Connections Per Torrent"))
o.description = translate("The max number of connections per torrent.")
o.datatype = "integer"
o.placeholder = "100"

o = s:option(Value, "MaxUploads", translate("Max Uploads"))
o.description = translate("The max number of connected peers.")
o.datatype = "integer"
o.placeholder = "8"

o = s:option(Value, "MaxUploadsPerTorrent", translate("Max Uploads Per Torrent"))
o.description = translate("The max number of connected peers per torrent.")
o.datatype = "integer"
o.placeholder = "4"

o = s:option(Value, "MaxRatio", translate("Max Ratio"))
o.description = translate("The max ratio for seeding. -1 is to disable the seeding.")
o.datatype = "float"
o.placeholder = "-1"

o = s:option(ListValue, "MaxRatioAction", translate("Max Ratio Action"))
o.description = translate("The action when reach the max seeding ratio.")
o:value("0", translate("Pause them"))
o:value("1", translate("Remove them"))
o.defaule = "0"

o = s:option(Value, "GlobalMaxSeedingMinutes", translate("Max Seeding Minutes"))
o.description = translate("Units: minutes")
o.datatype = "integer"

s = m:section(NamedSection, "main", "qbittorrent")
s.title = translate("Queueing Setting")

o = s:option(Flag, "QueueingEnabled", translate("Enable Torrent Queueing"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "MaxActiveDownloads", translate("Maximum Active Downloads"))
o.datatype = "integer"
o.placeholder = "3"

o = s:option(Value, "MaxActiveUploads", translate("Max Active Uploads"))
o.datatype = "integer"
o.placeholder = "3"

o = s:option(Value, "MaxActiveTorrents", translate("Max Active Torrents"))
o.datatype = "integer"
o.placeholder = "5"

o = s:option(Flag, "IgnoreSlowTorrents", translate("Ignore Slow Torrents"))
o.description = translate("Do not count slow torrents in these limits.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Value, "SlowTorrentsDownloadRate", translate("Download rate threshold"))
o.description = translate("Units: KiB/s")
o.datatype = "integer"
o.placeholder = "2"

o = s:option(Value, "SlowTorrentsUploadRate", translate("Upload rate threshold"))
o.description = translate("Units: KiB/s")
o.datatype = "integer"
o.placeholder = "2"

o = s:option(Value, "SlowTorrentsInactivityTimer", translate("Torrent inactivity timer"))
o.description = translate("Units: seconds")
o.datatype = "integer"
o.placeholder = "60"

return m
