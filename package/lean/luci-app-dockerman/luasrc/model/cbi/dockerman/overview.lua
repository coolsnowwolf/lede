--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local docker = require "luci.model.docker"
local uci = require "luci.model.uci"

function byte_format(byte)
  local suff = {"B", "KB", "MB", "GB", "TB"}
  for i=1, 5 do
    if byte > 1024 and i < 5 then
      byte = byte / 1024
    else
      return string.format("%.2f %s", byte, suff[i]) 
    end 
  end
end

local m = Map("dockerman", translate("Docker"))
local docker_info_table = {}
-- docker_info_table['0OperatingSystem'] = {_key=translate("Operating System"),_value='-'}
-- docker_info_table['1Architecture'] = {_key=translate("Architecture"),_value='-'}
-- docker_info_table['2KernelVersion'] = {_key=translate("Kernel Version"),_value='-'}
docker_info_table['3ServerVersion'] = {_key=translate("Docker Version"),_value='-'}
docker_info_table['4ApiVersion'] = {_key=translate("Api Version"),_value='-'}
docker_info_table['5NCPU'] = {_key=translate("CPUs"),_value='-'}
docker_info_table['6MemTotal'] = {_key=translate("Total Memory"),_value='-'}
docker_info_table['7DockerRootDir'] = {_key=translate("Docker Root Dir"),_value='-'}
docker_info_table['8IndexServerAddress'] = {_key=translate("Index Server Address"),_value='-'}

s = m:section(Table, docker_info_table)
s:option(DummyValue, "_key", translate("Info"))
s:option(DummyValue, "_value")

s = m:section(SimpleSection)
s.containers_running = '-'
s.images_used = '-'
s.containers_total = '-'
s.images_total = '-'
s.networks_total = '-'
s.volumes_total = '-'
local socket = luci.model.uci.cursor():get("dockerman", "local", "socket_path")
if nixio.fs.access(socket) and (require "luci.model.docker").new():_ping().code == 200 then
  local dk = docker.new()
  local containers_list = dk.containers:list({query = {all=true}}).body
  local images_list = dk.images:list().body
  local vol = dk.volumes:list()
  local volumes_list = vol and vol.body and vol.body.Volumes or {}
  local networks_list = dk.networks:list().body or {}
  local docker_info = dk:info()
  -- docker_info_table['0OperatingSystem']._value = docker_info.body.OperatingSystem
  -- docker_info_table['1Architecture']._value = docker_info.body.Architecture
  -- docker_info_table['2KernelVersion']._value = docker_info.body.KernelVersion
  docker_info_table['3ServerVersion']._value = docker_info.body.ServerVersion
  docker_info_table['4ApiVersion']._value = docker_info.headers["Api-Version"]
  docker_info_table['5NCPU']._value = tostring(docker_info.body.NCPU)
  docker_info_table['6MemTotal']._value = byte_format(docker_info.body.MemTotal)
  docker_info_table['7DockerRootDir']._value = docker_info.body.DockerRootDir
  docker_info_table['8IndexServerAddress']._value = docker_info.body.IndexServerAddress

  s.images_used = 0
  for i, v in ipairs(images_list) do
    for ci,cv in ipairs(containers_list) do
      if v.Id == cv.ImageID then
        s.images_used = s.images_used + 1
        break
      end
    end
  end
  s.containers_running = tostring(docker_info.body.ContainersRunning)
  s.images_used = tostring(s.images_used)
  s.containers_total = tostring(docker_info.body.Containers)
  s.images_total = tostring(#images_list)
  s.networks_total = tostring(#networks_list)
  s.volumes_total = tostring(#volumes_list)
end
s.template = "dockerman/overview"


s = m:section(NamedSection, "local", "section", translate("Setting"))

socket_path = s:option(Value, "socket_path", translate("Socket Path"))
status_path = s:option(Value, "status_path", translate("Action Status Tempfile Path"), translate("Where you want to save the docker status file"))
debug = s:option(Flag, "debug", translate("Enable Debug"), translate("For debug, It shows all docker API actions of luci-app-dockerman in Debug Tempfile Path"))
debug.enabled="true"
debug.disabled="false"
debug_path = s:option(Value, "debug_path", translate("Debug Tempfile Path"), translate("Where you want to save the debug tempfile"))

return m
