function $(s) {
       return document.getElementById(s.substring(1));
}

function show(s) {
       $(s).style.display = 'block';
}

function hide(s) {
       $(s).style.display = 'none';
}

function set_server() {
	hide("#error_box");
	data.url = $("#server").value;
	ubus_call("uci", "set", { "config": "attendedsysupgrade", "section": "server", values: { "url": data.url } })
	ubus_call("uci", "commit", { "config": "attendedsysupgrade" })
	var server_button = $("#server")
	server_button.type = 'button';
	server_button.className = 'cbi-button cbi-button-edit';
	server_button.parentElement.removeChild($("#button_set"));
	server_button.onclick = edit_server;
}

function edit_server() {
	$("#server").type = 'text';
	$("#server").onkeydown = function(event) {
		if(event.key === 'Enter') {
			set_server();
			return false;
		}
	}
	$("#server").className = '';
	$("#server").onclick = null;

	button_set = document.createElement("input");
	button_set.type = "button";
	button_set.value = "Save";
	button_set.name = "button_set";
	button_set.id = "button_set";
	button_set.className = 'cbi-button cbi-button-edit';
	button_set.style = 'background-image: url("/luci-static/resources/cbi/save.gif");'
	button_set.onclick = set_server
	$("#server").parentElement.appendChild(button_set);
}

function edit_packages() {
	data.edit_packages = true
	hide("#edit_button");
	$("#edit_packages").value = data.packages.join("\n");
	show("#edit_packages");
}

// requests to the upgrade server
function server_request(request_dict, path, callback) {
	request_dict.distro = data.release.distribution;
	request_dict.target = data.release.target.split("\/")[0];
	request_dict.subtarget = data.release.target.split("\/")[1];
	var request = new XMLHttpRequest();
	request.open("POST", data.url + "/" + path, true);
	request.setRequestHeader("Content-type", "application/json");
	request.send(JSON.stringify(request_dict));
	request.onerror = function(e) {
		error_box("upgrade server down")
		show("#server_div");
	}
	request.addEventListener('load', function(event) {
		callback(request)
	});
}

// initial setup, get system information
function setup() {
	ubus_call("rpc-sys", "packagelist", {}, "packages");
	ubus_call("system", "board", {}, "release");
	ubus_call("system", "board", {}, "board_name");
	ubus_call("system", "board", {}, "model");
	ubus_call("system", "info", {}, "memory");
	uci_get({ "config": "attendedsysupgrade", "section": "server", "option": "url" })
	uci_get({ "config": "attendedsysupgrade", "section": "client", "option": "upgrade_packages" })
	uci_get({ "config": "attendedsysupgrade", "section": "client", "option": "advanced_mode" })
	uci_get({ "config": "attendedsysupgrade", "section": "client", "option": "auto_search" })
	setup_ready();
}

function setup_ready() {
	// checks if a async ubus calls have finished
	if(ubus_counter != ubus_closed) {
		setTimeout(setup_ready, 300)
	} else {
		if(data.auto_search == 1) {
			upgrade_check();
		} else {
			show("#upgrade_button");
			show("#server_div");
			$("#server").value = data.url;
		}
	}
}

function uci_get(option) {
	// simple wrapper to get a uci value store in data.<option>
	ubus_call("uci", "get", option, option["option"])
}

ubus_counter = 0;
ubus_closed = 0;
function ubus_call(command, argument, params, variable) {
	var request_data = {};
	request_data.jsonrpc = "2.0";
	request_data.id = ubus_counter;
	request_data.method = "call";
	request_data.params = [ data.ubus_rpc_session, command, argument, params ]
	request_json = JSON.stringify(request_data)
	ubus_counter++;
	var request = new XMLHttpRequest();
	request.open("POST", ubus_url, true);
	request.setRequestHeader("Content-type", "application/json");
	request.onload = function(event) {
		if(request.status === 200) {
			var response = JSON.parse(request.responseText)
			if(!("error" in response) && "result" in response) {
				if(response.result.length === 2) {
					if(command === "uci") {
						data[variable] = response.result[1].value
					} else {
						data[variable] = response.result[1][variable]
					}
				}
			} else {
				error_box("<b>Ubus call faild:</b><br />Request: " + request_json + "<br />Response: " + JSON.stringify(response))
			}
			ubus_closed++;
		}
	}
	request.send(request_json);
}

