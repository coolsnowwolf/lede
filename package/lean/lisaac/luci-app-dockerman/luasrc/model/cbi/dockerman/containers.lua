--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local http = require "luci.http"
local docker = require "luci.model.docker"

local m, s, o
local images, networks, containers, res, lost_state
local urlencode = luci.http.protocol and luci.http.protocol.urlencode or luci.util.urlencode
local dk = docker.new()

if dk:_ping().code ~= 200 then
	lost_state = true
else
	res = dk.images:list()
	if res and res.code and res.code < 300 then
		images = res.body
	end

	res = dk.networks:list()
	if res and res.code and res.code < 300 then
		networks = res.body
	end

	res = dk.containers:list({
		query = {
			all = true
		}
	})
	if res and res.code and res.code < 300 then
		containers = res.body
	end
end

function get_containers()
	local data = {}
	if type(containers) ~= "table" then
		return nil
	end

	for i, v in ipairs(containers) do
		local index = (10^12 - v.Created) .. "_id_" .. v.Id

		data[index]={}
		data[index]["_selected"] = 0
		data[index]["_id"] = v.Id:sub(1,12)
		-- data[index]["name"] = v.Names[1]:sub(2)
		data[index]["_status"] = v.Status

		if v.Status:find("^Up") then
			data[index]["_name"] = "<font color='green'>"..v.Names[1]:sub(2).."</font>"
			data[index]["_status"] = "<a href='"..luci.dispatcher.build_url("admin/docker/container/"..v.Id).."/stats'><font color='green'>".. data[index]["_status"] .. "</font>" .. "<br /><font color='#9f9f9f' class='container_cpu_status'></font><br /><font color='#9f9f9f' class='container_mem_status'></font><br /><font color='#9f9f9f' class='container_network_status'></font></a>"
		else
			data[index]["_name"] = "<font color='red'>"..v.Names[1]:sub(2).."</font>"
			data[index]["_status"] = '<font class="container_not_running" color="red">'.. data[index]["_status"] .. "</font>"
		end

		if (type(v.NetworkSettings) == "table" and type(v.NetworkSettings.Networks) == "table") then
			for networkname, netconfig in pairs(v.NetworkSettings.Networks) do
				data[index]["_network"] = (data[index]["_network"] ~= nil and (data[index]["_network"] .." | ") or "").. networkname .. (netconfig.IPAddress ~= "" and (": " .. netconfig.IPAddress) or "")
			end
		end

		-- networkmode = v.HostConfig.NetworkMode ~= "default" and v.HostConfig.NetworkMode or "bridge"
		-- data[index]["_network"] = v.NetworkSettings.Networks[networkmode].IPAddress or nil
		-- local _, _, image = v.Image:find("^sha256:(.+)")
		-- if image ~= nil then
		--  image=image:sub(1,12)
		-- end

		if v.Ports and next(v.Ports) ~= nil then
			data[index]["_ports"] = nil
			local ip = require "luci.ip"
			for _,v2 in ipairs(v.Ports) do
				-- display ipv4 only
				if ip.new(v2.IP or "0.0.0.0"):is4() then
					data[index]["_ports"] = (data[index]["_ports"] and (data[index]["_ports"] .. ", ") or "")
					.. ((v2.PublicPort and v2.Type and v2.Type == "tcp") and ('<a href="javascript:void(0);" onclick="window.open((window.location.origin.match(/^(.+):\\d+$/) && window.location.origin.match(/^(.+):\\d+$/)[1] || window.location.origin) + \':\' + '.. v2.PublicPort ..', \'_blank\');">') or "")
					.. (v2.PublicPort and (v2.PublicPort .. ":") or "")  .. (v2.PrivatePort and (v2.PrivatePort .."/") or "") .. (v2.Type and v2.Type or "")
					.. ((v2.PublicPort and v2.Type and v2.Type == "tcp")and "</a>" or "")
				end
			end
		end

		for ii,iv in ipairs(images) do
			if iv.Id == v.ImageID then
				data[index]["_image"] = iv.RepoTags and iv.RepoTags[1] or (iv.RepoDigests[1]:gsub("(.-)@.+", "%1") .. ":&lt;none&gt;")
			end
		end
		data[index]["_id_name"] = '<a href='..luci.dispatcher.build_url("admin/docker/container/"..v.Id)..'  class="dockerman_link" title="'..translate("Container detail")..'">'.. data[index]["_name"] .. "<br /><font color='#9f9f9f'>ID: " ..	data[index]["_id"]
		.. "</font></a><br />Image: " .. (data[index]["_image"] or "&lt;none&gt;") 
		.. "<br /><font color='#9f9f9f' class='container_size_".. v.Id .."'></font>"

		if type(v.Mounts) == "table" and next(v.Mounts) then
			for _, v2 in pairs(v.Mounts) do
				if v2.Type ~= "volume" then
					local v_sorce_d, v_dest_d
					local v_sorce = ""
					local v_dest = ""
					for v_sorce_d in v2["Source"]:gmatch('[^/]+') do
						if v_sorce_d and #v_sorce_d > 12 then
							v_sorce = v_sorce .. "/" .. v_sorce_d:sub(1,8) .. ".."
						else
							v_sorce = v_sorce .."/".. v_sorce_d
						end
					end
					for v_dest_d in v2["Destination"]:gmatch('[^/]+') do
						if v_dest_d and #v_dest_d > 12 then
							v_dest = v_dest .. "/" .. v_dest_d:sub(1,8) .. ".."
						else
							v_dest = v_dest .."/".. v_dest_d
						end
					end
					data[index]["_mounts"] = (data[index]["_mounts"] and (data[index]["_mounts"] .. "<br />") or "") .. '<span title="'.. v2.Source.. "￫" .. v2.Destination .. '" ><a href="'..luci.dispatcher.build_url("admin/docker/container/"..v.Id)..'/file?path='..v2["Destination"]..'">' .. v_sorce .. "￫" .. v_dest..'</a></span>'
				end
			end
		end

		data[index]["_image_id"] = v.ImageID:sub(8,20)
		data[index]["_command"] = v.Command
	end
	return data
