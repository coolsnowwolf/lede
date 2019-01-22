module("luci.controller.ssrr", package.seeall)
function index()
		if not nixio.fs.access("/etc/config/ssrr") then
		return
	end
	entry({"admin", "services", "ssrr"},alias("admin", "services", "ssrr","general"),_("ShadowsocksR")).dependent = true
	entry({"admin", "services", "ssrr","general"}, cbi("shadowsocksr/general"),_("General"),10).leaf = true
	entry({"admin", "services", "ssrr","gfwlist"}, cbi("shadowsocksr/gfwlist"),_("GFWlist"),20).leaf = true
end

