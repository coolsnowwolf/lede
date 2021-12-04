--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-dockerman>
]]--

local docker = require "luci.model.docker"
-- local uci = (require "luci.model.uci").cursor()

module("luci.controller.dockerman",package.seeall)

function index()
	entry({"admin", "docker"},
		alias("admin", "docker", "config"),
		_("Docker"),
		40).acl_depends = { "luci-app-dockerman" }

	entry({"admin", "docker", "config"},cbi("dockerman/configuration"),_("Configuration"), 8).leaf=true

	-- local uci = (require "luci.model.uci").cursor()
	-- if uci:get_bool("dockerd", "dockerman", "remote_endpoint") then
	-- 	local host = uci:get("dockerd", "dockerman", "remote_host")
	-- 	local port = uci:get("dockerd", "dockerman", "remote_port")
	-- 	if not host or not port then
	-- 		return
	-- 	end
	-- else
	-- 	local socket = uci:get("dockerd", "dockerman", "socket_path") or "/var/run/docker.sock"
	-- 	if socket and not nixio.fs.access(socket) then
	-- 		return
	-- 	end
	-- end

	-- if (require "luci.model.docker").new():_ping().code ~= 200 then
	-- 	return
	-- end

	entry({"admin", "docker", "overview"}, form("dockerman/overview"),_("Overview"), 2).leaf=true
	entry({"admin", "docker", "containers"}, form("dockerman/containers"), _("Containers"), 3).leaf=true
	entry({"admin", "docker", "images"}, form("dockerman/images"), _("Images"), 4).leaf=true
	entry({"admin", "docker", "networks"}, form("dockerman/networks"), _("Networks"), 5).leaf=true
	entry({"admin", "docker", "volumes"}, form("dockerman/volumes"), _("Volumes"), 6).leaf=true
	entry({"admin", "docker", "events"}, call("action_events"), _("Events"), 7)

	entry({"admin", "docker", "newcontainer"}, form("dockerman/newcontainer")).leaf=true
	entry({"admin", "docker", "newnetwork"}, form("dockerman/newnetwork")).leaf=true
	entry({"admin", "docker", "container"}, form("dockerman/container")).leaf=true

	entry({"admin", "docker", "call"}, call("action_call_docker")).leaf=true

	entry({"admin", "docker", "container_stats"}, call("action_get_container_stats")).leaf=true
	entry({"admin", "docker", "containers_stats"}, call("action_get_containers_stats")).leaf=true
	entry({"admin", "docker", "get_system_df"}, call("action_get_system_df")).leaf=true
	entry({"admin", "docker", "container_get_archive"}, call("download_archive")).leaf=true
	entry({"admin", "docker", "container_put_archive"}, call("upload_archive")).leaf=true
	entry({"admin", "docker", "container_list_file"}, call("list_file")).leaf=true
	entry({"admin", "docker", "container_remove_file"}, call("remove_file")).leaf=true
	entry({"admin", "docker", "container_rename_file"}, call("rename_file")).leaf=true
	entry({"admin", "docker", "container_export"}, call("export_container")).leaf=true
	entry({"admin", "docker", "images_save"}, call("save_images")).leaf=true
	entry({"admin", "docker", "images_load"}, call("load_images")).leaf=true
	entry({"admin", "docker", "images_import"}, call("import_images")).leaf=true
	entry({"admin", "docker", "images_get_tags"}, call("get_image_tags")).leaf=true
	entry({"admin", "docker", "images_tag"}, call("tag_image")).leaf=true
	entry({"admin", "docker", "images_untag"}, call("untag_image")).leaf=true
	entry({"admin", "docker", "confirm"}, call("action_confirm")).leaf=true
end

function action_call_docker()

end

function action_get_system_df()
	local res = docker.new():df()
	luci.http.status(res.code, res.message)
	luci.http.prepare_content("application/json")
	luci.http.write_json(res.body)
end

