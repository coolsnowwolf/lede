-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

cbimap = Map("asterisk", "asterisk", "")

module = cbimap:section(TypedSection, "module", "Modules", "")
module.anonymous = true

codec_a_mu = module:option(ListValue, "codec_a_mu", "A-law and Mulaw direct Coder/Decoder", "")
codec_a_mu:value("yes", "Load")
codec_a_mu:value("no", "Do Not Load")
codec_a_mu:value("auto", "Load as Required")
codec_a_mu.rmempty = true

codec_adpcm = module:option(ListValue, "codec_adpcm", "Adaptive Differential PCM Coder/Decoder", "")
codec_adpcm:value("yes", "Load")
codec_adpcm:value("no", "Do Not Load")
codec_adpcm:value("auto", "Load as Required")
codec_adpcm.rmempty = true

codec_alaw = module:option(ListValue, "codec_alaw", "A-law Coder/Decoder", "")
codec_alaw:value("yes", "Load")
codec_alaw:value("no", "Do Not Load")
codec_alaw:value("auto", "Load as Required")
codec_alaw.rmempty = true

codec_g726 = module:option(ListValue, "codec_g726", "ITU G.726-32kbps G726 Transcoder", "")
codec_g726:value("yes", "Load")
codec_g726:value("no", "Do Not Load")
codec_g726:value("auto", "Load as Required")
codec_g726.rmempty = true

codec_gsm = module:option(ListValue, "codec_gsm", "GSM/PCM16 (signed linear) Codec Translation", "")
codec_gsm:value("yes", "Load")
codec_gsm:value("no", "Do Not Load")
codec_gsm:value("auto", "Load as Required")
codec_gsm.rmempty = true

codec_speex = module:option(ListValue, "codec_speex", "Speex/PCM16 (signed linear) Codec Translator", "")
codec_speex:value("yes", "Load")
codec_speex:value("no", "Do Not Load")
codec_speex:value("auto", "Load as Required")
codec_speex.rmempty = true

codec_ulaw = module:option(ListValue, "codec_ulaw", "Mu-law Coder/Decoder", "")
codec_ulaw:value("yes", "Load")
codec_ulaw:value("no", "Do Not Load")
codec_ulaw:value("auto", "Load as Required")
codec_ulaw.rmempty = true


return cbimap
