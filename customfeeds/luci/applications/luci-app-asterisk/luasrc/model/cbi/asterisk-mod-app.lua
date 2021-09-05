cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

app_alarmreceiver = module:option(ListValue, "app_alarmreceiver", "Alarm Receiver Application", "")
app_alarmreceiver:value("yes", "Load")
app_alarmreceiver:value("no", "Do Not Load")
app_alarmreceiver:value("auto", "Load as Required")
app_alarmreceiver.rmempty = true

app_authenticate = module:option(ListValue, "app_authenticate", "Authentication Application", "")
app_authenticate:value("yes", "Load")
app_authenticate:value("no", "Do Not Load")
app_authenticate:value("auto", "Load as Required")
app_authenticate.rmempty = true

app_cdr = module:option(ListValue, "app_cdr", "Make sure asterisk doesn't save CDR", "")
app_cdr:value("yes", "Load")
app_cdr:value("no", "Do Not Load")
app_cdr:value("auto", "Load as Required")
app_cdr.rmempty = true

app_chanisavail = module:option(ListValue, "app_chanisavail", "Check if channel is available", "")
app_chanisavail:value("yes", "Load")
app_chanisavail:value("no", "Do Not Load")
app_chanisavail:value("auto", "Load as Required")
app_chanisavail.rmempty = true

app_chanspy = module:option(ListValue, "app_chanspy", "Listen in on any channel", "")
app_chanspy:value("yes", "Load")
app_chanspy:value("no", "Do Not Load")
app_chanspy:value("auto", "Load as Required")
app_chanspy.rmempty = true

app_controlplayback = module:option(ListValue, "app_controlplayback", "Control Playback Application", "")
app_controlplayback:value("yes", "Load")
app_controlplayback:value("no", "Do Not Load")
app_controlplayback:value("auto", "Load as Required")
app_controlplayback.rmempty = true

app_cut = module:option(ListValue, "app_cut", "Cuts up variables", "")
app_cut:value("yes", "Load")
app_cut:value("no", "Do Not Load")
app_cut:value("auto", "Load as Required")
app_cut.rmempty = true

app_db = module:option(ListValue, "app_db", "Database access functions", "")
app_db:value("yes", "Load")
app_db:value("no", "Do Not Load")
app_db:value("auto", "Load as Required")
app_db.rmempty = true

app_dial = module:option(ListValue, "app_dial", "Dialing Application", "")
app_dial:value("yes", "Load")
app_dial:value("no", "Do Not Load")
app_dial:value("auto", "Load as Required")
app_dial.rmempty = true

app_dictate = module:option(ListValue, "app_dictate", "Virtual Dictation Machine Application", "")
app_dictate:value("yes", "Load")
app_dictate:value("no", "Do Not Load")
app_dictate:value("auto", "Load as Required")
app_dictate.rmempty = true

app_directed_pickup = module:option(ListValue, "app_directed_pickup", "Directed Call Pickup Support", "")
app_directed_pickup:value("yes", "Load")
app_directed_pickup:value("no", "Do Not Load")
app_directed_pickup:value("auto", "Load as Required")
app_directed_pickup.rmempty = true

app_directory = module:option(ListValue, "app_directory", "Extension Directory", "")
app_directory:value("yes", "Load")
app_directory:value("no", "Do Not Load")
app_directory:value("auto", "Load as Required")
app_directory.rmempty = true

app_disa = module:option(ListValue, "app_disa", "DISA (Direct Inward System Access) Application", "")
app_disa:value("yes", "Load")
app_disa:value("no", "Do Not Load")
app_disa:value("auto", "Load as Required")
app_disa.rmempty = true

app_dumpchan = module:option(ListValue, "app_dumpchan", "Dump channel variables Application", "")
app_dumpchan:value("yes", "Load")
app_dumpchan:value("no", "Do Not Load")
app_dumpchan:value("auto", "Load as Required")
app_dumpchan.rmempty = true

app_echo = module:option(ListValue, "app_echo", "Simple Echo Application", "")
app_echo:value("yes", "Load")
app_echo:value("no", "Do Not Load")
app_echo:value("auto", "Load as Required")
app_echo.rmempty = true

app_enumlookup = module:option(ListValue, "app_enumlookup", "ENUM Lookup", "")
app_enumlookup:value("yes", "Load")
app_enumlookup:value("no", "Do Not Load")
app_enumlookup:value("auto", "Load as Required")
app_enumlookup.rmempty = true

