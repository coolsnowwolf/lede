module("luci.controller.v2ray_server",package.seeall)
local http = require "luci.http"
local v2ray  = require "luci.model.cbi.v2ray_server.api.v2ray"

function index()
	if not nixio.fs.access("/etc/config/v2ray_server")then
		return
	end
	entry({"admin","vpn"}, firstchild(), "VPN", 45).dependent = false
	entry({"admin","vpn","v2ray_server"},cbi("v2ray_server/index"),_("V2ray Server"),3).dependent=true
	entry({"admin","vpn","v2ray_server","config"},cbi("v2ray_server/config")).leaf=true
	
	entry({"admin","vpn","v2ray_server","users_status"},call("v2ray_users_status")).leaf=true
	entry({"admin", "vpn", "v2ray_server", "check"}, call("v2ray_check")).leaf = true
	entry({"admin", "vpn", "v2ray_server", "update"}, call("v2ray_update")).leaf = true
end

local function http_write_json(content)
	http.prepare_content("application/json")
	http.write_json(content or { code = 1 })
end

function v2ray_users_status()
	local e={}
	e.index=luci.http.formvalue("index")
	e.status=luci.sys.call("ps -w| grep -v grep | grep '/var/etc/v2ray_server/" .. luci.http.formvalue("id") .. "' >/dev/null")==0
	http_write_json(e)
end

function v2ray_check()
	local json = v2ray.to_check("")
	http_write_json(json)
end

function v2ray_update()
	local json = nil
	local task = http.formvalue("task")
	if task == "extract" then
		json = v2ray.to_extract(http.formvalue("file"), http.formvalue("subfix"))
	elseif task == "move" then
		json = v2ray.to_move(http.formvalue("file"))
	else
		json = v2ray.to_download(http.formvalue("url"))
	end

	http_write_json(json)
end