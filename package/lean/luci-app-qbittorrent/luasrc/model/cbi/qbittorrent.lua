
local o=luci.sys.exec("uci get qbittorrent.main.Port | xargs echo -n") or 8080

local a=(luci.sys.call("pidof qbittorrent-nox > /dev/null")==0)

local t=""
if a then
t="<br /><br /><input class=\"cbi-button cbi-button-apply\" type=\"button\" value=\" "..translate("Open Web Interface").." \" onclick=\"window.open('http://'+window.location.hostname+':"..o.."')\"/>"
end

function titlesplit(Value)
    return "<p style=\"font-size:20px;font-weight:bold;color: DodgerBlue\">" .. translate(Value) .. "</p>"
end

m = Map("qbittorrent", translate("qBittorrent"), translate("qBittorrent is a cross-platform free and open-source BitTorrent client")..t)

s = m:section(NamedSection, "main", "qbittorrent")

s:tab("basic", translate("Basic Settings"))

o = s:taboption("basic", Flag, "enabled", translate("Enabled"))
o.default = "1"

o = s:taboption("basic", ListValue, "user", translate("Run daemon as user"))
local u
for u in luci.util.execi("cat /etc/passwd | cut -d ':' -f1") do
	o:value(u)
end

o = s:taboption("basic", Value, "profile", translate("Store configuration files in the Path"))
o.default = '/tmp'

o = s:taboption("basic", Value, "SavePath", translate("Store download files in the Path"))
o.placeholder = "/tmp/download"

o = s:taboption("basic", Value, "Port", translate("WEBUI listening port"))
o.datatype = "port"
o.placeholder = "8080"

