local m, s, o
local NXFS = require 'nixio.fs'

m = Map('vssr', translate('Router domain config'))

s = m:section(TypedSection, 'access_control')
s.anonymous = true

--Youtube proxy
s:tab('youtube', translate('Youtube Domain'))
local youtbeconf = '/etc/vssr/youtube_domain.list'
o = s:taboption('youtube', TextValue, 'youtbeconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(youtbeconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(youtbeconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(youtbeconf, '')
end

--tw_video proxy
s:tab('tw_video', translate('Tw Video Domain'))
local tw_videoconf = '/etc/vssr/tw_video_domain.list'
o = s:taboption('tw_video', TextValue, 'tw_videoconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(tw_videoconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(tw_videoconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(tw_videoconf, '')
end

--netflix proxy
s:tab('netflix', translate('Netflix Domain'))
local netflixconf = '/etc/vssr/netflix_domain.list'
o = s:taboption('netflix', TextValue, 'netflixconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(netflixconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(netflixconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(netflixconf, '')
end

--Diseny proxy
s:tab('disney', translate('Disney+ Domain'))
local disneyconf = '/etc/vssr/disney_domain.list'
o = s:taboption('disney', TextValue, 'disneyconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(disneyconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(disneyconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(disneyconf, '')
end

--prime proxy
s:tab('prime', translate('Prime Video Domain'))
local primeconf = '/etc/vssr/prime_domain.list'
o = s:taboption('prime', TextValue, 'primeconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(primeconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(primeconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(primeconf, '')
end

--tvb proxy
s:tab('tvb', translate('TVB Domain'))
local tvbconf = '/etc/vssr/tvb_domain.list'
o = s:taboption('tvb', TextValue, 'tvbconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(tvbconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(tvbconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(tvbconf, '')
end

--custom proxy
s:tab('custom', translate('Custom Domain'))
local customconf = '/etc/vssr/custom_domain.list'
o = s:taboption('custom', TextValue, 'customconf')
o.rows = 13
o.wrap = 'off'
o.rmempty = true
o.cfgvalue = function(self, section)
    return NXFS.readfile(customconf) or ' '
end
o.write = function(self, section, value)
    NXFS.writefile(customconf, value:gsub('\r\n', '\n'))
end
o.remove = function(self, section, value)
    NXFS.writefile(customconf, '')
end

return m
