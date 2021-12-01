--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local docker = require "luci.model.docker"

local m, s, o
local networks, dk, res, lost_state

dk = docker.new()

if dk:_ping().code ~= 200 then
	lost_state = true
else
	res = dk.networks:list()
	if res and res.code and res.code < 300 then
		networks = res.body
	end
end

local get_networks = function ()
	local data = {}

	if type(networks) ~= "table" then
		return nil
	end

	for i, v in ipairs(networks) do
		local index = v.Created .. v.Id

		data[index]={}
		data[index]["_selected"] = 0
		data[index]["_id"] = v.Id:sub(1,12)
		data[index]["_name"] = v.Name
		data[index]["_driver"] = v.Driver

		if v.Driver == "bridge" then
			data[index]["_interface"] = v.Options["com.docker.network.bridge.name"]
		elseif v.Driver == "macvlan" then
			data[index]["_interface"] = v.Options.parent
		end

		data[index]["_subnet"] = v.IPAM and v.IPAM.Config[1] and v.IPAM.Config[1].Subnet or nil
		data[index]["_gateway"] = v.IPAM and v.IPAM.Config[1] and v.IPAM.Config[1].Gateway or nil
	end

	return data
end

local network_list = not lost_state and get_networks() or {}

m = SimpleForm("docker",
	translate("Docker - Networks"),
	translate("This page displays all docker networks that have been created on the connected docker host."))
m.submit=false
m.reset=false

s = m:section(Table, network_list, translate("Networks overview"))
s.nodescr=true

o = s:option(Flag, "_selected","")
o.template = "dockerman/cbi/xfvalue"
o.disabled = 0
o.enabled = 1
o.default = 0
o.render = function(self, section, scope)
	self.disable = 0
	if network_list[section]["_name"] == "bridge" or network_list[section]["_name"] == "none" or network_list[section]["_name"] == "host" then
		self.disable = 1
	end
	Flag.render(self, section, scope)
end
o.write = function(self, section, value)
	network_list[section]._selected = value
end

o = s:option(DummyValue, "_id", translate("ID"))

o = s:option(DummyValue, "_name", translate("Network Name"))

o = s:option(DummyValue, "_driver", translate("Driver"))

o = s:option(DummyValue, "_interface", translate("Parent Interface"))

o = s:option(DummyValue, "_subnet", translate("Subnet"))

o = s:option(DummyValue, "_gateway", translate("Gateway"))

s = m:section(SimpleSection)
s.template = "dockerman/apply_widget"
s.err = docker:read_status()
s.err = s.err and s.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if s.err then
	docker:clear_status()
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template="cbi/nullsection"

o = s:option(Button, "_new")
o.inputtitle= translate("New")
o.template = "dockerman/cbi/inlinebutton"
o.notitle=true
o.inputstyle = "add"
o.forcewrite = true
o.disable = lost_state
o.write = function(self, section)
	luci.http.redirect(luci.dispatcher.build_url("admin/docker/newnetwork"))
end

o = s:option(Button, "_remove")
o.inputtitle= translate("Remove")
o.template = "dockerman/cbi/inlinebutton"
o.inputstyle = "remove"
o.forcewrite = true
o.disable = lost_state
o.write = function(self, section)
	local network_selected = {}
	local network_name_selected = {}
	local network_driver_selected = {}

	for k in pairs(network_list) do
		if network_list[k]._selected == 1 then
			network_selected[#network_selected + 1] = network_list[k]._id
			network_name_selected[#network_name_selected + 1] = network_list[k]._name
			network_driver_selected[#network_driver_selected + 1] = network_list[k]._driver
		end
	end

	if next(network_selected) ~= nil then
		local success = true
		docker:clear_status()

		for ii, net in ipairs(network_selected) do
			docker:append_status("Networks: " .. "remove" .. " " .. net .. "...")
			local res = dk.networks["remove"](dk, {id = net})

			if res and res.code and res.code >= 300 then
				docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
				success = false
			else
				docker:append_status("done\n")
				if network_driver_selected[ii] == "macvlan" then
					docker.remove_macvlan_interface(network_name_selected[ii])
				end
			end
		end

		if success then
			docker:clear_status()
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/docker/networks"))
	end
end

return m
