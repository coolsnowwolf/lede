-- This module is a demo to configure MTK' proprietary WiFi driver.
-- Basic idea is to bypass uci and edit wireless profile (mt76xx.dat) directly.
-- LuCI's WiFi configuration is more logical and elegent, but it's quite tricky to
-- translate uci into MTK's WiFi profile (like we did in "uci2dat").
-- And you will get your hands dirty.
--
-- Hua Shao <nossiac@163.com>

module("luci.controller.mtkwifi", package.seeall)
local http = require("luci.http")
local mtkwifi = require("mtkwifi")

function __mtkwifi_reload(devname)
    local profiles = mtkwifi.search_dev_and_profile()

    for dev,profile in pairs(profiles) do
        if not devname or devname == dev then
            local diff = mtkwifi.diff_profile(profile)
            -- Adding or deleting a vif will need to reinstall the wifi ko,
            -- so we call "mtkwifi restart" here.
            if diff.BssidNum then
                os.execute("/sbin/mtkwifi restart "..devname)
            else
                os.execute("/sbin/mtkwifi reload "..devname)
            end
            -- keep a backup for this commit
            -- it will be used in mtkwifi.diff_profile()
            os.execute("cp -f "..profile.." "..mtkwifi.__profile_bak_path(profile))
        end
    end
end


function index()

    entry({"admin", "network", "wifi"}, template("admin_mtk/mtk_wifi_overview"), _("Wireless"), 25)
    entry({"admin", "network", "wifi", "dev_cfg_view"}, template("admin_mtk/mtk_wifi_dev_cfg")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg"}, call("dev_cfg")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg_reset"}, call("dev_cfg_reset")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg_raw"}, call("dev_cfg_raw")).leaf = true
    entry({"admin", "network", "wifi", "vif_cfg_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_cfg"}, call("vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_add_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_add"}, call("vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_del"}, call("vif_del")).leaf = true
    entry({"admin", "network", "wifi", "vif_disable"}, call("vif_disable")).leaf = true
    entry({"admin", "network", "wifi", "vif_enable"}, call("vif_enable")).leaf = true
    entry({"admin", "network", "wifi", "get_station_list"}, call("get_station_list"))
    entry({"admin", "network", "wifi", "get_country_region_list"}, call("get_country_region_list")).leaf = true
    entry({"admin", "network", "wifi", "get_channel_list"}, call("get_channel_list"))
    entry({"admin", "network", "wifi", "get_HT_ext_channel_list"}, call("get_HT_ext_channel_list"))
    entry({"admin", "network", "wifi", "get_5G_2nd_80Mhz_channel_list"}, call("get_5G_2nd_80Mhz_channel_list"))
    entry({"admin", "network", "wifi", "reset"}, call("reset_wifi")).leaf = true
    entry({"admin", "network", "wifi", "reload"}, call("reload_wifi")).leaf = true
    entry({"admin", "network", "wifi", "get_raw_profile"}, call("get_raw_profile"))
    entry({"admin", "network", "wifi", "apcli_cfg_view"}, template("admin_mtk/mtk_wifi_apcli")).leaf = true
    entry({"admin", "network", "wifi", "apcli_cfg"}, call("apcli_cfg")).leaf = true
    entry({"admin", "network", "wifi", "apcli_disconnect"}, call("apcli_disconnect")).leaf = true
    entry({"admin", "network", "wifi", "apcli_connect"}, call("apcli_connect")).leaf = true
    entry({"admin", "network", "wifi", "apcli_scan"}, call("apcli_scan")).leaf = true;
end

