--[[
# Copyright (c) 2014-2016, latyas <latyas@gmail.com>
# Edit by Twinzo <1282055288@qq.com>
]]--
module("luci.controller.configEditor", package.seeall)

local fs = require "nixio.fs"
local sys = require "luci.sys"
local util = require "luci.util"
function index()
	entry({"admin", "system", "configEditor"}, cbi("configEditor"), _("ConfigEditor"), 45).dependent = true
	entry({"admin","system","configEditor","readfile"},call("readfile")).leaf=true
	entry({"admin","system","configEditor","makedir"},call("makedir")).leaf=true
	entry({"admin","system","configEditor","alter"},call("alter")).leaf=true
	entry({"admin","system","configEditor","get_upgrade"},call("get_upgrade")).leaf=true
end

--读取文件内容
function readfile()
	local set = luci.http.formvalue("set")
	local id, path = set:match("^(%S+) (%S+)")
	if id== "readfile" then
		if path then
			content = fs.readfile(path)
			if content then
				retstring = "1" 
			else 
				retstring = "0"
			end
		else 
			retstring = "0"
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=retstring;ret1=content })
end

--创建空白文件
function makedir()
	local set = luci.http.formvalue("set")
	local id, path = set:match("^(%S+) (%S+)")
	if id == "makedir" then
		if path then
			content = fs.readfile(path)
			if not content then
				io.open("/etc/configEditor","a+")
				aa=fs.rename("../etc/configEditor",path)
			end
			contents = fs.readfile(path)
			if contents then
				retstring = "1" 
			else 
				retstring = "0"
			end
		else 
			retstring = "0"
		end
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ret=retstring})
end

--修改文件内容
function alter()
	local set = luci.http.formvalue("set")
	local id, path = set:match("^(%S+) (%S+)")
	local data = luci.http.formvalue(tet)
	if path then
		local fd = fs.realpath(path)
		if fd then
			if id == "alter" then
				retstring = fs.writefile(path, data)
				if retstring then
					retstring = "1"
				else 
					retstring = "0"
				end
			end
		else
			retsring = "0"
		end
	else
		retstring = "0"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring})
end

--自定义升级备份文件
function get_upgrade()
	local id = luci.http.formvalue("set")
	local path = luci.http.formvalue("pet")
	
	if path then
		path = fs.basename(path)
	end
	
	if id == "read" then
		retstring = "1"
		keep_d = sys.exec("ls /lib/upgrade/keep.d")
		ETC_keep_d = sys.exec("ls /lib/upgrade/keep.d")
		list = sys.exec(
				"( find $(sed -ne '/^[[:space:]]*$/d; /^#/d; p' /etc/sysupgrade.conf " ..
				"/lib/upgrade/keep.d/* 2>/dev/null) -type f 2>/dev/null; " ..
				"opkg list-changed-conffiles ) | sort -u"
			)
		des1 = "/lib/upgrade/keep.d 目录下的文件（custom为当前自定义文件）："
		des2 = "下面是待备份的文件清单。包含了更改的配置文件、必要的基础文件和用户自定义的需备份文件。"
		des3 = "/etc/keep.d 备份目录下的可恢复文件："
		content = des1 .. "\n" .. keep_d .. des3 .. "\n" .. ETC_keep_d .. des2 .. "\n" .. list
		if content then
			ret1 = content
		else
			ret1 = "Empty"
		end
	elseif id == "read_custom" then --读取自定义备份配置文件
		retstring = "2"
		custom_content = fs.readfile("/lib/upgrade/keep.d/custom")
		if custom_content then
			ret1 = custom_content
		else
			ret1 = "Empty"
		end
	elseif id == "delete" then --删除keep.d目录下文件
		retstring = "3"
		if path == "*" then
			sys.exec("mkdir -p /etc/keep.d 2>/dev/null && mv -f /lib/upgrade/keep.d/* mv -f /etc/keep.d/ 2>/dev/null")
			ret1 = "<b><span style='color: green'>文件已全部删除</span></b>"
		else
			if path then 
				paths = "/lib/upgrade/keep.d/"..path
				pathss = "/etc/keep.d/"..path
				local fk = fs.realpath(paths)
				if fk then
					sys.call("mkdir -p /etc/keep.d/ 2>/dev/null")
					fs.rename(".."..paths,pathss)
					fks = fs.realpath(paths)
					if not fks then				
						ret1 = "<b><span style='color: green'>删除成功</span></b>"
					else
						ret1 = "<b><span style='color: red'>删除失败</span></b>"
					end
				else
					ret1 = "<b><span style='color: red'>文件不存在</span></b>"
				end
			else
				ret1 = "<b><span style='color: red'>输入为空</span></b>"
			end
		end
	elseif id == "add" then --创建并修改自定义备份配置文件
		retstring = "4"
		path = "/lib/upgrade/keep.d/custom"
		sys.call("touch /lib/upgrade/keep.d/custom")
		local data = luci.http.formvalue("tet")
		rets = fs.writefile(path, data)
		if rets then
			ret1 = "<b><span style='color: green'>Succed</span></b>"
		else 
			ret1 = "<b><span style='color: red'>Failed</span></b>"
		end
	elseif id == "recover" then --恢复升级备份文件
		retstring = "5"
		if path == "*" then
			sys.exec("mkdir -p /etc/keep.d 2>/dev/null && mv -f /etc/keep.d/* /lib/upgrade/keep.d/ 2>/dev/null")
			ret1 = "<b><span style='color: green'>文件已全部恢复</span></b>"
		else
			if path then
				paths = "/etc/keep.d/"..path
				fr  = fs.realpath(paths)
				if fr then
					pathss = "/lib/upgrade/keep.d/"..path
					fs.rename(".."..paths,pathss)
					frs  = fs.realpath(pathss)
					if frs then
						ret1 = "<b><span style='color: green'>恢复成功</span></b>"
					else
						ret1 = "<b><span style='color: red'>失败</span></b>"
					end
				else
					ret1 = "<b><span style='color: red'>文件不存在，未备份或已恢复或输错文件名</span></b>"
				end
			else
				ret1 = "<b><span style='color: red'>输入为空</span></b>"
			end
		end
	else
		retstring = "0"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring;ret1=ret1})
end