function info_box(info_output, loading) {
	// Shows notification if upgrade is available
	// If loading is true then an "processing" animation is added
	show("#info_box");
	var loading_image = '';
	if(loading) {
		loading_image = '<img src="/luci-static/resources/icons/loading.gif" alt="Loading" style="vertical-align:middle">';
	}
	$("#info_box").innerHTML = loading_image + info_output;
}

function error_box(error_output) {
	// Shows erros in red box
	show("#error_box");
	$("#error_box").innerHTML = error_output;
	hide("#info_box");
}

function upgrade_check() {
	// Asks server for new firmware
	// If data.upgrade_packages is set to true search for new package versions as well
	hide("#error_box");
	hide("#server_div");
	info_box("Searching for upgrades", true);
	var request_dict = {}
	request_dict.version = data.release.version;
	request_dict.packages = data.packages;
	request_dict.upgrade_packages = data.upgrade_packages
	server_request(request_dict, "api/upgrade-check", upgrade_check_callback)
}

function upgrade_check_callback(request_text) {
	var request_json = JSON.parse(request_text)

	// create simple output to tell user whats going to be upgrade (release/packages)
	var info_output = ""
	if(request_json.version != undefined) {
		info_output += "<h3>New firmware release available</h3>"
		info_output += data.release.version + " to " + request_json.version
		data.latest_version = request_json.version;
	}
	if(request_json.upgrades != undefined) {
		info_output += "<h3>Package upgrades available</h3>"
		for (upgrade in request_json.upgrades) {
			info_output += "<b>" + upgrade + "</b>: " + request_json.upgrades[upgrade][1] + " to " + request_json.upgrades[upgrade][0] + "<br />"
		}
	}
	data.packages = request_json.packages
	info_box(info_output)

	if(data.advanced_mode == 1) {
		show("#edit_button");
	}
	var upgrade_button = $("#upgrade_button")
	upgrade_button.value = "Request firmware";
	upgrade_button.style.display = "block";
	upgrade_button.disabled = false;
	upgrade_button.onclick = upgrade_request;

}

function upgrade_request() {
	// Request the image
	// Needed values
	// version/release
	// board_name or model (server tries to find the corrent profile)
	// packages
	// The rest is added by server_request()
	$("#upgrade_button").disabled = true;
	hide("#edit_packages");
	hide("#edit_button");
	hide("#keep_container");

	var request_dict = {}
	request_dict.version = data.latest_version;
	request_dict.board = data.board_name
	request_dict.model = data.model

	if(data.edit_packages == true) {
		request_dict.packages = $("#edit_packages").value.split("\n")
	} else {
		request_dict.packages = data.packages;
	}

	server_request(request_dict, "api/upgrade-request", upgrade_request_callback)
}

function upgrade_request_callback(request) {
	// ready to download
	var request_json = JSON.parse(request);
	data.sysupgrade_url = request_json.sysupgrade;

	var filename_split = data.sysupgrade_url.split("/")
	data.filename = filename_split[filename_split.length - 1]

	info_output = 'Firmware created: <a href="' + data.sysupgrade_url + '"><b>' + data.filename + '</b></a>'
	if(data.advanced_mode == 1) {
		info_output += '<br /><a target="_blank" href="' + data.sysupgrade_url + '.log">Build log</a>'
	}
	info_box(info_output);

	show("#keep_container");
	var upgrade_button = $("#upgrade_button")
	upgrade_button.disabled = false;
	upgrade_button.style.display = "block";
	upgrade_button.value = "Flash firmware";
	upgrade_button.onclick = download_image;
}

function flash_image() {
	// Flash image via rpc-sys upgrade_start
	info_box("Flashing firmware. Don't unpower device", true)
	ubus_call("rpc-sys", "upgrade_start", { "keep": $("#keep").checked }, 'message');
	ping_max = 3600; // in seconds
	setTimeout(ping_ubus, 10000)
}

function ping_ubus() {
	// Tries to connect to ubus. If the connection fails the device is likely still rebooting.
	// If more time than ping_max passes update may failed
	if(ping_max > 0) {
		ping_max--;
		var request = new XMLHttpRequest();
		request.open("GET", ubus_url, true);
		request.addEventListener('error', function(event) {
			info_box("Rebooting device", true);
			setTimeout(ping_ubus, 1000)
		});
		request.addEventListener('load', function(event) {
			info_box("Success! Please reload web interface");
			$("#upgrade_button").value = "Reload page";
			show("#upgrade_button");
			$("#upgrade_button").disabled = false;
			$("#upgrade_button").onclick = function() { location.reload(); }
		});
		request.send();
	} else {
		error_box("Web interface could not reconnect to your device. Please reload web interface or check device manually")
	}
}