function dev_cfg(devname)
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "dev_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "dev_cfg, special: "..k.."="..v)
        else
            cfgs[k] = v or ""
        end
    end

    if mtkwifi.band(cfgs.WirelessMode) == "5G" then
        cfgs.CountryRegionABand = http.formvalue("__cr");
    else
        cfgs.CountryRegion = http.formvalue("__cr");
    end

    if cfgs.Channel == "0" then -- Auto Channel Select
        cfgs.AutoChannelSelect = "3"
    else
        cfgs.AutoChannelSelect = "0"
    end

    if http.formvalue("__bw") == "20" then
        cfgs.HT_BW = 0
        cfgs.VHT_BW = 0
    elseif http.formvalue("__bw") == "40" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 0
        cfgs.HT_BSSCoexistence = 0
    elseif http.formvalue("__bw") == "60" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 0
        cfgs.HT_BSSCoexistence = 1
    elseif http.formvalue("__bw") == "80" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 1
    elseif http.formvalue("__bw") == "160" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 2
    elseif http.formvalue("__bw") == "161" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 3
        cfgs.VHT_Sec80_Channel = http.formvalue("VHT_Sec80_Channel") or ""
    end

    local mimo = http.formvalue("__mimo")
    if mimo == "0" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "1" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "2" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "3" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "4" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    else
       cfgs.ETxBfEnCond=1
       cfgs.MUTxRxEnable=0
       cfgs.ITxBfEn=1
    end

--    if cfgs.ApCliEnable == "1" then
--        cfgs.Channel = http.formvalue("__apcli_channel")
--    end

    -- VOW
    -- ATC should actually be scattered into each SSID, but I'm just lazy.
    if cfgs.VOW_Airtime_Fairness_En then
    for i = 1,tonumber(cfgs.BssidNum) do
        __atc_tp     = http.formvalue("__atc_vif"..i.."_tp")     or "0"
        __atc_min_tp = http.formvalue("__atc_vif"..i.."_min_tp") or "0"
        __atc_max_tp = http.formvalue("__atc_vif"..i.."_max_tp") or "0"
        __atc_at     = http.formvalue("__atc_vif"..i.."_at")     or "0"
        __atc_min_at = http.formvalue("__atc_vif"..i.."_min_at") or "0"
        __atc_max_at = http.formvalue("__atc_vif"..i.."_max_at") or "0"

        nixio.syslog("info", "ATC.__atc_tp     ="..i..__atc_tp     );
        nixio.syslog("info", "ATC.__atc_min_tp ="..i..__atc_min_tp );
        nixio.syslog("info", "ATC.__atc_max_tp ="..i..__atc_max_tp );
        nixio.syslog("info", "ATC.__atc_at     ="..i..__atc_at     );
        nixio.syslog("info", "ATC.__atc_min_at ="..i..__atc_min_at );
        nixio.syslog("info", "ATC.__atc_max_at ="..i..__atc_max_at );

        cfgs.VOW_Rate_Ctrl_En    = mtkwifi.token_set(cfgs.VOW_Rate_Ctrl_En,    i, __atc_tp)
        cfgs.VOW_Group_Min_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Min_Rate,  i, __atc_min_tp)
        cfgs.VOW_Group_Max_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Max_Rate,  i, __atc_max_tp)

        cfgs.VOW_Airtime_Ctrl_En = mtkwifi.token_set(cfgs.VOW_Airtime_Ctrl_En, i, __atc_at)
        cfgs.VOW_Group_Min_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Min_Ratio, i, __atc_min_at)
        cfgs.VOW_Group_Max_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Max_Ratio, i, __atc_max_at)
    end

    cfgs.VOW_RX_En = http.formvalue("VOW_RX_En") or "0"
    end

    -- http.write_json(http.formvalue())
    mtkwifi.save_profile(cfgs, profiles[devname])

    if http.formvalue("__apply") then
        __mtkwifi_reload(devname)
    end

    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view",devname))
end

function dev_cfg_raw(devname)
    -- http.write_json(http.formvalue())
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])

        local raw = http.formvalue("raw")
        raw = string.gsub(raw, "\r\n", "\n")
    local cfgs = mtkwifi.load_profile(nil, raw)
    mtkwifi.save_profile(cfgs, profiles[devname])

    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view", devname))
end

function dev_cfg_reset(devname)
    -- http.write_json(http.formvalue())
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])

    local fd = io.open("/rom"..profiles[devname], "r")
    if fd then
        fd:close() -- just to test if file exists.
        os.execute("cp -f /rom"..profiles[devname].." "..profiles[devname])
    else
        mtkwifi.debug("unable to find /rom"..profiles[devname])
    end
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view", devname))
end

