-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

format_au = module:option(ListValue, "format_au", "Sun Microsystems AU format (signed linear)", "")
format_au:value("yes", "Load")
format_au:value("no", "Do Not Load")
format_au:value("auto", "Load as Required")
format_au.rmempty = true

format_g723 = module:option(ListValue, "format_g723", "G.723.1 Simple Timestamp File Format", "")
format_g723:value("yes", "Load")
format_g723:value("no", "Do Not Load")
format_g723:value("auto", "Load as Required")
format_g723.rmempty = true

format_g726 = module:option(ListValue, "format_g726", "Raw G.726 (16/24/32/40kbps) data", "")
format_g726:value("yes", "Load")
format_g726:value("no", "Do Not Load")
format_g726:value("auto", "Load as Required")
format_g726.rmempty = true

format_g729 = module:option(ListValue, "format_g729", "Raw G729 data", "")
format_g729:value("yes", "Load")
format_g729:value("no", "Do Not Load")
format_g729:value("auto", "Load as Required")
format_g729.rmempty = true

format_gsm = module:option(ListValue, "format_gsm", "Raw GSM data", "")
format_gsm:value("yes", "Load")
format_gsm:value("no", "Do Not Load")
format_gsm:value("auto", "Load as Required")
format_gsm.rmempty = true

format_h263 = module:option(ListValue, "format_h263", "Raw h263 data", "")
format_h263:value("yes", "Load")
format_h263:value("no", "Do Not Load")
format_h263:value("auto", "Load as Required")
format_h263.rmempty = true

format_jpeg = module:option(ListValue, "format_jpeg", "JPEG (Joint Picture Experts Group) Image", "")
format_jpeg:value("yes", "Load")
format_jpeg:value("no", "Do Not Load")
format_jpeg:value("auto", "Load as Required")
format_jpeg.rmempty = true

format_pcm = module:option(ListValue, "format_pcm", "Raw uLaw 8khz Audio support (PCM)", "")
format_pcm:value("yes", "Load")
format_pcm:value("no", "Do Not Load")
format_pcm:value("auto", "Load as Required")
format_pcm.rmempty = true

format_pcm_alaw = module:option(ListValue, "format_pcm_alaw", "load => .so ; Raw aLaw 8khz PCM Audio support", "")
format_pcm_alaw:value("yes", "Load")
format_pcm_alaw:value("no", "Do Not Load")
format_pcm_alaw:value("auto", "Load as Required")
format_pcm_alaw.rmempty = true

format_sln = module:option(ListValue, "format_sln", "Raw Signed Linear Audio support (SLN)", "")
format_sln:value("yes", "Load")
format_sln:value("no", "Do Not Load")
format_sln:value("auto", "Load as Required")
format_sln.rmempty = true

format_vox = module:option(ListValue, "format_vox", "Dialogic VOX (ADPCM) File Format", "")
format_vox:value("yes", "Load")
format_vox:value("no", "Do Not Load")
format_vox:value("auto", "Load as Required")
format_vox.rmempty = true

format_wav = module:option(ListValue, "format_wav", "Microsoft WAV format (8000hz Signed Line", "")
format_wav:value("yes", "Load")
format_wav:value("no", "Do Not Load")
format_wav:value("auto", "Load as Required")
format_wav.rmempty = true

format_wav_gsm = module:option(ListValue, "format_wav_gsm", "Microsoft WAV format (Proprietary GSM)", "")
format_wav_gsm:value("yes", "Load")
format_wav_gsm:value("no", "Do Not Load")
format_wav_gsm:value("auto", "Load as Required")
format_wav_gsm.rmempty = true


return cbimap