app_eval = module:option(ListValue, "app_eval", "Reevaluates strings", "")
app_eval:value("yes", "Load")
app_eval:value("no", "Do Not Load")
app_eval:value("auto", "Load as Required")
app_eval.rmempty = true

app_exec = module:option(ListValue, "app_exec", "Executes applications", "")
app_exec:value("yes", "Load")
app_exec:value("no", "Do Not Load")
app_exec:value("auto", "Load as Required")
app_exec.rmempty = true

app_externalivr = module:option(ListValue, "app_externalivr", "External IVR application interface", "")
app_externalivr:value("yes", "Load")
app_externalivr:value("no", "Do Not Load")
app_externalivr:value("auto", "Load as Required")
app_externalivr.rmempty = true

app_forkcdr = module:option(ListValue, "app_forkcdr", "Fork The CDR into 2 separate entities", "")
app_forkcdr:value("yes", "Load")
app_forkcdr:value("no", "Do Not Load")
app_forkcdr:value("auto", "Load as Required")
app_forkcdr.rmempty = true

app_getcpeid = module:option(ListValue, "app_getcpeid", "Get ADSI CPE ID", "")
app_getcpeid:value("yes", "Load")
app_getcpeid:value("no", "Do Not Load")
app_getcpeid:value("auto", "Load as Required")
app_getcpeid.rmempty = true

app_groupcount = module:option(ListValue, "app_groupcount", "Group Management Routines", "")
app_groupcount:value("yes", "Load")
app_groupcount:value("no", "Do Not Load")
app_groupcount:value("auto", "Load as Required")
app_groupcount.rmempty = true

app_ices = module:option(ListValue, "app_ices", "Encode and Stream via icecast and ices", "")
app_ices:value("yes", "Load")
app_ices:value("no", "Do Not Load")
app_ices:value("auto", "Load as Required")
app_ices.rmempty = true

app_image = module:option(ListValue, "app_image", "Image Transmission Application", "")
app_image:value("yes", "Load")
app_image:value("no", "Do Not Load")
app_image:value("auto", "Load as Required")
app_image.rmempty = true

app_lookupblacklist = module:option(ListValue, "app_lookupblacklist", "Look up Caller*ID name/number from black", "")
app_lookupblacklist:value("yes", "Load")
app_lookupblacklist:value("no", "Do Not Load")
app_lookupblacklist:value("auto", "Load as Required")
app_lookupblacklist.rmempty = true

app_lookupcidname = module:option(ListValue, "app_lookupcidname", "Look up CallerID Name from local databas", "")
app_lookupcidname:value("yes", "Load")
app_lookupcidname:value("no", "Do Not Load")
app_lookupcidname:value("auto", "Load as Required")
app_lookupcidname.rmempty = true

app_macro = module:option(ListValue, "app_macro", "Extension Macros", "")
app_macro:value("yes", "Load")
app_macro:value("no", "Do Not Load")
app_macro:value("auto", "Load as Required")
app_macro.rmempty = true

app_math = module:option(ListValue, "app_math", "A simple math Application", "")
app_math:value("yes", "Load")
app_math:value("no", "Do Not Load")
app_math:value("auto", "Load as Required")
app_math.rmempty = true

app_md5 = module:option(ListValue, "app_md5", "MD5 checksum Application", "")
app_md5:value("yes", "Load")
app_md5:value("no", "Do Not Load")
app_md5:value("auto", "Load as Required")
app_md5.rmempty = true

app_milliwatt = module:option(ListValue, "app_milliwatt", "Digital Milliwatt (mu-law) Test Application", "")
app_milliwatt:value("yes", "Load")
app_milliwatt:value("no", "Do Not Load")
app_milliwatt:value("auto", "Load as Required")
app_milliwatt.rmempty = true

app_mixmonitor = module:option(ListValue, "app_mixmonitor", "Record a call and mix the audio during the recording", "")
app_mixmonitor:value("yes", "Load")
app_mixmonitor:value("no", "Do Not Load")
app_mixmonitor:value("auto", "Load as Required")
app_mixmonitor.rmempty = true

