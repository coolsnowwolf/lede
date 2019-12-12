-- Copyright (C) 2019 github.com/peter-tank
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.dnscrypt-proxy", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/dnscrypt-proxy") then
		return
	end

  entry({"admin", "services", "dnscrypt-proxy"},alias("admin", "services", "dnscrypt-proxy", "dnscrypt-proxy"),_("DNSCrypt Proxy"), 10).dependent = true

	entry({"admin", "services", "dnscrypt-proxy", "dnscrypt-proxy"},cbi("dnscrypt-proxy/dnscrypt-proxy"),_("DNSCrypt Proxy"), 10).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "dnscrypt-resolvers"},form("dnscrypt-proxy/dnscrypt-resolvers"),_("DNSCrypt Resolvers"), 20).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "dnscrypt-resolvers"},arcombine(cbi("dnscrypt-proxy/dnscrypt-resolvers"), cbi("dnscrypt-proxy/dnscrypt-resolvers-config")),_("DNSCrypt Servers"), 30).leaf = true
	entry({"admin", "services", "dnscrypt-proxy", "refresh_c"}, call("refresh_cmd"))
	entry({"admin", "services", "dnscrypt-proxy", "resolve_c"}, call("resolve_cmd"))
	entry({"admin", "services", "dnscrypt-proxy", "update_c"}, call("update_cmd"))
	
	entry({"admin", "services", "dnscrypt-proxy", "fileread"}, call("act_read"), nil).leaf=true

	entry({"admin", "services", "dnscrypt-proxy", "logview"}, cbi("dnscrypt-proxy/logview"), _("Log") ,80).leaf=true

end

function update_cmd()
local dc = require "luci.tools.dnscrypt".init()
local set = luci.http.formvalue("set")
local surl = luci.http.formvalue("url")
local ecount, retstring = 0, "-1"

if set == "dnscrypt_bin" then
	local bin_file = "/usr/sbin/dnscrypt-proxy"
	local ret = action_update(surl)
	retstring = type(ret) == "table" and table.concat(ret,", ") or ret
 	ecount = nixio.fs.chmod(bin_file, 755)
else
	ecount = -1
	retstring = "Unkown CMD: " .. set
end

luci.http.prepare_content("application/json")
luci.http.write_json({ err=ecount, status=retstring})
end

function refresh_cmd()
local set = luci.http.formvalue("set")
local icount, retstring = 0, "-1"

if set == "dnslist_up" then
	retstring = "1"
	icount = 1
elseif set == "resolvers_up" then
	local bin_file = "/usr/sbin/dnscrypt-proxy"
	retstring = "2"
	icount = 2
else
	ecount = -1
	retstring = "Unkown CMD: " .. set
end

luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring ,retcount=icount})
end

function resolve_cmd()
local set = luci.http.formvalue("set")
local retstring="<br /><br />"

retstring = luci.sys.exec("/usr/sbin/dnscrypt-proxy -resolve www.google.com")
luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring })
end

-- called by XHR.get from logview.htm
function act_read(lfile)
	local fs = require "nixio.fs"
	local http = require "luci.http"
	local lfile = http.formvalue("lfile")
	local ldata={}
	ldata[#ldata+1] = fs.readfile(lfile) or "_nofile_"
	if ldata[1] == "" then
		ldata[1] = "_nodata_"
	end
	http.prepare_content("application/json")
	http.write_json(ldata)
end

function action_data()
	local http = require "luci.http"

	local types = {
		csv = "text/csv",
		json = "application/json"
	}

	local args = { }
	local mtype = http.formvalue("type") or "json"

	http.prepare_content(types[mtype])
	exec("/usr/sbin/dnscrypt-proxy", args, http.write)
end

function action_list()
	local http = require "luci.http"

	local fd = io.popen("/usr/sbin/dnscrypt-proxy -c list")
	local periods = { }

	if fd then
		while true do
			local period = fd:read("*l")

			if not period then
				break
			end

			periods[#periods+1] = period
		end

		fd:close()
	end

	http.prepare_content("application/json")
	http.write_json(periods)
end

function action_update(surl)
	local tmp = "/tmp/dnscrypt-proxy_bin.tar.gz"
	local dir = "/tmp"
	local stype = luci.util.exec("uname"):lower()
	local sarch = luci.util.exec("uname -m")

	exec("/bin/wget", {"--no-check-certificate", "-O", tmp, surl})

	exec("/usr/sbin/dnscrypt-proxy", {"-service", "stop" })
	local files = { }
	local tar = io.popen("/bin/tar -tzf %s" % tmp, "r")
	if tar then
		while true do
			local file = tar:read("*l")
			if not file then
				break
			elseif file:match("^(.*\/dnscrypt.proxy)$") then
				files[#files+1] = file
			end
		end
		tar:close()
	end

	if #files == 0 then
		return {500, "Internal Server Error", stype, sarch}
	end


	local output = { }

	exec("/usr/sbin/dnscrypt-proxy", {"-service", "stop" })
	exec("/bin/mkdir", { "-p", dir })

	exec("/bin/tar", { "-C", dir, "-vxzf", tmp, unpack(files) },
		function(chunk) output[#output+1] = chunk:match("%S+") end)

	exec("/bin/cp", { "-f", dir .. "/" .. files[1], "/usr/sbin/dnscrypt-proxy"})
	exec("/bin/rm", { "-f", "%s/%s-%s/dnscrypt-proxy" % {dir, stype, sarch}})
	exec("/bin/rm", { "-f", tmp })
	exec("/usr/sbin/dnscrypt-proxy", {"-service", "start" })
	return out
end

function exec(cmd, args, writer)
	local os = require "os"
	local nixio = require "nixio"

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()

		while true do
			local buffer = fdi:read(2048)

			if not buffer or #buffer == 0 then
				break
			end

			if writer then
				writer(buffer)
			end
		end

		nixio.waitpid(pid)
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exece(cmd, args, nil)
		nixio.stdout:close()
		os.exit(1)
	end
end
