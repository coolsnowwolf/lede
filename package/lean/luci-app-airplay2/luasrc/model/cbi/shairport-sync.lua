-- Copyright 2020 Lean <coolsnowwolf@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("shairport-sync", translate("Apple AirPlay 2 Receiver"), translate("Apple AirPlay 2 Receiver is a simple AirPlay server implementation"))

m:section(SimpleSection).template  = "shairport-sync/shairport-sync_status"

s = m:section(TypedSection, "shairport-sync")
s.addremove = false
s.anonymous = true

enable=s:option(Flag, "enabled", translate("Enabled"))
enable.default = "0"
enable.rmempty = false

respawn=s:option(Flag, "respawn", translate("Respawn"))
respawn.default = false

apname = s:option(Value, "name", translate("Airport Name"))
apname.rmempty = false

auth=s:option(Flag, "auth", translate("Password Auth"))
auth.default = false

pw = s:option(Value, "password", translate("Password"))
pw.rmempty = true
pw.password = true
pw.default = ""
pw:depends("auth", "1")

interpolation=s:option(ListValue, "interpolation", translate("Interpolation"))
interpolation:value("basic", translate("Internal Basic Resampler"))
interpolation:value("soxr", translate("High quality SoX Resampler (fast CPU required)"))

port=s:option(Value, "port", translate("Port"))
port.rmempty = false
port.datatype = "port"

alsa_output_device=s:option(ListValue, "alsa_output_device", translate("Alsa Output Device"))
alsa_output_device:value("", translate("default"))
alsa_output_device:value("hw:0", translate("1st Soundcard (hw:0)"))
alsa_output_device:value("hw:1", translate("2nd Soundcard (hw:1)"))
alsa_output_device:value("hw:2", translate("3rd Soundcard (hw:2)"))

alsa_mixer_control_name=s:option(ListValue, "alsa_mixer_control_name", translate("Alsa Mixer Control Name"))
alsa_mixer_control_name:value("", translate("default (software volume)"))
alsa_mixer_control_name:value("PCM", translate("PCM"))
alsa_mixer_control_name:value("Speaker", translate("Speaker"))

alsa_output_rate=s:option(ListValue, "alsa_output_rate", translate("Alsa Output Rate"))
alsa_output_rate:value("auto", translate("auto"))
alsa_output_rate:value("44100", translate("44.1kHz"))
alsa_output_rate:value("88200", translate("88.2kHz"))
alsa_output_rate:value("176400", translate("176.4kHz"))
alsa_output_rate:value("352800", translate("352.8kHz"))

alsa_buffer_length=s:option(Value, "alsa_buffer_length", translate("Alsa Buffer Length"))
alsa_buffer_length.default = "6615"

allow_session_interruption=s:option(ListValue, "sesctl_session_interruption", translate("Allow session interruption"))
allow_session_interruption:value("no", translate("Not allow"))
allow_session_interruption:value("yes", translate("Allow"))

session_timeout=s:option(Value, "sesctl_session_timeout", translate("Session timeout"))
session_timeout.default = "120"

return m