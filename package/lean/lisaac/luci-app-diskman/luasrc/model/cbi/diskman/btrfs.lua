--[[
LuCI - Lua Configuration Interface
Copyright 2019 lisaac <https://github.com/lisaac/luci-app-diskman>
]]--

require "luci.util"
require("luci.tools.webadmin")
local dm = require "luci.model.diskman"
local uuid = arg[1]

if not uuid then luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman")) end

-- mount subv=/ to tempfs
mount_point = "/tmp/.btrfs_tmp"
nixio.fs.mkdirr(mount_point)
luci.util.exec(dm.command.umount .. " "..mount_point .. " >/dev/null 2>&1")
luci.util.exec(dm.command.mount .. " -t btrfs -o subvol=/ UUID="..uuid.." "..mount_point)

m = SimpleForm("btrfs", translate("Btrfs"), translate("Manage Btrfs"))
m.template = "diskman/cbi/xsimpleform"
m.redirect = luci.dispatcher.build_url("admin/system/diskman")
m.submit = false
m.reset = false

-- info
local btrfs_info = dm.get_btrfs_info(mount_point)
local table_btrfs_info = m:section(Table, {btrfs_info}, translate("Btrfs Info"))
table_btrfs_info:option(DummyValue, "uuid", translate("UUID"))
table_btrfs_info:option(DummyValue, "members", translate("Members"))
table_btrfs_info:option(DummyValue, "data_raid_level", translate("Data"))
table_btrfs_info:option(DummyValue, "metadata_raid_lavel", translate("Metadata"))
table_btrfs_info:option(DummyValue, "size_formated", translate("Size"))
table_btrfs_info:option(DummyValue, "used_formated", translate("Used"))
table_btrfs_info:option(DummyValue, "free_formated", translate("Free Space"))
table_btrfs_info:option(DummyValue, "usage", translate("Usage"))
local v_btrfs_label = table_btrfs_info:option(Value, "label", translate("Label"))
local value_btrfs_label = ""
v_btrfs_label.write = function(self, section, value)
  value_btrfs_label = value or ""
end
local btn_update_label = table_btrfs_info:option(Button, "_update_label")
btn_update_label.inputtitle = translate("Update")
btn_update_label.inputstyle = "edit"
btn_update_label.write = function(self, section, value)
  local cmd = dm.command.btrfs .. " filesystem label " .. mount_point .. " " .. value_btrfs_label
  local res = luci.util.exec(cmd)
  luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman/btrfs/" .. uuid))
end
-- subvolume
local subvolume_list = dm.get_btrfs_subv(mount_point)
subvolume_list["_"] = { ID = 0 }
table_subvolume = m:section(Table, subvolume_list, translate("SubVolumes"))
table_subvolume:option(DummyValue, "id", translate("ID"))
table_subvolume:option(DummyValue, "top_level", translate("Top Level"))
table_subvolume:option(DummyValue, "uuid", translate("UUID"))
table_subvolume:option(DummyValue, "otime", translate("Otime"))
table_subvolume:option(DummyValue, "snapshots", translate("Snapshots"))
local v_path = table_subvolume:option(Value, "path", translate("Path"))
v_path.forcewrite = true
v_path.render = function(self, section, scope)
  if subvolume_list[section].ID == 0 then
    self.template = "cbi/value"
    self.placeholder = "/my_subvolume"
    self.forcewrite = true
    Value.render(self, section, scope)
  else
    self.template = "cbi/dvalue"
    DummyValue.render(self, section, scope)
  end
end
local value_path
v_path.write = function(self, section, value)
  value_path = value
end
local btn_set_default = table_subvolume:option(Button, "_subv_set_default", translate("Set Default"))
btn_set_default.forcewrite = true
btn_set_default.inputstyle = "edit"
btn_set_default.template = "diskman/cbi/disabled_button"
btn_set_default.render = function(self, section, scope)
  if subvolume_list[section].default_subvolume then
    self.view_disabled = true
    self.inputtitle = translate("Set Default")
  elseif subvolume_list[section].ID == 0 then
    self.template = "cbi/dvalue"
  else
    self.inputtitle = translate("Set Default")
    self.view_disabled = false
  end
  Button.render(self, section, scope)
end
btn_set_default.write = function(self, section, value)
  local cmd
  if value == translate("Set Default") then
    cmd = dm.command.btrfs .. " subvolume set-default " .. mount_point..subvolume_list[section].path
  else
    cmd = dm.command.btrfs .. " subvolume set-default " .. mount_point.."/"
  end
  local res = luci.util.exec(cmd.. " 2>&1")
  if res and (res:match("ERR") or res:match("not enough arguments")) then
    m.errmessage = res
  else
    luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman/btrfs/" .. uuid))
  end
