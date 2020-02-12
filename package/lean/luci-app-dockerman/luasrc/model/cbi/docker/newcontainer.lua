--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

require "luci.util"
require "math"
local uci = luci.model.uci.cursor()
local docker = require "luci.model.docker"
local dk = docker.new()
local cmd_line = table.concat(arg, '/')
local create_body = {}

local images = dk.images:list().body
local networks = dk.networks:list().body
local containers = dk.containers:list(nil, {all=true}).body

local is_quot_complete = function(str)
  if not str then return true end
  local num = 0, w
  for w in str:gmatch("[\"\']") do
    num = num + 1
  end
  if math.fmod(num, 2) ~= 0 then
    return false
  else
    return true
  end
end

-- reslvo default config
local default_config = { }
if cmd_line and cmd_line:match("^docker.+") then
  local key = nil, _key
  --cursor = 0: docker run
  --cursor = 1: resloving para
  --cursor = 2: resloving image
  --cursor > 2: resloving command
  local cursor = 0
  for w in cmd_line:gmatch("[^%s]+") do 
    -- skip '\'
    if w == '\\' then
    elseif _key then
      -- there is a value that unpair quotation marks:
      -- "i was a ok man"
      -- now we only get: "i
      if _key == "mount" or _key == "link" or _key == "env" or _key == "dns" or _key == "port" or _key == "device" or _key == "tmpfs" then
        default_config[_key][#default_config[_key]] = default_config[_key][#default_config[_key]] .. " " .. w
        if is_quot_complete(default_config[_key][#default_config[_key]]) then
          -- clear quotation marks
          default_config[_key][#default_config[_key]] = default_config[_key][#default_config[_key]]:gsub("[\"\']", "")
          _key = nil
        end
      else
        default_config[_key] = default_config[_key] .. " ".. w
        if is_quot_complete(default_config[_key]) then
          -- clear quotation marks
          default_config[_key] = default_config[_key]:gsub("[\"\']", "")
          _key = nil
        end
      end
    -- start with '-'
    elseif w:match("^%-+.+") and cursor <= 1 then
      --key=value
      local val
      key, val = w:match("^%-+(.-)=(.+)")
      -- -dit
      if not key then key = w:match("^%-+(.+)") end

      if not key then
        key = w:match("^%-(.+)")
        if key:match("i") or key:match("t") or key:match("d") then
          if key:match("i") then default_config["interactive"] = true end
          if key:match("t") then default_config["tty"] = true end
          -- clear key
          key = nil
        end
      end

      if key == "v" or key == "volume" then
        key = "mount"
      elseif key == "p" or key == "publish" then
        key = "port"
      elseif key == "e" then
        key = "env"
      elseif key == "dns" then
        key = "dns"
      elseif key == "net" then
        key = "network"
      elseif key == "h" or key == "hostname" then
        key = "hostname"
      elseif key == "cpu-shares" then
        key = "cpushares"
      elseif key == "m" then
        key = "memory"
      elseif key == "blkio-weight" then
        key = "blkioweight"
      elseif key == "privileged" then
        default_config["privileged"] = true
        key = nil
      elseif key == "cap-add" then
        default_config["privileged"] = true
      end
      --key=value
      if val then
        if key == "mount" or key == "link" or key == "env" or key == "dns" or key == "port" or key == "device" or key == "tmpfs" then
          if not default_config[key] then default_config[key] = {} end
          table.insert( default_config[key], val )
          -- clear quotation marks
          default_config[key][#default_config[key]] = default_config[key][#default_config[key]]:gsub("[\"\']", "")
        else
          default_config[key] = val
          -- clear quotation marks
          default_config[key] = default_config[key]:gsub("[\"\']", "")
        end
        -- if there are " or ' in val and separate by space, we need keep the _key to link with next w
        if is_quot_complete(val) then
          _key = nil
        else
          _key = key
        end
        -- clear key
        key = nil
      end
      cursor = 1
    -- value
    elseif key and type(key) == "string" and cursor == 1 then
      if key == "mount" or key == "link" or key == "env" or key == "dns" or key == "port" or key == "device" or key == "tmpfs" then
        if not default_config[key] then default_config[key] = {} end
        table.insert( default_config[key], w )
        -- clear quotation marks
        default_config[key][#default_config[key]] = default_config[key][#default_config[key]]:gsub("[\"\']", "")
      else
        default_config[key] = w
        -- clear quotation marks
        default_config[key] = default_config[key]:gsub("[\"\']", "")
      end
      if key == "cpus" or key == "cpushare" or key == "memory" or key == "blkioweight" or key == "device" or key == "tmpfs" then
        default_config["advance"] = 1
      end
      -- if there are " or ' in val and separate by space, we need keep the _key to link with next w
      if is_quot_complete(w) then
        _key = nil
      else
        _key = key
      end
      key = nil
      cursor = 1
    --image and command
    elseif cursor >= 1 and  key == nil then
      if cursor == 1 then
        default_config["image"] = w
      elseif cursor > 1 then
        default_config["command"] = (default_config["command"] and (default_config["command"] .. " " )or "")  .. w
      end
      cursor = cursor + 1
    end
  end
elseif cmd_line and cmd_line:match("^duplicate/[^/]+$") then
  local container_id = cmd_line:match("^duplicate/(.+)")
  create_body = dk:containers_duplicate_config(container_id)
  if not create_body.HostConfig then create_body.HostConfig = {} end
  if next(create_body) ~= nil then
    default_config.name = nil
    default_config.image = create_body.Image
    default_config.hostname = create_body.Hostname
    default_config.tty = create_body.Tty and true or false
    default_config.interactive = create_body.OpenStdin and true or false
    default_config.privileged = create_body.HostConfig.Privileged and true or false
    default_config.restart =  create_body.HostConfig.RestartPolicy and create_body.HostConfig.RestartPolicy.name or nil
    -- default_config.network = create_body.HostConfig.NetworkMode == "default" and "bridge" or create_body.HostConfig.NetworkMode
    -- if container has leave original network, and add new network, .HostConfig.NetworkMode is INcorrect, so using first child of .NetworkingConfig.EndpointsConfig
    default_config.network = next(create_body.NetworkingConfig.EndpointsConfig)
    default_config.ip = default_config.network and default_config.network ~= "bridge" and default_config.network ~= "host" and default_config.network ~= "null" and create_body.NetworkingConfig.EndpointsConfig[default_config.network].IPAMConfig and create_body.NetworkingConfig.EndpointsConfig[default_config.network].IPAMConfig.IPv4Address or nil
    default_config.link = create_body.HostConfig.Links
    default_config.env = create_body.Env
    default_config.dns = create_body.HostConfig.Dns
    default_config.mount = create_body.HostConfig.Binds

    if create_body.HostConfig.PortBindings and type(create_body.HostConfig.PortBindings) == "table" then
      default_config.port = {}
      for k, v in pairs(create_body.HostConfig.PortBindings) do
        table.insert( default_config.port, v[1].HostPort..":"..k:match("^(%d+)/.+").."/"..k:match("^%d+/(.+)") )
      end
    end

    default_config.user = create_body.User or nil
    default_config.command = create_body.Cmd and type(create_body.Cmd) == "table" and table.concat(create_body.Cmd, " ") or nil
    default_config.advance = 1
    default_config.cpus = create_body.HostConfig.NanoCPUs
    default_config.cpushares =  create_body.HostConfig.CpuShares
    default_config.memory = create_body.HostConfig.Memory
    default_config.blkioweight = create_body.HostConfig.BlkioWeight

    if create_body.HostConfig.Devices and type(create_body.HostConfig.Devices) == "table" then
      default_config.device = {}
      for _, v in ipairs(create_body.HostConfig.Devices) do
        table.insert( default_config.device, v.PathOnHost..":"..v.PathInContainer..(v.CgroupPermissions ~= "" and (":" .. v.CgroupPermissions) or "") )
      end
    end

    default_config.tmpfs = create_body.HostConfig.Tmpfs
  end
end

local m = SimpleForm("docker", translate("Docker"))
m.template = "docker/cbi/xsimpleform"
m.redirect = luci.dispatcher.build_url("admin", "services","docker", "containers")
-- m.reset = false
-- m.submit = false
-- new Container

docker_status = m:section(SimpleSection)
docker_status.template="docker/apply_widget"
docker_status.err=nixio.fs.readfile(dk.options.status_path)
if docker_status.err then docker:clear_status() end

local s = m:section(SimpleSection, translate("New Container"))
s.addremove = true
s.anonymous = true

local d = s:option(DummyValue,"cmd_line", translate("Resolv CLI"))
d.rawhtml  = true
d.template = "docker/resolv_container"

d = s:option(Value, "name", translate("Container Name"))
d.rmempty = true
d.default = default_config.name or nil

d = s:option(Flag, "interactive", translate("Interactive (-i)"))
d.rmempty = true
d.disabled = 0
d.enabled = 1
d.default = default_config.interactive and 1 or 0

d = s:option(Flag, "tty", translate("TTY (-t)"))
d.rmempty = true
d.disabled = 0
d.enabled = 1
d.default = default_config.tty and 1 or 0

d = s:option(Value, "image", translate("Docker Image"))
d.rmempty = true
d.default = default_config.image or nil
for _, v in ipairs (images) do
  if v.RepoTags then
    d:value(v.RepoTags[1], v.RepoTags[1])
  end
end

d = s:option(Flag, "_force_pull", translate("Always pull image first"))
d.rmempty = true
d.disabled = 0
d.enabled = 1
d.default = 0

d = s:option(Flag, "privileged", translate("Privileged"))
d.rmempty = true
d.disabled = 0
d.enabled = 1
d.default = default_config.privileged and 1 or 0

d = s:option(ListValue, "restart", translate("Restart Policy"))
d.rmempty = true

d:value("no", "No")
d:value("unless-stopped", "Unless stopped")
d:value("always", "Always")
d:value("on-failure", "On failure")
d.default = default_config.restart or "unless-stopped"

local d_network = s:option(ListValue, "network", translate("Networks"))
d_network.rmempty = true
d_network.default = default_config.network or "bridge"

local d_ip = s:option(Value, "ip", translate("IPv4 Address"))
d_ip.datatype="ip4addr"
d_ip:depends("network", "nil")
d_ip.default = default_config.ip or nil

d = s:option(DynamicList, "link", translate("Links with other containers"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "container_name:alias"
d.rmempty = true
d:depends("network", "bridge")
d.default = default_config.link or nil

d = s:option(DynamicList, "dns", translate("Set custom DNS servers"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "8.8.8.8"
d.rmempty = true
d.default = default_config.dns or nil

d = s:option(Value, "user", translate("User(-u)"), translate("The user that commands are run as inside the container.(format: name|uid[:group|gid])"))
d.placeholder = "1000:1000"
d.rmempty = true
d.default = default_config.user or nil

d = s:option(DynamicList, "env", translate("Environmental Variable(-e)"), translate("Set environment variables to inside the container"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "TZ=Asia/Shanghai"
d.rmempty = true
d.default = default_config.env or nil

d = s:option(DynamicList, "mount", translate("Bind Mount(-v)"), translate("Bind mount a volume"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "/media:/media:slave"
d.rmempty = true
d.default = default_config.mount or nil

local d_ports = s:option(DynamicList, "port", translate("Exposed Ports(-p)"), translate("Publish container's port(s) to the host"))
d_ports.template = "docker/cbi/xdynlist"
d_ports.placeholder = "2200:22/tcp"
d_ports.rmempty = true
d_ports.default = default_config.port or nil

d = s:option(Value, "command", translate("Run command"))
d.placeholder = "/bin/sh init.sh"
d.rmempty = true
d.default = default_config.command or nil

d = s:option(Flag, "advance", translate("Advance"))
d.rmempty = true
d.disabled = 0
d.enabled = 1
d.default = default_config.advance or 0

d = s:option(Value, "hostname", translate("Host Name"))
d.rmempty = true
d.default = default_config.hostname or nil
d:depends("advance", 1)

d = s:option(DynamicList, "device", translate("Device(--device)"), translate("Add host device to the container"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "/dev/sda:/dev/xvdc:rwm"
d.rmempty = true
d:depends("advance", 1)
d.default = default_config.device or nil

d = s:option(DynamicList, "tmpfs", translate("Tmpfs(--tmpfs)"), translate("Mount tmpfs directory"))
d.template = "docker/cbi/xdynlist"
d.placeholder = "/run:rw,noexec,nosuid,size=65536k"
d.rmempty = true
d:depends("advance", 1)
d.default = default_config.tmpfs or nil

d = s:option(Value, "cpus", translate("CPUs"), translate("Number of CPUs. Number is a fractional number. 0.000 means no limit."))
d.placeholder = "1.5"
d.rmempty = true
d:depends("advance", 1)
d.datatype="ufloat"
d.default = default_config.cpus or nil

d = s:option(Value, "cpushares", translate("CPU Shares Weight"), translate("CPU shares relative weight, if 0 is set, the system will ignore the value and use the default of 1024."))
d.placeholder = "1024"
d.rmempty = true
d:depends("advance", 1)
d.datatype="uinteger"
d.default = default_config.cpushares or nil

d = s:option(Value, "memory", translate("Memory"), translate("Memory limit (format: <number>[<unit>]). Number is a positive integer. Unit can be one of b, k, m, or g. Minimum is 4M."))
d.placeholder = "128m"
d.rmempty = true
d:depends("advance", 1)
d.default = default_config.memory or nil

d = s:option(Value, "blkioweight", translate("Block IO Weight"), translate("Block IO weight (relative weight) accepts a weight value between 10 and 1000."))
d.placeholder = "500"
d.rmempty = true
d:depends("advance", 1)
d.datatype="uinteger"
d.default = default_config.blkioweight or nil


for _, v in ipairs (networks) do
  if v.Name then
    local parent = v.Options and v.Options.parent or nil
    local ip = v.IPAM and v.IPAM.Config and v.IPAM.Config[1] and v.IPAM.Config[1].Subnet or nil
    ipv6 =  v.IPAM and v.IPAM.Config and v.IPAM.Config[2] and v.IPAM.Config[2].Subnet or nil
    local network_name = v.Name .. " | " .. v.Driver  .. (parent and (" | " .. parent) or "") .. (ip and (" | " .. ip) or "").. (ipv6 and (" | " .. ipv6) or "")
    d_network:value(v.Name, network_name)

    if v.Name ~= "none" and v.Name ~= "bridge" and v.Name ~= "host" then
      d_ip:depends("network", v.Name)
    end

    if v.Driver == "bridge" then
      d_ports:depends("network", v.Name)
    end
  end
end

m.handle = function(self, state, data)
  if state ~= FORM_VALID then return end
  local tmp
  local name = data.name or ("luci_" .. os.date("%Y%m%d%H%M%S"))
  local hostname = data.hostname
  local tty = type(data.tty) == "number" and (data.tty == 1 and true or false) or default_config.tty or false
  local interactive = type(data.interactive) == "number" and (data.interactive == 1 and true or false) or default_config.interactive or false
  local image = data.image
  local user = data.user
  if image and not image:match(".-:.+") then
    image = image .. ":latest"
  end
  local privileged = type(data.privileged) == "number" and (data.privileged == 1 and true or false) or default_config.privileged or false
  local restart = data.restart
  local env = data.env
  local dns = data.dns
  local network = data.network
  local ip = (network ~= "bridge" and network ~= "host" and network ~= "none") and data.ip or nil
  local mount = data.mount
  local memory = data.memory or 0
  local cpushares = data.cpushares or 0
  local cpus = data.cpus or 0
  local blkioweight = data.blkioweight or 500

  local portbindings = {}
  local exposedports = {}
  local tmpfs = {}
  tmp = data.tmpfs
  if type(tmp) == "table" then
    for i, v in ipairs(tmp)do
      local _,_, k,v1 = v:find("(.-):(.+)")
      if k and v1 then
        tmpfs[k]=v1
      end
    end
  end

  local device = {}
  tmp = data.device
  if type(tmp) == "table" then
    for i, v in ipairs(tmp) do
      local t = {}
      local _,_, h, c, p = v:find("(.-):(.-):(.+)")
      if h and c then
        t['PathOnHost'] = h
        t['PathInContainer'] = c
        t['CgroupPermissions'] = p or "rwm"
      else
        local _,_, h, c = v:find("(.-):(.+)")
        if h and c then
          t['PathOnHost'] = h
          t['PathInContainer'] = c
          t['CgroupPermissions'] = "rwm"
        end
      end
      if next(t) ~= nil then
        table.insert( device, t )
      end
    end
  end

  tmp = data.port or {}
  for i, v in ipairs(tmp) do
    for v1 ,v2 in string.gmatch(v, "(%d+):([^%s]+)") do
      local _,_,p= v2:find("^%d+/(%w+)")
      if p == nil then
        v2=v2..'/tcp'
      end
      portbindings[v2] = {{HostPort=v1}}
      exposedports[v2] = {HostPort=v1}
    end
  end

  local link = data.link
  tmp = data.command
  local command = {}
  if tmp ~= nil then
    for v in string.gmatch(tmp, "[^%s]+") do
      command[#command+1] = v
    end 
  end
  if memory ~= 0 then
    _,_,n,unit = memory:find("([%d%.]+)([%l%u]+)")
    if n then
      unit = unit and unit:sub(1,1):upper() or "B"
      if  unit == "M" then
        memory = tonumber(n) * 1024 * 1024
      elseif unit == "G" then
        memory = tonumber(n) * 1024 * 1024 * 1024
      elseif unit == "K" then
        memory = tonumber(n) * 1024
      else
        memory = tonumber(n)
      end
    end
  end

  create_body.Hostname = network ~= "host" and (hostname or name) or nil
  create_body.Tty = tty and true or false
  create_body.OpenStdin = interactive and true or false
  create_body.User = user
  create_body.Cmd = (#command ~= 0) and command or nil
  create_body.Env = env
  create_body.Image = image
  create_body.ExposedPorts = (next(exposedports) ~= nil) and exposedports or nil
  create_body.HostConfig = create_body.HostConfig or {}
  create_body.HostConfig.Dns = dns
  create_body.HostConfig.Binds = (#mount ~= 0) and mount or nil
  create_body.HostConfig.RestartPolicy = { Name = restart, MaximumRetryCount = 0 }
  create_body.HostConfig.Privileged = privileged and true or false
  create_body.HostConfig.PortBindings = (next(portbindings) ~= nil) and portbindings or nil
  create_body.HostConfig.Memory = tonumber(memory)
  create_body.HostConfig.CpuShares = tonumber(cpushares)
  create_body.HostConfig.NanoCPUs = tonumber(cpus) * 10 ^ 9
  create_body.HostConfig.BlkioWeight = tonumber(blkioweight)
  if create_body.HostConfig.NetworkMode ~= network then
    -- network mode changed, need to clear duplicate config
    create_body.NetworkingConfig = nil
  end
  create_body.HostConfig.NetworkMode = network
  if ip then
    if create_body.NetworkingConfig and create_body.NetworkingConfig.EndpointsConfig and type(create_body.NetworkingConfig.EndpointsConfig) == "table" then
      -- ip + duplicate config
      for k, v in pairs (create_body.NetworkingConfig.EndpointsConfig) do
        if k == network and v.IPAMConfig and v.IPAMConfig.IPv4Address then
          v.IPAMConfig.IPv4Address = ip
        else
          create_body.NetworkingConfig.EndpointsConfig = { [network] = { IPAMConfig = { IPv4Address = ip } } }
        end
        break
      end
    else
      -- ip + no duplicate config
      create_body.NetworkingConfig = { EndpointsConfig = { [network] = { IPAMConfig = { IPv4Address = ip } } } }
    end
  elseif not create_body.NetworkingConfig then
    -- no ip + no duplicate config
    create_body.NetworkingConfig = nil
  end

  create_body["HostConfig"]["Tmpfs"] = (next(tmpfs) ~= nil) and tmpfs or nil
  create_body["HostConfig"]["Devices"] = (next(device) ~= nil) and device or nil

  if network == "bridge" and next(link) ~= nil then
    create_body["HostConfig"]["Links"] = link
  end
  local pull_image = function(image)
    local server = "index.docker.io"
    local json_stringify = luci.json and luci.json.encode or luci.jsonc.stringify
    docker:append_status("Images: " .. "pulling" .. " " .. image .. "...")
    local x_auth = nixio.bin.b64encode(json_stringify({serveraddress= server}))
    local res = dk.images:create(nil, {fromImage=image,_header={["X-Registry-Auth"]=x_auth}})
    if res and res.code == 200 then
      docker:append_status("done<br>")
    else
      docker:append_status("fail code:" .. res.code.." ".. (res.body.message and res.body.message or res.message).. "<br>")
      luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/newcontainer"))
    end
  end
  docker:clear_status()
  local exist_image = false
  if image then
    for _, v in ipairs (images) do
      if v.RepoTags and v.RepoTags[1] == image then
        exist_image = true
        break
      end
    end
    if not exist_image then
      pull_image(image)
    elseif data._force_pull == 1 then
      pull_image(image)
    end
  end

  docker:append_status("Container: " .. "create" .. " " .. name .. "...")
  local res = dk.containers:create(name, nil, create_body)
  if res and res.code == 201 then
    docker:clear_status()
    luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/containers"))
  else
    docker:append_status("fail code:" .. res.code.." ".. (res.body.message and res.body.message or res.message))
    luci.http.redirect(luci.dispatcher.build_url("admin/services/docker/newcontainer"))
  end
end

return m
