#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path

local vif_prefix = {"ra", "rai", "rae", "rax", "ray", "raz",
    "apcli", "apclix", "apclii", "apcliy", "apclie", "apcliz", }

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
    local brvifs = mtkwifi.__trim(
        mtkwifi.read_pipe("uci get network.lan.ifname"))
    if not string.match(brvifs, esc(vif)) then
        nixio.syslog("debug", "mt7615_up: add "..vif.." into lan")
        brvifs = brvifs.." "..vif
        os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
        os.execute("uci commit")
        -- os.execute("brctl addif br-lan "..vif)
        os.execute("ubus call network.interface.lan add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
    end
end

function mt7615_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")

    nixio.syslog("debug", "mt7615_up called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615_up: dev "..devname.." not found!")
            return
        end
        -- we have to bring up main_ifname first, main_ifname will create all other vifs.
        if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("debug", "mt7615_up: ifconfig "..dev.main_ifname.." up")
            os.execute("ifconfig "..dev.main_ifname.." up")
            add_vif_into_lan(dev.main_ifname)
        else
            nixio.syslog("err", "mt7615_up: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif ~= dev.main_ifname and
            (  string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
            or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+"))
            then
                nixio.syslog("debug", "mt7615_up: ifconfig "..vif.."0 up")
                os.execute("ifconfig "..vif.." up")
                add_vif_into_lan(vif)
            -- else nixio.syslog("debug", "mt7615_up: skip "..vif..", prefix not match "..pre)
            end
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
            -- we have to bring up root vif first, root vif will create all other vifs.
            if mtkwifi.exists("/sys/class/net/"..pre.."0") then
                nixio.syslog("debug", "mt7615_up: ifconfig "..pre.."0 up")
                os.execute("ifconfig "..pre.."0 up")
                add_vif_into_lan(pre.."0")
            end

            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                -- nixio.syslog("debug", "mt7615_up: navigate "..pre)
                if string.match(vif, pre.."[1-9]+") then
                    nixio.syslog("debug", "mt7615_up: ifconfig "..vif.." up")
                    os.execute("ifconfig "..vif.." up")
                    add_vif_into_lan(vif)
                -- else nixio.syslog("debug", "mt7615_up: skip "..vif..", prefix not match "..pre)
                end
            end
        end
    else nixio.syslog("debug", "mt7615_up: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615*.need_reload")
end

function mt7615_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615_down called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615_down: dev "..devname.." not found!")
            return
        end

        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif == dev.main_ifname
            or string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
            or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+")
            then
                nixio.syslog("debug", "mt7615_down: ifconfig "..vif.." down")
                os.execute("ifconfig "..vif.." down")
                local brvifs = mtkwifi.__trim(mtkwifi.read_pipe("uci get network.lan.ifname"))
                if string.match(brvifs, esc(vif)) then
                    brvifs = mtkwifi.__trim(string.gsub(brvifs, esc(vif), ""))
                    nixio.syslog("debug", "mt7615_down: remove "..vif.." from lan")
                    os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
                    os.execute("uci commit")
                    os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
                end
            -- else nixio.syslog("debug", "mt7615_down: skip "..vif..", prefix not match "..pre)
            end
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                if string.match(vif, pre.."[0-9]+") then
                    nixio.syslog("debug", "mt7615_down: ifconfig "..vif.."down")
                    os.execute("ifconfig "..vif.." down")
                    local brvifs = mtkwifi.read_pipe("uci get network.lan.ifname")
                    if string.match(brvifs, vif) then
                    brvifs = mtkwifi.__trim(string.gsub(brvifs, vif, ""))
                    nixio.syslog("debug", "mt7615_down: remove "..vif.." from lan")
                    os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
                    os.execute("uci commit")
                    os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
                    end
                -- else nixio.syslog("debug", "mt7615_down: skip "..vif..", prefix not match "..pre)
                end
            end
        end
    else nixio.syslog("debug", "mt7615_down: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615*.need_reload")
end

function mt7615_reload(devname)
    local nixio = require("nixio")
    nixio.syslog("debug", "mt7615_reload called!")
    mt7615_down(devname)
    mt7615_up(devname)
end

function mt7615_restart(devname)
    local nixio = require("nixio")
    nixio.syslog("debug", "mt7615_restart called!")
    mt7615_down(devname)
    os.execute("rmmod mt7615")
    os.execute("modprobe mt7615")
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