end
local btn_remove = table_subvolume:option(Button, "_subv_remove")
btn_remove.template = "diskman/cbi/disabled_button"
btn_remove.forcewrite = true
btn_remove.render = function(self, section, scope)
  if subvolume_list[section].ID == 0 then
    btn_remove.inputtitle = translate("Create")
    btn_remove.inputstyle = "add"
    self.view_disabled = false
  elseif subvolume_list[section].path == "/" or subvolume_list[section].default_subvolume then
    btn_remove.inputtitle = translate("Delete")
    btn_remove.inputstyle = "remove"
    self.view_disabled = true
  else
    btn_remove.inputtitle = translate("Delete")
    btn_remove.inputstyle = "remove"
    self.view_disabled = false
  end
  Button.render(self, section, scope)
end

btn_remove.write = function(self, section, value)
  local cmd
  if value == translate("Delete") then
    cmd = dm.command.btrfs .. " subvolume delete " .. mount_point .. subvolume_list[section].path
  elseif value == translate("Create") then
    if value_path and value_path:match("^/") then
      cmd = dm.command.btrfs .. " subvolume create " .. mount_point .. value_path
    else
      m.errmessage = translate("Please input Subvolume Path, Subvolume must start with '/'")
      return
    end
  end
  local res = luci.util.exec(cmd.. " 2>&1")
  if res and (res:match("ERR") or res:match("not enough arguments")) then
    m.errmessage = luci.util.pcdata(res)
  else
    luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman/btrfs/" .. uuid))
  end
end
-- snapshot
-- local snapshot_list = dm.get_btrfs_subv(mount_point, 1)
-- table_snapshot = m:section(Table, snapshot_list, translate("Snapshots"))
-- table_snapshot:option(DummyValue, "id", translate("ID"))
-- table_snapshot:option(DummyValue, "top_level", translate("Top Level"))
-- table_snapshot:option(DummyValue, "uuid", translate("UUID"))
-- table_snapshot:option(DummyValue, "otime", translate("Otime"))
-- table_snapshot:option(DummyValue, "path", translate("Path"))
-- local snp_remove = table_snapshot:option(Button, "_snp_remove")
-- snp_remove.inputtitle = translate("Delete")
-- snp_remove.inputstyle = "remove"
-- snp_remove.write = function(self, section, value)
--   local cmd = dm.command.btrfs .. " subvolume delete " .. mount_point .. snapshot_list[section].path
--   local res = luci.util.exec(cmd.. " 2>&1")
--   if res and (res:match("ERR") or res:match("not enough arguments")) then
--     m.errmessage = luci.util.pcdata(res)
--   else
--     luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman/btrfs/" .. uuid))
--   end
-- end

-- new snapshots
local s_snapshot = m:section(SimpleSection, translate("New Snapshot"))
local value_sorce, value_dest, value_readonly
local v_sorce = s_snapshot:option(Value, "_source", translate("Source Path"), translate("The source path for create the snapshot"))
v_sorce.placeholder = "/data"
v_sorce.forcewrite = true
v_sorce.write = function(self, section, value)
  value_sorce = value
end

local v_readonly = s_snapshot:option(Flag, "_readonly", translate("Readonly"), translate("The path where you want to store the snapshot"))
v_readonly.forcewrite = true
v_readonly.rmempty = false
v_readonly.disabled = 0
v_readonly.enabled = 1
v_readonly.default = 1
v_readonly.write = function(self, section, value)
  value_readonly = value
end
local v_dest = s_snapshot:option(Value, "_dest", translate("Destination Path (optional)"))
v_dest.forcewrite = true
v_dest.placeholder = "/.snapshot/202002051538"
v_dest.write = function(self, section, value)
  value_dest = value
end
local btn_snp_create = s_snapshot:option(Button, "_snp_create")
btn_snp_create.title = " "
btn_snp_create.inputtitle = translate("New Snapshot")
btn_snp_create.inputstyle = "add"
btn_snp_create.write = function(self, section, value)
  if value_sorce and value_sorce:match("^/") then
    if not value_dest then value_dest = "/.snapshot"..value_sorce.."/"..os.date("%Y%m%d%H%M%S") end
    nixio.fs.mkdirr(mount_point..value_dest:match("(.-)[^/]+$"))
    local cmd = dm.command.btrfs .. " subvolume snapshot" .. (value_readonly == 1 and " -r " or " ") .. mount_point..value_sorce .. " " .. mount_point..value_dest
    local res = luci.util.exec(cmd .. " 2>&1")
    if res and (res:match("ERR") or res:match("not enough arguments")) then
      m.errmessage = luci.util.pcdata(res)
    else
      luci.http.redirect(luci.dispatcher.build_url("admin/system/diskman/btrfs/" .. uuid))
    end
  else
    m.errmessage = translate("Please input Source Path of snapshot, Source Path must start with '/'")
  end
end

return m
