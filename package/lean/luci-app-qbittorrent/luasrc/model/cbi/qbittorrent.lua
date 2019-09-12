require("luci.sys")
local e=require"luci.model.uci".cursor()
local o=e:get_first("qbittorrent","Preferences","port")or 8080
local a=(luci.sys.call("pidof qbittorrent-nox > /dev/null")==0)
local t=""
local e=""
if a then
t="&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input class=\"cbi-button cbi-button-apply\" type=\"submit\" value=\" "..translate("Open Web Interface").." \" onclick=\"window.open('http://'+window.location.hostname+':"..o.."')\"/>"
end
if a then
e="<b><font color=\"green\">"..translate("Running").."</font></b>"
else
e="<b><font color=\"red\">"..translate("Not running").."</font></b>"
end

m = Map("qbittorrent", translate("qbittorrent"), translate("A BT/PT downloader base on Qt")..t
.."<br/><br/>"..translate("qbittorrent Run Status").." : "..e.."<br/>")

s_basic = m:section(TypedSection, "basic", translate("Basic Settings"))
s_basic.anonymous = true
enable = s_basic:option(Flag, "enable", translate("Enable"))
profile_dir = s_basic:option(Value,"profile_dir",translate("profile_dir"),translate("Store configuration files in the Path"))
profile_dir.default = "/tmp"
program_dir = s_basic:option(Value,"program_dir",translate("program_dir"),translate("Store Program files in the Path"))
program_dir.default = "/usr/bin"
library_dir = s_basic:option(Value,"library_dir",translate("library_dir"),translate("Store Library in the Path"))
library_dir.default = "/usr/lib"

s_download = m:section(TypedSection, "Preferences", translate("Download Settings"))
s_download.anonymous = true
download_dir = s_download:option(Value,"download_dir",translate("download_dir"),translate("Store download files in the Path"))
download_dir.default = "/tmp/download"

s_webui = m:section(TypedSection, "Preferences", translate("WEBUI Settings"))
s_webui.anonymous = true
port = s_webui:option(Value,"port",translate("port"),translate("WEBUI listening port"))
port.default = "8080"

local apply = luci.http.formvalue("cbi.apply")
if apply then
    io.popen("/etc/init.d/qbittorrent restart")
end

return m
