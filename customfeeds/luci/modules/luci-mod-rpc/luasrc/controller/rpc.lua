-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.rpc", package.seeall)


function session_retrieve(sid, allowed_users)
	local util = require "luci.util"
	local sdat = util.ubus("session", "get", {
		ubus_rpc_session = sid
	})

	if type(sdat) == "table" and
	   type(sdat.values) == "table" and
	   type(sdat.values.token) == "string" and
	   type(sdat.values.secret) == "string" and
	   type(sdat.values.username) == "string" and
	   util.contains(allowed_users, sdat.values.username)
	then
		return sid, sdat.values
	end

	return nil
end

function authenticator(validator, accs)
	local http = require "luci.http"
	local ctrl = require "luci.controller.rpc"
	local auth = http.formvalue("auth", true) or http.getcookie("sysauth")

	if auth then -- if authentication token was given
		local sid, sdat = ctrl.session_retrieve(auth, accs)
		if sdat then -- if given token is valid
			return sdat.username, sid
		end
		http.status(403, "Forbidden")
	end
end


function index()
	local ctrl = require "luci.controller.rpc"

	local rpc = node("rpc")
	rpc.sysauth = "root"
	rpc.sysauth_authenticator = ctrl.authenticator
	rpc.notemplate = true

	entry({"rpc", "uci"}, call("rpc_uci"))
	entry({"rpc", "fs"}, call("rpc_fs"))
	entry({"rpc", "sys"}, call("rpc_sys"))
	entry({"rpc", "ipkg"}, call("rpc_ipkg"))
	entry({"rpc", "ip"}, call("rpc_ip"))
	entry({"rpc", "auth"}, call("rpc_auth")).sysauth = false
end

function rpc_auth()
	local ctrl    = require "luci.controller.rpc"
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local sys     = require "luci.sys"
	local ltn12   = require "luci.ltn12"
	local util    = require "luci.util"

	local server = {}
	server.challenge = function(user, pass)
		local config = require "luci.config"
		local login = util.ubus("session", "login", {
			username = user,
			password = pass,
			timeout  = tonumber(config.sauth.sessiontime)
		})

		if type(login) == "table" and
		   type(login.ubus_rpc_session) == "string"
		then
			util.ubus("session", "set", {
				ubus_rpc_session = login.ubus_rpc_session,
				values = {
					token = sys.uniqueid(16),
					secret = sys.uniqueid(16)
				}
			})

			local sid, sdat = ctrl.session_retrieve(login.ubus_rpc_session, { user })
			if sdat then
				return {
					sid = sid,
					token = sdat.token,
					secret = sdat.secret
				}
			end
		end

		return nil
	end

	server.login = function(...)
		local challenge = server.challenge(...)
		if challenge then
			http.header("Set-Cookie", 'sysauth=%s; path=%s' %{
				challenge.sid,
				http.getenv("SCRIPT_NAME")
			})
			return challenge.sid
		end
	end

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(server, http.source()), http.write)
end

function rpc_uci()
	if not pcall(require, "luci.model.uci") then
		luci.http.status(404, "Not Found")
		return nil
	end
	local uci     = require "luci.jsonrpcbind.uci"
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local ltn12   = require "luci.ltn12"

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(uci, http.source()), http.write)
end

function rpc_fs()
	local util    = require "luci.util"
	local io      = require "io"
	local fs2     = util.clone(require "nixio.fs")
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local ltn12   = require "luci.ltn12"

	function fs2.readfile(filename)
		local stat, mime = pcall(require, "mime")
		if not stat then
			error("Base64 support not available. Please install LuaSocket.")
		end

		local fp = io.open(filename)
		if not fp then
			return nil
		end

		local output = {}
		local sink = ltn12.sink.table(output)
		local source = ltn12.source.chain(ltn12.source.file(fp), mime.encode("base64"))
		return ltn12.pump.all(source, sink) and table.concat(output)
	end

	function fs2.writefile(filename, data)
		local stat, mime = pcall(require, "mime")
		if not stat then
			error("Base64 support not available. Please install LuaSocket.")
		end

		local  file = io.open(filename, "w")
		local  sink = file and ltn12.sink.chain(mime.decode("base64"), ltn12.sink.file(file))
		return sink and ltn12.pump.all(ltn12.source.string(data), sink) or false
	end

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(fs2, http.source()), http.write)
end

function rpc_sys()
	local util    = require "luci.util"
	local sys     = require "luci.sys"
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local ltn12   = require "luci.ltn12"

	local sys2 = util.clone(sys)
	      sys2.net = util.clone(sys.net)
	      sys2.wifi = util.clone(sys.wifi)

	function sys2.wifi.getiwinfo(ifname, operation)
		local iw = sys.wifi.getiwinfo(ifname)
		if iw then
			if operation then
				assert(type(iwinfo[iw.type][operation]) == "function")
				return iw[operation]
			end

			local n, f
			local rv = { ifname = ifname }
			for n, f in pairs(iwinfo[iw.type]) do
				if type(f) == "function" and
				   n ~= "scanlist" and n ~= "countrylist"
				then
					rv[n] = iw[n]
				end
			end
			return rv
		end
		return nil
	end

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(sys2, http.source()), http.write)
end

function rpc_ipkg()
	if not pcall(require, "luci.model.ipkg") then
		luci.http.status(404, "Not Found")
		return nil
	end
	local ipkg    = require "luci.model.ipkg"
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local ltn12   = require "luci.ltn12"

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(ipkg, http.source()), http.write)
end

function rpc_ip()
	if not pcall(require, "luci.ip") then
		luci.http.status(404, "Not Found")
		return nil
	end

	local util    = require "luci.util"
	local ip      = require "luci.ip"
	local jsonrpc = require "luci.jsonrpc"
	local http    = require "luci.http"
	local ltn12   = require "luci.ltn12"

	local ip2 = util.clone(ip)

	local _, n
	for _, n in ipairs({ "new", "IPv4", "IPv6", "MAC" }) do
		ip2[n] = function(address, netmask, operation, argument)
			local cidr = ip[n](address, netmask)
			if cidr and operation then
				assert(type(cidr[operation]) == "function")
				local cidr2 = cidr[operation](cidr, argument)
				return (type(cidr2) == "userdata") and cidr2:string() or cidr2
			end
			return (type(cidr) == "userdata") and cidr:string() or cidr
		end
	end

	http.prepare_content("application/json")
	ltn12.pump.all(jsonrpc.handle(ip2, http.source()), http.write)
end
