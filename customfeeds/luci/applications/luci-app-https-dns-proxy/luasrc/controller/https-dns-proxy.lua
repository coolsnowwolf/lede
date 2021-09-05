module("luci.controller.https-dns-proxy", package.seeall)
function index()
	if nixio.fs.access("/etc/config/https-dns-proxy") then
		entry({"admin", "services", "https-dns-proxy"}, cbi("https-dns-proxy"), _("DNS HTTPS Proxy")).acl_depends = { "luci-app-https-dns-proxy" }
		entry({"admin", "services", "https-dns-proxy", "action"}, call("https_dns_proxy_action"), nil).leaf = true
	end
end

function https_dns_proxy_action(name)
	local packageName = "https-dns-proxy"
	if name == "start" then
		luci.sys.init.start(packageName)
	elseif name == "action" then
		luci.util.exec("/etc/init.d/" .. packageName .. " reload >/dev/null 2>&1")
		luci.util.exec("/etc/init.d/dnsmasq restart >/dev/null 2>&1")
	elseif name == "stop" then
		luci.sys.init.stop(packageName)
	elseif name == "enable" then
		luci.sys.init.enable(packageName)
	elseif name == "disable" then
		luci.sys.init.disable(packageName)
	end
	luci.http.prepare_content("text/plain")
	luci.http.write("0")
end