function vif_del(dev, vif)
    mtkwifi.debug("vif_del("..dev..vif..")")
    local devname,vifname = dev, vif
    mtkwifi.debug("devname="..devname)
    mtkwifi.debug("vifname="..vifname)
    local devs = mtkwifi.get_all_devs()
    local idx = devs[devname]["vifs"][vifname].vifidx -- or tonumber(string.match(vifname, "%d+")) + 1
    mtkwifi.debug("idx="..idx, devname, vifname)
    local profile = devs[devname].profile
    assert(profile)
    if idx and tonumber(idx) >= 0 then
        local cfgs = mtkwifi.load_profile(profile)
        if cfgs then
            mtkwifi.debug("ssid"..idx.."="..cfgs["SSID"..idx].."<br>")
            cfgs["SSID"..idx] = ""
            mtkwifi.debug("ssid"..idx.."="..cfgs["SSID"..idx].."<br>")
            mtkwifi.debug("wpapsk"..idx.."="..cfgs["WPAPSK"..idx].."<br>")
            cfgs["WPAPSK"..idx] = ""
            local ssidlist = {}
            local j = 1
            for i = 1,16 do
                if cfgs["SSID"..i] ~= "" then
                    ssidlist[j] =  cfgs["SSID"..i]
                    j = j + 1
                end
            end
            for i,v in ipairs(ssidlist) do
                mtkwifi.debug("ssidlist"..i.."="..v)
            end
            mtkwifi.debug("cfgs.BssidNum="..cfgs.BssidNum.." #ssidlist="..#ssidlist)
            assert(tonumber(cfgs.BssidNum) == #ssidlist + 1, "Please delete vif from larger idx.")
            cfgs.BssidNum = #ssidlist
            for i = 1,16 do
                if i <= cfgs.BssidNum then
                    cfgs["SSID"..i] = ssidlist[i]
                elseif cfgs["SSID"..i] then
                    cfgs["SSID"..i] = ""
                end
            end

            mtkwifi.save_profile(cfgs, profile)
        else
            mtkwifi.debug(profile.." cannot be found!")
        end
    end
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function vif_disable(iface)
    os.execute("ifconfig "..iface.." down")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function vif_enable(iface)
    os.execute("ifconfig "..iface.." up")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end


--[[
-- security config in mtk wifi is quite complicated!
-- cfgs listed below are attached with vif and combined like "0;0;0;0". They need specicial treatment.
        TxRate, WmmCapable, NoForwarding,
        HideSSID, IEEE8021X, PreAuth,
        AuthMode, EncrypType, RekeyMethod,
        RekeyInterval, PMKCachePeriod,
        DefaultKeyId, Key{n}Type, HT_EXTCHA,
        RADIUS_Server, RADIUS_Port,
]]

local function conf_wep_keys(cfgs,vifidx)
    cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vifidx, http.formvalue("__DefaultKeyID") or 1)
    cfgs["Key1Str"..vifidx]  = http.formvalue("Key1Str"..vifidx)
    cfgs["Key2Str"..vifidx]  = http.formvalue("Key2Str"..vifidx)
    cfgs["Key3Str"..vifidx]  = http.formvalue("Key3Str"..vifidx)
    cfgs["Key4Str"..vifidx]  = http.formvalue("Key4Str"..vifidx)

    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vifidx, http.formvalue("WEP1Type"..vifidx))
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vifidx, http.formvalue("WEP2Type"..vifidx))
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vifidx, http.formvalue("WEP3Type"..vifidx))
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vifidx, http.formvalue("WEP4Type"..vifidx))

    return cfgs
end

