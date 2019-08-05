
mp = Map("unblockmusic", translate("解锁网易云灰色歌曲"))
mp.description = translate("原理：采用 [QQ/虾米/百度/酷狗/酷我/咕咪/JOOX]等音源 替换网易云变灰歌曲链接<br />具体使用方法可查看github：<br />https://github.com/maxlicheng/luci-app-unblockmusic")

mp:section(SimpleSection).template  = "unblockmusic/unblockmusic_status"

s = mp:section(TypedSection, "unblockmusic")
s.anonymous=true
s.addremove=false

enabled = s:option(Flag, "enabled", translate("启用解锁"))
enabled.default = 0
enabled.rmempty = false
enabled.description = translate("启用后，路由器自动分流解锁，大部分设备无需设置代理。<br />苹果系列设备需要设置 WIFI/有线代理方式为自动即可")

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

account = s:option(Value, "port", translate("端口号"))
account.datatype = "string"


return mp
