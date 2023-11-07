--[[
LuCI - Lua Configuration Interface
Copyright 2023 jjm2473
]]--

local m, s, o
require "luci.util"

local fs = require "nixio.fs"

local sys = require "luci.sys"

local words = luci.util.split(luci.util.trim(sys.exec("/usr/libexec/fan-control get 2>/dev/null")), " ")

if #words < 3 then
  return
end

m = Map("luci-fan", translate("Fan Control"), translate("Control fan start and stop temperature. This plugin can only configure the lowest gear, not necessarily applicable to all devices."))

s = m:section(SimpleSection)
s.template = "luci-fan"
s.thermal_zone = words[1]
s.trip_point = words[2]
s.thermal_type = luci.util.trim(fs.readfile("/sys/class/thermal/"..words[1].."/type"))

s = m:section(TypedSection, "luci-fan", translate("Fan Settings"))
s.addremove = false
s.anonymous = true

o = s:option(Flag, "enabled", translate("Enabled"), translate("Whether to apply the settings"))
o.default = 0

local on_temp = s:option(Value, "on_temp", translate("Fan start temperature") .. " (&#8451;)")
on_temp.datatype = "and(uinteger,min(5))"
on_temp.rmempty = false
on_temp.default = math.floor(tonumber(words[3])/1000)
if #words > 3 then
  on_temp.datatype = string.format("and(uinteger,range(5,%u))", math.floor(tonumber(words[4])/1000 - 1))
end

o = s:option(Value, "off_temp", translate("Fan stop temperature") .. " (&#8451;)", translate("Optional, MUST be lower than the fan start temperature. Default fan start temperature minus 5 &#8451;"))
o.datatype = "uinteger"
o.rmempty = true
function o.parse(self, section, novld)
	local fvalue = self:formvalue(section)
	local fexist = ( fvalue and (#fvalue > 0) )	-- not "nil" and "not empty"

  local vvalue, errtxt = self:validate(fvalue)
  if not vvalue then
    if novld then 		-- and "novld" set
      return		-- then exit without raising an error
    end
		if fexist then		-- and there is a formvalue
			self:add_error(section, "invalid", errtxt or self.title .. ": invalid")
			return		-- so data are invalid
    end
  end
  local rm_opt = ( self.rmempty or self.optional )
  local vexist = ( vvalue and (#vvalue > 0) ) and true or false	-- not "nil" and "not empty"
	local eq_def = ( vvalue == self.default )			-- equal_default flag
	-- (rmempty or optional) and (no data or equal_default)
	if rm_opt and (not vexist or eq_def) then
		if self:remove(section) then		-- remove data from UCI
			self.section.changed = true	-- and push events
		end
		return
	end

  local cvalue = self:cfgvalue(section)
  local eq_cfg = ( vvalue == cvalue )					-- update equal_config flag
  -- not forcewrite and no changes, so nothing to write
	if not self.forcewrite and eq_cfg then
		return
	end

  local onvalue = on_temp:validate(on_temp:formvalue(section))
  if onvalue and #onvalue > 0 then
    if tonumber(vvalue) >= tonumber(onvalue) then
      self:add_error(section, "invalid", translate("Fan stop temperature MUST be lower than fan start temperature"))
      return
    end
  else
    return
  end

	-- write data to UCI; raise event only on changes
	if self:write(section, vvalue) and not eq_cfg then
		self.section.changed = true
	end
end

return m
