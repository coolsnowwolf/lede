-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

if luci.http.formvalue("cbid.luci.1._list") then
	luci.http.redirect(luci.dispatcher.build_url("admin/system/flashops/backupfiles") .. "?display=list")
elseif luci.http.formvalue("cbid.luci.1._edit") then
	luci.http.redirect(luci.dispatcher.build_url("admin/system/flashops/backupfiles") .. "?display=edit")
	return
end

m = SimpleForm("luci", translate("Backup file list"))
m:append(Template("admin_system/backupfiles"))

if luci.http.formvalue("display") ~= "list" then
	f = m:section(SimpleSection, nil, translate("This is a list of shell glob patterns for matching files and directories to include during sysupgrade. Modified files in /etc/config/ and certain other configurations are automatically preserved."))

	l = f:option(Button, "_list", translate("Show current backup file list"))
	l.inputtitle = translate("Open list...")
	l.inputstyle = "apply"

	c = f:option(TextValue, "_custom")
	c.rmempty = false
	c.cols = 70
	c.rows = 30

	c.cfgvalue = function(self, section)
		return nixio.fs.readfile("/etc/sysupgrade.conf")
	end

	c.write = function(self, section, value)
		value = value:gsub("\r\n?", "\n")
		return nixio.fs.writefile("/etc/sysupgrade.conf", value)
	end
else
	m.submit = false
	m.reset  = false

	f = m:section(SimpleSection, nil, translate("Below is the determined list of files to backup. It consists of changed configuration files marked by opkg, essential base files and the user defined backup patterns."))

	l = f:option(Button, "_edit", translate("Back to configuration"))
	l.inputtitle = translate("Close list...")
	l.inputstyle = "link"


	d = f:option(DummyValue, "_detected")
	d.rawhtml = true
	d.cfgvalue = function(s)
		local list = io.popen(
			"( find $(sed -ne '/^[[:space:]]*$/d; /^#/d; p' /etc/sysupgrade.conf " ..
			"/lib/upgrade/keep.d/* 2>/dev/null) -type f 2>/dev/null; " ..
			"opkg list-changed-conffiles ) | sort -u"
		)

		if list then
			local files = { "<ul>" }

			while true do
				local ln = list:read("*l")
				if not ln then
					break
				else
					files[#files+1] = "<li>"
					files[#files+1] = luci.util.pcdata(ln)
					files[#files+1] = "</li>"
				end
			end

			list:close()
			files[#files+1] = "</ul>"

			return table.concat(files, "")
		end

		return "<em>" .. translate("No files found") .. "</em>"
	end

end

return m
