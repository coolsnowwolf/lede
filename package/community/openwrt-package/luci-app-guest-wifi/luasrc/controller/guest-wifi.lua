module("luci.controller.guest-wifi", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/guest-wifi") then
		return
	end
	entry({"admin","network","guest-wifi"}, cbi("guest-wifi"), translate("Guest-wifi"), 16)
end
