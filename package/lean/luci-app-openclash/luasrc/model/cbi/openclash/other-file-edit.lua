local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local fs = require "luci.openclash"
local file_path = ""
local edit_file_name = "/tmp/openclash_edit_file_name"

for i = 2, #(arg) do
	file_path = file_path .. "/" .. luci.http.urlencode(arg[i])
end

if not fs.isfile(file_path) and file_path ~= "" then
	file_path = luci.http.urldecode(file_path)
end

--re-get file path to save
if NXFS.readfile(edit_file_name) ~= file_path and fs.isfile(file_path) then
	NXFS.writefile(edit_file_name, file_path)
else
	if not fs.isfile(file_path) and fs.isfile(edit_file_name) then
		file_path = NXFS.readfile(edit_file_name)
		fs.unlink(edit_file_name)
	end
end

m = Map("openclash", translate("File Edit"))
m.pageaction = false
m.redirect = luci.dispatcher.build_url("admin/services/openclash/"..arg[1])
s = m:section(TypedSection, "openclash")
s.anonymous = true
s.addremove=false

o = s:option(TextValue, "edit_file")
o.rows = 50
o.wrap = "off"

function o.write(self, section, value)
	if value then
		value = value:gsub("\r\n?", "\n")
		local old_value = NXFS.readfile(file_path)
		if value ~= old_value then
			NXFS.writefile(file_path, value)
		end
	end
end

function o.cfgvalue(self, section)
	return NXFS.readfile(file_path) or ""
end

local t = {
    {Commit, Back}
}

a = m:section(Table, t)

o = a:option(Button, "Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
	luci.http.redirect(m.redirect)
end

o = a:option(Button,"Back", " ")
o.inputtitle = translate("Back Settings")
o.inputstyle = "reset"
o.write = function()
	luci.http.redirect(m.redirect)
end

m:append(Template("openclash/config_editor"))
m:append(Template("openclash/toolbar_show"))
return m