-- Copyright 2016 Eric Luehrsen <ericluehrsen@hotmail.com>
-- Licensed to the public under the Apache License 2.0.

local m2, s2, frm
local filename = "/etc/unbound/unbound.conf"
local description =  translatef("Here you may edit raw 'unbound.conf' when you don't use UCI:")
description = description .. " (" .. filename .. ")"

m2 = SimpleForm("editing", nil)
m2:append(Template("unbound/css-editing"))
m2.submit = translate("Save")
m2.reset = false
s2 = m2:section(SimpleSection, "Unbound Conf", description)
frm = s2:option(TextValue, "data")
frm.datatype = "string"
frm.rows = 20


function frm.cfgvalue()
  return nixio.fs.readfile(filename) or ""
end


function frm.write(self, section, data)
  return nixio.fs.writefile(filename, luci.util.trim(data:gsub("\r\n", "\n")))
end


return m2

