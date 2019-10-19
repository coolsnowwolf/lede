module("luci.controller.baidupcs-web", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/baidupcs-web") then
		return
	end
	local page
	page = entry({"admin", "nas", "baidupcs-web"}, cbi("baidupcs-web"), _("BaiduPCS Web"), 300)
	page.dependent = true
end
