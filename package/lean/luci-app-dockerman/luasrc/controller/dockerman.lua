--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--
require "luci.util"
local docker = require "luci.model.docker"
local uci = require "luci.model.uci"

module("luci.controller.dockerman",package.seeall)

function index()

  entry({"admin", "services","docker"}, firstchild(), "Docker", 40).dependent = false
  entry({"admin","services","docker","overview"},cbi("dockerman/overview"),_("Overview"),0).leaf=true

  local socket = luci.model.uci.cursor():get("dockerman", "local", "socket_path")
  if not nixio.fs.access(socket) then return end
  if (require "luci.model.docker").new():_ping().code ~= 200 then return end
  entry({"admin","services","docker","containers"},form("dockerman/containers"),_("Containers"),1).leaf=true
  entry({"admin","services","docker","images"},form("dockerman/images"),_("Images"),2).leaf=true
  entry({"admin","services","docker","networks"},form("dockerman/networks"),_("Networks"),3).leaf=true
  entry({"admin","services","docker","volumes"},form("dockerman/volumes"),_("Volumes"),4).leaf=true
  entry({"admin","services","docker","events"},call("action_events"),_("Events"),5)
  entry({"admin","services","docker","newcontainer"},form("dockerman/newcontainer")).leaf=true
  entry({"admin","services","docker","newnetwork"},form("dockerman/newnetwork")).leaf=true
  entry({"admin","services","docker","container"},form("dockerman/container")).leaf=true
  entry({"admin","services","docker","container_stats"},call("action_get_container_stats")).leaf=true
  entry({"admin","services","docker","container_get_archive"},call("download_archive")).leaf=true
  entry({"admin","services","docker","container_put_archive"},call("upload_archive")).leaf=true
  entry({"admin","services","docker","confirm"},call("action_confirm")).leaf=true

end


function action_events()
  local logs = ""
  local dk = docker.new()
  local query ={}
  query["until"] = os.time()
	local events = dk:events({query = query})
  for _, v in ipairs(events.body) do
    if v.Type == "container" then
      logs = (logs ~= "" and (logs .. "\n") or logs) .. "[" .. os.date("%Y-%m-%d %H:%M:%S", v.time) .."] "..v.Type.. " " .. (v.Action or "null") .. " Container ID:"..  (v.Actor.ID or "null") .. " Container Name:" .. (v.Actor.Attributes.name or "null")
    elseif v.Type == "network" then
      logs = (logs ~= "" and (logs .. "\n") or logs) .. "[" .. os.date("%Y-%m-%d %H:%M:%S", v.time) .."] "..v.Type.. " " .. v.Action .. " Container ID:"..( v.Actor.Attributes.container or "null" ) .. " Network Name:" .. (v.Actor.Attributes.name or "null") .. " Network type:".. v.Actor.Attributes.type or ""
    elseif v.Type == "image" then
      logs = (logs ~= "" and (logs .. "\n") or logs) .. "[" .. os.date("%Y-%m-%d %H:%M:%S", v.time) .."] "..v.Type.. " " .. v.Action .. " Image:".. (v.Actor.ID or "null").. " Image Name:" .. (v.Actor.Attributes.name or "null")
    end
  end
  luci.template.render("dockerman/logs", {self={syslog = logs, title="Docker Events"}})
end

local calculate_cpu_percent = function(d)
  if type(d) ~= "table" then return end
   cpu_count = tonumber(d["cpu_stats"]["online_cpus"])
   cpu_percent = 0.0
   cpu_delta = tonumber(d["cpu_stats"]["cpu_usage"]["total_usage"]) - tonumber(d["precpu_stats"]["cpu_usage"]["total_usage"])
   system_delta = tonumber(d["cpu_stats"]["system_cpu_usage"]) - tonumber(d["precpu_stats"]["system_cpu_usage"])
  if system_delta > 0.0 then
    cpu_percent = string.format("%.2f", cpu_delta / system_delta * 100.0 * cpu_count)
  end
  -- return cpu_percent .. "%"
  return cpu_percent
end