function scandir(id, directory)
	local cmd_docker = luci.util.exec("command -v docker"):match("^.+docker") or nil
	if not cmd_docker or cmd_docker:match("^%s+$") then
		return
	end
	local i, t, popen = 0, {}, io.popen
	local uci = (require "luci.model.uci").cursor()
	local remote = uci:get_bool("dockerd", "dockerman", "remote_endpoint")
	local socket_path = not remote and uci:get("dockerd", "dockerman", "socket_path") or nil
	local host = remote and uci:get("dockerd", "dockerman", "remote_host") or nil
	local port = remote and uci:get("dockerd", "dockerman", "remote_port") or nil
	if remote and host and port then
		hosts = "tcp://" .. host .. ':'.. port
	elseif socket_path then
		hosts = "unix://" .. socket_path
	else
		return
	end
	local pfile = popen(cmd_docker .. ' -H "'.. hosts ..'" exec ' ..id .." ls -lh \""..directory.."\" | egrep -v '^total'")
	for fileinfo in pfile:lines() do
			i = i + 1
			t[i] = fileinfo
	end
	pfile:close()
	return t
end

function list_response(id, path, success)
	luci.http.prepare_content("application/json")
	local result
	if success then
			local rv = scandir(id, path)
			result = {
					ec = 0,
					data = rv
			}
	else
			result = {
					ec = 1
			}
	end
	luci.http.write_json(result)
end

function list_file(id)
	local path = luci.http.formvalue("path")
	list_response(id, path, true)
end

function rename_file(id)
	local filepath = luci.http.formvalue("filepath")
	local newpath = luci.http.formvalue("newpath")
	local cmd_docker = luci.util.exec("command -v docker"):match("^.+docker") or nil
	if not cmd_docker or cmd_docker:match("^%s+$") then
		return
	end
	local uci = (require "luci.model.uci").cursor()
	local remote = uci:get_bool("dockerd", "dockerman", "remote_endpoint")
	local socket_path = not remote and  uci:get("dockerd", "dockerman", "socket_path") or nil
	local host = remote and uci:get("dockerd", "dockerman", "remote_host") or nil
	local port = remote and uci:get("dockerd", "dockerman", "remote_port") or nil
	if remote and host and port then
		hosts = "tcp://" .. host .. ':'.. port
	elseif socket_path then
		hosts = "unix://" .. socket_path
	else
		return
	end
	local success = os.execute(cmd_docker .. ' -H "'.. hosts ..'" exec '.. id ..' mv "'..filepath..'" "'..newpath..'"')
	list_response(nixio.fs.dirname(filepath), success)
end

function remove_file(id)
	local path = luci.http.formvalue("path")
	local isdir = luci.http.formvalue("isdir")
	local cmd_docker = luci.util.exec("command -v docker"):match("^.+docker") or nil
	if not cmd_docker or cmd_docker:match("^%s+$") then
		return
	end
	local uci = (require "luci.model.uci").cursor()
	local remote = uci:get_bool("dockerd", "dockerman", "remote_endpoint")
	local socket_path = not remote and  uci:get("dockerd", "dockerman", "socket_path") or nil
	local host = remote and uci:get("dockerd", "dockerman", "remote_host") or nil
	local port = remote and uci:get("dockerd", "dockerman", "remote_port") or nil
	if remote and host and port then
		hosts = "tcp://" .. host .. ':'.. port
	elseif socket_path then
		hosts = "unix://" .. socket_path
	else
		return
	end
	path = path:gsub("<>", "/")
	path = path:gsub(" ", "\ ")
	local success
	if isdir then
			success = os.execute(cmd_docker .. ' -H "'.. hosts ..'" exec '.. id ..' rm -r "'..path..'"')
	else
			success = os.remove(path)
	end
	list_response(nixio.fs.dirname(path), success)
end

