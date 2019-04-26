--[[
LuCI - Lua Configuration Interface - Internet access control

Copyright 2015 Krzysztof Szuster.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local CONFIG_FILE_RULES = "firewall"  
local CONFIG_FILE_AC    = "access_control"
local ma, mr, s, o

ma = Map(CONFIG_FILE_AC, translate("Internet Access Schedule Control"),
    translate("Access Schedule Control Settins"))
if CONFIG_FILE_AC==CONFIG_FILE_RULES then
    mr = ma
else
    mr = Map(CONFIG_FILE_RULES)
end
---------------------------------------------------------------------------------------------
--  General switch

s = ma:section(NamedSection, "general", "access_control")
    o_global_enable = s:option(Flag, "enabled", translate("Enabled"))
        o_global_enable.rmempty = false
        
---------------------------------------------------------------------------------------------
-- Rule table

s = mr:section(TypedSection, "rule", translate("Client Rules"))
    s.addremove = true
    s.anonymous = true
--    s.sortable  = true
    s.template = "cbi/tblsection"
    -- hidden, constant options
    s.defaults.enabled = "0"
    s.defaults.src     = "*" --"lan", "guest" or enything on local side
    s.defaults.dest    = "wan"
    s.defaults.target  = "REJECT"
    s.defaults.proto    = "0"
--    s_rule.defaults.extra = "--kerneltz"
    
    -- only AC-related rules
    s.filter = function(self, section)
	      return self.map:get (section, "ac_enabled") ~= nil
    end
        
    o = s:option(Flag, "ac_enabled", translate("Enabled"))
        o.default = '1'
        o.rmempty  = false
    
        -- ammend "enabled" option and set weekdays  
        function o.write(self, section, value)
            wd_write (self, section, value)
            local key = o_global_enable:cbid (o_global_enable.section.section)
            --  "cbid.access_control.general.enabled"
            local global_enable = o_global_enable.map:formvalue (key)
            if global_enable == "1" then
                self.map:set(section, "enabled", value)
            else
                self.map:set(section, "enabled", "0")
            end	
--            self.map:set(section, "src",  "*")
--            self.map:set(section, "dest", "wan")
--            self.map:set(section, "target", "REJECT")
--            self.map:set(section, "proto", "0")
--            self.map:set(section, "extra", "--kerneltz")
            return Flag.write(self, section, value)
        end
      
--o = s:option(Value, "name", translate("Description"))
--        o.rmempty = false  -- force validate
--        -- better validate, then: o.datatype = "minlength(1)"
--        o.validate = function(self, val, sid)
--            if type(val) ~= "string" or #val == 0 then
--                return nil, translate("Name must be specified!")
--            end
--            return val
--        end
        
     o = s:option(Value, "src_mac", translate("MAC address (Computer Name)")) 
        o.rmempty = false
        o.datatype = "macaddr"
        luci.sys.net.mac_hints(function(mac, name)
            o:value(mac, "%s (%s)" %{ mac, name })
        end)

    function validate_time(self, value, section)
        local hh, mm, ss
        hh, mm, ss = string.match (value, "^(%d?%d):(%d%d):(%d%d)$")
        hh = tonumber (hh)
        mm = tonumber (mm)
        ss = tonumber (ss)
        if hh and mm and hh <= 23 and mm <= 59 and ss <= 59 then
            return value
        else
            return nil, "Time value must be HH:MM:SS or empty"
        end
    end
    o = s:option(Value, "start_time", translate("Start time"))
        o.rmempty = true  -- do not validae blank
        o.validate = validate_time 
        o.size = 5
    o = s:option(Value, "stop_time", translate("End time")) 
        o.rmempty = true  -- do not validae blank
        o.validate = validate_time
        o.size = 5

    local Days = {'Mon','Tue','Wed','Thu','Fri','Sat','Sun'}
    local Days1 = {'Mon','Tue','Wed','Thu','Fri','Sat','Sun'}
    
    function make_day (nday)
        local day = Days[nday]
        local label = Days1[nday]
        local o = s:option(Flag, day, translate(label))
        o.default = '1'
        o.rmempty = false  --  always call write
        
        -- read from weekdays actually
        function o.cfgvalue(self, s)
            local days = self.map:get (s, "weekdays")
            if days==nil then
                return '1'
            end
            return string.find (days, day) and '1' or '0'
        end
     
        --  prevent saveing option in config file   
        function o.write(self, section, value)
            self.map:set(section, self.option, '')
        end
    end
  
    for i=1,7 do   
        make_day (i)
    end   
    
    function wd_write(self, section, value)
        value=''
        local cnt=0
        for _,day in ipairs (Days) do
            local key = "cbid."..self.map.config.."."..section.."."..day
--io.stderr:write (tostring(key)..'='..tostring(mr:formvalue(key))..'\n')
            if mr:formvalue(key) then
                value = value..' '..day
                cnt = cnt+1
            end
        end
        if cnt==7  then  --all days means no filterung 
            value = ''
        end
        self.map:set(section, "weekdays", value)
    end


if CONFIG_FILE_AC==CONFIG_FILE_RULES then
  return ma
else
  return ma, mr
end

