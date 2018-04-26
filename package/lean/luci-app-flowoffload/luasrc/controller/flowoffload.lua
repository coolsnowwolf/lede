module("luci.controller.flowoffload", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/flowoffload") then
		return
	end
	local page
	page = entry({"admin", "network", "flowoffload"}, cbi("flowoffload"), _(" Flow Offload ACC"), 101)
	page.i18n = "flowoffload"
	page.dependent = true
end
