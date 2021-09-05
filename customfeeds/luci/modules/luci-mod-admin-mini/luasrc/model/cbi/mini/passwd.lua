-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

f = SimpleForm("password", translate("Admin Password"), translate("Change the password of the system administrator (User <code>root</code>)"))

pw1 = f:field(Value, "pw1", translate("Password"))
pw1.password = true
pw1.rmempty = false

pw2 = f:field(Value, "pw2", translate("Confirmation"))
pw2.password = true
pw2.rmempty = false

function pw2.validate(self, value, section)
	return pw1:formvalue(section) == value and value
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		local stat = luci.sys.user.setpasswd("root", data.pw1) == 0
		
		if stat then
			f.message = translate("Password successfully changed")
		else
			f.errmessage = translate("Unknown Error")
		end
		
		data.pw1 = nil
		data.pw2 = nil
	end
	return true
end

return f