function upload_image(blob) {
	// Uploads received blob data to the server using cgi-io
	var request = new XMLHttpRequest();
	var form_data  = new FormData();

	form_data.append("sessionid", data.ubus_rpc_session)
	form_data.append("filename", "/tmp/firmware.bin")
	form_data.append("filemode", 755) // insecure?
	form_data.append("filedata", blob)

	request.addEventListener('load', function(event) {
		request_json = JSON.parse(request.responseText)
		flash_image();
	});

	request.addEventListener('error', function(event) {
		info_box("Upload of firmware failed, please retry by reloading web interface")
	});

	request.open('POST', origin + '/cgi-bin/cgi-upload');
	request.send(form_data);
}


function download_image() {
	// Download image from server once the url was received by upgrade_request
	hide("#keep_container");
	hide("#upgrade_button");
	var download_request = new XMLHttpRequest();
	download_request.open("GET", data.sysupgrade_url);
	download_request.responseType = "arraybuffer";

	download_request.onload = function () {
		if (this.status === 200) {
			var blob = new Blob([download_request.response], {type: "application/octet-stream"});
			upload_image(blob)
		}
	};
	info_box("Downloading firmware", true);
	download_request.send();
}

function server_request(request_dict, path, callback) {
	request_dict.distro = data.release.distribution;
	request_dict.target = data.release.target.split("\/")[0];
	request_dict.subtarget = data.release.target.split("\/")[1];
	var request = new XMLHttpRequest();
	request.open("POST", data.url + "/" + path, true);
	request.setRequestHeader("Content-type", "application/json");
	request.send(JSON.stringify(request_dict));
	request.onerror = function(e) {
		error_box("Upgrade server down or could not connect")
		show("#server_div");
	}
	request.addEventListener('load', function(event) {
		request_text = request.responseText;
		if (request.status === 200) {
			callback(request_text)

		} else if (request.status === 202) {
			var imagebuilder = request.getResponseHeader("X-Imagebuilder-Status");
			if(imagebuilder === "queue") {
				// in queue
				var queue = request.getResponseHeader("X-Build-Queue-Position");
				info_box("In build queue position " + queue, true)
				console.log("queued");
			} else if(imagebuilder === "initialize") {
				info_box("Setting up ImageBuilder", true)
				console.log("Setting up imagebuilder");
			} else if(imagebuilder === "building") {
				info_box("Building image", true);
				console.log("building");
			} else {
				// fallback if for some reasons the headers are missing e.g. browser blocks access
				info_box("Processing request", true);
				console.log(imagebuilder)
			}
			setTimeout(function() { server_request(request_dict, path, callback) }, 5000)

		} else if (request.status === 204) {
			// no upgrades available
			info_box("No upgrades available")

		} else if (request.status === 400) {
			// bad request
			request_json = JSON.parse(request_text)
			error_box(request_json.error)

		} else if (request.status === 412) {
			// this is a bit generic
			error_box("Unsupported device, release, target, subtraget or board")

		} else if (request.status === 413) {
		error_box("No firmware created due to image size. Try again with less packages selected.")

		} else if (request.status === 422) {
			var package_missing = request.getResponseHeader("X-Unknown-Package");
			error_box("Unknown package in request: <b>" + package_missing + "</b>")
		} else if (request.status === 500) {
			request_json = JSON.parse(request_text)

			error_box_content = "<b>Internal server error</b><br />"
			error_box_content += request_json.error
			if(request_json.log != undefined) {
				data.log_url = request_json.log
			}
			error_box(error_box_content)

		} else if (request.status === 501) {
			error_box("No sysupgrade file produced, may not supported by modell.")

		} else if (request.status === 502) {
			// python part offline
			error_box("Server down for maintenance")
			setTimeout(function() { server_request(request_dict, path, callback) }, 30000)
		} else if (request.status === 503) {
			error_box("Server overloaded")
			setTimeout(function() { server_request(request_dict, path, callback) }, 30000)
		}
	});
}
document.onload = setup()

