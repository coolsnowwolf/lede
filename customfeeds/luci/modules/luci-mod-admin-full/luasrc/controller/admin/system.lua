-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.system", package.seeall)

function index()
	local fs = require "nixio.fs"

	entry({"admin", "system"}, alias("admin", "system", "system"), _("System"), 30).index = true
	entry({"admin", "system", "system"}, cbi("admin_system/system"), _("System"), 1)
	entry({"admin", "system", "clock_status"}, post_on({ set = true }, "action_clock_status"))

	entry({"admin", "system", "admin"}, cbi("admin_system/admin"), _("Administration"), 2)

	if fs.access("/bin/opkg") then
		entry({"admin", "system", "packages"}, post_on({ exec = "1" }, "action_packages"), _("Software"), 10)
		entry({"admin", "system", "packages", "ipkg"}, form("admin_system/ipkg"))
	end

	entry({"admin", "system", "startup"}, form("admin_system/startup"), _("Startup"), 45)
	entry({"admin", "system", "crontab"}, form("admin_system/crontab"), _("Scheduled Tasks"), 46)

	if fs.access("/sbin/block") and fs.access("/etc/config/fstab") then
		entry({"admin", "system", "fstab"}, cbi("admin_system/fstab"), _("Mount Points"), 50)
		entry({"admin", "system", "fstab", "mount"}, cbi("admin_system/fstab/mount"), nil).leaf = true
		entry({"admin", "system", "fstab", "swap"},  cbi("admin_system/fstab/swap"),  nil).leaf = true
	end

	local nodes, number = nixio.fs.glob("/sys/class/leds/*")
	if number > 0 then
		entry({"admin", "system", "leds"}, cbi("admin_system/leds"), _("<abbr title=\"Light Emitting Diode\">LED</abbr> Configuration"), 60)
	end

	entry({"admin", "system", "flashops"}, call("action_flashops"), _("Backup / Flash Firmware"), 70)
	entry({"admin", "system", "flashops", "reset"}, post("action_reset"))
	entry({"admin", "system", "flashops", "backup"}, post("action_backup"))
	entry({"admin", "system", "flashops", "backupmtdblock"}, post("action_backupmtdblock"))
	entry({"admin", "system", "flashops", "backupfiles"}, form("admin_system/backupfiles"))

	-- call() instead of post() due to upload handling!
	entry({"admin", "system", "flashops", "restore"}, call("action_restore"))
	entry({"admin", "system", "flashops", "sysupgrade"}, call("action_sysupgrade"))

	entry({"admin", "system", "reboot"}, template("admin_system/reboot"), _("Reboot"), 90)
	entry({"admin", "system", "reboot", "call"}, post("action_reboot"))
end

function action_clock_status()
	local set = tonumber(luci.http.formvalue("set"))
	if set ~= nil and set > 0 then
		local date = os.date("*t", set)
		if date then
			luci.sys.call("date -s '%04d-%02d-%02d %02d:%02d:%02d'" %{
				date.year, date.month, date.day, date.hour, date.min, date.sec
			})
			luci.sys.call("/etc/init.d/sysfixtime restart")
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = os.date("%c") })
end