local function __security_cfg(cfgs, vif_idx)
    mtkwifi.debug("__security_cfg, before, HideSSID="..tostring(cfgs.HideSSID))
    mtkwifi.debug("__security_cfg, before, NoForwarding="..tostring(cfgs.NoForwarding))
    mtkwifi.debug("__security_cfg, before, WmmCapable="..tostring(cfgs.WmmCapable))
    mtkwifi.debug("__security_cfg, before, TxRate="..tostring(cfgs.TxRate))
    mtkwifi.debug("__security_cfg, before, RekeyInterval="..tostring(cfgs.RekeyInterval))
    mtkwifi.debug("__security_cfg, before, AuthMode="..tostring(cfgs.AuthMode))
    mtkwifi.debug("__security_cfg, before, EncrypType="..tostring(cfgs.EncrypType))
    mtkwifi.debug("__security_cfg, before, WscModeOption="..tostring(cfgs.WscModeOption))
    mtkwifi.debug("__security_cfg, before, RekeyMethod="..tostring(cfgs.RekeyMethod))
    mtkwifi.debug("__security_cfg, before, IEEE8021X="..tostring(cfgs.IEEE8021X))
    mtkwifi.debug("__security_cfg, before, DefaultKeyID="..tostring(cfgs.DefaultKeyID))
    mtkwifi.debug("__security_cfg, before, PMFMFPC="..tostring(cfgs.PMFMFPC))
    mtkwifi.debug("__security_cfg, before, PMFMFPR="..tostring(cfgs.PMFMFPR))
    mtkwifi.debug("__security_cfg, before, PMFSHA256="..tostring(cfgs.PMFSHA256))
    mtkwifi.debug("__security_cfg, before, RADIUS_Server="..tostring(cfgs.RADIUS_Server))
    mtkwifi.debug("__security_cfg, before, RADIUS_Port="..tostring(cfgs.RADIUS_Port))
    mtkwifi.debug("__security_cfg, before, session_timeout_interval="..tostring(cfgs.session_timeout_interval))
    mtkwifi.debug("__security_cfg, before, PMKCachePeriod="..tostring(cfgs.PMKCachePeriod))
    mtkwifi.debug("__security_cfg, before, PreAuth="..tostring(cfgs.PreAuth))
    mtkwifi.debug("__security_cfg, before, Wapiifname="..tostring(cfgs.Wapiifname))

    cfgs.HideSSID = mtkwifi.token_set(cfgs.HideSSID, vif_idx, http.formvalue("__hidessid") or "0")
    cfgs.NoForwarding = mtkwifi.token_set(cfgs.NoForwarding, vif_idx, http.formvalue("__noforwarding") or "0")
    cfgs.WmmCapable = mtkwifi.token_set(cfgs.WmmCapable, vif_idx, http.formvalue("__wmmcapable") or "0")
    cfgs.TxRate = mtkwifi.token_set(cfgs.TxRate, vif_idx, http.formvalue("__txrate") or "0");
    cfgs.RekeyInterval = mtkwifi.token_set(cfgs.RekeyInterval, vif_idx, http.formvalue("__rekeyinterval") or "0");

    local __authmode = http.formvalue("__authmode") or "Disable"
    cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, __authmode)

    --[[ need to handle disable case as it conflicts with OPENWEP
    case = disable; authmode = OPEN, encryption = NONE
    case = open/OPENWEP; authmode = OPEN, encryption = WEP
    ]]
    if __authmode == "Disable" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "NONE")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "DISABLE")
    elseif __authmode == "OPEN" or __authmode == "SHARED" or __authmode == "WEPAUTO" then
        --[[ the following required cfgs are already set in loop above
        cfgs.Key{n}Str, ...
        cfgs.Key{n}Type, ...
        cfgs.DefaultKeyID
        ]]
        cfgs.WscModeOption = "0"
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "WEP")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "DISABLE")
        cfgs = conf_wep_keys(cfgs,vif_idx)
        cfgs.WscConfMode = mtkwifi.token_set(cfgs.WscConfMode, vif_idx, 0)
    elseif __authmode == "WPAPSK" or __authmode == "WPAPSKWPA2PSK" then
        --[[ the following required cfgs are already set in loop above
        cfgs.WPAPSK{n}, ...
        cfgs.RekeyInterval
        ]]

        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
        cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vif_idx, "2")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__wpa_encrypttype") or "NONE")
        --cfgs["WPAPSK"..vif_idx] = http.formvalue("WPAPSK"..vif_idx)
    elseif __authmode == "WPA2PSK" then
        --[[ the following required cfgs are already set in loop above
        cfgs.WPAPSK{n}, ...
        cfgs.RekeyInterval
        ]]
        -- for DOT11W_PMF_SUPPORT
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__wpa_encrypttype") or "0")
        if cfgs.PMFMFPC then
            cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        end
        if cfgs.PMFMFPR then
            cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        end
        if cfgs.PMFSHA256 then
            cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")
        end

        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
        cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vif_idx, "2")
    elseif __authmode == "WPA" then
        --[[ the following required cfgs are already set in loop above
        cfgs.WPAPSK{n}, ...
        cfgs.RekeyInterval
        cfgs.RADIUS_Key
        ]]
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__wpa_encrypttype") or "0")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")

        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
        cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vif_idx, "2")
    elseif __authmode == "WPA2" then
        --[[ the following required cfgs are already set in loop above
        cfgs.WPAPSK{n}, ...
        cfgs.RekeyInterval
        cfgs.RADIUS_Key
        ]]
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__wpa_encrypttype") or "0")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        if cfgs.PMFMFPC then
            cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        end
        if cfgs.PMFMFPR then
            cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        end
        if cfgs.PMFSHA256 then
            cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")
        end

        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
        cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vif_idx, "2")
    elseif __authmode == "WPA1WPA2" then
        --[[ the following required cfgs are already set in loop above
        cfgs.WPAPSK{n}, ...
        cfgs.RekeyInterval
        cfgs.RADIUS_Key
        ]]
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__wpa_encrypttype") or "0")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "1812")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")

        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
        cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vif_idx, "2")
    elseif __authmode == "IEEE8021X" then
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "DISABLE")
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "1")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        if http.formvalue("__8021x_wep") then
            cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "WEP")
        else
            cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "NONE")
        end
    elseif __authmode == "WAICERT" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        cfgs.Wapiifname = mtkwifi.token_set(cfgs.Wapiifname, vif_idx, "br-lan")
        -- cfgs.wapicert_asipaddr
        -- cfgs.WapiAsPort
        -- cfgs.wapicert_ascert
        -- cfgs.wapicert_usercert
    elseif __authmode == "WAIPSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        -- cfgs.wapipsk_keytype
        -- cfgs.wapipsk_prekey
    end

    mtkwifi.debug("__security_cfg, after, HideSSID="..tostring(cfgs.HideSSID))
    mtkwifi.debug("__security_cfg, after, NoForwarding="..tostring(cfgs.NoForwarding))
    mtkwifi.debug("__security_cfg, after, WmmCapable="..tostring(cfgs.WmmCapable))
    mtkwifi.debug("__security_cfg, after, TxRate="..tostring(cfgs.TxRate))
    mtkwifi.debug("__security_cfg, after, RekeyInterval="..tostring(cfgs.RekeyInterval))
    mtkwifi.debug("__security_cfg, after, AuthMode="..tostring(cfgs.AuthMode))
    mtkwifi.debug("__security_cfg, after, EncrypType="..tostring(cfgs.EncrypType))
    mtkwifi.debug("__security_cfg, after, WscModeOption="..tostring(cfgs.WscModeOption))
    mtkwifi.debug("__security_cfg, after, RekeyMethod="..tostring(cfgs.RekeyMethod))
    mtkwifi.debug("__security_cfg, after, IEEE8021X="..tostring(cfgs.IEEE8021X))
    mtkwifi.debug("__security_cfg, after, DefaultKeyID="..tostring(cfgs.DefaultKeyID))
    mtkwifi.debug("__security_cfg, after, PMFMFPC="..tostring(cfgs.PMFMFPC))
    mtkwifi.debug("__security_cfg, after, PMFMFPR="..tostring(cfgs.PMFMFPR))
    mtkwifi.debug("__security_cfg, after, PMFSHA256="..tostring(cfgs.PMFSHA256))
    mtkwifi.debug("__security_cfg, after, RADIUS_Server="..tostring(cfgs.RADIUS_Server))
    mtkwifi.debug("__security_cfg, after, RADIUS_Port="..tostring(cfgs.RADIUS_Port))
    mtkwifi.debug("__security_cfg, after, session_timeout_interval="..tostring(cfgs.session_timeout_interval))
    mtkwifi.debug("__security_cfg, after, PMKCachePeriod="..tostring(cfgs.PMKCachePeriod))
    mtkwifi.debug("__security_cfg, after, PreAuth="..tostring(cfgs.PreAuth))
    mtkwifi.debug("__security_cfg, after, Wapiifname="..tostring(cfgs.Wapiifname))
