module("luci.controller.olsr", package.seeall)

local neigh_table = nil
local ifaddr_table = nil

function index()
	local ipv4,ipv6
	if nixio.fs.access("/etc/config/olsrd") then
		ipv4 = 1
	end
	if nixio.fs.access("/etc/config/olsrd6") then
		ipv6 = 1
	end
	if not ipv4 and not ipv6 then
		return
	end

	require("luci.model.uci")
	local uci = luci.model.uci.cursor_state()

	uci:foreach("olsrd", "olsrd", function(s)
		if s.SmartGateway and s.SmartGateway == "yes" then has_smartgw  = true end
	end)

	local page  = node("admin", "status", "olsr")
	page.target = template("status-olsr/overview")
	page.title  = _("OLSR")
	page.subindex = true

	local page  = node("admin", "status", "olsr", "json")
	page.target = call("action_json")
	page.title = nil
	page.leaf = true

	local page  = node("admin", "status", "olsr", "neighbors")
	page.target = call("action_neigh")
	page.title  = _("Neighbours")
	page.subindex = true
	page.order  = 5

	local page  = node("admin", "status", "olsr", "routes")
	page.target = call("action_routes")
	page.title  = _("Routes")
	page.order  = 10

	local page  = node("admin", "status", "olsr", "topology")
	page.target = call("action_topology")
	page.title  = _("Topology")
	page.order  = 20

	local page  = node("admin", "status", "olsr", "hna")
	page.target = call("action_hna")
	page.title  = _("HNA")
	page.order  = 30

	local page  = node("admin", "status", "olsr", "mid")
	page.target = call("action_mid")
	page.title  = _("MID")
	page.order  = 50

	if has_smartgw then
		local page  = node("admin", "status", "olsr", "smartgw")
		page.target = call("action_smartgw")
		page.title  = _("SmartGW")
		page.order  = 60
	end

	local page  = node("admin", "status", "olsr", "interfaces")
	page.target = call("action_interfaces")
	page.title  = _("Interfaces")
	page.order  = 70

	odsp = entry(
		{"admin", "services", "olsrd", "display"},
		cbi("olsr/olsrddisplay"), _("Display")
	)

end

function action_json()
	local http = require "luci.http"
	local utl = require "luci.util"
	local uci = require "luci.model.uci".cursor()
	local jsonreq4
	local jsonreq6

	local v4_port = tonumber(uci:get("olsrd", "olsrd_jsoninfo", "port") or "") or 9090
	local v6_port = tonumber(uci:get("olsrd6", "olsrd_jsoninfo", "port") or "") or 9090

	jsonreq4 = utl.exec("(echo /status | nc 127.0.0.1 %d | sed -n '/^[}{ ]/p') 2>/dev/null" % v4_port)
	jsonreq6 = utl.exec("(echo /status | nc ::1 %d | sed -n '/^[}{ ]/p') 2>/dev/null" % v6_port)
	http.prepare_content("application/json")
	if not jsonreq4 or jsonreq4 == "" then
		jsonreq4 = "{}"
	end
	if not jsonreq6 or jsonreq6 == "" then
		jsonreq6 = "{}"
	end
	http.write('{"v4":' .. jsonreq4 .. ', "v6":' .. jsonreq6 .. '}')
end


local function local_mac_lookup(ipaddr)
	local _, rt
	for _, rt in ipairs(luci.ip.routes({ type = 1, src = ipaddr })) do
		local link = rt.dev and luci.ip.link(rt.dev)
		local mac = link and luci.ip.checkmac(link.mac)
		if mac then return mac end
	end
end

local function remote_mac_lookup(ipaddr)
	local _, n
	for _, n in ipairs(luci.ip.neighbors({ dest = ipaddr })) do
		local mac = luci.ip.checkmac(n.mac)
		if mac then return mac end
	end
end