end

local container_list = not lost_state and get_containers() or {}

m = SimpleForm("docker",
	translate("Docker - Containers"),
	translate("This page displays all containers that have been created on the connected docker host."))
m.submit=false
m.reset=false
m:append(Template("dockerman/containers_running_stats"))

s = m:section(SimpleSection)
s.template = "dockerman/apply_widget"
s.err=docker:read_status()
s.err=s.err and s.err:gsub("\n","<br />"):gsub(" ","&#160;")
if s.err then
	docker:clear_status()
end

s = m:section(Table, container_list, translate("Containers"))
s.nodescr=true
s.config="containers"

o = s:option(Flag, "_selected","")
o.disabled = 0
o.enabled = 1
o.default = 0
o.width = "1%"
o.write=function(self, section, value)
	container_list[section]._selected = value
end

-- o = s:option(DummyValue, "_id", translate("ID"))
-- o.width="10%"

-- o = s:option(DummyValue, "_name", translate("Container Name"))
-- o.rawhtml = true

o = s:option(DummyValue, "_id_name", translate("Container Info"))
o.rawhtml = true
o.width="15%"

o = s:option(DummyValue, "_status", translate("Status"))
o.width="15%"
o.rawhtml=true

o = s:option(DummyValue, "_network", translate("Network"))
o.width="10%"

o = s:option(DummyValue, "_ports", translate("Ports"))
o.width="5%"
o.rawhtml = true
o = s:option(DummyValue, "_mounts", translate("Mounts"))
o.width="25%"
o.rawhtml = true

-- o = s:option(DummyValue, "_image", translate("Image"))
-- o.width="8%"

o = s:option(DummyValue, "_command", translate("Command"))
o.width="15%"

local start_stop_remove = function(m, cmd)
	local container_selected = {}
	-- 遍历table中sectionid
	for k in pairs(container_list) do
		 -- 得到选中项的名字
		if container_list[k]._selected == 1 then
			container_selected[#container_selected + 1] = container_list[k]["_id"]
		end
	end
	if #container_selected  > 0 then
		local success = true

		docker:clear_status()
		for _, cont in ipairs(container_selected) do
			docker:append_status("Containers: " .. cmd .. " " .. cont .. "...")
			local res = dk.containers[cmd](dk, {id = cont})
			if res and res.code and res.code >= 300 then
				success = false
				docker:append_status("code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "\n")
			else
				docker:append_status("done\n")
			end
		end

		if success then
			docker:clear_status()
		end

		luci.http.redirect(luci.dispatcher.build_url("admin/docker/containers"))
	end
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template="cbi/nullsection"

o = s:option(Button, "_new")
o.inputtitle = translate("Add")
o.template = "dockerman/cbi/inlinebutton"
o.inputstyle = "add"
o.forcewrite = true
o.write = function(self, section)
	luci.http.redirect(luci.dispatcher.build_url("admin/docker/newcontainer"))
end
o.disable = lost_state

o = s:option(Button, "_start")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle = translate("Start")
o.inputstyle = "apply"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"start")
end
o.disable = lost_state

o = s:option(Button, "_restart")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle = translate("Restart")
o.inputstyle = "reload"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"restart")
end
o.disable = lost_state

o = s:option(Button, "_stop")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle = translate("Stop")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"stop")
end
o.disable = lost_state

o = s:option(Button, "_kill")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle = translate("Kill")
o.inputstyle = "reset"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m,"kill")
end
o.disable = lost_state

o = s:option(Button, "_remove")
o.template = "dockerman/cbi/inlinebutton"
o.inputtitle = translate("Remove")
o.inputstyle = "remove"
o.forcewrite = true
o.write = function(self, section)
	start_stop_remove(m, "remove")
end
o.disable = lost_state

return m
