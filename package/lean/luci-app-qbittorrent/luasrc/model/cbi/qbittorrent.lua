local e=require"luci.model.uci".cursor()
local o=e:get_first("qbittorrent","Preferences","port") or 8080

local a=(luci.sys.call("pidof qbittorrent-nox > /dev/null")==0)

local t=""
if a then
t="<br /><br /><input class=\"cbi-button cbi-button-apply\" type=\"submit\" value=\" "..translate("Open Web Interface").." \" onclick=\"window.open('http://'+window.location.hostname+':"..o.."')\"/>"
end

m = Map("qbittorrent", translate("qBittorrent"), translate("qBittorrent is a cross-platform free and open-source BitTorrent client")..t)

m:section(SimpleSection).template="qbittorrent/qbittorrent_status"

s_basic = m:section(TypedSection, "basic", translate("Basic Settings"))
s_basic.anonymous = true

enable = s_basic:option(Flag, "enable", translate("Enable"))
profile_dir = s_basic:option(Value,"profile_dir",translate("profile_dir"),translate("Store configuration files in the Path"))
profile_dir.default = "/root"

s_download = m:section(TypedSection, "Preferences", translate("Download Settings"))
s_download.anonymous = true
download_dir = s_download:option(Value,"download_dir",translate("download_dir"),translate("Store download files in the Path"))
download_dir.default = "/root/download"

s_webui = m:section(TypedSection, "Preferences", translate("WEBUI Settings"))
s_webui.anonymous = true
port = s_webui:option(Value,"port",translate("port"),translate("WEBUI listening port"))
port.default = "8080"

return m