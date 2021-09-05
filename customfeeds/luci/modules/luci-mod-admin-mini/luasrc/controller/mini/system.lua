-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.mini.system", package.seeall)

function index()
	entry({"mini", "system"}, alias("mini", "system", "index"), _("System"), 40).index = true
	entry({"mini", "system", "index"}, cbi("mini/system", {autoapply=true}), _("General"), 1)
	entry({"mini", "system", "passwd"}, form("mini/passwd"), _("Admin Password"), 10)
	entry({"mini", "system", "backup"}, call("action_backup"), _("Backup / Restore"), 80)
	entry({"mini", "system", "upgrade"}, call("action_upgrade"), _("Flash Firmware"), 90)
	entry({"mini", "system", "reboot"}, call("action_reboot"), _("Reboot"), 100)
end

function action_backup()
	local reset_avail = os.execute([[grep '"rootfs_data"' /proc/mtd >/dev/null 2>&1]]) == 0
	local restore_cmd = "gunzip | tar -xC/ >/dev/null 2>&1"
	local backup_cmd  = "tar -c %s | gzip 2>/dev/null"
	
	local restore_fpi 
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not restore_fpi then
				restore_fpi = io.popen(restore_cmd, "w")
			end
			if chunk then
				restore_fpi:write(chunk)
			end
			if eof then
				restore_fpi:close()
			end
		end
	)
		  
	local upload = luci.http.formvalue("archive")
	local backup = luci.http.formvalue("backup")
	local reset  = reset_avail and luci.http.formvalue("reset")
	
	if upload and #upload > 0 then
		luci.template.render("mini/applyreboot")
		luci.sys.reboot()
	elseif backup then
		local reader = ltn12_popen(backup_cmd:format(_keep_pattern()))
		luci.http.header('Content-Disposition', 'attachment; filename="backup-%s-%s.tar.gz"' % {
			luci.sys.hostname(), os.date("%Y-%m-%d")})
		luci.http.prepare_content("application/x-targz")
		luci.ltn12.pump.all(reader, luci.http.write)
	elseif reset then
		luci.template.render("mini/applyreboot")
		luci.util.exec("mtd -r erase rootfs_data")
	else
		luci.template.render("mini/backup", {reset_avail = reset_avail})
	end
end

function action_reboot()
	local reboot = luci.http.formvalue("reboot")
	luci.template.render("mini/reboot", {reboot=reboot})
	if reboot then
		luci.sys.reboot()
	end
end

function action_upgrade()
	require("luci.model.uci")

	local tmpfile = "/tmp/firmware.img"
	
	local function image_supported()
		-- XXX: yay...
		return ( 0 == os.execute(
			". /lib/functions.sh; " ..
			"include /lib/upgrade; " ..
			"platform_check_image %q >/dev/null"
				% tmpfile
		) )
	end
	
	local function image_checksum()
		return (luci.sys.exec("md5sum %q" % tmpfile):match("^([^%s]+)"))
	end
	
	local function storage_size()
		local size = 0
		if nixio.fs.access("/proc/mtd") then
			for l in io.lines("/proc/mtd") do
				local d, s, e, n = l:match('^([^%s]+)%s+([^%s]+)%s+([^%s]+)%s+"([^%s]+)"')
				if n == "linux" then
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


	-- Install upload handler
	local file
	luci.http.setfilehandler(
		function(meta, chunk, eof)
			if not nixio.fs.access(tmpfile) and not file and chunk and #chunk > 0 then
				file = io.open(tmpfile, "w")
			end
			if file and chunk then
				file:write(chunk)
			end
			if file and eof then
				file:close()
			end
		end
	)


	-- Determine state
	local keep_avail   = true
	local step         = tonumber(luci.http.formvalue("step") or 1)
	local has_image    = nixio.fs.access(tmpfile)
	local has_support  = image_supported()
	local has_platform = nixio.fs.access("/lib/upgrade/platform.sh")
	local has_upload   = luci.http.formvalue("image")
	
	-- This does the actual flashing which is invoked inside an iframe
	-- so don't produce meaningful errors here because the the 
	-- previous pages should arrange the stuff as required.
	if step == 4 then
		if has_platform and has_image and has_support then
			-- Mimetype text/plain
			luci.http.prepare_content("text/plain")
			luci.http.write("Starting luci-flash...\n")

			-- Now invoke sysupgrade
			local keepcfg = keep_avail and luci.http.formvalue("keepcfg") == "1"
			local flash = ltn12_popen("/sbin/luci-flash %s %q" %{
				keepcfg and "-k %q" % _keep_pattern() or "", tmpfile
			})

			luci.ltn12.pump.all(flash, luci.http.write)

			-- Make sure the device is rebooted
			luci.sys.reboot()
		end


	--
	-- This is step 1-3, which does the user interaction and
	-- image upload.
	--

	-- Step 1: file upload, error on unsupported image format
	elseif not has_image or not has_support or step == 1 then
		-- If there is an image but user has requested step 1
		-- or type is not supported, then remove it.
		if has_image then
			nixio.fs.unlink(tmpfile)
		end
			
		luci.template.render("mini/upgrade", {
			step=1,
			bad_image=(has_image and not has_support or false),
			keepavail=keep_avail,
			supported=has_platform
		} )

	-- Step 2: present uploaded file, show checksum, confirmation
	elseif step == 2 then
		luci.template.render("mini/upgrade", {
			step=2,
			checksum=image_checksum(),
			filesize=nixio.fs.stat(tmpfile).size,
			flashsize=storage_size(),
			keepconfig=(keep_avail and luci.http.formvalue("keepcfg") == "1")
		} )
	
	-- Step 3: load iframe which calls the actual flash procedure
	elseif step == 3 then
		luci.template.render("mini/upgrade", {
			step=3,
			keepconfig=(keep_avail and luci.http.formvalue("keepcfg") == "1")
		} )
	end	
end

function _keep_pattern()
	local kpattern = ""
	local files = luci.model.uci.cursor():get_all("luci", "flash_keep")
	if files then
		kpattern = ""
		for k, v in pairs(files) do
			if k:sub(1,1) ~= "." and nixio.fs.glob(v)() then
				kpattern = kpattern .. " " ..  v
			end
		end
	end
	return kpattern
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
