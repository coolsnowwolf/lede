
mp = Map("unblockmusic", translate("解锁网易云灰色歌曲"))
mp.description = translate("采用 [QQ/虾米/百度/酷狗/酷我/咕咪/JOOX]等音源，替换网易云变灰歌曲链接")

mp:section(SimpleSection).template  = "unblockmusic/unblockmusic_status"

s = mp:section(TypedSection, "unblockmusic")
s.anonymous=true
s.addremove=false

enabled = s:option(Flag, "enabled", translate("启用解锁"))
enabled.default = 0
enabled.rmempty = false
enabled.description = translate("启用后，路由器自动分流解锁，大部分设备无需设置代理。<br />苹果系列设备需要设置 WIFI/有线代理方式为 自动 ,并安装 CA根证书并信任。")

speedtype = s:option(ListValue, "musicapptype", translate("音源选择"))
speedtype:value("default", translate("默认"))
speedtype:value("netease", translate("网易云音乐"))
speedtype:value("qq", translate("QQ音乐"))
speedtype:value("xiami", translate("虾米音乐"))
speedtype:value("baidu", translate("百度音乐"))
speedtype:value("kugou", translate("酷狗音乐"))
speedtype:value("kuwo", translate("酷我音乐(高音质推荐)"))
speedtype:value("migu", translate("咕咪音乐"))
speedtype:value("joox", translate("JOOX音乐"))

endpoint = s:option(Value, "endpoint", translate("转发HTTPS音源地址"))
endpoint.default = "https://music.163.com"
endpoint.rmempty = true
endpoint.description = translate("默认为 https://music.163.com")

o = s:option(Button,"certificate",translate("HTTPS 证书"))
o.inputtitle = translate("下载 CA 根证书")
o.description = translate("iOS 13 系统需要在“设置 -> 通用 -> 关于本机 -> 证书信任设置” 中，信任 UnblockNeteaseMusic Root CA )")
o.inputstyle = "reload"
o.write = function()
  	Download()
end

function Download()
	local t,e
	t=nixio.open("/usr/share/UnblockNeteaseMusic/ca.crt","r")
	luci.http.header('Content-Disposition','attachment; filename="ca.crt"')
	luci.http.prepare_content("application/octet-stream")
	while true do
		e=t:read(nixio.const.buffersize)
		if(not e)or(#e==0)then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end

return mp
