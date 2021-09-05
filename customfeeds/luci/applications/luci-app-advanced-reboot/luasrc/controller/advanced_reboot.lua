-- Copyright 2017-2020 Stan Grishin <stangri@melmac.net>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.advanced_reboot", package.seeall)

local util = require "luci.util"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local http = require "luci.http"
local dispatcher = require "luci.dispatcher"
local i18n = require "luci.i18n"
local ltemplate = require "luci.template"
local ip = require "luci.ip"
local http = require "luci.http"
local sys = require "luci.sys"
local dispatcher = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()
local packageName = "advanced-reboot"
local devices_dir = "/usr/lib/lua/luci/" .. packageName .. "/devices/"

function logger(t)
	util.exec("logger -t " .. packageName .. " '" .. tostring(t) .. "'")
end

function is_alt_mountable(p1_mtd, p2_mtd)
	if p1_mtd:sub(1,3) == "mtd" and
				 p2_mtd:sub(1,3) == "mtd" and
				 fs.access("/usr/sbin/ubiattach") and
				 fs.access("/usr/sbin/ubiblock") and 
				 fs.access("/bin/mount") then
		return true
	else
		return false
	end
end

function get_partition_os_info(op_ubi)
	local cp_info, op_info
	if fs.access("/etc/os-release") then
		cp_info = util.trim(util.exec('. /etc/os-release && echo "$PRETTY_NAME"'))
		if cp_info:find("SNAPSHOT") then
			cp_info = util.trim(util.exec('. /etc/os-release && echo "$OPENWRT_RELEASE"'))
		end 
	end
	logger(i18n.translatef("attempting to mount alternative partition (mtd%s)", tostring(op_ubi)))
	alt_partition_unmount(op_ubi)
	alt_partition_mount(op_ubi)
	if fs.access("/alt/rom/etc/os-release") then
		op_info = util.trim(util.exec('. /alt/rom/etc/os-release && echo "$PRETTY_NAME"'))
		if op_info:find("SNAPSHOT") then
			op_info = util.trim(util.exec('. /alt/rom/etc/os-release && echo "$OPENWRT_RELEASE"'))
		end 
	end
	logger(i18n.translatef("attempting to unmount alternative partition (mtd%s)", tostring(op_ubi)))
	alt_partition_unmount(op_ubi)
	return cp_info, op_info
end

function alt_partition_mount(op_ubi)
	local ubi_dev
	util.exec('for i in rom overlay firmware; do [ ! -d "$i" ] && mkdir -p "/alt/${i}"; done')
	ubi_dev = tostring(util.exec("ubiattach -m " .. tostring(op_ubi)))
	_, _, ubi_dev = ubi_dev:find("UBI device number (%d+)")
	if not ubi_dev then 
		util.exec("ubidetach -m " .. tostring(op_ubi))
		return 
	end
	util.exec("ubiblock --create /dev/ubi" .. ubi_dev .. "_0")
	util.exec("mount -t squashfs -o ro /dev/ubiblock" .. ubi_dev .. "_0 /alt/rom")
	util.exec("mount -t ubifs /dev/ubi" .. ubi_dev .. "_1 /alt/overlay")
--	util.exec("mount -t overlay overlay -o noatime,lowerdir=/alt/rom,upperdir=/alt/overlay/upper,workdir=/alt/overlay/work /alt/firmware")
end

function alt_partition_unmount(op_ubi)
	local i
	local mtdCount = tonumber(util.exec("ubinfo | grep 'Present UBI devices' | grep -c ','"))
	mtdCount = mtdCount and mtdCount + 1 or 10
--	util.exec("[ -d /alt/firmware ] && umount /alt/firmware")
	util.exec("[ -d /alt/overlay ] && umount /alt/overlay")
	util.exec("[ -d /alt/rom ] && umount /alt/rom")
	for i = 0, mtdCount do
		if not fs.access("/sys/devices/virtual/ubi/ubi" .. tostring(i) .. "/mtd_num") then break end
		ubi_mtd =  tonumber(util.trim(util.exec("cat /sys/devices/virtual/ubi/ubi" .. i .. "/mtd_num")))
		if ubi_mtd and ubi_mtd == op_ubi then
			util.exec("ubiblock --remove /dev/ubi" .. tostring(i) .. "_0")
			util.exec("ubidetach -m " .. tostring(op_ubi))
			util.exec('rm -rf /alt')
		end
	end