end

function initialize_multiBssParameters(cfgs,vif_idx)
    cfgs["WPAPSK"..vif_idx]="12345678"
    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vif_idx,"0")
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vif_idx,"0")
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vif_idx,"0")
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vif_idx,"0")
    cfgs["RADIUS_Server"]=mtkwifi.token_set(cfgs["RADIUS_Server"],vif_idx,"0")
    cfgs["RADIUS_Key"..vif_idx]="ralink"
    cfgs["DefaultKeyID"]=mtkwifi.token_set(cfgs["DefaultKeyID"],vif_idx,"1")
    cfgs["IEEE8021X"]=mtkwifi.token_set(cfgs["IEEE8021X"],vif_idx,"0")
    cfgs["WscConfMode"]=mtkwifi.token_set(cfgs["WscConfMode"],vif_idx,"0")
    cfgs["PreAuth"]=mtkwifi.token_set(cfgs["PreAuth"],vif_idx,"0")
    return cfgs
end

function vif_cfg(dev, vif)
    local devname, vifname = dev, vif
    if not devname then devname = vif end
    mtkwifi.debug("devname="..devname)
    mtkwifi.debug("vifname="..(vifname or ""))
    local devs = mtkwifi.get_all_devs()
    local profile = devs[devname].profile
    assert(profile)

    local cfgs = mtkwifi.load_profile(profile)

    for k,v in pairs(http.formvalue()) do
        if type(v) == type("") or type(v) == type(0) then
            nixio.syslog("debug", "post."..k.."="..tostring(v))
        else
            nixio.syslog("debug", "post."..k.." invalid, type="..type(v))
        end
    end

    -- sometimes vif_idx start from 0, like AccessPolicy0
    -- sometimes it starts from 1, like WPAPSK1. nice!
    local vif_idx
    local to_url
    if http.formvalue("__action") == "vif_cfg_view" then
        vif_idx = devs[devname]["vifs"][vifname].vifidx
        mtkwifi.debug("vif_idx=", vif_idx, devname, vifname)
        to_url = luci.dispatcher.build_url("admin", "network", "wifi", "vif_cfg_view", devname, vifname)
    elseif http.formvalue("__action") == "vif_add_view" then
        cfgs.BssidNum = tonumber(cfgs.BssidNum) + 1
        vif_idx = tonumber(cfgs.BssidNum)
        to_url = luci.dispatcher.build_url("admin", "network", "wifi")
        -- initializing ; separated parameters for the new interface
        cfgs = initialize_multiBssParameters(cfgs, vif_idx)

    end

    -- "__" should not be in the name if user wants to copy formvalue data directly in the dat file variable
    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "vif_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
                mtkwifi.debug("copying values for "..k)
            cfgs[k] = v or ""
        end
    end

    cfgs["AccessPolicy"..vif_idx-1] = http.formvalue("__accesspolicy")
    local t = mtkwifi.parse_mac(http.formvalue("__maclist"))
    cfgs["AccessControlList"..vif_idx-1] = table.concat(t, ";")

        __security_cfg(cfgs, vif_idx)

    mtkwifi.debug(devname, profile)
    mtkwifi.save_profile(cfgs, profile)
    if http.formvalue("__apply") then
        __mtkwifi_reload(devname)
    end
    return luci.http.redirect(to_url)
