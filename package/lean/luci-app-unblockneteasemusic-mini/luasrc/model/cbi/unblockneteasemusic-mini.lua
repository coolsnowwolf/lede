-- Created By [CTCGFW]Project-OpenWrt
-- https://github.com/project-openwrt

mp = Map("unblockneteasemusic-mini", translate("解除网易云音乐播放限制 (Mini)"))
mp.description = translate("原理：采用 [QQ/酷狗/酷我/咕咪] 等音源，替换网易云音乐 无版权/收费 歌曲链接<br/>由 [CTCGFW]Project-OpenWrt & hyird & Sunsky 提供服务器支持<br/>详细说明参见：https://github.com/project-openwrt/luci-app-unblockneteasemusic-mini")

mp:section(SimpleSection).template = "unblockneteasemusic-mini/unblockneteasemusic_mini_status"

s = mp:section(TypedSection, "unblockneteasemusic-mini")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("启用本插件"))
enable.description = translate("启用本插件以解除网易云音乐播放限制")
enable.default = 0
enable.rmempty = false

select_server = s:option(ListValue, "select_server", translate("服务器位置"))
select_server:value("tencent_shanghai_nodejs", translate("[CTCGFW] 腾讯云上海（高音质）"))
select_server:value("aliyun_beijing_nodejs", translate("[hyird] 阿里云北京（高音质）"))
select_server:value("aliyun_beijing_nodejs_2", translate("[Sunsky] 阿里云北京（高音质）"))
select_server:value("mobile_henan_nodejs",translate("[CTCGFW] 移动河南（无损音质）"))
select_server.description = translate("请合理使用本插件与各个服务器，请勿滥用")
select_server.default = "tencent_shanghai_nodejs"
select_server.rmempty = false

download_certificate=s:option(DummyValue,"opennewwindow",translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"下载CA根证书\" onclick=\"window.open('https://raw.githubusercontent.com/nondanee/UnblockNeteaseMusic/master/ca.crt')\" />"))
download_certificate.description = translate("Linux/iOS/MacOSX设备在信任根证书后方可正常使用解锁功能。")

return mp
