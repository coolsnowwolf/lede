--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local uci = luci.model.uci.cursor()
local docker = require "luci.model.docker"
local dk = docker.new()

local containers, volumes
local res = dk.volumes:list()
if res.code <300 then volumes = res.body.Volumes else return end
res = dk.containers:list({query = {all=true}})
if res.code <300 then containers = res.body else return end

function get_volumes()
  local data = {}
  for i, v in ipairs(volumes) do
    -- local index = v.CreatedAt .. v.Name
    local index = v.Name
    data[index]={}
    data[index]["_selected"] = 0
    data[index]["_nameraw"] = v.Name
    data[index]["_name"] = v.Name:sub(1,12)
    for ci,cv in ipairs(containers) do
      if cv.Mounts and type(cv.Mounts) ~= "table" then break end
      for vi, vv in ipairs(cv.Mounts) do
        if v.Name == vv.Name then
          data[index]["_containers"] = (data[index]["_containers"] and (data[index]["_containers"] .. " | ") or "")..
          "<a href=/cgi-bin/luci/admin/services/docker/container/"..cv.Id.." >".. cv.Names[1]:sub(2).."</a>"
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

local volume_list = get_volumes()

-- m = Map("docker", translate("Docker"))
m = SimpleForm("docker", translate("Docker"))
m.template = "dockerman/cbi/xsimpleform"
m.submit=false
m.reset=false


volume_table = m:section(Table, volume_list, translate("Volumes"))

volume_selecter = volume_table:option(Flag, "_selected","")
volume_selecter.disabled = 0
volume_selecter.enabled = 1
volume_selecter.default = 0

volume_id = volume_table:option(DummyValue, "_name", translate("Name"))
volume_table:option(DummyValue, "_driver", translate("Driver"))
volume_table:option(DummyValue, "_containers", translate("Containers")).rawhtml = true
volume_table:option(DummyValue, "_mountpoint", translate("Mount Point"))
volume_table:option(DummyValue, "_created", translate("Created"))
volume_selecter.write = function(self, section, value)
  volume_list[section]._selected = value
end

docker_status = m:section(SimpleSection)
docker_status.template = "dockerman/apply_widget"
docker_status.err=nixio.fs.readfile(dk.options.status_path)
if docker_status.err then docker:clear_status() end

action = m:section(Table,{{}})
action.notitle=true
action.rowcolors=false
action.template="cbi/nullsection"
btnremove = action:option(Button, "remove")
btnremove.inputtitle= translate("Remove")
btnremove.template = "dockerman/cbi/inlinebutton"
btnremove.inputstyle = "remove"
btnremove.forcewrite = true
btnremove.write = function(self, section)
  local volume_selected = {}
  -- 遍历table中sectionid
  local volume_table_sids = volume_table:cfgsections()
  for _, volume_table_sid in ipairs(volume_table_sids) do
    -- 得到选中项的名字
    if volume_list[volume_table_sid]._selected == 1 then
      -- volume_selected[#volume_selected+1] = volume_id:cfgvalue(volume_table_sid)
      volume_selected[#volume_selected+1] = volume_table_sid
    end
  end
  if next(volume_selected) ~= nil then
    local success = true
    docker:clear_status()
    for _,vol in ipairs(volume_selected) do
      docker:append_status("Volumes: " .. "remove" .. " " .. vol .. "...")
      local msg = dk.volumes["remove"](dk, {id = vol})
      if msg.code ~= 204 then
        docker:append_status("fail code:" .. msg.code.." ".. (msg.body.message and msg.body.message or msg.message).. "<br>")
        success = false
      else
        docker:append_status("done<br>")
      end
    end
    if success then docker:clear_status() end
    luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/volumes"))
  end
end
return m