function action_events()
	local logs = ""
	local query ={}

	local dk = docker.new()
	query["until"] = os.time()
	local events = dk:events({query = query})

	if events.code == 200 then
		for _, v in ipairs(events.body) do
			local date = "unknown"
			if v and v.time then
				date = os.date("%Y-%m-%d %H:%M:%S", v.time)
			end

			local name = v.Actor.Attributes.name or "unknown"
			local action = v.Action or "unknown"

			if v and v.Type == "container" then
				local id = v.Actor.ID or "unknown"
				logs = logs .. string.format("[%s] %s %s Container ID: %s Container Name: %s\n", date, v.Type, action, id, name)
			elseif v.Type == "network" then
				local container = v.Actor.Attributes.container or "unknown"
				local network = v.Actor.Attributes.type or "unknown"
				logs = logs .. string.format("[%s] %s %s Container ID: %s Network Name: %s Network type: %s\n", date, v.Type, action, container, name, network)
			elseif v.Type == "image" then
				local id = v.Actor.ID or "unknown"
				logs = logs .. string.format("[%s] %s %s Image: %s Image name: %s\n", date, v.Type, action, id, name)
			end
		end
	end

	luci.template.render("dockerman/logs", {self={syslog = logs, title="Events"}})
end

local calculate_cpu_percent = function(d)
	if type(d) ~= "table" then
		return
	end

	local cpu_count = tonumber(d["cpu_stats"]["online_cpus"])
	local cpu_percent = 0.0
	local cpu_delta = tonumber(d["cpu_stats"]["cpu_usage"]["total_usage"]) - tonumber(d["precpu_stats"]["cpu_usage"]["total_usage"])
	local system_delta = tonumber(d["cpu_stats"]["system_cpu_usage"]) -- tonumber(d["precpu_stats"]["system_cpu_usage"])
	if system_delta > 0.0 then
		cpu_percent = string.format("%.2f", cpu_delta / system_delta * 100.0 * cpu_count)
	end

	return cpu_percent
end

local get_memory = function(d)
	if type(d) ~= "table" then
		return
	end

	-- local limit = string.format("%.2f", tonumber(d["memory_stats"]["limit"]) / 1024 / 1024)
	-- local usage = string.format("%.2f", (tonumber(d["memory_stats"]["usage"]) - tonumber(d["memory_stats"]["stats"]["total_cache"])) / 1024 / 1024)
	-- return usage .. "MB / " .. limit.. "MB"

	local limit =tonumber(d["memory_stats"]["limit"])
	local usage = tonumber(d["memory_stats"]["usage"])
	-- - tonumber(d["memory_stats"]["stats"]["total_cache"])

	return usage, limit
end

local get_rx_tx = function(d)
	if type(d) ~="table" then
		return
	end

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

local function get_stat(container_id)
	if container_id then
		local dk = docker.new()
		local response = dk.containers:inspect({id = container_id})
		if response.code == 200 and response.body.State.Running then
			response = dk.containers:stats({id = container_id, query = {stream = false,  ["one-shot"] = true}})
			if response.code == 200 then
				local container_stats = response.body
				local cpu_percent = calculate_cpu_percent(container_stats)
				local mem_useage, mem_limit = get_memory(container_stats)
				local bw_rxtx = get_rx_tx(container_stats)
				return response.code, response.body.message, {
					cpu_percent = cpu_percent,
					memory = {
						mem_useage = mem_useage,
						mem_limit = mem_limit
					},
					bw_rxtx = bw_rxtx
				}
			else
				return response.code, response.body.message
			end
		else
			if response.code == 200 then
				return 500, "container "..container_id.." not running"
			else
				return response.code, response.body.message
			end
		end
	else
		return 404, "No container name or id"
	end
end
function action_get_container_stats(container_id)
	local code, msg, res = get_stat(container_id)
	luci.http.status(code, msg)
	luci.http.prepare_content("application/json")
	luci.http.write_json(res)
end

function action_get_containers_stats()
	local res = luci.http.formvalue(containers) or ""
	local stats = {}
	res = luci.jsonc.parse(res.containers)
	if res and type(res) == "table" then
		for i, v in ipairs(res) do
			_,_,stats[v] = get_stat(v)
		end
	end
	luci.http.status(200, "OK")
	luci.http.prepare_content("application/json")
	luci.http.write_json(stats)
end

function action_confirm()
	local data = docker:read_status()
	if data then
		data = data:gsub("\n","<br>"):gsub(" ","&nbsp;")
		code = 202
		msg = data
	else
		code = 200
		msg = "finish"
		data = "finish"
	end

	luci.http.status(code, msg)
	luci.http.prepare_content("application/json")
	luci.http.write_json({info = data})
end

