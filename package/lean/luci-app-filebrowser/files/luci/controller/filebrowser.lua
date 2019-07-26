module("luci.controller.filebrowser", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/filebrowser") then
		return
	end
	local page
	page = entry({"admin", "services", "filebrowser"}, cbi("filebrowser"), _("文件管理器"), 100)
	page.dependent = true
end