end

function obtain_device_info()
	local p, boardName, n, p1_label, p1_version, p2_label, p2_version, p1_os, p2_os
	local errorMessage, current_partition
	local op_ubi, cp_info, op_info, zyxelFlagPartition
	local romBoardName = util.trim(util.exec("cat /tmp/sysinfo/board_name"))
	for filename in fs.dir(devices_dir) do
		local p_func = loadfile(devices_dir .. filename)
		setfenv(p_func, { _ = i18n.translate })
		p = p_func()
		if p.boardName then
			boardName = p.boardName:gsub('%p','')
		end
		if p.boardNames then
			for i, v in pairs(p.boardNames) do
				boardName = v:gsub('%p','')
				if romBoardName and romBoardName:gsub('%p',''):match(boardName) then break end
			end
		end
		if romBoardName and romBoardName:gsub('%p',''):match(boardName) then
			if p.labelOffset then
				if p.partition1MTD then
					p1_label = util.trim(util.exec("dd if=/dev/" .. p.partition1MTD .. " bs=1 skip=" .. p.labelOffset .. " count=128" .. " 2>/dev/null"))
					n, p1_version = p1_label:match('(Linux)-([%d|.]+)')
				end
				if p.partition2MTD then
					p2_label = util.trim(util.exec("dd if=/dev/" .. p.partition2MTD .. " bs=1 skip=" .. p.labelOffset .. " count=128" .. " 2>/dev/null"))
					n, p2_version = p2_label:match('(Linux)-([%d|.]+)')
				end
				if p1_label and p1_label:find("LEDE") then p1_os = "LEDE" end
				if p1_label and p1_label:find("OpenWrt") then p1_os = "OpenWrt" end
				if p1_label and p.vendorName and p1_label:find(p.vendorName) then p1_os = p.vendorName end
				if p2_label and p2_label:find("LEDE") then p2_os = "LEDE" end
				if p2_label and p2_label:find("OpenWrt") then p2_os = "OpenWrt" end
				if p2_label and p.vendorName and p2_label:find(p.vendorName) then p2_os = p.vendorName end
				if not p1_os then p1_os = p.vendorName .. "/" .. i18n.translate("Unknown") end
				if not p2_os then p2_os = p.vendorName .. "/" .. i18n.translate("Unknown") end
				if p1_os and p1_version then p1_os = p1_os .. " (Linux " .. p1_version .. ")" end
				if p2_os and p2_version then p2_os = p2_os .. " (Linux " .. p2_version .. ")" end
			else
				p1_os = p.vendorName .. "/" .. i18n.translate("Unknown") .. " (" .. i18n.translate("Compressed") .. ")"
				p2_os = p.vendorName .. "/" .. i18n.translate("Unknown") .. " (" .. i18n.translate("Compressed") .. ")"
			end

			if p.bootEnv1 then
				if fs.access("/usr/sbin/fw_printenv") and fs.access("/usr/sbin/fw_setenv") then
					current_partition = tonumber(util.trim(util.exec("fw_printenv -n " .. p.bootEnv1)))
				end
			else
				if not zyxelFlagPartition then zyxelFlagPartition = util.trim(util.exec(". /lib/functions.sh; find_mtd_part 0:DUAL_FLAG")) end
				if zyxelFlagPartition then
					current_partition = tonumber(util.exec("dd if=" .. zyxelFlagPartition .. " bs=1 count=1 2>/dev/null | hexdump -n 1 -e '1/1 \"%d\"'"))
				else
					errorMessage = errorMessage or i18n.translate("Unable to find Dual Boot Flag Partition.")
					util.perror(i18n.translate("Unable to find Dual Boot Flag Partition."))
				end
			end
			
			if is_alt_mountable(p.partition1MTD, p.partition2MTD) then
				if current_partition == p.bootEnv1Partition1Value then
					op_ubi = tonumber(p.partition2MTD:sub(4)) + 1
				else
					op_ubi = tonumber(p.partition1MTD:sub(4)) + 1
				end
				local cp_info, op_info = get_partition_os_info(op_ubi)
				if current_partition == p.bootEnv1Partition1Value then
					p1_os = cp_info or p1_os
					p2_os = op_info or p2_os
				else
					p1_os = op_info or p1_os
					p2_os = cp_info or p2_os
				end
			end
			return romBoardName, p.vendorName .. " " .. p.deviceName, p.bootEnv1, p.bootEnv1Partition1Value, p1_os, p.bootEnv1Partition2Value, p2_os, p.bootEnv2, p.bootEnv2Partition1Value, p.bootEnv2Partition1Value, current_partition
		end
	end
	return romBoardName, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil
end

function index()
	entry({"admin", "system", "advanced_reboot"}, call("action_template"), _("Advanced Reboot"), 90)
	entry({"admin", "system", "advanced_reboot", "reboot"}, post("action_reboot"))
	entry({"admin", "system", "advanced_reboot", "alternative_reboot"}, post("action_altreboot"))
	entry({"admin", "system", "advanced_reboot", "power_off"}, post("action_poweroff"))
end

function action_template()
	local romBoardName, device_name, bev1, bev1p1, p1_os, bev1p2, p2_os, bev2, bev2p1, bev2p2, current_partition = obtain_device_info()
	ltemplate.render("advanced_reboot/advanced_reboot",{
				romBoardName=romBoardName,
				device_name=device_name,
				bev1p1=bev1p1,
				p1_os=p1_os,
				bev1p2=bev1p2,
				p2_os=p2_os,
				current_partition=current_partition,
				errorMessage=errorMessage})
end

function action_reboot()
	ltemplate.render("advanced_reboot/applyreboot", {
				title = i18n.translate("Rebooting..."),
				msg   = i18n.translate("The system is rebooting now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
				addr  = ip.new(util.imatch(uci:get("network", "lan", "ipaddr"))() or "192.168.1.1")
			})
	sys.reboot()
end

function action_altreboot()
	local romBoardName, device_name, bev1, bev1p1, p1_os, bev1p2, p2_os, bev2, bev2p1, bev2p2, current_partition = obtain_device_info()
	local zyxelFlagPartition, zyxelBootFlag, zyxelNewBootFlag, errorCode, curEnvSetting, newEnvSetting
	errorMessage = nil
	errorCode = 0
	if http.formvalue("cancel") then
		http.redirect(dispatcher.build_url('admin/system/advanced_reboot'))
		return
	end
	local step = tonumber(http.formvalue("step") or 1)
	if step == 1 then
		if fs.access("/usr/sbin/fw_printenv") and fs.access("/usr/sbin/fw_setenv") then
			ltemplate.render("advanced_reboot/alternative_reboot",{})
		else
			ltemplate.render("advanced_reboot/advanced_reboot",{errorMessage = i18n.translate("No access to fw_printenv or fw_printenv!")})
		end
	elseif step == 2 then
		if bev1 or bev2 then -- Linksys devices
			if bev1 then
				curEnvSetting = tonumber(util.trim(util.exec("fw_printenv -n " .. bev1)))
				if not curEnvSetting then
					errorMessage = errorMessage .. i18n.translatef("Unable to obtain firmware environment variable: %s.", bev1)
					util.perror(i18n.translatef("Unable to obtain firmware environment variable: %s.", bev1))
				else
					newEnvSetting = curEnvSetting == bev1p1 and bev1p2 or bev1p1
					errorCode = sys.call("fw_setenv " .. bev1 .. " " .. newEnvSetting)
						if errorCode ~= 0 then
							errorMessage = errorMessage or "" .. i18n.translatef("Unable to set firmware environment variable: %s to %s.", bev1, newEnvSetting)
							util.perror(i18n.translatef("Unable to set firmware environment variable: %s to %s.", bev1, newEnvSetting))
						end
				end
			end
			if bev2 then
				curEnvSetting = util.trim(util.exec("fw_printenv -n " .. bev2))
				if not curEnvSetting then
					errorMessage = errorMessage or "" .. i18n.translatef("Unable to obtain firmware environment variable: %s.", bev2)
					util.perror(i18n.translatef("Unable to obtain firmware environment variable: %s.", bev2))
				else
					newEnvSetting = curEnvSetting == bev2p1 and bev2p2 or bev2p1
					errorCode = sys.call("fw_setenv " .. bev2 .. " '" .. newEnvSetting .. "'")
					if errorCode ~= 0 then
						errorMessage = errorMessage or "" .. i18n.translatef("Unable to set firmware environment variable: %s to %s.", bev2, newEnvSetting)
						util.perror(i18n.translatef("Unable to set firmware environment variable: %s to %s.", bev2, newEnvSetting))
					end
				end
			end
		else -- NetGear device
			if not zyxelFlagPartition then zyxelFlagPartition = util.trim(util.exec(". /lib/functions.sh; find_mtd_part 0:DUAL_FLAG")) end
			if not zyxelFlagPartition then
				errorMessage = errorMessage .. i18n.translate("Unable to find Dual Boot Flag Partition." .. " ")
				util.perror(i18n.translate("Unable to find Dual Boot Flag Partition."))
			else
				zyxelBootFlag = tonumber(util.exec("dd if=" .. zyxelFlagPartition .. " bs=1 count=1 2>/dev/null | hexdump -n 1 -e '1/1 \"%d\"'"))
				zyxelNewBootFlag = zyxelBootFlag and zyxelBootFlag == 1 and "\\xff" or "\\x01"
				if zyxelNewBootFlag then
					errorCode = sys.call("printf \"" .. zyxelNewBootFlag .. "\" >" .. zyxelFlagPartition )
					if errorCode ~= 0 then
						errorMessage = errorMessage or "" .. i18n.translatef("Unable to set Dual Boot Flag Partition entry for partition: %s.", zyxelFlagPartition)
						util.perror(i18n.translatef("Unable to set Dual Boot Flag Partition entry for partition: %s.", zyxelFlagPartition))
					end
				end
			end
		end
		if not errorMessage then
			ltemplate.render("advanced_reboot/applyreboot", {
						title = i18n.translate("Rebooting..."),
						msg   = i18n.translate("The system is rebooting to an alternative partition now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
						addr  = ip.new(util.imatch(uci:get("network", "lan", "ipaddr"))() or "192.168.1.1")
					})
			sys.reboot()
		else
			ltemplate.render("advanced_reboot/advanced_reboot",{
				romBoardName=romBoardName,
				device_name=device_name,
				bev1p1=bev1p1,
				p1_os=p1_os,
				bev1p2=bev1p2,
				p2_os=p2_os,
				current_partition=current_partition,
				errorMessage = errorMessage})
		end
	end
end

function action_poweroff()
	if http.formvalue("cancel") then
		http.redirect(dispatcher.build_url('admin/system/advanced_reboot'))
		return
	end
	local step = tonumber(http.formvalue("step") or 1)
	if step == 1 then
		if fs.access("/sbin/poweroff") then
			ltemplate.render("advanced_reboot/power_off",{})
		else
			ltemplate.render("advanced_reboot/advanced_reboot",{})
		end
	elseif step == 2 then
		ltemplate.render("advanced_reboot/applyreboot", {
					title = i18n.translate("Shutting down..."),
					msg   = i18n.translate("The system is shutting down now.<br /> DO NOT POWER OFF THE DEVICE!<br /> It might be necessary to renew the address of your computer to reach the device again, depending on your settings."),
					addr  = ip.new(util.imatch(uci:get("network", "lan", "ipaddr"))() or "192.168.1.1")
				})
		sys.call("/sbin/poweroff")
	end
end