function export_container(id)
  local dk = docker.new()
  local first

  local cb = function(res, chunk)
    if res.code == 200 then
      if not first then
        first = true
        luci.http.header('Content-Disposition', 'inline; filename="'.. id ..'.tar"')
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

  local res = dk.containers:export({id = id}, cb)
end

function download_archive()
	local id = luci.http.formvalue("id")
	local path = luci.http.formvalue("path")
	local filename = luci.http.formvalue("filename") or "archive"
	local dk = docker.new()
	local first

	local cb = function(res, chunk)
		if res and res.code and res.code == 200 then
			if not first then
				first = true
				luci.http.header('Content-Disposition', 'inline; filename="'.. filename .. '.tar"')
				luci.http.header('Content-Type', 'application\/x-tar')
			end
			luci.ltn12.pump.all(chunk, luci.http.write)
		else
			if not first then
				first = true
				luci.http.status(res and res.code or 500, msg or "unknow")
				luci.http.prepare_content("text/plain")
			end
			luci.ltn12.pump.all(chunk, luci.http.write)
		end
	end

	local res = dk.containers:get_archive({
		id = id,
		query = {
			path = luci.http.urlencode(path)
		}
	}, cb)
end

function upload_archive(container_id)
	local path = luci.http.formvalue("upload-path")
	local dk = docker.new()
	local ltn12 = require "luci.ltn12"

	local rec_send = function(sinkout)
		luci.http.setfilehandler(function (meta, chunk, eof)
			if chunk then
				ltn12.pump.step(ltn12.source.string(chunk), sinkout)
			end
		end)
	end

	local res = dk.containers:put_archive({
		id = container_id,
		query = {
			path = luci.http.urlencode(path)
		},
		body = rec_send
	})

	local msg = res and res.body and res.body.message or nil
	luci.http.status(res and res.code or 500, msg or "unknow")
	luci.http.prepare_content("application/json")
	luci.http.write_json({message = msg or "unknow"})
end

-- function save_images()
-- 	local names = luci.http.formvalue("names")
-- 	local dk = docker.new()
-- 	local first

-- 	local cb = function(res, chunk)
-- 		if res.code == 200 then
-- 			if not first then
-- 				first = true
-- 				luci.http.status(res.code, res.message)
-- 				luci.http.header('Content-Disposition', 'inline; filename="'.. "images" ..'.tar"')
-- 				luci.http.header('Content-Type', 'application\/x-tar')
-- 			end
-- 			luci.ltn12.pump.all(chunk, luci.http.write)
-- 		else
-- 			if not first then
-- 				first = true
-- 				luci.http.prepare_content("text/plain")
-- 			end
-- 			luci.ltn12.pump.all(chunk, luci.http.write)
-- 		end
-- 	end

-- 	docker:write_status("Images: saving" .. " " .. names .. "...")
-- 	local res = dk.images:get({
-- 		query = {
-- 			names = luci.http.urlencode(names)
-- 		}
-- 	}, cb)
-- 	docker:clear_status()

-- 	local msg = res and res.body and res.body.message or nil
-- 	luci.http.status(res.code, msg)
-- 	luci.http.prepare_content("application/json")
-- 	luci.http.write_json({message = msg})
-- end

function load_images()
	local archive = luci.http.formvalue("upload-archive")
	local dk = docker.new()
	local ltn12 = require "luci.ltn12"

	local rec_send = function(sinkout)
		luci.http.setfilehandler(function (meta, chunk, eof)
			if chunk then
				ltn12.pump.step(ltn12.source.string(chunk), sinkout)
			end
		end)
	end

	docker:write_status("Images: loading...")
	local res = dk.images:load({body = rec_send})
	local msg = res and res.body and ( res.body.message or res.body.stream or res.body.error ) or nil
	if res and res.code == 200 and msg and msg:match("Loaded image ID") then
		docker:clear_status()
	else
		docker:append_status("code:" .. (res and res.code or "500") .." ".. (msg or "unknow"))
	end

	luci.http.status(res and res.code or 500, msg or "unknow")
	luci.http.prepare_content("application/json")
	luci.http.write_json({message = msg or "unknow"})
end

