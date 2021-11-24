m = Map("qbittorrent")
m.title = translate("qBittorrent")
m.description = translate("qBittorrent is a cross-platform free and open-source BitTorrent client")

m:section(SimpleSection).template = "qbittorrent/qbittorrent_status"

s = m:section(NamedSection, "main", "qbittorrent")

o = s:option(Flag, "enabled", translate("Enabled"))
o.default = "1"

o = s:option(ListValue, "user", translate("Run daemon as user"))
local u
for u in luci.util.execi("cat /etc/passwd | cut -d ':' -f1") do
	o:value(u)
end

o = s:option(Value, "profile", translate("Store configuration files in the Path"))
o.default = '/tmp'

o = s:option(Value, "SavePath", translate("Store download files in the Path"))
o.placeholder = "/tmp/download"

o = s:option(Value, "Port", translate("WEBUI listening port"))
o.datatype = "port"
o.placeholder = "8080"

o = s:option(Flag, "UseRandomPort", translate("Use Random Port"))
o.description = translate("Randomly assigns a different port every time qBittorrent starts up.")
o.enabled = "true"
o.disabled = "false"
o.default = o.enabled

o = s:option(Value, "PortRangeMin", translate("Connection Port"))
o.description = translate("Incoming connection port.")
o:depends("UseRandomPort", false)
o.datatype = "range(1024,65535)"

return m
