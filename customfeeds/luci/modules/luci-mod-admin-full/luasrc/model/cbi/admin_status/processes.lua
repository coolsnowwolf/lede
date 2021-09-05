-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

f = SimpleForm("processes", translate("Processes"), translate("This list gives an overview over currently running system processes and their status."))
f.reset = false
f.submit = false

t = f:section(Table, luci.sys.process.list())
t:option(DummyValue, "PID", translate("PID"))
t:option(DummyValue, "USER", translate("Owner"))
t:option(DummyValue, "COMMAND", translate("Command"))
t:option(DummyValue, "%CPU", translate("CPU usage (%)"))
t:option(DummyValue, "%MEM", translate("Memory usage (%)"))

hup = t:option(Button, "_hup", translate("Hang Up"))
hup.inputstyle = "reload"
function hup.write(self, section)
	null, self.tag_error[section] = luci.sys.process.signal(section, 1)
end

term = t:option(Button, "_term", translate("Terminate"))
term.inputstyle = "remove"
function term.write(self, section)
	null, self.tag_error[section] = luci.sys.process.signal(section, 15)
end

kill = t:option(Button, "_kill", translate("Kill"))
kill.inputstyle = "reset"
function kill.write(self, section)
	null, self.tag_error[section] = luci.sys.process.signal(section, 9)
end

return f