local USER_FILE_PATH = "/etc/wrtbwmon.user"

local fs = require "nixio.fs"

local f = SimpleForm("wrtbwmon", 
    "自定义MAC地址对应的主机名", 
    "每一行的格式为  00:aa:bb:cc:ee:ff,username （不支持中文主机名）")

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
