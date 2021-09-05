module("luci.controller.splash.splash", package.seeall)

local uci = luci.model.uci.cursor()
local util = require "luci.util"
local ipc = require "luci.ip"

function index()
	entry({"admin", "services", "splash"}, cbi("splash/splash"), _("Client-Splash"), 90)
	entry({"admin", "services", "splash", "splashtext" }, form("splash/splashtext"), _("Splashtext"), 10)

	local e

	e = node("splash")
	e.target = call("action_dispatch")

	node("splash", "activate").target = call("action_activate")
	node("splash", "splash").target   = template("splash_splash/splash")
	node("splash", "blocked").target  = template("splash/blocked")

	entry({"admin", "status", "splash"}, post("action_status_admin"), _("Client-Splash"))

	local page  = node("splash", "publicstatus")
	page.target = call("action_status_public")
	page.leaf   = true
end

function ip_to_mac(ip)
	local i, n
	for i, n in ipairs(ipc.neighbors({ dest = ip })) do
		local mac = ipc.checkmac(n.mac)
		if mac then return mac end
	end
end

function action_dispatch()
	local uci = luci.model.uci.cursor_state()
	local mac = ip_to_mac(luci.http.getenv("REMOTE_ADDR"))
	local access = false

	if mac then
		uci:foreach("luci_splash", "lease", function(s)
			if ipc.checkmac(s.mac) == mac then
				access = true
				return false
			end
		end)

		uci:foreach("luci_splash", "whitelist", function(s)
			if ipc.checkmac(s.mac) == mac then
				access = true
				return false
			end
		end)
	end

	if access then
		luci.http.redirect(luci.dispatcher.build_url())
	else
		luci.http.redirect(luci.dispatcher.build_url("splash", "splash"))
	end
end

function blacklist()
	leased_macs = { }
	uci:foreach("luci_splash", "blacklist", function(s)
		local m = ipc.checkmac(s.mac)
		if m then leased_macs[m] = true end
	end)
	return leased_macs
end

function action_activate()
	local ipc = require "luci.ip"
	local mac = ip_to_mac(luci.http.getenv("REMOTE_ADDR") or "127.0.0.1")
	local uci_state = require "luci.model.uci".cursor_state()
	local blacklisted = false
	if mac and luci.http.formvalue("accept") then
		uci:foreach("luci_splash", "blacklist", function(s)
			if ipc.checkmac(s.mac) == mac then
				blacklisted = true
				return false
			end
	        end)

		if blacklisted then
			luci.http.redirect(luci.dispatcher.build_url("splash" ,"blocked"))
		else
			local id = tostring(mac):gsub(':', ''):lower()
			local redirect_url = uci:get("luci_splash", "general", "redirect_url")
			if not redirect_url then
				redirect_url = uci_state:get("luci_splash_locations", id, "location")
			end
			if not redirect_url then
				redirect_url = luci.model.uci.cursor():get("freifunk", "community", "homepage") or 'http://www.freifunk.net'
			end
			remove_redirect(id)
			os.execute("luci-splash lease "..tostring(mac).." >/dev/null 2>&1")
			luci.http.redirect(redirect_url)
		end
	else
		luci.http.redirect(luci.dispatcher.build_url())
	end
end

function action_status_admin()
	local uci = luci.model.uci.cursor_state()
	local macs = luci.http.formvaluetable("save")

	local changes = {
		whitelist = { },
		blacklist = { },
		lease     = { },
		remove    = { }
	}

	local key, _
	for key, _ in pairs(macs) do
		local policy = luci.http.formvalue("policy.%s" % key)
		local mac    = luci.http.protocol.urldecode(key)

		if policy == "whitelist" or policy == "blacklist" then
			changes[policy][#changes[policy]+1] = mac
		elseif policy == "normal" then
			changes["lease"][#changes["lease"]+1] = mac
		elseif policy == "kicked" then
			changes["remove"][#changes["remove"]+1] = mac
		end
	end

	if #changes.whitelist > 0 then
		os.execute("luci-splash whitelist %s >/dev/null"
			% util.shellquote(table.concat(changes.whitelist)))
	end

	if #changes.blacklist > 0 then
		os.execute("luci-splash blacklist %s >/dev/null"
			% util.shellquote(table.concat(changes.blacklist)))
	end

	if #changes.lease > 0 then
		os.execute("luci-splash lease %s >/dev/null"
			% util.shellquote(table.concat(changes.lease)))
	end

	if #changes.remove > 0 then
		os.execute("luci-splash remove %s >/dev/null"
			% util.shellquote(table.concat(changes.remove)))
	end

	luci.template.render("admin_status/splash", { is_admin = true })
end

function action_status_public()
	luci.template.render("admin_status/splash", { is_admin = false })
end

function remove_redirect(id)
	local uci = require "luci.model.uci".cursor_state()
	local redirects = uci:get_all("luci_splash_locations")
	--uci:load("luci_splash_locations")
	uci:revert("luci_splash_locations")

	-- For all redirects
	local k, v
	for k, v in pairs(redirects) do
		if v[".type"] == "redirect" then
			if v[".name"] ~= id then
				-- Rewrite state
				uci:section("luci_splash_locations", "redirect", v[".name"], {
					location = v.location
				})
			end
		end
	end

	uci:save("luci_splash_redirects")
end
