local fs = require "nixio.fs"

mp = Map("unblockmusic")
mp.title = translate("解锁网易云灰色歌曲")
mp.description = translate("采用 [QQ/百度/酷狗/酷我/咪咕/JOOX]等音源，替换网易云变灰歌曲链接")

mp:section(SimpleSection).template = "unblockmusic/unblockmusic_status"

s = mp:section(TypedSection, "unblockmusic")
s.anonymous = true
s.addremove = false

enabled = s:option(Flag, "enabled", translate("启用"))
enabled.description = translate("启用后，路由器自动分流解锁，大部分设备无需设置代理")
enabled.default = 0
enabled.rmempty = false

apptype = s:option(ListValue, "apptype", translate("解锁程序选择"))
if nixio.fs.access("/usr/bin/UnblockNeteaseMusic") then
apptype:value("go", translate("Golang 版本"))
end
if nixio.fs.access("/usr/share/UnblockNeteaseMusic/app.js") then
apptype:value("nodejs", translate("NodeJS 版本"))
end
apptype:value("cloud", translate("云解锁（ [CTCGFW] 云服务器）"))

speedtype = s:option(Value, "musicapptype", translate("音源选择"))
speedtype:value("default", translate("默认"))
speedtype:value("netease", translate("网易云音乐"))
speedtype:value("qq", translate("QQ音乐"))
speedtype:value("baidu", translate("百度音乐"))
speedtype:value("kugou", translate("酷狗音乐"))
speedtype:value("kuwo", translate("酷我音乐"))
speedtype:value("migu", translate("咪咕音乐"))
speedtype:value("joox", translate("JOOX音乐"))
speedtype.default = "kuwo"
speedtype:depends("apptype", "nodejs")
speedtype:depends("apptype", "go")

cloudserver = s:option(Value, "cloudserver", translate("服务器位置"))
cloudserver.description = translate("自定义服务器格式为 IP[域名]:HTTP端口:HTTPS端口<br />如果服务器为LAN内网IP，需要将这个服务器IP放入例外客户端 (不代理HTTP和HTTPS)")
cloudserver.default = ""
cloudserver.rmempty = true
cloudserver:depends("apptype", "cloud")

search_limit = s:option(Value, "search_limit", translate("搜索结果限制"))
search_limit.description = translate("在搜索页面显示其他平台搜索结果个数，可填（0-3）")
search_limit.default = "0"
search_limit:depends("apptype", "go")

flac = s:option(Flag, "flac_enabled", translate("启用无损音质"))
flac.description = translate("目前仅支持酷我、QQ、咪咕")
flac.default = "1"
flac.rmempty = false
flac:depends("apptype", "nodejs")
flac:depends("apptype", "go")

force = s:option(Flag, "force_enabled", translate("强制替换为高音质歌曲"))
force.description = translate("如果歌曲音质在 320Kbps 以内，则尝试强制替换为高音质版本")
force.default = "1"
force.rmempty = false
force:depends("apptype", "nodejs")

autoupdate = s:option(Flag, "autoupdate", translate("自动检查更新主程序"))
autoupdate.description = translate("每天自动检测并更新到最新版本")
autoupdate.default = "1"
autoupdate.rmempty = false
autoupdate:depends("apptype", "nodejs")

download_certificate = s:option(DummyValue, "opennewwindow", translate("HTTPS 证书"))
download_certificate.description = translate("<input type=\"button\" class=\"btn cbi-button cbi-button-apply\" value=\"下载CA根证书\" onclick=\"window.open('https://raw.githubusercontent.com/UnblockNeteaseMusic/server/enhanced/ca.crt')\" /><br />Mac/iOS客户端需要安装 CA根证书并信任<br />iOS系统需要在“设置 -> 通用 -> 关于本机 -> 证书信任设置”中，信任 UnblockNeteaseMusic Root CA <br />Linux 设备请在启用时加入 --ignore-certificate-errors 参数")

local ver = fs.readfile("/usr/share/UnblockNeteaseMusic/core_ver") or "0.00"

restart = s:option(Button, "restart", translate("手动更新"))
restart.inputtitle = translate("更新核心版本")
restart.description = string.format(translate("NodeJS 解锁主程序版本") ..  "<strong><font color=\"green\">: %s </font></strong>", ver)
restart.inputstyle = "reload"
restart.write = function()
	luci.sys.exec("/usr/share/UnblockNeteaseMusic/update_core.sh luci_update 2>&1")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "unblockmusic"))
end
restart:depends("apptype", "nodejs")

t = mp:section(TypedSection, "acl_rule")
t.title = translate("例外客户端规则")
t.description = translate("可以为局域网客户端分别设置不同的例外模式，默认无需设置")
t.template = "cbi/tblsection"
t.sortable = true
t.anonymous = true
t.addremove = true

ipaddr = t:option(Value, "ipaddr", translate("IP 地址"))
ipaddr.width = "40%"
ipaddr.datatype = "ip4addr"
ipaddr.placeholder = "0.0.0.0/0"
luci.ip.neighbors({ family = 4 }, function(entry)
	if entry.reachable then
		ipaddr:value(entry.dest:string())
	end
end)

filter_mode = t:option(ListValue, "filter_mode", translate("例外协议"))
filter_mode.width = "40%"
filter_mode.default = "disable"
filter_mode.rmempty = false
filter_mode:value("disable", translate("不代理HTTP和HTTPS"))
filter_mode:value("http", translate("不代理HTTP"))
filter_mode:value("https", translate("不代理HTTPS"))

return mp
