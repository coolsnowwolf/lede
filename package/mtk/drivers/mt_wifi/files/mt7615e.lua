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
    local brvifs = mtkwifi.__trim( mtkwifi.read_pipe("uci get network.lan.ifname"))
    if not string.match(brvifs, esc(vif)) then
        nixio.syslog("debug", "add "..vif.." into lan")
        -- brvifs = brvifs.." "..vif
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"") --netifd will down vif form /etc/config/network
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        os.execute("brctl addif br-lan "..vif) -- double insurance for rare failure
    end
end

function del_vif_from_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim(mtkwifi.read_pipe("uci get network.lan.ifname"))
    if string.match(brvifs, esc(vif)) then
        -- brvifs = mtkwifi.__trim(string.gsub(brvifs, esc(vif), ""))
        nixio.syslog("debug", "del "..vif.." from lan")
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        os.execute("brctl delif br-lan "..vif)
    end
end

function mt7615e_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7615e_up called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615e_up: dev "..devname.." not found!")
            return
        end
        -- we have to bring up main_ifname first, main_ifname will create all other vifs.
        if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("debug", "mt7615e_up: ifconfig "..dev.main_ifname.." up")
            os.execute("ifconfig "..dev.main_ifname.." up")
            add_vif_into_lan(dev.main_ifname)
            if wifi_services_exist then
                miniupnpd_chk(devname,dev.main_ifname,true)
            end
        else
            nixio.syslog("err", "mt7615e_up: main_ifname "..dev.main_ifname.." missing, quit!")
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
                nixio.syslog("debug", "mt7615e_up: ifconfig "..vif.."0 up")
                os.execute("ifconfig "..vif.." up")
                add_vif_into_lan(vif)
                if wifi_services_exist and string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                    miniupnpd_chk(devname,vif, true)
                end
            -- else nixio.syslog("debug", "mt7615e_up: skip "..vif..", prefix not match "..pre)
            end
        end
        os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=1")
        if wifi_services_exist then
             d8021xd_chk(devname,dev.ext_ifname,dev.main_ifname,true)
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615e/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
            -- we have to bring up root vif first, root vif will create all other vifs.
            if mtkwifi.exists("/sys/class/net/"..pre.."0") then
                nixio.syslog("debug", "mt7615e_up: ifconfig "..pre.."0 up")
                os.execute("ifconfig "..pre.."0 up")
                add_vif_into_lan(pre.."0")
            end

            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                -- nixio.syslog("debug", "mt7615e_up: navigate "..pre)
                if string.match(vif, pre.."[1-9]+") then
                    nixio.syslog("debug", "mt7615e_up: ifconfig "..vif.." up")
                    os.execute("ifconfig "..vif.." up")
                    add_vif_into_lan(vif)
                    if wifi_services_exist then
                        miniupnpd_chk(devname, vif, true)
                    end
                -- else nixio.syslog("debug", "mt7615e_up: skip "..vif..", prefix not match "..pre)
                end
            end
            if wifi_services_exist  then
                d8021xd_chk(devname,pre,pre.."0",true)
            end
        end
    else nixio.syslog("debug", "mt7615e_up: skip "..devname..", config not exist")
    end

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
        os.execute("/etc/init.d/man start")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615e*.need_reload")
end

function mt7615e_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7615e_down called!")

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
    end

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615e_down: dev "..devname.." not found!")
            return
        end
        os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=0")
        if wifi_services_exist then
            d8021xd_chk(devname,dev.ext_ifname,dev.main_ifname)
        end
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif == dev.main_ifname
            or string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
            or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+")
            then
                if wifi_services_exist then
                    if vif == dev.main_ifname
                    or string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                        miniupnpd_chk(devname,vif)
                    end
                end
                nixio.syslog("debug", "mt7615e_down: ifconfig "..vif.." down")
                os.execute("ifconfig "..vif.." down")
                del_vif_from_lan(vif)
            -- else nixio.syslog("debug", "mt7615e_down: skip "..vif..", prefix not match "..pre)
            end
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615e/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
	    if wifi_services_exist then
                d8021xd_chk(devname,pre,pre.."0")
            end
            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                if string.match(vif, pre.."[0-9]+") then
                    nixio.syslog("debug", "mt7615e_down: ifconfig "..vif.."down")
                    os.execute("ifconfig "..vif.." down")
                    del_vif_from_lan(vif)
                    if wifi_services_exist then
                        miniupnpd_chk(devname,vif)
                    end
                -- else nixio.syslog("debug", "mt7615e_down: skip "..vif..", prefix not match "..pre)
                end
            end
        end
    else nixio.syslog("debug", "mt7615e_down: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615e*.need_reload")
end

function mt7615e_reload(devname)
    local nixio = require("nixio")
    nixio.syslog("debug", "mt7615e_reload called!")
    mt7615e_down(devname)
    mt7615e_up(devname)
end

function mt7615e_restart(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()
    nixio.syslog("debug", "mt7615e_restart called!")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7615e_down(devname)
            end
        end
    else
         for devname, dev in pairs(devs.devname_ridx) do
             mt7615e_down(devname)
         end
    end
    os.execute("rmmod wifi_forward")
    os.execute("rmmod mt_whnat")
    os.execute("rmmod mt_wifi")
    os.execute("rmmod hw_nat")
    os.execute("insmod /lib/modules/ralink/hw_nat.ko")
    os.execute("modprobe mt_wifi")
    os.execute("modprobe mt_whnat")
    os.execute("modprobe wifi_forward")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7615e_up(devname)
            end
        end
    else
        for devname, dev in pairs(devs.devname_ridx) do
            mt7615e_up(devname)
        end
    end
end

function mt7615e_reset(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615e_reset called!")
    if mtkwifi.exists("/rom/etc/wireless/mt7615e/") then
        os.execute("rm -rf /etc/wireless/mt7615e/")
        os.execute("cp -rf /rom/etc/wireless/mt7615e/ /etc/wireless/")
        mt7615e_reload(devname)
    else
        nixio.syslog("debug", "mt7615e_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mt7615e_status(devname)
    return wifi_common_status()
end

function mt7615e_detect(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615e_detect called!")

    for _,dev in ipairs(mtkwifi.get_all_devs()) do
        local relname = string.format("%s%d%d",dev.maindev,dev.mainidx,dev.subidx)
        print([[
config wifi-device ]]..relname.."\n"..[[
    option type mt7615e
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
