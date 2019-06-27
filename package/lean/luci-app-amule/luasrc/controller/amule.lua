--[[
LuCI - Lua Configuration Interface - amule support

Copyright 2016 maz-1 <ohmygod19993@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

module("luci.controller.amule", package.seeall)
local uci     = luci.model.uci.cursor()
local configdir = uci:get("amule", "main", "config_dir")

function index()
	if not nixio.fs.access("/etc/config/amule") then
		return
	end

	entry({"admin", "nas"}, firstchild(), "NAS", 45).dependent = false
        
	local page = entry({"admin", "nas", "amule"}, cbi("amule"), _("aMule Settings"))
	page.dependent = true
	entry( {"admin", "nas", "amule", "logview"}, call("logread") ).leaf = true
	entry( {"admin", "nas", "amule", "status"}, call("get_pid") ).leaf = true
	entry( {"admin", "nas", "amule", "amulecmd"}, call("amulecmd") ).leaf = true
	entry( {"admin", "nas", "amule", "startstop"}, post("startstop") ).leaf = true
	entry( {"admin", "nas", "amule", "down_kad"}, post("down_kad") ).leaf = true
	entry( {"admin", "nas", "amule", "down_ed2k"}, post("down_ed2k") ).leaf = true

end

-- called by XHR.get from detail_logview.htm
function logread()
	-- read application settings
	local uci     = luci.model.uci.cursor()
	local logdir  = uci:get("amule", "main", "config_dir") or "/var/run/amule"
	uci:unload("amule")

	local ldata=nixio.fs.readfile(logdir .. "/logfile")
	if not ldata or #ldata == 0 then
		ldata="_nodata_"
	end
	luci.http.write(ldata)
end

-- called by XHR.get from detail_startstop.htm
function startstop()
	local pid = get_pid(true)
	if pid > 0 then
		luci.sys.call("/etc/init.d/amule stop")
		nixio.nanosleep(1)		-- sleep a second
		if nixio.kill(pid, 0) then	-- still running
			nixio.kill(pid, 9)	-- send SIGKILL
		end
		pid = 0
	else
		luci.sys.call("/etc/init.d/amule start")
		nixio.nanosleep(1)		-- sleep a second
		pid = tonumber(luci.sys.exec("pidof amuled")) or 0 
		if pid > 0 and not nixio.kill(pid, 0) then
			pid = 0		-- process did not start
		end
	end
	luci.http.write(tostring(pid))	-- HTTP needs string not number
end

function down_kad()
	url = uci:get("amule", "main", "kad_nodes_url")
        data_path = configdir .. "/nodes.dat"
        proto = string.gsub(url, "://%S*", "")
        proto_opt = ( proto == "https" ) and " --no-check-certificate" or ""
        cmd = "wget -O /tmp/down_nodes.dat \"" .. url .. "\"" .. proto_opt ..
                " && cat /tmp/down_nodes.dat > " .. "\"" .. data_path .. "\""
        luci.sys.call(cmd)
end

function down_ed2k()
	url = uci:get("amule", "main", "ed2k_servers_url")
	data_path = configdir .. "/server.met"
        proto = string.gsub(url, "://%S*", "")
        proto_opt = ( proto == "https" ) and " --no-check-certificate" or ""
        cmd = "wget -O /tmp/down_server.met \"" .. url .. "\"" .. proto_opt ..
                " && cat /tmp/down_server.met > " .. "\"" .. data_path .. "\""
        luci.sys.call(cmd)
end

-- called by XHR.poll from detail_startstop.htm
-- and from lua (with parameter "true")
function get_pid(from_lua)
	local pid_amuled = tonumber(luci.sys.exec("pidof amuled")) or 0 
        local amuled_stat =false
	if pid_amuled > 0 and not nixio.kill(pid_amuled, 0) then
		pid_amuled = 0
	end
        
        if pid_amuled > 0 then
            amuled_stat =true
        else
            amuled_stat =false
        end
        
	local pid_amuleweb = tonumber(luci.sys.exec("pidof amuleweb")) or 0 
        local amuleweb_stat = false
	if pid_amuleweb > 0 and not nixio.kill(pid_amuleweb, 0) then
		pid_amuleweb = 0
	end
        
        if pid_amuleweb > 0 then
            amuleweb_stat =true
        else
            amuleweb_stat =false
        end
        
	local status = {
		amuled = amuled_stat,
		amuled_pid = pid_amuled,
		amuleweb = amuleweb_stat
	}
        
	if from_lua then
		return pid_amuled
	else
		luci.http.prepare_content("application/json")
		luci.http.write_json(status)	
	end
end


function amulecmd()
	local re =""
	local rv   = { }
	local cmd = luci.http.formvalue("cmd")
	local full_cmd = "HOME=\""..configdir.. "\" /usr/bin/amulecmd".." -c \""..cmd.."\" 2>&1"

	local shellpipe = io.popen(full_cmd,"rw")
	re = shellpipe:read("*a")
	shellpipe:close()
	if not re then 
		re=""
	end
	
	re = string.gsub(re, "This is amulecmd %S*\n", "")
	re = string.gsub(re, "Creating client%S*\n", "")
	re = string.gsub(re, "Succeeded! Connection established to aMule %S*\n", "")
	
	re = string.gsub(re, "\n", "\r\n")
	
	rv[#rv+1]=re
	
	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
				return
	end

	luci.http.status(404, "No such device")
end