app_parkandannounce = module:option(ListValue, "app_parkandannounce", "Call Parking and Announce Application", "")
app_parkandannounce:value("yes", "Load")
app_parkandannounce:value("no", "Do Not Load")
app_parkandannounce:value("auto", "Load as Required")
app_parkandannounce.rmempty = true

app_playback = module:option(ListValue, "app_playback", "Trivial Playback Application", "")
app_playback:value("yes", "Load")
app_playback:value("no", "Do Not Load")
app_playback:value("auto", "Load as Required")
app_playback.rmempty = true

app_privacy = module:option(ListValue, "app_privacy", "Require phone number to be entered", "")
app_privacy:value("yes", "Load")
app_privacy:value("no", "Do Not Load")
app_privacy:value("auto", "Load as Required")
app_privacy.rmempty = true

app_queue = module:option(ListValue, "app_queue", "True Call Queueing", "")
app_queue:value("yes", "Load")
app_queue:value("no", "Do Not Load")
app_queue:value("auto", "Load as Required")
app_queue.rmempty = true

app_random = module:option(ListValue, "app_random", "Random goto", "")
app_random:value("yes", "Load")
app_random:value("no", "Do Not Load")
app_random:value("auto", "Load as Required")
app_random.rmempty = true

app_read = module:option(ListValue, "app_read", "Read Variable Application", "")
app_read:value("yes", "Load")
app_read:value("no", "Do Not Load")
app_read:value("auto", "Load as Required")
app_read.rmempty = true

app_readfile = module:option(ListValue, "app_readfile", "Read in a file", "")
app_readfile:value("yes", "Load")
app_readfile:value("no", "Do Not Load")
app_readfile:value("auto", "Load as Required")
app_readfile.rmempty = true

app_realtime = module:option(ListValue, "app_realtime", "Realtime Data Lookup/Rewrite", "")
app_realtime:value("yes", "Load")
app_realtime:value("no", "Do Not Load")
app_realtime:value("auto", "Load as Required")
app_realtime.rmempty = true

app_record = module:option(ListValue, "app_record", "Trivial Record Application", "")
app_record:value("yes", "Load")
app_record:value("no", "Do Not Load")
app_record:value("auto", "Load as Required")
app_record.rmempty = true

app_sayunixtime = module:option(ListValue, "app_sayunixtime", "Say time", "")
app_sayunixtime:value("yes", "Load")
app_sayunixtime:value("no", "Do Not Load")
app_sayunixtime:value("auto", "Load as Required")
app_sayunixtime.rmempty = true

app_senddtmf = module:option(ListValue, "app_senddtmf", "Send DTMF digits Application", "")
app_senddtmf:value("yes", "Load")
app_senddtmf:value("no", "Do Not Load")
app_senddtmf:value("auto", "Load as Required")
app_senddtmf.rmempty = true

app_sendtext = module:option(ListValue, "app_sendtext", "Send Text Applications", "")
app_sendtext:value("yes", "Load")
app_sendtext:value("no", "Do Not Load")
app_sendtext:value("auto", "Load as Required")
app_sendtext.rmempty = true

app_setcallerid = module:option(ListValue, "app_setcallerid", "Set CallerID Application", "")
app_setcallerid:value("yes", "Load")
app_setcallerid:value("no", "Do Not Load")
app_setcallerid:value("auto", "Load as Required")
app_setcallerid.rmempty = true

app_setcdruserfield = module:option(ListValue, "app_setcdruserfield", "CDR user field apps", "")
app_setcdruserfield:value("yes", "Load")
app_setcdruserfield:value("no", "Do Not Load")
app_setcdruserfield:value("auto", "Load as Required")
app_setcdruserfield.rmempty = true

app_setcidname = module:option(ListValue, "app_setcidname", "load => .so ; Set CallerID Name", "")
app_setcidname:value("yes", "Load")
app_setcidname:value("no", "Do Not Load")
app_setcidname:value("auto", "Load as Required")
app_setcidname.rmempty = true

app_setcidnum = module:option(ListValue, "app_setcidnum", "load => .so ; Set CallerID Number", "")
app_setcidnum:value("yes", "Load")
app_setcidnum:value("no", "Do Not Load")
app_setcidnum:value("auto", "Load as Required")
app_setcidnum.rmempty = true

