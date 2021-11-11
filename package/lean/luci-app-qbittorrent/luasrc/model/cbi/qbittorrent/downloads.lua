m = Map("qbittorrent")

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "CreateTorrentSubfolder", translate("Create Subfolder"))
o.description = translate("Create subfolder for torrents with multiple files.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Flag, "StartInPause", translate("Start In Pause"))
o.description = translate("Do not start the download automatically.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "AutoDeleteAddedTorrentFile", translate("Auto Delete Torrent File"))
o.description = translate("The .torrent files will be deleted afterwards.")
o.enabled = "IfAdded"
o.disabled = "Never"
o.default = o.disabled

o = s:option(Flag, "PreAllocation", translate("Pre Allocation"))
o.description = translate("Pre-allocate disk space for all files.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "UseIncompleteExtension", translate("Use Incomplete Extension"))
o.description = translate("The incomplete task will be added the extension of !qB.")
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:option(Flag, "TempPathEnabled", translate("Temp Path Enabled"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "TempPath", translate("Temp Path"))
o.description = translate("The absolute and relative path can be set.")
o:depends("TempPathEnabled", "true")
o.placeholder = "temp/"

o = s:option(Value, "DiskWriteCacheSize", translate("Disk Cache Size (MiB)"))
o.description = translate("The value -1  is auto and 0 is disable. In default, it is set to 64MiB.")
o.datatype = "integer"
o.placeholder = "64"

o = s:option(Value, "DiskWriteCacheTTL", translate("Disk Cache TTL (s)"))
o.description = translate("In default, it is set to 60s.")
o.datatype = "integer"
o.placeholder = "60"

s = m:section(NamedSection, "main", "qbittorrent")
s.title = translate("Saving Management")

o = s:option(ListValue, "DisableAutoTMMByDefault", translate("Default Torrent Management Mode"))
o:value("true", translate("Manual"))
o:value("false", translate("Automaic"))
o.default = "true"

o = s:option(ListValue, "CategoryChanged", translate("Torrent Category Changed"))
o.description = translate("Choose the action when torrent category changed.")
o:value("true", translate("Switch torrent to Manual Mode"))
o:value("false", translate("Relocate torrent"))
o.default = "false"

o = s:option(ListValue, "DefaultSavePathChanged", translate("Default Save Path Changed"))
o.description = translate("Choose the action when default save path changed.")
o:value("true", translate("Switch affected torrent to Manual Mode"))
o:value("false", translate("Relocate affected torrent"))
o.default = "true"

o = s:option(ListValue, "CategorySavePathChanged", translate("Category Save Path Changed"))
o.description = translate("Choose the action when category save path changed.")
o:value("true", translate("Switch affected torrent to Manual Mode"))
o:value("false", translate("Relocate affected torrent"))
o.default = "true"

o = s:option(Value, "TorrentExportDir", translate("Torrent Export Dir"))
o.description = translate("The .torrent files will be copied to the target directory.")

o = s:option(Value, "FinishedTorrentExportDir", translate("Finished Torrent Export Dir"))
o.description = translate("The .torrent files for finished downloads will be copied to the target directory.")

return m