o = s:taboption("basic", Flag, "UseRandomPort", translate("Use Random Port"), translate("Randomly assigns a different port every time qBittorrent starts up"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("basic", Value, "PortRangeMin", translate("Connection Port"), translate("Incoming connection port"))
o:depends("UseRandomPort", false)
o.datatype = "range(1024,65535)"


s:tab("connection", translate("Connection Settings"))

o = s:taboption("connection", Flag, "UPnP", translate("Use UPnP for Connections"), translate("Use UPnP/ NAT-PMP port forwarding from my router. Refer to the "
			.. "<a href='https://en.wikipedia.org/wiki/Port_forwarding' target='_blank'>wiki</a>."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled


o = s:taboption("connection", Value, "GlobalDLLimit", translate("Global Download Speed"), translate("Global Download Speed Limit(KiB/s)."))
o.datatype = "float"
o.placeholder = "0"

o = s:taboption("connection", Value, "GlobalUPLimit", translate("Global Upload Speed"), translate("Global Upload Speed Limit(KiB/s)."))
o.datatype = "float"
o.placeholder = "0"

o = s:taboption("connection", Value, "GlobalDLLimitAlt", translate("Alternative Download Speed"), translate("Alternative Download Speed Limit(KiB/s)."))
o.datatype = "float"
o.placeholder = "10"

o = s:taboption("connection", Value, "GlobalUPLimitAlt", translate("Alternative Upload Speed"), translate("Alternative Upload Speed Limit(KiB/s)."))
o.datatype = "float"
o.placeholder = "10"

o = s:taboption("connection", ListValue, "BTProtocol", translate("Enabled protocol"), translate("The protocol that was enabled."))
o:value("Both", translate("TCP and UTP"))
o:value("TCP", translate("TCP"))
o:value("UTP", translate("UTP"))
o.default = "Both"

o = s:taboption("connection", Value, "InetAddress", translate("Inet Address"), translate("The address that respond to the trackers."))

s:tab("downloads", translate("Download Settings"))

o = s:taboption("downloads", Flag, "CreateTorrentSubfolder", translate("Create Subfolder"), translate("Create subfolder for torrents with multiple files."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("downloads", Flag, "StartInPause", translate("Start In Pause"), translate("Do not start the download automatically."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("downloads", Flag, "AutoDeleteAddedTorrentFile", translate("Auto Delete Torrent File"), translate("The .torrent files will be deleted afterwards."))
o.enabled = "IfAdded"
o.disabled = "Never"
o.default = o.disabled

o = s:taboption("downloads", Flag, "PreAllocation", translate("Pre Allocation"), translate("Pre-allocate disk space for all files."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("downloads", Flag, "UseIncompleteExtension", translate("Use Incomplete Extension"), translate("The incomplete task will be added the extension of !qB."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("downloads", Flag, "TempPathEnabled", translate("Temp Path Enabled"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("downloads", Value, "TempPath", translate("Temp Path"), translate("The absolute and relative path can be set."))
o:depends("TempPathEnabled", "true")
o.placeholder = "temp/"

o = s:taboption("downloads", Value, "DiskWriteCacheSize", translate("Disk Cache Size (MiB)"), translate("The value -1  is auto and 0 is disable. In default, it is set to 64MiB."))
o.datatype = "integer"
o.placeholder = "64"

o = s:taboption("downloads", Value, "DiskWriteCacheTTL", translate("Disk Cache TTL (s)"), translate("In default, it is set to 60s."))
o.datatype = "integer"
o.placeholder = "60"

o = s:taboption("downloads", DummyValue, "Saving Management", titlesplit("Saving Management"))

o = s:taboption("downloads", ListValue, "DisableAutoTMMByDefault", translate("Default Torrent Management Mode"))
o:value("true", translate("Manual"))
o:value("false", translate("Automaic"))
o.default = "true"

o = s:taboption("downloads", ListValue, "CategoryChanged", translate("Torrent Category Changed"), translate("Choose the action when torrent category changed."))
o:value("true", translate("Switch torrent to Manual Mode"))
o:value("false", translate("Relocate torrent"))
o.default = "false"

o = s:taboption("downloads", ListValue, "DefaultSavePathChanged", translate("Default Save Path Changed"), translate("Choose the action when default save path changed."))
o:value("true", translate("Switch affected torrent to Manual Mode"))
o:value("false", translate("Relocate affected torrent"))
o.default = "true"

o = s:taboption("downloads", ListValue, "CategorySavePathChanged", translate("Category Save Path Changed"), translate("Choose the action when category save path changed."))
o:value("true", translate("Switch affected torrent to Manual Mode"))
o:value("false", translate("Relocate affected torrent"))
o.default = "true"

o = s:taboption("downloads", Value, "TorrentExportDir", translate("Torrent Export Dir"), translate("The .torrent files will be copied to the target directory."))

o = s:taboption("downloads", Value, "FinishedTorrentExportDir", translate("Finished Torrent Export Dir"), translate("The .torrent files for finished downloads will be copied to the target directory."))

s:tab("bittorrent", translate("Bittorrent Settings"))

o = s:taboption("bittorrent", Flag, "DHT", translate("Enable DHT"), translate("Enable DHT (decentralized network) to find more peers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("bittorrent", Flag, "PeX", translate("Enable PeX"), translate("Enable Peer Exchange (PeX) to find more peers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("bittorrent", Flag, "LSD", translate("Enable LSD"), translate("Enable Local Peer Discovery to find more peers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("bittorrent", Flag, "uTP_rate_limited", translate("uTP Rate Limit"), translate("Apply rate limit to μTP protocol."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("bittorrent", ListValue, "Encryption", translate("Encryption Mode"), translate("Enable DHT (decentralized network) to find more peers"))
o:value("0", translate("Prefer Encryption"))
o:value("1", translate("Require Encryption"))
o:value("2", translate("Disable Encryption"))
o.default = "0"

o = s:taboption("bittorrent", Value, "MaxConnecs", translate("Max Connections"), translate("The max number of connections."))
o.datatype = "integer"
o.placeholder = "500"

o = s:taboption("bittorrent", Value, "MaxConnecsPerTorrent", translate("Max Connections Per Torrent"), translate("The max number of connections per torrent."))
o.datatype = "integer"
o.placeholder = "100"

o = s:taboption("bittorrent", Value, "MaxUploads", translate("Max Uploads"), translate("The max number of connected peers."))
o.datatype = "integer"
o.placeholder = "8"

o = s:taboption("bittorrent", Value, "MaxUploadsPerTorrent", translate("Max Uploads Per Torrent"), translate("The max number of connected peers per torrent."))
o.datatype = "integer"
o.placeholder = "4"

o = s:taboption("bittorrent", Value, "MaxRatio", translate("Max Ratio"), translate("The max ratio for seeding. -1 is to disable the seeding."))
o.datatype = "float"
o.placeholder = "-1"

o = s:taboption("bittorrent", ListValue, "MaxRatioAction", translate("Max Ratio Action"), translate("The action when reach the max seeding ratio."))
o:value("0", translate("Pause them"))
o:value("1", translate("Remove them"))
o.defaule = "0"

o = s:taboption("bittorrent", Value, "GlobalMaxSeedingMinutes", translate("Max Seeding Minutes"), translate("Units: minutes"))
o.datatype = "integer"

o = s:taboption("bittorrent", DummyValue, "Queueing Setting", titlesplit("Queueing Setting"))

o = s:taboption("bittorrent", Flag, "QueueingEnabled", translate("Enable Torrent Queueing"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("bittorrent", Value, "MaxActiveDownloads", translate("Maximum Active Downloads"))
o.datatype = "integer"
o.placeholder = "3"

o = s:taboption("bittorrent", Value, "MaxActiveUploads", translate("Max Active Uploads"))
o.datatype = "integer"
o.placeholder = "3"

o = s:taboption("bittorrent", Value, "MaxActiveTorrents", translate("Max Active Torrents"))
o.datatype = "integer"
o.placeholder = "5"

o = s:taboption("bittorrent", Flag, "IgnoreSlowTorrents", translate("Ignore Slow Torrents"), translate("Do not count slow torrents in these limits."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("bittorrent", Value, "SlowTorrentsDownloadRate", translate("Download rate threshold"), translate("Units: KiB/s"))
o.datatype = "integer"
o.placeholder = "2"

o = s:taboption("bittorrent", Value, "SlowTorrentsUploadRate", translate("Upload rate threshold"), translate("Units: KiB/s"))
o.datatype = "integer"
o.placeholder = "2"

o = s:taboption("bittorrent", Value, "SlowTorrentsInactivityTimer", translate("Torrent inactivity timer"), translate("Units: seconds"))
o.datatype = "integer"
o.placeholder = "60"

s:tab("webgui", translate("WebUI Settings"))

o = s:taboption("webgui", Flag, "UseUPnP", translate("Use UPnP for WebUI"), translate("Using the UPnP / NAT-PMP port of the router for connecting to WebUI."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("webgui", Value, "Username", translate("Username"), translate("The login name for WebUI."))
o.placeholder = "admin"

o = s:taboption("webgui", Value, "Password", translate("Password"), translate("The login password for WebUI."))
o.password  =  true

o = s:taboption("webgui", Value, "Locale", translate("Locale Language"))
o:value("en", translate("English"))
o:value("zh", translate("Chinese"))
o.default = "en"

o = s:taboption("webgui", Flag, "CSRFProtection", translate("CSRF Protection"), translate("Enable Cross-Site Request Forgery (CSRF) protection."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("webgui", Flag, "ClickjackingProtection", translate("Clickjacking Protection"), translate("Enable clickjacking protection."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("webgui", Flag, "HostHeaderValidation", translate("Host Header Validation"), translate("Validate the host header."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("webgui", Flag, "LocalHostAuth", translate("Local Host Authentication"), translate("Force authentication for clients on localhost."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("webgui", Flag, "AuthSubnetWhitelistEnabled", translate("Enable Subnet Whitelist"))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("webgui", DynamicList, "AuthSubnetWhitelist", translate("Subnet Whitelist"))
o:depends("AuthSubnetWhitelistEnabled", "true")

s:tab("advanced", translate("Advance Settings"))

o = s:taboption("advanced", Flag, "AnonymousMode", translate("Anonymous Mode"), translate("When enabled, qBittorrent will take certain measures to try"
				.. " to mask its identity. Refer to the <a href='https://github.com/qbittorrent/qBittorrent/wiki/Anonymous-Mode'  target='_blank'>wiki</a>"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Flag, "SuperSeeding", translate("Super Seeding"), translate("The super seeding mode."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("advanced", Value, "configuration", translate("Profile Folder Suffix"), translate("Suffix for profile folder"))

o = s:taboption("advanced", Flag, "IncludeOverhead", translate("Limit Overhead Usage"), translate("The overhead usage is been limitted."))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("advanced", Flag, "IgnoreLimitsLAN", translate("Ignore LAN Limit"), translate("Ignore the speed limit to LAN."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Flag, "osCache", translate("Use os Cache"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Value, "OutgoingPortsMax", translate("Max Outgoing Port"), translate("The max outgoing port."))
o.datatype = "port"

o = s:taboption("advanced", Value, "OutgoingPortsMin", translate("Min Outgoing Port"), translate("The min outgoing port."))
o.datatype = "port"

o = s:taboption("advanced", ListValue, "SeedChokingAlgorithm", translate("Choking Algorithm"), translate("The strategy of choking algorithm."))
o:value("RoundRobin", translate("Round Robin"))
o:value("FastestUpload", translate("Fastest Upload"))
o:value("AntiLeech", translate("Anti-Leech"))
o.default = "FastestUpload"

o = s:taboption("advanced", Flag, "AnnounceToAllTrackers", translate("Announce To All Trackers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.disabled

o = s:taboption("advanced", Flag, "AnnounceToAllTiers", translate("Announce To All Tiers"))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Flag, "Enabled", translate("Enable Log"), translate("Enable logger to log file."))
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Value, "Path", translate("Log Path"), translate("The path for qbittorrent log."))
o:depends("Enabled", "true")

o = s:taboption("advanced", Flag, "Backup", translate("Enable Backup"), translate("Backup log file when oversize the given size."))
o:depends("Enabled", "true")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Flag, "DeleteOld", translate("Delete Old Backup"), translate("Delete the old log file."))
o:depends("Enabled", "true")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:taboption("advanced", Value, "MaxSizeBytes", translate("Log Max Size"), translate("The max size for qbittorrent log (Unit: Bytes)."))
o:depends("Enabled", "true")
o.placeholder = "66560"

o = s:taboption("advanced", Value, "SaveTime", translate("Log Saving Period"), translate("The log file will be deteted after given time. 1d -- 1 day, 1m -- 1 month, 1y -- 1 year"))
o:depends("Enabled", "true")
o.datatype = "string"

return m