app_setrdnis = module:option(ListValue, "app_setrdnis", "Set RDNIS Number", "")
app_setrdnis:value("yes", "Load")
app_setrdnis:value("no", "Do Not Load")
app_setrdnis:value("auto", "Load as Required")
app_setrdnis.rmempty = true

app_settransfercapability = module:option(ListValue, "app_settransfercapability", "Set ISDN Transfer Capability", "")
app_settransfercapability:value("yes", "Load")
app_settransfercapability:value("no", "Do Not Load")
app_settransfercapability:value("auto", "Load as Required")
app_settransfercapability.rmempty = true

app_sms = module:option(ListValue, "app_sms", "SMS/PSTN handler", "")
app_sms:value("yes", "Load")
app_sms:value("no", "Do Not Load")
app_sms:value("auto", "Load as Required")
app_sms.rmempty = true

app_softhangup = module:option(ListValue, "app_softhangup", "Hangs up the requested channel", "")
app_softhangup:value("yes", "Load")
app_softhangup:value("no", "Do Not Load")
app_softhangup:value("auto", "Load as Required")
app_softhangup.rmempty = true

app_stack = module:option(ListValue, "app_stack", "Stack Routines", "")
app_stack:value("yes", "Load")
app_stack:value("no", "Do Not Load")
app_stack:value("auto", "Load as Required")
app_stack.rmempty = true

app_system = module:option(ListValue, "app_system", "Generic System() application", "")
app_system:value("yes", "Load")
app_system:value("no", "Do Not Load")
app_system:value("auto", "Load as Required")
app_system.rmempty = true

app_talkdetect = module:option(ListValue, "app_talkdetect", "Playback with Talk Detection", "")
app_talkdetect:value("yes", "Load")
app_talkdetect:value("no", "Do Not Load")
app_talkdetect:value("auto", "Load as Required")
app_talkdetect.rmempty = true

app_test = module:option(ListValue, "app_test", "Interface Test Application", "")
app_test:value("yes", "Load")
app_test:value("no", "Do Not Load")
app_test:value("auto", "Load as Required")
app_test.rmempty = true

app_transfer = module:option(ListValue, "app_transfer", "Transfer", "")
app_transfer:value("yes", "Load")
app_transfer:value("no", "Do Not Load")
app_transfer:value("auto", "Load as Required")
app_transfer.rmempty = true

app_txtcidname = module:option(ListValue, "app_txtcidname", "TXTCIDName", "")
app_txtcidname:value("yes", "Load")
app_txtcidname:value("no", "Do Not Load")
app_txtcidname:value("auto", "Load as Required")
app_txtcidname.rmempty = true

app_url = module:option(ListValue, "app_url", "Send URL Applications", "")
app_url:value("yes", "Load")
app_url:value("no", "Do Not Load")
app_url:value("auto", "Load as Required")
app_url.rmempty = true

app_userevent = module:option(ListValue, "app_userevent", "Custom User Event Application", "")
app_userevent:value("yes", "Load")
app_userevent:value("no", "Do Not Load")
app_userevent:value("auto", "Load as Required")
app_userevent.rmempty = true

app_verbose = module:option(ListValue, "app_verbose", "Send verbose output", "")
app_verbose:value("yes", "Load")
app_verbose:value("no", "Do Not Load")
app_verbose:value("auto", "Load as Required")
app_verbose.rmempty = true

app_voicemail = module:option(ListValue, "app_voicemail", "Voicemail", "")
app_voicemail:value("yes", "Load")
app_voicemail:value("no", "Do Not Load")
app_voicemail:value("auto", "Load as Required")
app_voicemail.rmempty = true

app_waitforring = module:option(ListValue, "app_waitforring", "Waits until first ring after time", "")
app_waitforring:value("yes", "Load")
app_waitforring:value("no", "Do Not Load")
app_waitforring:value("auto", "Load as Required")
app_waitforring.rmempty = true

app_waitforsilence = module:option(ListValue, "app_waitforsilence", "Wait For Silence Application", "")
app_waitforsilence:value("yes", "Load")
app_waitforsilence:value("no", "Do Not Load")
app_waitforsilence:value("auto", "Load as Required")
app_waitforsilence.rmempty = true

app_while = module:option(ListValue, "app_while", "While Loops and Conditional Execution", "")
app_while:value("yes", "Load")
app_while:value("no", "Do Not Load")
app_while:value("auto", "Load as Required")
app_while.rmempty = true


return cbimap
