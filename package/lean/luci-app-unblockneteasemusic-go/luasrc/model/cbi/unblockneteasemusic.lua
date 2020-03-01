local fs = require "luci.fs"
local http = luci.http

mp = Map("unblockneteasemusic", translate("解除网易云音乐播放限制 (Golang)"))
mp.description = translate("原理：采用 [酷我/酷狗/咕咪] 音源(后续有空补充)，替换网易云音乐 灰色 歌曲链接<br/>具体使用方法参见：https://github.com/cnsilvan/luci-app-unblockneteasemusic<br/>首次使用会自动生成证书，所以较慢")
mp:section(SimpleSection).template = "unblockneteasemusic/unblockneteasemusic_status"

s = mp:section(TypedSection, "unblockneteasemusic")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("启用本插件"))
enable.description = translate("启用本插件以解除网易云音乐播放限制")
enable.default = 0
enable.rmempty = false

http_port = s:option(Value, "http_port", translate("[HTTP] 监听端口"))
http_port.description = translate("本插件监听的HTTP端口，不可与其他程序/HTTPS共用一个端口")
http_port.placeholder = "5210"
http_port.default = "5210"
http_port.datatype = "port"
http_port.rmempty = false

https_port = s:option(Value, "https_port", translate("[HTTPS] 监听端口"))
https_port.description = translate("本插件监听的HTTPS端口，不可与其他程序/HTTP共用一个端口")
https_port.placeholder = "5211"
https_port.default = "5211"
https_port.datatype = "port"
https_port.rmempty = false

music_source = s:option(ListValue, "music_source", translate("音源选择"))
music_source:value("default", translate("默认"))
music_source:value("customize", translate("自定义"))
music_source.description = translate("默认为kuwo:kugou")
music_source.default = "default"
music_source.rmempty = false

music_customize_source = s:option(Value, "music_customize_source", translate("自定义音源"))
music_customize_source.description = translate("自定义音源设置，如kuwo:kugou:migu ,以:隔开,migu在某些运营商下无法使用可能会导致卡顿")
music_customize_source.default = "kuwo:kugou"
music_customize_source.rmempty = false
music_customize_source:depends("music_source", "customize")

hijack = s:option(ListValue, "hijack_ways", translate("劫持方法"))
hijack:value("dont_hijack", translate("不开启劫持"))
hijack:value("use_ipset", translate("使用IPSet劫持"))
hijack:value("use_hosts", translate("使用Hosts劫持"))
hijack.description = translate("如果使用Hosts劫持，请将HTTP/HTTPS端口设置为80/443，路由器不建议使用Hosts劫持")
hijack.default = "dont_hijack"
hijack.rmempty = false

daemon_enable = s:option(Flag, "daemon_enable", translate("启用进程守护"))
daemon_enable.description = translate("开启后，附属程序会自动检测主程序运行状态，在主程序退出时自动重启")
daemon_enable.default = 0
daemon_enable.rmempty = false

download = s:option(FileUpload,"", translate("下载根证书"))
download.description = translate("请在客户端信任该证书。该证书由你设备自动生成，安全可靠")
download.rmempty = false
download.template = "unblockneteasemusic/unblockneteasemusic_download"
function Download()
	local sPath, sFile, fd, block
	sPath = "/usr/share/UnblockNeteaseMusic/ca.crt"
	sFile = nixio.fs.basename(sPath)
	if luci.fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
    if not fd then
		download.description = string.format('请在客户端信任该证书。该证书由你设备自动生成，安全可靠<br/><span style="color: red">%s</span>', translate("Couldn't open file: ") .. sPath)
		return
    end
    download.description = translate("请在客户端信任该证书。该证书由你设备自动生成，安全可靠")
	http.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	http.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			http.write(block)
		end
	end
	fd:close()
	http.close()
end
if luci.http.formvalue("download") then
	Download()
end
return mp
