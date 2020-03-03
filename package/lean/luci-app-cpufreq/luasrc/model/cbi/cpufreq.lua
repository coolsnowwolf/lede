
mp = Map("cpufreq", translate("CPU Freq Settings"))
mp.description = translate("Set CPU Scaling Governor to Max Performance or Balance Mode")

s = mp:section(NamedSection, "cpufreq", "settings")
s.anonymouse = true

governor = s:option(ListValue, "governor", translate("CPU Scaling Governor"))
governor:value("ondemand", translate("Ondemand Balance Mode"))
governor:value("performance", translate("Performance Mode"))

minifreq = s:option(Value, "minifreq", translate("Min Idle CPU Freq"))
minifreq.datatype="range(48000,716000)"
minifreq.rmempty = false
minifreq.description = translate("CPU Freq from 48000 to 716000 (Khz)")
minifreq.placeholder = 48000
minifreq.default = 48000

maxfreq = s:option(Value, "maxfreq", translate("Max Turbo Boost CPU Freq"))
maxfreq.datatype="range(48000,716000)"
maxfreq.rmempty = false
maxfreq.description = translate("CPU Freq from 48000 to 716000 (Khz)")
maxfreq.placeholder = 716000
maxfreq.default = 716000

upthreshold = s:option(Value, "upthreshold", translate("CPU Switching Threshold"))
upthreshold.datatype="range(1,99)"
upthreshold.rmempty = false
upthreshold.description = translate("Kernel make a decision on whether it should increase the frequency (%)")
upthreshold.placeholder = 50
upthreshold.default = 50

factor = s:option(Value, "factor", translate("CPU Switching Sampling rate"))
factor.datatype="range(1,100000)"
factor.rmempty = false
factor.description = translate("The sampling rate determines how frequently the governor checks to tune the CPU (ms)")
factor.placeholder = 10
factor.default = 10


return mp
