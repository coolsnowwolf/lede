
module("luci.controller.xunlei", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/xunlei") then
		return
	end

	local page
	page = entry({"admin", "services", "xunlei"}, cbi("xunlei"), _("迅雷远程下载"), 199)
	page.i18n = "xunlei"
	page.dependent = true
end
