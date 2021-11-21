mp = Map("familycloud")
mp.title = translate("天翼家庭云/天翼云盘提速")
mp.description = translate("天翼家庭云/天翼云盘提速 (最高可达500Mbps)")

mp:section(SimpleSection).template = "familycloud/familycloud_status"

s = mp:section(TypedSection, "familycloud")
s.anonymous = true
s.addremove = false

enabled = s:option(Flag, "enabled", translate("启用提速"))
enabled.default = 0
enabled.rmempty = false

speedtype = s:option(ListValue, "speedertype", translate("天翼提速包类型"))
speedtype:value("CloudDisk", translate("天翼云盘提速"))
speedtype:value("FamilyCloud", translate("天翼家庭云提速"))

account = s:option(Value, "token", translate("AccessToken"))
account.datatype = "string"

return mp