function action_packages()
	local fs = require "nixio.fs"
	local ipkg = require "luci.model.ipkg"
	local submit = (luci.http.formvalue("exec") == "1")
	local update, upgrade
	local changes = false
	local install = { }
	local remove  = { }
	local stdout  = { "" }
	local stderr  = { "" }
	local out, err

	-- Display
	local display = luci.http.formvalue("display") or "installed"

	-- Letter
	local letter = string.byte(luci.http.formvalue("letter") or "A", 1)
	letter = (letter == 35 or (letter >= 65 and letter <= 90)) and letter or 65

	-- Search query
	local query = luci.http.formvalue("query")
	query = (query ~= '') and query or nil


	-- Modifying actions
	if submit then
		-- Packets to be installed
		local ninst = luci.http.formvalue("install")
		local uinst = nil

		-- Install from URL
		local url = luci.http.formvalue("url")
		if url and url ~= '' then
			uinst = url
		end

		-- Do install
		if ninst then
			install[ninst], out, err = ipkg.install(ninst)
			stdout[#stdout+1] = out
			stderr[#stderr+1] = err
			changes = true
		end

		if uinst then
			local pkg
			for pkg in luci.util.imatch(uinst) do
				install[uinst], out, err = ipkg.install(pkg)
				stdout[#stdout+1] = out
				stderr[#stderr+1] = err
				changes = true
			end
		end

		-- Remove packets
		local rem = luci.http.formvalue("remove")
		if rem then
			remove[rem], out, err = ipkg.remove(rem)
			stdout[#stdout+1] = out
			stderr[#stderr+1] = err
			changes = true
		end


		-- Update all packets
		update = luci.http.formvalue("update")
		if update then
			update, out, err = ipkg.update()
			stdout[#stdout+1] = out
			stderr[#stderr+1] = err
		end


		-- Upgrade all packets
		upgrade = luci.http.formvalue("upgrade")
		if upgrade then
			upgrade, out, err = ipkg.upgrade()
			stdout[#stdout+1] = out
			stderr[#stderr+1] = err
		end
	end


	-- List state
	local no_lists = true
	local old_lists = false
	if fs.access("/var/opkg-lists/") then
		local list
		for list in fs.dir("/var/opkg-lists/") do
			no_lists = false
			if (fs.stat("/var/opkg-lists/"..list, "mtime") or 0) < (os.time() - (24 * 60 * 60)) then
				old_lists = true
				break
			end
		end
	end


	luci.template.render("admin_system/packages", {
		display   = display,
		letter    = letter,
		query     = query,
		install   = install,
		remove    = remove,
		update    = update,
		upgrade   = upgrade,
		no_lists  = no_lists,
		old_lists = old_lists,
		stdout    = table.concat(stdout, ""),
		stderr    = table.concat(stderr, "")
	})

	-- Remove index cache
	if changes then
		fs.unlink("/tmp/luci-indexcache")
	end
end

local function image_supported(image)
	return (os.execute("sysupgrade -T %q >/dev/null" % image) == 0)
end

local function image_checksum(image)
	return (luci.sys.exec("md5sum %q" % image):match("^([^%s]+)"))
end

local function image_sha256_checksum(image)
	return (luci.sys.exec("sha256sum %q" % image):match("^([^%s]+)"))
end

local function supports_sysupgrade()
	return nixio.fs.access("/lib/upgrade/platform.sh")
end

local function supports_reset()
	return (os.execute([[grep -sq "^overlayfs:/overlay / overlay " /proc/mounts]]) == 0)
end

local function storage_size()
	local size = 0
	if nixio.fs.access("/proc/mtd") then
		for l in io.lines("/proc/mtd") do
			local d, s, e, n = l:match('^([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+"([^%s]+)"')
			if n == "linux" or n == "firmware" then
				size = tonumber(s, 16)
				break
			end
		end
	elseif nixio.fs.access("/proc/partitions") then
		for l in io.lines("/proc/partitions") do
			local x, y, b, n = l:match('^%s*(%d+)%s+(%d+)%s+([^%s]+)%s+([^%s]+)')
			if b and n and not n:match('[0-9]') then
				size = tonumber(b) * 1024
				break
			end
		end
	end
	return size
end


function action_flashops()
	--
	-- Overview
	--
	luci.template.render("admin_system/flashops", {
		reset_avail   = supports_reset(),
		upgrade_avail = supports_sysupgrade()
	})
end

function action_sysupgrade()
	local fs = require "nixio.fs"
	local http = require "luci.http"
	local image_tmp = "/tmp/firmware.img"

	local fp
	http.setfilehandler(
		function(meta, chunk, eof)
			if not fp and meta and meta.name == "image" then
				fp = io.open(image_tmp, "w")
			end
			if fp and chunk then
				fp:write(chunk)
			end
			if fp and eof then
				fp:close()
			end
		end
	)

	if not luci.dispatcher.test_post_security() then
		fs.unlink(image_tmp)
		return
	end

	--
	-- Cancel firmware flash
	--
	if http.formvalue("cancel") then
		fs.unlink(image_tmp)
		http.redirect(luci.dispatcher.build_url('admin/system/flashops'))
		return
	end

	--
	-- Initiate firmware flash
	--
	local step = tonumber(http.formvalue("step") or 1)
	if step == 1 then
		if image_supported(image_tmp) then
			luci.template.render("admin_system/upgrade", {
				checksum = image_checksum(image_tmp),
				sha256ch = image_sha256_checksum(image_tmp),
				storage  = storage_size(),
				size     = (fs.stat(image_tmp, "size") or 0),
				keep     = (not not http.formvalue("keep"))
			})
		else
			fs.unlink(image_tmp)
			luci.template.render("admin_system/flashops", {
				reset_avail   = supports_reset(),
				upgrade_avail = supports_sysupgrade(),
				image_invalid = true
			})
		end
	--
	-- Start sysupgrade flash
	--
	elseif step == 2 then
		local keep = (http.formvalue("keep") == "1") and "" or "-n"
		luci.template.render("admin_system/applyreboot", {
			title = luci.i18n.translate("Flashing..."),
			msg   = luci.i18n.translate("The system is flashing now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
			addr  = (#keep > 0) and "192.168.1.1" or nil
		})
		fork_exec("sleep 1; killall dropbear uhttpd; sleep 1; /sbin/sysupgrade %s %q" %{ keep, image_tmp })
	end
end

function action_backup()
	local reader = ltn12_popen("sysupgrade --create-backup - 2>/dev/null")

	luci.http.header(
		'Content-Disposition', 'attachment; filename="backup-%s-%s.tar.gz"' %{
			luci.sys.hostname(),
			os.date("%Y-%m-%d")
		})

	luci.http.prepare_content("application/x-targz")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function action_backupmtdblock()
	local http = require "luci.http"
	local mv = http.formvalue("mtdblockname")
	local m, s, n = mv:match('^([^%s]+)/([^%s]+)/([^%s]+)')

	local reader = ltn12_popen("cat /dev/mtd%s" % n)

	luci.http.header(
		'Content-Disposition', 'attachment; filename="backup-%s-%s-%s.bin"' %{
			luci.sys.hostname(), m,
			os.date("%Y-%m-%d")
		})

	luci.http.prepare_content("application/octet-stream")
	luci.ltn12.pump.all(reader, luci.http.write)
end

function action_restore()
	local fs = require "nixio.fs"
	local http = require "luci.http"
	local archive_tmp = "/tmp/restore.tar.gz"

	local fp
	http.setfilehandler(
		function(meta, chunk, eof)
			if not fp and meta and meta.name == "archive" then
				fp = io.open(archive_tmp, "w")
			end
			if fp and chunk then
				fp:write(chunk)
			end
			if fp and eof then
				fp:close()
			end
		end
	)

	if not luci.dispatcher.test_post_security() then
		fs.unlink(archive_tmp)
		return
	end

	local upload = http.formvalue("archive")
	if upload and #upload > 0 then
		luci.template.render("admin_system/applyreboot")
		os.execute("tar -C / -xzf %q >/dev/null 2>&1" % archive_tmp)
		luci.sys.reboot()
		return
	end

	http.redirect(luci.dispatcher.build_url('admin/system/flashops'))
end

function action_reset()
	if supports_reset() then
		luci.template.render("admin_system/applyreboot", {
			title = luci.i18n.translate("Erasing..."),
			msg   = luci.i18n.translate("The system is erasing the configuration partition now and will reboot itself when finished."),
			addr  = "192.168.1.1"
		})

		fork_exec("sleep 1; killall dropbear uhttpd; sleep 1; jffs2reset -y && reboot")
		return
	end

	http.redirect(luci.dispatcher.build_url('admin/system/flashops'))
end

function action_passwd()
	local p1 = luci.http.formvalue("pwd1")
	local p2 = luci.http.formvalue("pwd2")
	local stat = nil

	if p1 or p2 then
		if p1 == p2 then
			stat = luci.sys.user.setpasswd("root", p1)
		else
			stat = 10
		end
	end

	luci.template.render("admin_system/passwd", {stat=stat})
end

function action_reboot()
	luci.sys.reboot()
end

function fork_exec(command)
	local pid = nixio.fork()
	if pid > 0 then
		return
	elseif pid == 0 then
		-- change to root dir
		nixio.chdir("/")

		-- patch stdin, out, err to /dev/null
		local null = nixio.open("/dev/null", "w+")
		if null then
			nixio.dup(null, nixio.stderr)
			nixio.dup(null, nixio.stdout)
			nixio.dup(null, nixio.stdin)
			if null:fileno() > 2 then
				null:close()
			end
		end

		-- replace with target command
		nixio.exec("/bin/sh", "-c", command)
	end
end

function ltn12_popen(command)

	local fdi, fdo = nixio.pipe()
	local pid = nixio.fork()

	if pid > 0 then
		fdo:close()
		local close
		return function()
			local buffer = fdi:read(2048)
			local wpid, stat = nixio.waitpid(pid, "nohang")
			if not close and wpid and stat == "exited" then
				close = true
			end

			if buffer and #buffer > 0 then
				return buffer
			elseif close then
				fdi:close()
				return nil
			end
		end
	elseif pid == 0 then
		nixio.dup(fdo, nixio.stdout)
		fdi:close()
		fdo:close()
		nixio.exec("/bin/sh", "-c", command)
	end
end
