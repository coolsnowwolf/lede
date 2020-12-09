local m, s, o
local NXFS = require 'nixio.fs'
local router_table = {
    yotube = {
        name = 'youtube',
        des = 'Youtube Domain'
    },
    tw_video = {
        name = 'tw_video',
        des = 'Tw Video Domain'
    },
    netflix = {
        name = 'netflix',
        des = 'Netflix Domain'
    },
    disney = {
        name = 'disney',
        des = 'Disney+ Domain'
    },
    prime = {
        name = 'prime',
        des = 'Prime Video Domain'
    },
    tvb = {
        name = 'tvb',
        des = 'TVB Domain'
    },
    custom = {
        name = 'custom',
        des = 'Custom Domain'
    }
}

m = Map('vssr', translate('Router domain config'))
s = m:section(TypedSection, 'access_control')
s.anonymous = true
for _, v in pairs(router_table) do
    s:tab(v.name, translate(v.des))
    local conf = '/etc/vssr/'.. v.name ..'_domain.list'
    o = s:taboption(v.name, TextValue, v.name ..'conf')
    o.rows = 13
    o.wrap = 'off'
    o.rmempty = true
    o.cfgvalue = function(self, section)
        return NXFS.readfile(conf) or ' '
    end
    o.write = function(self, section, value)
        NXFS.writefile(conf, value:gsub('\r\n', '\n'))
    end
    o.remove = function(self, section, value)
        NXFS.writefile(conf, '')
    end
end

return m