function action_neigh(json)
	local data, has_v4, has_v6, error = fetch_jsoninfo('links')

	if error then
		return
	end

	local uci = require "luci.model.uci".cursor_state()
	local resolve = uci:get("luci_olsr", "general", "resolve")
	local ntm = require "luci.model.network".init()
	local devices  = ntm:get_wifidevs()
	local sys = require "luci.sys"
	local assoclist = {}
	--local neightbl = require "neightbl"
	local ntm = require "luci.model.network"
	local ipc = require "luci.ip"
	local nxo = require "nixio"
	local defaultgw

	ipc.routes({ family = 4, type = 1, dest_exact = "0.0.0.0/0" },
		function(rt) defaultgw = rt.gw end)

	local function compare(a,b)
		if a.proto == b.proto then
			return a.linkCost < b.linkCost
		else
			return a.proto < b.proto
		end
	end

	for _, dev in ipairs(devices) do
		for _, net in ipairs(dev:get_wifinets()) do
			local radio = net:get_device()
			assoclist[#assoclist+1] = {}
			assoclist[#assoclist]['ifname'] = net:ifname()
			assoclist[#assoclist]['network'] = net:network()[1]
			assoclist[#assoclist]['device'] = radio and radio:name() or nil
			assoclist[#assoclist]['list'] = net:assoclist()
		end
	end

	for k, v in ipairs(data) do
		local snr = 0
		local signal = 0
		local noise = 0
		local mac = ""
		local ip
		local neihgt = {}

		if resolve == "1" then
			hostname = nixio.getnameinfo(v.remoteIP, nil, 100)
			if hostname then
				v.hostname = hostname
			end
		end

		local interface = ntm:get_status_by_address(v.localIP)
		local lmac = local_mac_lookup(v.localIP)
		local rmac = remote_mac_lookup(v.remoteIP)

		for _, val in ipairs(assoclist) do
			if val.network == interface and val.list then
				local assocmac, assot
				for assocmac, assot in pairs(val.list) do
					if rmac == luci.ip.checkmac(assocmac) then
						signal = tonumber(assot.signal)
						noise = tonumber(assot.noise)
						snr = (noise*-1) - (signal*-1)
					end
				end
			end
		end
		if interface then
			v.interface = interface
		end
		v.snr = snr
		v.signal = signal
		v.noise = noise
		if rmac then
			v.remoteMAC = rmac
		end
		if lmac then
			v.localMAC = lmac
		end

		if defaultgw == v.remoteIP then
			v.defaultgw = 1
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/neighbors", {links=data, has_v4=has_v4, has_v6=has_v6})
end

function action_routes()
	local data, has_v4, has_v6, error = fetch_jsoninfo('routes')
	if error then
		return
	end

	local uci = require "luci.model.uci".cursor_state()
	local resolve = uci:get("luci_olsr", "general", "resolve")

	for k, v in ipairs(data) do
		if resolve == "1" then
			local hostname = nixio.getnameinfo(v.gateway, nil, 100)
			if hostname then
				v.hostname = hostname
			end
		end
	end

	local function compare(a,b)
		if a.proto == b.proto then
			return a.rtpMetricCost < b.rtpMetricCost
		else
			return a.proto < b.proto
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/routes", {routes=data, has_v4=has_v4, has_v6=has_v6})
end

function action_topology()
	local data, has_v4, has_v6, error = fetch_jsoninfo('topology')
	if error then
		return
	end

	local function compare(a,b)
		if a.proto == b.proto then
			return a.tcEdgeCost < b.tcEdgeCost
		else
			return a.proto < b.proto
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/topology", {routes=data, has_v4=has_v4, has_v6=has_v6})
end

function action_hna()
	local data, has_v4, has_v6, error = fetch_jsoninfo('hna')
	if error then
		return
	end

	local uci = require "luci.model.uci".cursor_state()
	local resolve = uci:get("luci_olsr", "general", "resolve")

	local function compare(a,b)
		if a.proto == b.proto then
			return a.genmask < b.genmask
		else
			return a.proto < b.proto
		end
	end

	for k, v in ipairs(data) do
		if resolve == "1" then
			hostname = nixio.getnameinfo(v.gateway, nil, 100)
			if hostname then
				v.hostname = hostname
			end
		end
		if v.validityTime then
			v.validityTime = tonumber(string.format("%.0f", v.validityTime / 1000))
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/hna", {hna=data, has_v4=has_v4, has_v6=has_v6})
end

function action_mid()
	local data, has_v4, has_v6, error = fetch_jsoninfo('mid')
	if error then
		return
	end

	local function compare(a,b)
		if a.proto == b.proto then
			return a.ipAddress < b.ipAddress
		else
			return a.proto < b.proto
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/mid", {mids=data, has_v4=has_v4, has_v6=has_v6})
end

function action_smartgw()
	local data, has_v4, has_v6, error = fetch_jsoninfo('gateways')
	if error then
		return
	end

	local function compare(a,b)
		if a.proto == b.proto then
			return a.tcPathCost < b.tcPathCost
		else
			return a.proto < b.proto
		end
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/smartgw", {gws=data, has_v4=has_v4, has_v6=has_v6})
end

function action_interfaces()
	local data, has_v4, has_v6, error = fetch_jsoninfo('interfaces')
	if error then
		return
	end

	local function compare(a,b)
		return a.proto < b.proto
	end

	table.sort(data, compare)
	luci.template.render("status-olsr/interfaces", {iface=data, has_v4=has_v4, has_v6=has_v6})
end

-- Internal
function fetch_jsoninfo(otable)
	local uci = require "luci.model.uci".cursor_state()
	local utl = require "luci.util"
	local json = require "luci.json"
	local IpVersion = uci:get_first("olsrd", "olsrd","IpVersion")
	local jsonreq4 = ""
	local jsonreq6 = ""
	local v4_port = tonumber(uci:get("olsrd", "olsrd_jsoninfo", "port") or "") or 9090
	local v6_port = tonumber(uci:get("olsrd6", "olsrd_jsoninfo", "port") or "") or 9090

	jsonreq4 = utl.exec("(echo /%s | nc 127.0.0.1 %d | sed -n '/^[}{ ]/p') 2>/dev/null" %{ otable, v4_port })
	jsonreq6 = utl.exec("(echo /%s | nc ::1 %d | sed -n '/^[}{ ]/p') 2>/dev/null" %{ otable, v6_port })
	local jsondata4 = {}
	local jsondata6 = {}
	local data4 = {}
	local data6 = {}
	local has_v4 = False
	local has_v6 = False

	if jsonreq4 == '' and jsonreq6 == '' then
		luci.template.render("status-olsr/error_olsr")
		return nil, 0, 0, true
	end

	if jsonreq4 ~= "" then
		has_v4 = 1
		jsondata4 = json.decode(jsonreq4)
		if otable == 'status' then
			data4 = jsondata4 or {}
		else
			data4 = jsondata4[otable] or {}
		end

		for k, v in ipairs(data4) do
			data4[k]['proto'] = '4'
		end

	end
	if jsonreq6 ~= "" then
		has_v6 = 1
		jsondata6 = json.decode(jsonreq6)
		if otable == 'status' then
			data6 = jsondata6 or {}
		else
			data6 = jsondata6[otable] or {}
		end
		for k, v in ipairs(data6) do
			data6[k]['proto'] = '6'
		end
	end

	for k, v in ipairs(data6) do
		table.insert(data4, v)
	end

	return data4, has_v4, has_v6, false
end

