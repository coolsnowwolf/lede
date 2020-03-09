--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
local docker = require "luci.docker"
local uci = (require "luci.model.uci").cursor()

local _docker = {}

--pull image and return iamge id
local update_image = function(self, image_name)
  local server = "index.docker.io"

  local json_stringify = luci.jsonc and luci.jsonc.stringify
  _docker:append_status("Images: " .. "pulling" .. " " .. image_name .. "...")
  local x_auth = nixio.bin.b64encode(json_stringify({serveraddress= server}))
  local res = self.images:create({query = {fromImage=image_name}, header={["X-Registry-Auth"]=x_auth}})
  if res and res.code < 300 then
    _docker:append_status("done<br>")
  else
    _docker:append_status("fail code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "<br>")
  end
  new_image_id = self.images:inspect({name = image_name}).body.Id
  return new_image_id, res
end

local table_equal = function(t1, t2)
  if not t1 then return true end
  if not t2 then return false end
  if #t1 ~= #t2 then return false end
  for i, v in ipairs(t1) do
    if t1[i] ~= t2[i] then return false end
  end
  return true
end

local table_subtract = function(t1, t2)
  if not t1 or next(t1) == nil then return nil end
  if not t2 or next(t2) == nil then return t1 end
  local res = {}
  for _, v1 in ipairs(t1) do
    local found = false
    for _, v2 in ipairs(t2) do
      if v1 == v2 then
        found= true
        break
      end
    end
    if not found then
      table.insert(res, v1)
    end
  end
  return next(res) == nil and nil or res
end

local map_subtract = function(t1, t2)
  if not t1 or next(t1) == nil then return nil end
  if not t2 or next(t2) == nil then return t1 end
  local res = {}
  for k1, v1 in pairs(t1) do
    local found = false
    for k2, v2 in ipairs(t2) do
      if k1 == k2 and luci.util.serialize_data(v1) == luci.util.serialize_data(v2) then
        found= true
        break
      end
    end
    if not found then
      if v1 and type(v1) == "table" then
        if next(v1) == nil then 
          res[k1] = { k = 'v' }
        else
          res[k1] = v1
        end
      end
    end
  end

  return next(res) ~= nil and res or nil
end

-- return create_body, extra_network
local get_config = function(old_config, old_host_config, old_network_setting, image_config)
  local config = old_config
  if config.WorkingDir == image_config.WorkingDir then config.WorkingDir = "" end
  if config.User == image_config.User then config.User = "" end
  if table_equal(config.Cmd, image_config.Cmd) then config.Cmd = nil end
  if table_equal(config.Entrypoint, image_config.Entrypoint) then config.Entrypoint = nil end
  if table_equal(config.ExposedPorts, image_config.ExposedPorts) then config.ExposedPorts = nil end
  config.Env = table_subtract(config.Env, image_config.Env)
  config.Labels = table_subtract(config.Labels, image_config.Labels)
  config.Volumes = map_subtract(config.Volumes, image_config.Volumes)
  -- subtract ports exposed in image from container
  if old_host_config.PortBindings and next(old_host_config.PortBindings) ~= nil then
    config.ExposedPorts = {}
    for p, v in pairs(old_host_config.PortBindings) do
      config.ExposedPorts[p] = { HostPort=v[1] and v[1].HostPort }
    end
  end

  -- handle network config, we need only one network, extras need to network connect action
  local network_setting = {}
  local multi_network = false
  local extra_network = {}
  for k, v in pairs(old_network_setting) do
    if multi_network then
      extra_network[k] = v
    else
      network_setting[k] = v
    end
    multi_network = true
  end

  -- handle hostconfig
  local host_config = old_host_config
  if host_config.PortBindings and next(host_config.PortBindings) == nil then host_config.PortBindings = nil end
  host_config.LogConfig = nil

  -- merge configs
  local create_body = config
  create_body["HostConfig"] = host_config
  create_body["NetworkingConfig"] = {EndpointsConfig = network_setting}

  return create_body, extra_network
end

local upgrade = function(self, request)
  _docker:clear_status()
  -- get image name, image id, container name, configuration information
  local container_info = self.containers:inspect({id = request.id})
  if container_info.code > 300 and type(container_info.body) == "table" then
    return container_info
  end
  local image_name = container_info.body.Config.Image
  if not image_name:match(".-:.+") then image_name = image_name .. ":latest" end
  local old_image_id = container_info.body.Image
  local container_name = container_info.body.Name:sub(2)
  local old_config = container_info.body.Config
  local old_host_config = container_info.body.HostConfig
  local old_network_setting = container_info.body.NetworkSettings.Networks or {}

  local image_id, res = update_image(self, image_name)
  if res and res.code > 300 then return res end
  if image_id == old_image_id then
    return {code = 305, body = {message = "Already up to date"}}
  end

  _docker:append_status("Container: " .. "Stop" .. " " .. container_name .. "...")
  res = self.containers:stop({name = container_name})
  if res and res.code < 305 then
    _docker:append_status("done<br>")
  else
    return res
  end

  _docker:append_status("Container: rename" .. " " .. container_name .. " to ".. container_name .. "_old ...")
  res = self.containers:rename({name = container_name, query = { name = container_name .. "_old" }})
  if res and res.code < 300 then
    _docker:append_status("done<br>")
  else
    return res
  end

  -- handle config
  local image_config = self.images:inspect({id = old_image_id}).body.Config
  local create_body, extra_network = get_config(old_config, old_host_config, old_network_setting, image_config)

  -- create new container
  _docker:append_status("Container: Create" .. " " .. container_name .. "...")
  res = self.containers:create({name = container_name, body = create_body})
  if res and res.code > 300 then return res end
  _docker:append_status("done<br>")

  -- extra networks need to network connect action
  for k, v in pairs(extra_network) do
    if v.IPAMConfig and next(v.IPAMConfig) == nil then v.IPAMConfig =nil end
    if v.DriverOpts and next(v.DriverOpts) == nil then v.DriverOpts =nil end
    if v.Aliases and next(v.Aliases) == nil then v.Aliases =nil end

    _docker:append_status("Networks: Connect" .. " " .. container_name .. "...")
    res = self.networks:connect({id = k, body = {Container = container_name, EndpointConfig = v}})
    if res.code > 300 then return res end

    _docker:append_status("done<br>")
  end
  _docker:clear_status()
  return res
end

local duplicate_config = function (self, request)
  local container_info = self.containers:inspect({id = request.id})
  if container_info.code > 300 and type(container_info.body) == "table" then return nil end
  local old_image_id = container_info.body.Image
  local old_config = container_info.body.Config
  local old_host_config = container_info.body.HostConfig
  local old_network_setting = container_info.body.NetworkSettings.Networks or {}
  local image_config = self.images:inspect({id = old_image_id}).body.Config
  return get_config(old_config, old_host_config, old_network_setting, image_config)
end

_docker.new = function(option)
  local option = option or {}
  options = {
    socket_path = option.socket_path or uci:get("dockerman", "local", "socket_path"),
    debug = option.debug or uci:get("dockerman", "local", "debug") == 'true' and true or false,
    debug_path = option.debug_path or uci:get("dockerman", "local", "debug_path")
  }
  local _new = docker.new(options)
  _new.options.status_path = uci:get("dockerman", "local", "status_path")
  _new.containers_upgrade = upgrade
  _new.containers_duplicate_config = duplicate_config
  return _new
end
_docker.options={}
_docker.options.status_path = uci:get("dockerman", "local", "status_path")

_docker.append_status=function(self,val)
  local file_docker_action_status=io.open(self.options.status_path, "a+")
  file_docker_action_status:write(val)
  file_docker_action_status:close()
end

_docker.clear_status=function(self)
  nixio.fs.remove(self.options.status_path)
end

return _docker
