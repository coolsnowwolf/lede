#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path

local vif_prefix = {"ra", "rai", "rae", "rax", "ray", "raz", }

local function esc(x)
   return (x:gsub('%%', '%%%%')
            :gsub('^%^', '%%^')
            :gsub('%$$', '%%$')
            :gsub('%(', '%%(')
            :gsub('%)', '%%)')
            :gsub('%.', '%%.')
            :gsub('%[', '%%[')
            :gsub('%]', '%%]')
            :gsub('%*', '%%*')
            :gsub('%+', '%%+')
            :gsub('%-', '%%-')
            :gsub('%?', '%%?'))
end

function add_vif_into_lan(vif)
    local mtkwifi = require("mtkwifi")
end

function mt7615_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")

    nixio.syslog("debug", "mt7615_up called!")

    os.execute("ifconfig ra0 up")
    os.execute("ifconfig apcli0 up" )
    os.execute("ifconfig rax0 up")
    os.execute("ifconfig apclix0 up" )
    os.execute("ifconfig rai0 up")
    os.execute("ifconfig apclii0 up " )
    os.execute("brctl addif br-lan ra0 " )
    os.execute("brctl addif br-lan rax0 "  )
    os.execute("brctl addif br-lan rai0 " )
    os.execute(" rm -rf /tmp/mtk/wifi/mt7615*.need_reload")
end

function mt7615_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615_down called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    os.execute("ifconfig ra0 down")
    os.execute("ifconfig apcli0 down")
    os.execute("ifconfig rax0 down")
    os.execute("ifconfig apclix0 down")
    os.execute("ifconfig rai0 down")
    os.execute("ifconfig apclii0 down")
    os.execute(" rm -rf /tmp/mtk/wifi/mt7615*.need_reload")
end

function mt7615_reload(devname)
    local nixio = require("nixio")
    mt7615_down(devname)
    mt7615_up(devname)
end

function mt7615_restart(devname)
    local nixio = require("nixio")
    mt7615_down(devname)
    os.execute("rmmod mt_wifi")
    os.execute("modprobe mt_wifi")
    mt7615_up(devname)
end

function mt7615_reset(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615_reset called!")
    if mtkwifi.exists("/rom/etc/wireless/mt7615/") then
        os.execute("rm -rf /etc/wireless/mt7615/")
        os.execute("cp -rf /rom/etc/wireless/mt7615/ /etc/wireless/")
        mt7615_reload(devname)
    else
        nixio.syslog("debug", "mt7615_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mt7615_status(devname)
    return wifi_common_status()
end

function mt7615_detect(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615_detect called!")

    for _,dev in ipairs(mtkwifi.get_all_devs()) do
        local relname = string.format("%s%d%d",dev.maindev,dev.mainidx,dev.subidx)
        print([[
config wifi-device ]]..relname.."\n"..[[
    option type mt7615
    option vendor ralink
]])
        for _,vif in ipairs(dev.vifs) do
            print([[
config wifi-iface
    option device ]]..relname.."\n"..[[
    option ifname ]]..vif.vifname.."\n"..[[
    option network lan
    option mode ap
    option ssid ]]..vif.__ssid.."\n")
        end
    end
end
