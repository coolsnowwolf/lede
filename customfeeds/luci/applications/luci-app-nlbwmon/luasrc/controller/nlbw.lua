-- Copyright 2017 Jo-Philipp Wich <jo@mein.io>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.nlbw", package.seeall)

function index()
	entry({"admin", "nlbw"}, firstchild(), _("Bandwidth Monitor"), 80)
	entry({"admin", "nlbw", "display"}, template("nlbw/display"), _("Display"), 1)
	entry({"admin", "nlbw", "config"}, cbi("nlbw/config"), _("Configuration"), 2)
	entry({"admin", "nlbw", "backup"}, template("nlbw/backup"), _("Backup"), 3)
	entry({"admin", "nlbw", "data"}, call("action_data"), nil, 4)
	entry({"admin", "nlbw", "list"}, call("action_list"), nil, 5)
	entry({"admin", "nlbw", "ptr"}, call("action_ptr"), nil, 6).leaf = true
	entry({"admin", "nlbw", "download"}, call("action_download"), nil, 7)
	entry({"admin", "nlbw", "restore"}, post("action_restore"), nil, 8)
	entry({"admin", "nlbw", "commit"}, call("action_commit"), nil, 9)
end

local function exec(cmd, args, writer)
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

function action_data()
	local http = require "luci.http"

	local types = {
		csv = "text/csv",
		json = "application/json"
	}

	local args = { }
	local mtype = http.formvalue("type") or "json"
	local delim = http.formvalue("delim") or ";"
	local period = http.formvalue("period")
	local group_by = http.formvalue("group_by")
	local order_by = http.formvalue("order_by")

	if types[mtype] then
		args[#args+1] = "-c"
		args[#args+1] = mtype
	else
		http.status(400, "Unsupported type")
		return
	end

	if delim and #delim > 0 then
		args[#args+1] = "-s%s" % delim
	end

	if period and #period > 0 then
		args[#args+1] = "-t"
		args[#args+1] = period
	end

	if group_by and #group_by > 0 then
		args[#args+1] = "-g"
		args[#args+1] = group_by
	end

	if order_by and #order_by > 0 then
		args[#args+1] = "-o"
		args[#args+1] = order_by
	end

	http.prepare_content(types[mtype])
	exec("/usr/sbin/nlbw", args, http.write)
end

function action_list()
	local http = require "luci.http"

	local fd = io.popen("/usr/sbin/nlbw -c list")
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

function action_ptr(...)
	local http = require "luci.http"
	local util = require "luci.util"

	http.prepare_content("application/json")
	http.write_json(util.ubus("network.rrdns", "lookup", {
		addrs = {...}, timeout = 3000
	}))
end

function action_download()
	local nixio = require "nixio"
	local http = require "luci.http"
	local sys = require "luci.sys"
	local uci = require "luci.model.uci".cursor()

	local dir = uci:get_first("nlbwmon", "nlbwmon", "database_directory")
		or "/var/lib/nlbwmon"

	if dir and nixio.fs.stat(dir, "type") == "dir" then
		local n = "nlbwmon-backup-%s-%s.tar.gz"
			%{ sys.hostname(), os.date("%Y-%m-%d") }

		http.prepare_content("application/octet-stream")
		http.header("Content-Disposition", "attachment; filename=\"%s\"" % n)
		exec("/bin/tar", { "-C", dir, "-c", "-z", ".", "-f", "-" }, http.write)
	else
		http.status(500, "Unable to find database directory")
	end
end

function action_restore()
	local nixio = require "nixio"
	local http = require "luci.http"
	local i18n = require "luci.i18n"
	local tpl = require "luci.template"
	local uci = require "luci.model.uci".cursor()

	local tmp = "/tmp/nlbw-restore.tar.gz"
	local dir = uci:get_first("nlbwmon", "nlbwmon", "database_directory")
		or "/var/lib/nlbwmon"

	local fp
	http.setfilehandler(
		function(meta, chunk, eof)
			if not fp and meta and meta.name == "archive" then
				fp = io.open(tmp, "w")
			end
			if fp and chunk then
				fp:write(chunk)
			end
			if fp and eof then
				fp:close()
			end
		end)

	local files = { }
	local tar = io.popen("/bin/tar -tzf %s" % tmp, "r")
	if tar then
		while true do
			local file = tar:read("*l")
			if not file then
				break
			elseif file:match("^%d%d%d%d%d%d%d%d%.db%.gz$") or
			       file:match("^%./%d%d%d%d%d%d%d%d%.db%.gz$") then
				files[#files+1] = file
			end
		end
		tar:close()
	end

	if #files == 0 then
		http.status(500, "Internal Server Error")
		tpl.render("nlbw/backup", {
			message = i18n.translate("Invalid or empty backup archive")
		})
		return
	end


	local output = { }

	exec("/etc/init.d/nlbwmon", { "stop" })
	exec("/bin/mkdir", { "-p", dir })

	exec("/bin/tar", { "-C", dir, "-vxzf", tmp, unpack(files) },
		function(chunk) output[#output+1] = chunk:match("%S+") end)

	exec("/bin/rm", { "-f", tmp })
	exec("/etc/init.d/nlbwmon", { "start" })

	tpl.render("nlbw/backup", {
		message = i18n.translatef(
			"The following database files have been restored: %s",
			table.concat(output, ", "))
	})
end

function action_commit()
	local http = require "luci.http"
	local disp = require "luci.dispatcher"

	http.redirect(disp.build_url("admin/nlbw/display"))
	exec("/usr/sbin/nlbw", { "-c", "commit" })
end