local get_memory = function(d)
  if type(d) ~= "table" then return end
  -- local limit = string.format("%.2f", tonumber(d["memory_stats"]["limit"]) / 1024 / 1024)
  -- local usage = string.format("%.2f", (tonumber(d["memory_stats"]["usage"]) - tonumber(d["memory_stats"]["stats"]["total_cache"])) / 1024 / 1024)
  -- return usage .. "MB / " .. limit.. "MB" 
  local limit =tonumber(d["memory_stats"]["limit"])
  local usage = tonumber(d["memory_stats"]["usage"]) - tonumber(d["memory_stats"]["stats"]["total_cache"])
  return usage, limit
end

local get_rx_tx = function(d)
  if type(d) ~="table" then return end
  -- local data
  -- if type(d["networks"]) == "table" then
  --   for e, v in pairs(d["networks"]) do
  --     data = (data and (data .. "<br>") or "") .. e .. "  Total Tx:" .. string.format("%.2f",(tonumber(v.tx_bytes)/1024/1024)) .. "MB  Total Rx: ".. string.format("%.2f",(tonumber(v.rx_bytes)/1024/1024)) .. "MB"
  --   end
  -- end
  local data = {}
  if type(d["networks"]) == "table" then
    for e, v in pairs(d["networks"]) do
      data[e] = {
        bw_tx = tonumber(v.tx_bytes),
        bw_rx = tonumber(v.rx_bytes)
      }
    end
  end
  return data
end

function action_get_container_stats(container_id)
  if container_id then
    local dk = docker.new()
    local response = dk.containers:inspect({id = container_id})
    if response.code == 200 and response.body.State.Running then
      response = dk.containers:stats({id = container_id, query = {stream = false}})
      if response.code == 200 then
        local container_stats = response.body
        local cpu_percent = calculate_cpu_percent(container_stats)
        local mem_useage, mem_limit = get_memory(container_stats)
        local bw_rxtx = get_rx_tx(container_stats)
        luci.http.status(response.code, response.body.message)
        luci.http.prepare_content("application/json")
        luci.http.write_json({ 
          cpu_percent = cpu_percent,
          memory = {
            mem_useage = mem_useage,
            mem_limit = mem_limit
          },
          bw_rxtx = bw_rxtx
        })
      else
        luci.http.status(response.code, response.body.message)
        luci.http.prepare_content("text/plain")
        luci.http.write(response.body.message)
      end
    else
      if response.code == 200 then
        luci.http.status(500, "container "..container_id.." not running")
        luci.http.prepare_content("text/plain")
        luci.http.write("Container "..container_id.." not running")
      else
        luci.http.status(response.code, response.body.message)
        luci.http.prepare_content("text/plain")
        luci.http.write(response.body.message)
      end
    end
  else
    luci.http.status(404, "No container name or id")
    luci.http.prepare_content("text/plain")
		luci.http.write("No container name or id")
  end
end

function action_confirm()
  local status_path=luci.model.uci.cursor():get("dockerman", "local", "status_path")
  local data = nixio.fs.readfile(status_path)
  if data then
    code = 202
    msg = data
  else
    code = 200
    msg = "finish"
    data = "finish"
  end
  -- luci.util.perror(data)
  luci.http.status(code, msg)
  luci.http.prepare_content("application/json")
  luci.http.write_json({info = data})
end

function download_archive()
  local id = luci.http.formvalue("id")
  local path = luci.http.formvalue("path")
  local dk = docker.new()
  local first

  local cb = function(res, chunk)
    if res.code == 200 then
      if not first then
        first = true
        luci.http.header('Content-Disposition', 'inline; filename="archive.tar"')
        luci.http.header('Content-Type', 'application\/x-tar')
      end
      luci.ltn12.pump.all(chunk, luci.http.write)
    else
      if not first then
        first = true
        luci.http.prepare_content("text/plain")
      end
      luci.ltn12.pump.all(chunk, luci.http.write)
    end
  end

  local res = dk.containers:get_archive({id = id, query = {path = path}}, cb)
end

function upload_archive(container_id)
  local path = luci.http.formvalue("upload-path")
  local dk = docker.new()
  local ltn12 = require "luci.ltn12"

  rec_send = function(sinkout)
    luci.http.setfilehandler(function (meta, chunk, eof)
      if chunk then
        ltn12.pump.step(ltn12.source.string(chunk), sinkout)
      end
    end)
  end

  local res = dk.containers:put_archive({id = container_id, query = {path = path}, body = rec_send})
  local msg = res and res.body and res.body.message or nil
  luci.http.status(res.code, msg)
  luci.http.prepare_content("application/json")
  luci.http.write_json({message = msg})
end