end

function apcli_scan(ifname)
    local aplist = mtkwifi.scan_ap(ifname)
    http.write_json(aplist)
end

function get_station_list()
    http.write("get_station_list")
end

function reset_wifi(devname)
    if devname then
        os.execute("cp -f /rom/etc/wireless/"..devname.."/ /etc/wireless/")
    else
        os.execute("cp -rf /rom/etc/wireless /etc/")
    end
    os.execute("rm -rf /tmp/mtk/wifi")
    __mtkwifi_reload(devname)
    return luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function reload_wifi(devname)
    __mtkwifi_reload(devname)
    return luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function get_raw_profile()
    local sid = http.formvalue("sid")
    http.write_json("get_raw_profile")
end

function get_country_region_list()
    local mode = http.formvalue("mode")
    local cr_list;

    if mtkwifi.band(mode) == "5G" then
        cr_list = mtkwifi.CountryRegionList_5G_All
    else
        cr_list = mtkwifi.CountryRegionList_2G_All
    end

    http.write_json(cr_list)
end

function remove_ch_by_region(ch_list, region)
    for i = #ch_list,2,-1 do
        if not ch_list[i].region[region] then
            table.remove(ch_list, i)
        end
    end
end

function get_channel_list()
    local mode = http.formvalue("mode")
    local region = tonumber(http.formvalue("country_region")) or 1
    local ch_list

    if mtkwifi.band(mode) == "5G" then
        ch_list = mtkwifi.ChannelList_5G_All
    else
        ch_list = mtkwifi.ChannelList_2G_All
    end

    remove_ch_by_region(ch_list, region)
    http.write_json(ch_list)
