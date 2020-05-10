local e = require "nixio.fs"
local e = require "luci.sys"

m = Map("passwall")

-- [[ Subscribe Settings ]]--
s = m:section(TypedSection, "global_subscribe", "")
s.anonymous = true

---- Subscribe via proxy
o = s:option(Flag, "subscribe_proxy", translate("Subscribe via proxy"))
o.default = 0
o.rmempty = false

---- Enable auto update subscribe
o = s:option(Flag, "auto_update_subscribe",
             translate("Enable auto update subscribe"))
o.default = 0
o.rmempty = false

---- Week update rules
o = s:option(ListValue, "week_update_subscribe", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update_subscribe", 1)

---- Day update rules
o = s:option(ListValue, "time_update_subscribe", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update_subscribe", 1)

---- Manual subscription
o = s:option(Button, "_update", translate("Manual subscription"))
o.inputstyle = "apply"
function o.write(e, e)
    luci.sys.call(
        "lua /usr/share/passwall/subscribe.lua start log > /dev/null 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "log"))
end

---- Subscribe Delete All
o = s:option(Button, "_stop", translate("Delete All Subscribe Node"))
o.inputstyle = "remove"
function o.write(e, e)
    luci.sys.call(
        "lua /usr/share/passwall/subscribe.lua truncate log > /dev/null 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "log"))
end

filter_enabled = s:option(Flag, "filter_enabled", translate("Filter keyword switch"), translate("When checked, below options can only be take effect."))
o.default = 1
o.rmempty = false

filter_keyword = s:option(DynamicList, "filter_keyword", translate("Filter keyword"))
    
o = s:option(Flag, "filter_keyword_discarded", translate("Filter keyword discarded"), translate("When checked, the keywords in the list are discarded. Otherwise, it is not discarded."))
o.default = "1"
o.rmempty = false

s = m:section(TypedSection, "subscribe_list", "",
              "<font color='red'>" .. translate(
                  "Please input the subscription url first, save and submit before updating. If you subscribe to update, it is recommended to delete all subscriptions and then re-subscribe.") ..
                  "</font>")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(Value, "remark", translate("Subscribe Remark"))
o.width = "auto"
o.rmempty = false

o = s:option(Value, "url", translate("Subscribe URL"))
o.width = "auto"
o.rmempty = false

return m
