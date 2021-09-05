-- Copyright 2014 Roger D <rogerdammit@gmail.com>
-- Licensed to the public under the Apache License 2.0.

m = Map("mjpg-streamer", "MJPG-streamer", translate("mjpg streamer is a streaming application for Linux-UVC compatible webcams"))

--- General settings ---

section_gen = m:section(TypedSection, "mjpg-streamer", translate("General"))
    section_gen.addremove=false
    section_gen.anonymous=true

enabled = section_gen:option(Flag, "enabled", translate("Enabled"), translate("Enable MJPG-streamer"))

input = section_gen:option(ListValue, "input",  translate("Input plugin"))
   input:depends("enabled", "1")
   input:value("uvc", "UVC")
   ---input:value("file", "File")
   input.optional = false

output = section_gen:option(ListValue, "output",  translate("Output plugin"))
   output:depends("enabled", "1")
   output:value("http", "HTTP")
   output:value("file", "File")
   output.optional = false


--- Plugin settings ---

s = m:section(TypedSection, "mjpg-streamer", translate("Plugin settings"))
    s.addremove=false
    s.anonymous=true

    s:tab("output_http", translate("HTTP output"))
    s:tab("output_file", translate("File output"))
    s:tab("input_uvc", translate("UVC input"))
    ---s:tab("input_file", translate("File input"))


--- Input UVC settings ---

this_tab = "input_uvc"

device = s:taboption(this_tab, Value, "device", translate("Device"))
    device.default="/dev/video0"
    --device.datatype = "device"
    device:value("/dev/video0", "/dev/video0")
    device:value("/dev/video1", "/dev/video1")
    device:value("/dev/video2", "/dev/video2")
    device.optional = false

resolution = s:taboption(this_tab, Value, "resolution", translate("Resolution"))
    resolution.default = "640x480"
    resolution:value("320x240", "320x240")
    resolution:value("640x480", "640x480")
    resolution:value("800x600", "800x600")
    resolution:value("864x480", "864x480")
    resolution:value("960x544", "960x544")
    resolution:value("960x720", "960x720")
    resolution:value("1280x720", "1280x720")
    resolution:value("1280x960", "1280x960")
    resolution:value("1920x1080", "1920x1080")
    resolution.optional = true

fps = s:taboption(this_tab, Value, "fps", translate("Frames per second"))
    fps.datatype = "and(uinteger, min(1))"
    fps.placeholder = "5"
    fps.optional = true

yuv = s:taboption(this_tab, Flag, "yuv", translate("Enable YUYV format"), translate("Automatic disabling of MJPEG mode"))

quality = s:taboption(this_tab, Value, "quality", translate("JPEG compression quality"), translate("Set the quality in percent. This setting activates YUYV format, disables MJPEG"))
    quality.datatype = "range(0, 100)"

minimum_size = s:taboption(this_tab, Value, "minimum_size", translate("Drop frames smaller then this limit"),translate("Set the minimum size if the webcam produces small-sized garbage frames. May happen under low light conditions"))
    minimum_size.datatype = "uinteger"

no_dynctrl = s:taboption(this_tab, Flag, "no_dynctrl", translate("Don't initalize dynctrls"), translate("Do not initalize dynctrls of Linux-UVC driver"))

led = s:taboption(this_tab, ListValue, "led", translate("Led control"))
    led:value("on", translate("On"))
    led:value("off", translate("Off"))
    led:value("blink", translate("Blink"))
    led:value("auto", translate("Auto"))
    led.optional = true


--- Output HTTP settings ---

this_tab = "output_http"

port=s:taboption(this_tab, Value, "port", translate("Port"), translate("TCP port for this HTTP server"))
    port.datatype = "port"
    port.placeholder = "8080"

enable_auth = s:taboption(this_tab, Flag, "enable_auth", translate("Authentication required"), translate("Ask for username and password on connect"))
    enable_auth.default = false

username = s:taboption(this_tab, Value, "username", translate("Username"))
    username:depends("enable_auth", "1")
    username.optional = false

password = s:taboption(this_tab, Value, "password", translate("Password"))
    password:depends("enable_auth", "1")
    password.password = true
    password.optional = false
    password.default = false

www = s:taboption(this_tab, Value, "www", translate("WWW folder"), translate("Folder that contains webpages"))
    www.datatype = "directory"
    www.default = "/www/webcam/"
    www.optional = false


--- HTTP preview  ---

html = [[
<style media="screen" type="text/css">
    .img-preview {
        display: inline-block;
        height: auto;
        width: 640px;
        padding: 4px;
        line-height: 1.428571429;
        background-color: #fff;
        border: 1px solid #ddd;
        border-radius: 4px;
        -webkit-transition: all .2s ease-in-out;
        transition: all .2s ease-in-out;
        margin-bottom: 5px;
	display: none;
    }
</style>

<div id="videodiv">
	<img id="video_preview" class="img-preview" onerror="on_error()" onload="on_load()"/>
        <p id="stream_status" style="text-align: center; color: orange; font-weight:bold;">Stream unavailable</p>
</div>

<script type="text/javascript">

function init_stream() {
    console.log('init_stream');
    start_stream()
}

function _start_stream() {
	console.log('_start_stream');

        port = document.getElementById('cbid.mjpg-streamer.core.port').value

        if (document.getElementById('cbid.mjpg-streamer.core.enable_auth').checked) {
            user = document.getElementById('cbid.mjpg-streamer.core.username').value
            pass = document.getElementById('cbid.mjpg-streamer.core.password').value
            login = user + ":" + pass + "@"
        } else {
            login = ""
        }

	img = document.getElementById('video_preview');
	img.src = 'http://' + login + location.hostname + ':' + port + '/?action=snapshot';
}

function start_stream() {
	console.log('start_stream');

	setTimeout(function() { _start_stream(); }, 500);
}

function on_error() {
    console.log('on_error');

    img = document.getElementById('video_preview');
    img.style.display = 'none';

    stream_stat = document.getElementById('stream_status');
    stream_stat.style.display = 'block';

    start_stream();
}

function on_load() {
    console.log('on_load');

    img = document.getElementById('video_preview');
    img.style.display = 'block';

    stream_stat = document.getElementById('stream_status');
    stream_stat.style.display = 'none';
}

init_stream()

</script>
]]

preview = s:taboption(this_tab, DummyValue, "_dummy", html)
    preview:depends("output", "http")

--- Output file settings ---

this_tab = "output_file"

folder=s:taboption(this_tab, Value, "folder", translate("Folder"), translate("Set folder to save pictures"))
    folder.placeholder="/tmp/images"
    folder.datatype = "directory"

--mjpeg=s:taboption(this_tab, Value, "mjpeg", translate("Mjpeg output"), translate("Check to save the stream to an mjpeg file"))

delay=s:taboption(this_tab, Value, "delay", translate("Interval between saving pictures"), translate("Set the inteval in millisecond"))
    delay.placeholder="5000"
    delay.datatype = "uinteger"

ringbuffer=s:taboption(this_tab, Value, "ringbuffer", translate("Ring buffer size"), translate("Max. number of pictures to hold"))
    ringbuffer.placeholder="10"
    ringbuffer.datatype = "uinteger"

exceed=s:taboption(this_tab, Value, "exceed", translate("Exceed"), translate("Allow ringbuffer to exceed limit by this amount"))
    exceed.datatype = "uinteger"

command=s:taboption(this_tab, Value, "command", translate("Command to run"), translate("Execute command after saving picture. Mjpg-streamer parse the filename as first parameter to your script."))


return m
