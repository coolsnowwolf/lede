-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008-2011 Jo-Philipp Wich <jow@openwrt.org>
-- Copyright 2012 Daniel Golle <dgolle@allnet.de>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.failsafe.failsafe", package.seeall)

function index()
	local root = node()
	if not root.target then
		root.target = alias("failsafe")
		root.index = true
	end

	page          = node()
	page.lock     = true
	page.target   = alias("failsafe")
	page.subindex = true
	page.index    = false

	page          = node("failsafe")
	page.title    = _("Fail-safe")
	page.target   = alias("failsafe", "flashops")
	page.order    = 5
	page.setuser  = "root"
	page.setgroup = "root"
	page.index    = true

	entry({"failsafe", "flashops"}, call("action_flashops"), _("Flash Firmware"), 70).index = true
	entry({"failsafe", "reboot"}, call("action_reboot"), _("Reboot"), 90)
end

function action_flashops()
	local sys = require "luci.sys"
	local fs  = require "nixio.fs"

	local upgrade_avail = fs.access("/lib/upgrade/platform.sh")
	local reset_avail   = os.execute([[grep '"rootfs_data"' /proc/mtd >/dev/null 2>&1]]) == 0

	local image_tmp   = "/tmp/firmware.img"

	local function image_supported()
		-- XXX: yay...
		return ( 0 == os.execute(
			". /lib/functions.sh; " ..
			"include /lib/upgrade; " ..
			"platform_check_image %q >/dev/null"
				% image_tmp
		) )
	end

	local function image_checksum()
		return (luci.sys.exec("md5sum %q" % image_tmp):match("^([^%s]+)"))
	end

	local function storage_size()
		local size = 0
		if fs.access("/proc/mtd") then
			for l in io.lines("/proc/mtd") do
				local d, s, e, n = l:match('^([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+"([^%s]+)"')
				if n == "linux" or n == "firmware" then
					size = tonumber(s, 16)
					break
				end
			end
		elseif fs.access("/proc/partitions") then
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


	local fp
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not fp then
				if meta and meta.name == "image" then
					fp = io.open(image_tmp, "w")
				end
			end
			if fp then
				if chunk then
					fp:write(chunk)
				end
				if eof then
					fp:close()
				end
			end
		end
	)

	if luci.http.formvalue("image") or luci.http.formvalue("step") then
		--
		-- Initiate firmware flash
		--
		local step = tonumber(luci.http.formvalue("step") or 1)
		if step == 1 then
			if image_supported() then
				luci.template.render("failsafe/upgrade", {
					checksum = image_checksum(),
					storage  = storage_size(),
					size     = (fs.stat(image_tmp, "size") or 0),
					keep     = false
				})
			else
				fs.unlink(image_tmp)
				luci.template.render("failsafe/flashops", {
					reset_avail   = reset_avail,
					upgrade_avail = upgrade_avail,
					image_invalid = true
				})
			end
		--
		-- Start sysupgrade flash
		--
		elseif step == 2 then
			local keep = (luci.http.formvalue("keep") == "1") and "" or "-n"
			luci.template.render("failsafe/applyreboot", {
				title = luci.i18n.translate("Flashing..."),
				msg   = luci.i18n.translate("The system is flashing now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
				addr  = (#keep > 0) and "192.168.1.1" or nil
			})
			fork_exec("killall dropbear uhttpd; sleep 1; /sbin/sysupgrade %s %q" %{ keep, image_tmp })
		end
	else
		--
		-- Overview
		--
		luci.template.render("failsafe/flashops", {
			reset_avail   = reset_avail,
			upgrade_avail = upgrade_avail
		})
	end
end

function action_reboot()
	local reboot = luci.http.formvalue("reboot")
	luci.template.render("failsafe/reboot", {reboot=reboot})
	if reboot then
		luci.sys.reboot()
	end
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
