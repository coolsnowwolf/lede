--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local docker = require "luci.model.docker"
local dk = docker.new()

local m, s, o

local res, containers, volumes, lost_state

function get_volumes()
	local data = {}
	for i, v in ipairs(volumes) do
		local index = v.Name
		data[index]={}
		data[index]["_selected"] = 0
		data[index]["_nameraw"] = v.Name
		data[index]["_name"] = v.Name:sub(1,12)

		for ci,cv in ipairs(containers) do
			if cv.Mounts and type(cv.Mounts) ~= "table" then
				break
			end
			for vi, vv in ipairs(cv.Mounts) do
				if v.Name == vv.Name then
					data[index]["_containers"] = (data[index]["_containers"] and (data[index]["_containers"] .. " | ") or "")..
					'<a href='..luci.dispatcher.build_url("admin/docker/container/"..cv.Id)..' class="dockerman_link" title="'..translate("Container detail")..'">'.. cv.Names[1]:sub(2)..'</a>'
				end
			end
		end
		data[index]["_driver"] = v.Driver
		data[index]["_mountpoint"] = nil

		for v1 in v.Mountpoint:gmatch('[^/]+') do
			if v1 == index then 
				data[index]["_mountpoint"] = data[index]["_mountpoint"] .."/" .. v1:sub(1,12) .. "..."
			else
				data[index]["_mountpoint"] = (data[index]["_mountpoint"] and data[index]["_mountpoint"] or "").."/".. v1
			end
		end
		data[index]["_created"] = v.CreatedAt
	end

	return data
end

if dk:_ping().code ~= 200 then
	lost_state = true
else
	res = dk.volumes:list()
	if res and res.code and res.code <300 then
		volumes = res.body.Volumes
	end

	res = dk.containers:list({
		query = {
			all=true
		}
	})
	if res and res.code and res.code <300 then
		containers = res.body
	end
end

local volume_list = not lost_state and get_volumes() or {}

m = SimpleForm("docker", translate("Docker - Volumes"))
m.submit=false
m.reset=false

s = m:section(Table, volume_list, translate("Volumes overview"))

o = s:option(Flag, "_selected","")
o.disabled = 0
o.enabled = 1
o.default = 0
o.write = function(self, section, value)
	volume_list[section]._selected = value
end

o = s:option(DummyValue, "_name", translate("Name"))

o = s:option(DummyValue, "_driver", translate("Driver"))

o = s:option(DummyValue, "_containers", translate("Containers"))
o.rawhtml = true

o = s:option(DummyValue, "_mountpoint", translate("Mount Point"))

o = s:option(DummyValue, "_created", translate("Created"))

s = m:section(SimpleSection)
s.template = "dockerman/apply_widget"
s.err=docker:read_status()
s.err=s.err and s.err:gsub("\n","<br>"):gsub(" ","&nbsp;")
if s.err then
	docker:clear_status()
end

s = m:section(Table,{{}})
s.notitle=true
s.rowcolors=false
s.template="cbi/nullsection"

o = s:option(Button, "remove")
o.inputtitle= translate("Remove")
o.template = "dockerman/cbi/inlinebutton"
o.inputstyle = "remove"
o.forcewrite = true
o.disable = lost_state
o.write = function(self, section)
	local volume_selected = {}

	for k in pairs(volume_list) do
		if volume_list[k]._selected == 1 then
			volume_selected[#volume_selected+1] = k
		end
	end

	if next(volume_selected) ~= nil then
		local success = true
		docker:clear_status()
		for _,vol in ipairs(volume_selected) do
			docker:append_status("Volumes: " .. "remove" .. " " .. vol .. "...")
			local msg = dk.volumes["remove"](dk, {id = vol})
			if msg and msg.code and msg.code ~= 204 then
				docker:append_status("code:" .. msg.code.." ".. (msg.body.message and msg.body.message or msg.message).. "\n")
				success = false
			else
				docker:append_status("done\n")
			end
		end

		if success then
			docker:clear_status()
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/docker/volumes"))
	end
end

return m