end

function get_HT_ext_channel_list()
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region")) or 1
    local ext_ch_list = {}

    if ch_cur <= 14 then -- 2.4G Channel
        local ch_list = mtkwifi.ChannelList_2G_All
        local below_ch = ch_cur - 4
        local above_ch = ch_cur + 4
        local i = 1

        if below_ch > 0 and ch_list[below_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 0
            ext_ch_list[i].text = ch_list[below_ch + 1].text
            i = i + 1
        end

        if above_ch <= 14 and ch_list[above_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 1
            ext_ch_list[i].text = ch_list[above_ch + 1].text
        end
    else  -- 5G Channel
        local ch_list = mtkwifi.ChannelList_5G_All
        local ext_ch_idx = -1
        local len = 0

        for k, v in ipairs(ch_list) do
            len = len + 1
            if v.channel == ch_cur then
                ext_ch_idx = (k % 2 == 0) and k + 1 or k - 1
            end
        end

        if ext_ch_idx > 0 and ext_ch_idx < len and ch_list[ext_ch_idx].region[region] then
            ext_ch_list[1] = {}
            ext_ch_list[1].val = ext_ch_idx % 2
            ext_ch_list[1].text = ch_list[ext_ch_idx].text
        end
    end

    http.write_json(ext_ch_list)
end

function get_5G_2nd_80Mhz_channel_list()
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region"))
    local ch_list = mtkwifi.ChannelList_5G_2nd_80MHZ_ALL
    local ch_list_5g = mtkwifi.ChannelList_5G_All
    local i, j, test_ch, test_idx
    local bw80_1st_idx = -1

    -- remove adjacent freqencies starting from list tail.
    for i = #ch_list,1,-1 do
        for j = 0,3 do
            if ch_list[i].channel == -1 then
                break
            end

            test_ch = ch_list[i].channel + j * 4
            test_idx = ch_list[i].chidx + j

            if test_ch == ch_cur then
            if i + 1 <= #ch_list and ch_list[i + 1] then
                table.remove(ch_list, i + 1)
            end
            table.remove(ch_list, i)
                bw80_1st_idx = i
                break
            end

            if i == (bw80_1st_idx - 1) or (not ch_list_5g[test_idx].region[region]) then
                table.remove(ch_list, i)
            break
        end
    end
    end

    -- remove unused channel.
    for i = #ch_list,1,-1 do
        if ch_list[i].channel == -1 then
            table.remove(ch_list, i)
        end
    end
    http.write_json(ch_list)
end

function apcli_cfg(dev, vif)
    local devname = dev
    mtkwifi.debug(devname)
    local profiles = mtkwifi.search_dev_and_profile()
    mtkwifi.debug(profiles[devname])
    assert(profiles[devname])

    local cfgs = mtkwifi.load_profile(profiles[devname])

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "apcli_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            cfgs[k] = v or ""
        end
    end

    cfgs.ApCliSsid = ''..mtkwifi.__handleSpecialChars(http.formvalue("ApCliSsid"))
    local __authmode = http.formvalue("ApCliAuthMode")
    if __authmode == "Disable" then
        cfgs.ApCliAuthMode = "OPEN"
        cfgs.ApCliEncrypType = "NONE"
    elseif __authmode == "OPEN" or __authmode == "SHARED" then
        cfgs.ApCliAuthMode = __authmode
        cfgs.ApCliEncrypType = http.formvalue("wep_ApCliEncrypType")
    else
        cfgs.ApCliAuthMode = __authmode
        cfgs.ApCliEncrypType = http.formvalue("wpa_ApCliEncrypType")
    end

    mtkwifi.save_profile(cfgs, profiles[devname])

    if http.formvalue("__apply") then
        __mtkwifi_reload(devname)
    end
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "apcli_cfg_view", dev, vif))
end

