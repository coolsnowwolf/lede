module("luci.controller.rclone", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/rclone") then return end
	entry({"admin", "nas"}, firstchild(), _("NAS") , 45).dependent = false
	entry({"admin", "nas", "rclone"}, cbi("rclone"), _("Rclone"), 100 ).dependent = false
end
