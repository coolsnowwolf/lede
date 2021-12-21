--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local docker = require "luci.model.docker"

local m, s, o

local dk = docker.new()
if dk:_ping().code ~= 200 then
	lost_state = true
end

m = SimpleForm("docker", translate("Docker - Network"))
m.redirect = luci.dispatcher.build_url("admin", "docker", "networks")
if lost_state then
	m.submit=false
	m.reset=false
end


s = m:section(SimpleSection)
s.template = "dockerman/apply_widget"
s.err=docker:read_status()
s.err=s.err and s.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if s.err then
	docker:clear_status()
end

s = m:section(SimpleSection, translate("Create new docker network"))
s.addremove = true
s.anonymous = true

o = s:option(Value, "name",
	translate("Network Name"),
	translate("Name of the network that can be selected during container creation"))
o.rmempty = true

o = s:option(ListValue, "driver", translate("Driver"))
o.rmempty = true
o:value("bridge", translate("Bridge device"))
o:value("macvlan", translate("MAC VLAN"))
o:value("ipvlan",  translate("IP VLAN"))
o:value("overlay", translate("Overlay network"))

o = s:option(Value, "parent", translate("Base device"))
o.rmempty = true
o:depends("driver", "macvlan")
local interfaces = luci.sys and luci.sys.net and luci.sys.net.devices() or {}
for _, v in ipairs(interfaces) do
	o:value(v, v)
end
o.default="br-lan"
o.placeholder="br-lan"

o = s:option(ListValue, "macvlan_mode", translate("Mode"))
o.rmempty = true
o:depends("driver", "macvlan")
o.default="bridge"
o:value("bridge", translate("Bridge (Support direct communication between MAC VLANs)"))
o:value("private", translate("Private (Prevent communication between MAC VLANs)"))
o:value("vepa", translate("VEPA (Virtual Ethernet Port Aggregator)"))
o:value("passthru", translate("Pass-through (Mirror physical device to single MAC VLAN)"))

o = s:option(ListValue, "ipvlan_mode", translate("Ipvlan Mode"))
o.rmempty = true
o:depends("driver", "ipvlan")
o.default="l3"
o:value("l2", translate("L2 bridge"))
o:value("l3", translate("L3 bridge"))

o = s:option(Flag, "ingress",
	translate("Ingress"),
	translate("Ingress network is the network which provides the routing-mesh in swarm mode"))
o.rmempty = true
o.disabled = 0
o.enabled = 1
o.default = 0
o:depends("driver", "overlay")

o = s:option(DynamicList, "options", translate("Options"))
o.rmempty = true
o.placeholder="com.docker.network.driver.mtu=1500"

o = s:option(Flag, "internal", translate("Internal"), translate("Restrict external access to the network"))
o.rmempty = true
o:depends("driver", "overlay")
o.disabled = 0
o.enabled = 1
o.default = 0

if nixio.fs.access("/etc/config/network") and nixio.fs.access("/etc/config/firewall")then
	o = s:option(Flag, "op_macvlan", translate("Create macvlan interface"), translate("Auto create macvlan interface in Openwrt"))
	o:depends("driver", "macvlan")
	o.disabled = 0
	o.enabled = 1
	o.default = 1
end

o = s:option(Value, "subnet", translate("Subnet"))
o.rmempty = true
o.placeholder="10.1.0.0/16"
o.datatype="ip4addr"

o = s:option(Value, "gateway", translate("Gateway"))
o.rmempty = true
o.placeholder="10.1.1.1"
o.datatype="ip4addr"

o = s:option(Value, "ip_range", translate("IP range"))
o.rmempty = true
o.placeholder="10.1.1.0/24"
o.datatype="ip4addr"

o = s:option(DynamicList, "aux_address", translate("Exclude IPs"))
o.rmempty = true
o.placeholder="my-route=10.1.1.1"

o = s:option(Flag, "ipv6", translate("Enable IPv6"))
o.rmempty = true
o.disabled = 0
o.enabled = 1
o.default = 0

o = s:option(Value, "subnet6", translate("IPv6 Subnet"))
o.rmempty = true
o.placeholder="fe80::/10"
o.datatype="ip6addr"
o:depends("ipv6", 1)

o = s:option(Value, "gateway6", translate("IPv6 Gateway"))
o.rmempty = true
o.placeholder="fe80::1"
o.datatype="ip6addr"
o:depends("ipv6", 1)

m.handle = function(self, state, data)
	if state == FORM_VALID then
		local name = data.name
		local driver = data.driver

		local internal = data.internal == 1 and true or false

		local subnet = data.subnet
		local gateway = data.gateway
		local ip_range = data.ip_range

		local aux_address = {}
		local tmp = data.aux_address or {}
		for i,v in ipairs(tmp) do
			_,_,k1,v1 = v:find("(.-)=(.+)")
			aux_address[k1] = v1
		end

		local options = {}
		tmp = data.options or {}
		for i,v in ipairs(tmp) do
			_,_,k1,v1 = v:find("(.-)=(.+)")
			options[k1] = v1
		end

		local ipv6 = data.ipv6 == 1 and true or false

		local create_body = {
			Name = name,
			Driver = driver,
			EnableIPv6 = ipv6,
			IPAM = {
				Driver= "default"
			},
			Internal = internal
		}

		if subnet or gateway or ip_range then
			create_body["IPAM"]["Config"] = {
				{
					Subnet = subnet,
					Gateway = gateway,
					IPRange = ip_range,
					AuxAddress = aux_address,
					AuxiliaryAddresses = aux_address
				}
			}
		end

		if driver == "macvlan" then
			create_body["Options"] = {
				macvlan_mode = data.macvlan_mode,
				parent = data.parent
			}
		elseif driver == "ipvlan" then
			create_body["Options"] = {
				ipvlan_mode = data.ipvlan_mode
		}
		elseif driver == "overlay" then
			create_body["Ingress"] = data.ingerss == 1 and true or false
		end

		if ipv6 and data.subnet6 and data.subnet6 then
			if type(create_body["IPAM"]["Config"]) ~= "table" then
				create_body["IPAM"]["Config"] = {}
			end
			local index = #create_body["IPAM"]["Config"]
			create_body["IPAM"]["Config"][index+1] = {
				Subnet = data.subnet6,
				 Gateway = data.gateway6
			}
		end

		if next(options) ~= nil then
			create_body["Options"] = create_body["Options"] or {}
			for k, v in pairs(options) do
				create_body["Options"][k] = v
			end
		end

		create_body = docker.clear_empty_tables(create_body)
		docker:write_status("Network: " .. "create" .. " " .. create_body.Name .. "...")

		local res = dk.networks:create({
			body = create_body
		})

		if res and res.code == 201 then
			docker:write_status("Network: " .. "create macvlan interface...")
			res = dk.networks:inspect({
				name = create_body.Name
			})

			if driver == "macvlan" and
				data.op_macvlan ~= 0 and
				res and
				res.code and
				res.code == 200 and
				res.body and
				res.body.IPAM and
				res.body.IPAM.Config and
				res.body.IPAM.Config[1] and
				res.body.IPAM.Config[1].Gateway and
				res.body.IPAM.Config[1].Subnet then

				docker.create_macvlan_interface(data.name,
					data.parent,
					res.body.IPAM.Config[1].Gateway,
					res.body.IPAM.Config[1].Subnet)
			end

			docker:clear_status()
			luci.http.redirect(luci.dispatcher.build_url("admin/docker/networks"))
		else
			docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
			luci.http.redirect(luci.dispatcher.build_url("admin/docker/newnetwork"))
		end
	end
end

return m