function import_images()
	local src = luci.http.formvalue("src")
	local itag = luci.http.formvalue("tag")
	local dk = docker.new()
	local ltn12 = require "luci.ltn12"

	local rec_send = function(sinkout)
		luci.http.setfilehandler(function (meta, chunk, eof)
			if chunk then
				ltn12.pump.step(ltn12.source.string(chunk), sinkout)
			end
		end)
	end

	docker:write_status("Images: importing".. " ".. itag .."...\n")
	local repo = itag and itag:match("^([^:]+)")
	local tag = itag and itag:match("^[^:]-:([^:]+)")
	local res = dk.images:create({
		query = {
			fromSrc = luci.http.urlencode(src or "-"),
			repo = repo or nil,
			tag = tag or nil
		},
		body = not src and rec_send or nil
	}, docker.import_image_show_status_cb)

	local msg = res and res.body and ( res.body.message )or nil
	if not msg and #res.body == 0 then
		msg = res.body.status or res.body.error
	elseif not msg and #res.body >= 1 then
		msg = res.body[#res.body].status or res.body[#res.body].error
	end

	if res.code == 200 and msg and msg:match("sha256:") then
		docker:clear_status()
	else
		docker:append_status("code:" .. (res and res.code or "500") .." ".. (msg or "unknow"))
	end

	luci.http.status(res and res.code or 500, msg or "unknow")
	luci.http.prepare_content("application/json")
	luci.http.write_json({message = msg or "unknow"})
end

function get_image_tags(image_id)
	if not image_id then
		luci.http.status(400, "no image id")
		luci.http.prepare_content("application/json")
		luci.http.write_json({message = "no image id"})
		return
	end

	local dk = docker.new()
	local res = dk.images:inspect({
		id = image_id
	})
	local msg = res and res.body and res.body.message or nil
	luci.http.status(res and res.code or 500, msg or "unknow")
	luci.http.prepare_content("application/json")

	if res.code == 200 then
		local tags = res.body.RepoTags
		luci.http.write_json({tags = tags})
	else
		local msg = res and res.body and res.body.message or nil
		luci.http.write_json({message = msg or "unknow"})
	end
end

function tag_image(image_id)
	local src = luci.http.formvalue("tag")
	local image_id = image_id or luci.http.formvalue("id")

	if type(src) ~= "string" or not image_id then
		luci.http.status(400, "no image id or tag")
		luci.http.prepare_content("application/json")
		luci.http.write_json({message = "no image id or tag"})
		return
	end

	local repo = src:match("^([^:]+)")
	local tag = src:match("^[^:]-:([^:]+)")
	local dk = docker.new()
	local res = dk.images:tag({
		id = image_id,
		query={
			repo=repo,
			tag=tag
		}
	})
	local msg = res and res.body and res.body.message or nil
	luci.http.status(res and res.code or 500, msg or "unknow")
	luci.http.prepare_content("application/json")

	if res.code == 201 then
		local tags = res.body.RepoTags
		luci.http.write_json({tags = tags})
	else
		local msg = res and res.body and res.body.message or nil
		luci.http.write_json({message = msg or "unknow"})
	end
end

function untag_image(tag)
	local tag = tag or luci.http.formvalue("tag")

	if not tag then
		luci.http.status(400, "no tag name")
		luci.http.prepare_content("application/json")
		luci.http.write_json({message = "no tag name"})
		return
	end

	local dk = docker.new()
	local res = dk.images:inspect({name = tag})

	if res.code == 200 then
		local tags = res.body.RepoTags
		if #tags > 1 then
			local r = dk.images:remove({name = tag})
			local msg = r and r.body and r.body.message or nil
			luci.http.status(r.code, msg)
			luci.http.prepare_content("application/json")
			luci.http.write_json({message = msg})
		else
			luci.http.status(500, "Cannot remove the last tag")
			luci.http.prepare_content("application/json")
			luci.http.write_json({message = "Cannot remove the last tag"})
		end
	else
		local msg = res and res.body and res.body.message or nil
		luci.http.status(res and res.code or 500, msg or "unknow")
		luci.http.prepare_content("application/json")
		luci.http.write_json({message = msg or "unknow"})
	end
end
