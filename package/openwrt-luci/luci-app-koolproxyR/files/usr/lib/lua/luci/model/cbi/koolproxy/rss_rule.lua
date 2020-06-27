local m, s, o
local koolproxy = "koolproxy"
local sid = arg[1]

m = Map(koolproxy, "%s - %s" %{translate("koolproxy"), translate("编辑规则")})
m.redirect = luci.dispatcher.build_url("admin/services/koolproxy")

if not arg[1] or m.uci:get(koolproxy, sid) ~= "rss_rule" then
	luci.http.redirect(m.redirect)
	return
end

-- [[ Edit Rule ]]--
s = m:section(NamedSection, sid, "rss_rule")
s.anonymous = true
s.addremove = true

o=s:option(Flag,"load",translate("启用"))
o.default=0
o.rmempty=false

o=s:option(Value,"name",translate("规则描述"))
o.rmempty=true

o=s:option(Value,"url",translate("规则地址"))
o.rmempty=false
o.placeholder="[https|http|ftp]://[Hostname]/[File]"
function o.validate(self, value)
	if not value then
		return nil
	else
		return value
	end
end

return m
