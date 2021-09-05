--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-diskman>
]]--

require "luci.util"
module("luci.controller.diskman",package.seeall)

function index()
  -- check all used executables in disk management are existed
  local CMD = {"parted", "blkid", "smartctl"}
  local executables_all_existed = true
  for _, cmd in ipairs(CMD) do
    local command = luci.sys.exec("/usr/bin/which " .. cmd)
    if not command:match(cmd) then
      executables_all_existed = false
      break
    end
  end

  if not executables_all_existed then return end
  -- entry(path, target, title, order)
  -- set leaf attr to true to pass argument throughe url (e.g. admin/system/disk/partition/sda)
  entry({"admin", "system", "diskman"}, alias("admin", "system", "diskman", "disks"), _("Disk Man"), 55)
  entry({"admin", "system", "diskman", "disks"}, form("diskman/disks"), nil).leaf = true
  entry({"admin", "system", "diskman", "partition"}, form("diskman/partition"), nil).leaf = true
  entry({"admin", "system", "diskman", "btrfs"}, form("diskman/btrfs"), nil).leaf = true
  entry({"admin", "system", "diskman", "format_partition"}, call("format_partition"), nil).leaf = true
  entry({"admin", "system", "diskman", "get_disk_info"}, call("get_disk_info"), nil).leaf = true
  entry({"admin", "system", "diskman", "mk_p_table"}, call("mk_p_table"), nil).leaf = true
  entry({"admin", "system", "diskman", "smartdetail"}, call("smart_detail"), nil).leaf = true
  entry({"admin", "system", "diskman", "smartattr"}, call("smart_attr"), nil).leaf = true
end

function format_partition()
  local partation_name = luci.http.formvalue("partation_name")
  local fs = luci.http.formvalue("file_system")
  if not partation_name then
    luci.http.status(500, "Partition NOT found!")
    luci.http.write_json("Partition NOT found!")
    return
  elseif not nixio.fs.access("/dev/"..partation_name) then
    luci.http.status(500, "Partition NOT found!")
    luci.http.write_json("Partition NOT found!")
    return
  elseif not fs then
    luci.http.status(500, "no file system")
    luci.http.write_json("no file system")
    return
  end
  local dm = require "luci.model.diskman"
  code, msg = dm.format_partition(partation_name, fs)
  luci.http.status(code, msg)
  luci.http.write_json(msg)
end

function get_disk_info(dev)
  if not dev then
    luci.http.status(500, "no device")
    luci.http.write_json("no device")
    return
  elseif not nixio.fs.access("/dev/"..dev) then
    luci.http.status(500, "no device")
    luci.http.write_json("no device")
    return
  end
  local dm = require "luci.model.diskman"
  local device_info = dm.get_disk_info(dev)
  luci.http.status(200, "ok")
  luci.http.prepare_content("application/json")
  luci.http.write_json(device_info)
end

function mk_p_table()
  local p_table = luci.http.formvalue("p_table")
  local dev = luci.http.formvalue("dev")
  if not dev then
    luci.http.status(500, "no device")
    luci.http.write_json("no device")
    return
  elseif not nixio.fs.access("/dev/"..dev) then
    luci.http.status(500, "no device")
    luci.http.write_json("no device")
    return
  end
  local dm = require "luci.model.diskman"
  if p_table == "GPT" or p_table == "MBR" then
    p_table = p_table == "MBR" and "msdos" or "gpt"
    local res = luci.sys.call(dm.command.parted .. " -s /dev/" .. dev .. " mktable ".. p_table)
    if res == 0 then
      luci.http.status(200, "ok")
    else
      luci.http.status(500, "command exec error")
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json({code=res})
  else
    luci.http.status(404, "not support")
    luci.http.prepare_content("application/json")
    luci.http.write_json({code="1"})
  end
end

function smart_detail(dev)
  luci.template.render("diskman/smart_detail", {dev=dev})
end

function smart_attr(dev)
  local dm = require "luci.model.diskman"
  local cmd = io.popen(dm.command.smartctl ..  " --attributes /dev/%s" % dev)
  if cmd then
    local attr = { }
    if dev:match("nvme")then
      while true do
        local ln = cmd:read("*l")
        if not ln then
          break
        elseif ln:match("^(.-):%s+(.+)") then
          local key, value = ln:match("^(.-):%s+(.+)")
          attr[#attr+1]= {
              key = key,
              value = value
            }
        end
      end
    else
      while true do
        local ln = cmd:read("*l")
        if not ln then
          break
        elseif ln:match("^.*%d+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+%s+.+") then
          local id,attrbute,flag,value,worst,thresh,type,updated,raw = ln:match("^%s*(%d+)%s+([%a%p]+)%s+(%w+)%s+(%d+)%s+(%d+)%s+(%d+)%s+([%a%p]+)%s+(%a+)%s+[%w%p]+%s+(.+)")
          id= "%x" % id
          if not id:match("^%w%w") then
            id = "0%s" % id
          end
          attr[#attr+1]= {
              id = id:upper(),
              attrbute = attrbute,
              flag  = flag,
              value = value,
              worst = worst,
              thresh  = thresh,
              type = type,
              updated = updated,
              raw  = raw
            }
        end
      end
    end
  cmd:close()
  luci.http.prepare_content("application/json")
  luci.http.write_json(attr)
  end
end