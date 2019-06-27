local USER_FILE_PATH = "/etc/config/wrtbwmon.user"

local fs = require "nixio.fs"

local f = SimpleForm("wrtbwmon", 
    translate("Usage - Custom User File"), 
    translate("This file is used to match users with MAC addresses and it must have the following format: 00:aa:bb:cc:ee:ff,username"))

local o = f:field(Value, "_custom")

o.template = "cbi/tvalue"
o.rows = 20

function o.cfgvalue(self, section)
    return fs.readfile(USER_FILE_PATH)
end

function o.write(self, section, value)
    value = value:gsub("\r\n?", "\n")
    fs.writefile(USER_FILE_PATH, value)
end

return f
