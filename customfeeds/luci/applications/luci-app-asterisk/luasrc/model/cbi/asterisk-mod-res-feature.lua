-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

featuremap = cbimap:section(TypedSection, "featuremap", "Feature Key maps", "")
featuremap.anonymous = true
featuremap.addremove = true

atxfer = featuremap:option(Value, "atxfer", "Attended transfer key", "")
atxfer.rmempty = true

blindxfer = featuremap:option(Value, "blindxfer", "Blind transfer key", "")
blindxfer.rmempty = true

disconnect = featuremap:option(Value, "disconnect", "Key to Disconnect call", "")
disconnect.rmempty = true

parkcall = featuremap:option(Value, "parkcall", "Key to Park call", "")
parkcall.rmempty = true


featurepark = cbimap:section(TypedSection, "featurepark", "Parking Feature", "")
featurepark.anonymous = true

parkenabled = featurepark:option(Flag, "parkenabled", "Enable Parking", "")

adsipark = featurepark:option(Flag, "adsipark", "ADSI Park", "")
adsipark.rmempty = true
adsipark:depends({ parkenabled = "1" })

atxfernoanswertimeout = featurepark:option(Value, "atxfernoanswertimeout", "Attended transfer timeout (sec)", "")
atxfernoanswertimeout.rmempty = true
atxfernoanswertimeout:depends({ parkenabled = "1" })

automon = featurepark:option(Value, "automon", "One touch record key", "")
automon.rmempty = true
automon:depends({ parkenabled = "1" })

context = featurepark:option(Value, "context", "Name of call context for parking", "")
context.rmempty = true
context:depends({ parkenabled = "1" })

courtesytone = featurepark:option(Value, "courtesytone", "Sound file to play to parked caller", "")
courtesytone.rmempty = true
courtesytone:depends({ parkenabled = "1" })

featuredigittimeout = featurepark:option(Value, "featuredigittimeout", "Max time (ms) between digits for feature activation", "")
featuredigittimeout.rmempty = true
featuredigittimeout:depends({ parkenabled = "1" })

findslot = featurepark:option(ListValue, "findslot", "Method to Find Parking slot", "")
findslot:value("first", "First available slot")
findslot:value("next", "Next free parking space")
findslot.rmempty = true
findslot:depends({ parkenabled = "1" })

parkedmusicclass = featurepark:option(ListValue, "parkedmusicclass", "Music on Hold class for the parked channel", "")
parkedmusicclass.titleref = luci.dispatcher.build_url( "admin", "services", "asterisk" )
parkedmusicclass:depends({ parkenabled = "1" })
cbimap.uci:foreach( "asterisk", "moh", function(s) parkedmusicclass:value(s['.name']) end )

parkedplay = featurepark:option(ListValue, "parkedplay", "Play courtesy tone to", "")
parkedplay:value("caller", "Caller")
parkedplay:value("parked", "Parked user")
parkedplay:value("both", "Both")
parkedplay.rmempty = true
parkedplay:depends({ parkenabled = "1" })

parkext = featurepark:option(Value, "parkext", "Extension to dial to park", "")
parkext.rmempty = true
parkext:depends({ parkenabled = "1" })

parkingtime = featurepark:option(Value, "parkingtime", "Parking time (secs)", "")
parkingtime.rmempty = true
parkingtime:depends({ parkenabled = "1" })

parkpos = featurepark:option(Value, "parkpos", "Range of extensions for call parking", "")
parkpos.rmempty = true
parkpos:depends({ parkenabled = "1" })

pickupexten = featurepark:option(Value, "pickupexten", "Pickup extension", "")
pickupexten.rmempty = true
pickupexten:depends({ parkenabled = "1" })

transferdigittimeout = featurepark:option(Value, "transferdigittimeout", "Seconds to wait between digits when transferring", "")
transferdigittimeout.rmempty = true
transferdigittimeout:depends({ parkenabled = "1" })

xferfailsound = featurepark:option(Value, "xferfailsound", "sound when attended transfer is complete", "")
xferfailsound.rmempty = true
xferfailsound:depends({ parkenabled = "1" })

xfersound = featurepark:option(Value, "xfersound", "Sound when attended transfer fails", "")
xfersound.rmempty = true
xfersound:depends({ parkenabled = "1" })


return cbimap