function apcli_connect(dev, vif)
    -- dev_vif can be
    --  1. mt7620.apcli0         # simple case
    --  2. mt7615e.1.apclix0     # multi-card
    --  3. mt7615e.1.2G.apclix0  # multi-card & multi-profile
    local devname,vifname = dev, vif
    mtkwifi.debug("devname=", dev, "vifname=", vif)
    mtkwifi.debug(devname)
    mtkwifi.debug(vifname)
    local profiles = mtkwifi.search_dev_and_profile()
    mtkwifi.debug(profiles[devname])
    assert(profiles[devname])

    local cfgs = mtkwifi.load_profile(profiles[devname])
    cfgs.ApCliEnable = "1"
    mtkwifi.save_profile(cfgs, profiles[devname])
    os.execute("iwpriv "..vifname.." set ApCliEnable=0")
    os.execute("ifconfig "..vifname.." up")
    os.execute("iwpriv "..vifname.." set Channel="..cfgs.Channel)
    os.execute("iwpriv "..vifname.." set ApCliAuthMode="..cfgs.ApCliAuthMode)
    os.execute("iwpriv "..vifname.." set ApCliEncrypType="..cfgs.ApCliEncrypType)
    if cfgs.ApCliAuthMode == "WEP" then
        os.execute("#iwpriv "..vifname.." set ApCliDefaultKeyID="..cfgs.ApCliDefaultKeyID)
        os.execute("#iwpriv "..vifname.." set ApCliKey1="..cfgs.ApCliKey1Str)
    elseif cfgs.ApCliAuthMode == "WPAPSK"
        or cfgs.ApCliAuthMode == "WPA2PSK"
        or cfgs.ApCliAuthMode == "WPA1PSKWPA2PSK" then
        os.execute("iwpriv "..vifname.." set ApCliWPAPSK="..cfgs.ApCliWPAPSK)
    end
    os.execute("iwpriv "..vifname.." set ApCliSsid=\""..cfgs.ApCliSsid.."\"")
    os.execute("iwpriv "..vifname.." set ApCliEnable=1")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function apcli_disconnect(dev, vif)
    -- dev_vif can be
    --  1. mt7620.apcli0         # simple case
    --  2. mt7615e.1.apclix0     # multi-card
    --  3. mt7615e.1.2G.apclix0  # multi-card & multi-profile
    local devname,vifname = dev, vif
    mtkwifi.debug("devname=", dev, "vifname", vif)
    mtkwifi.debug(devname)
    mtkwifi.debug(vifname)
    local profiles = mtkwifi.search_dev_and_profile()
    mtkwifi.debug(profiles[devname])
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])
    cfgs.ApCliEnable = "0"
    mtkwifi.save_profile(cfgs, profiles[devname])
    os.execute("iwpriv "..vifname.." set ApCliEnable=0")
    os.execute("ifconfig "..vifname.." down")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

