/*
 * xhr.js - XMLHttpRequest helper class
 * (c) 2008-2010 Jo-Philipp Wich
 */

XHR = function()
{
	this.reinit = function()
	{
		if (window.XMLHttpRequest) {
			this._xmlHttp = new XMLHttpRequest();
		}
		else if (window.ActiveXObject) {
			this._xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
		else {
			alert("xhr.js: XMLHttpRequest is not supported by this browser!");
		}
	}

	this.busy = function() {
		if (!this._xmlHttp)
			return false;

		switch (this._xmlHttp.readyState)
		{
			case 1:
			case 2:
			case 3:
				return true;

			default:
				return false;
		}
	}

	this.abort = function() {
		if (this.busy())
			this._xmlHttp.abort();
	}

	this.get = function(url,data,callback)
	{
		this.reinit();

		var xhr  = this._xmlHttp;
		var code = this._encode(data);

		url = location.protocol + '//' + location.host + url;

		if (code)
			if (url.substr(url.length-1,1) == '&')
				url += code;
			else
				url += '?' + code;

		xhr.open('GET', url, true);

		xhr.onreadystatechange = function()
		{
			if (xhr.readyState == 4) {
				var json = null;
				if (xhr.getResponseHeader("Content-Type") == "application/json") {
					try {
						json = eval('(' + xhr.responseText + ')');
					}
					catch(e) {
						json = null;
					}
				}

				callback(xhr, json);
			}
		}

		xhr.send(null);
	}

	this.post = function(url,data,callback)
	{
		this.reinit();

		var xhr  = this._xmlHttp;
		var code = this._encode(data);

		xhr.onreadystatechange = function()
		{
			if (xhr.readyState == 4)
				callback(xhr);
		}

		xhr.open('POST', url, true);
		xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		xhr.send(code);
	}

	this.cancel = function()
	{
		this._xmlHttp.onreadystatechange = function(){};
		this._xmlHttp.abort();
	}

	this.send_form = function(form,callback,extra_values)
	{
		var code = '';

		for (var i = 0; i < form.elements.length; i++)
		{
			var e = form.elements[i];

			if (e.options)
			{
				code += (code ? '&' : '') +
					form.elements[i].name + '=' + encodeURIComponent(
						e.options[e.selectedIndex].value
					);
			}
			else if (e.length)
			{
				for (var j = 0; j < e.length; j++)
					if (e[j].name) {
						code += (code ? '&' : '') +
							e[j].name + '=' + encodeURIComponent(e[j].value);
					}
			}
			else
			{
				code += (code ? '&' : '') +
					e.name + '=' + encodeURIComponent(e.value);
			}
		}

		if (typeof extra_values == 'object')
			for (var key in extra_values)
				code += (code ? '&' : '') +
					key + '=' + encodeURIComponent(extra_values[key]);

		return(
			(form.method == 'get')
				? this.get(form.getAttribute('action'), code, callback)
				: this.post(form.getAttribute('action'), code, callback)
		);
	}

	this._encode = function(obj)
	{
		obj = obj ? obj : { };
		obj['_'] = Math.random();

		if (typeof obj == 'object')
		{
			var code = '';
			var self = this;

			for (var k in obj)
				code += (code ? '&' : '') +
					k + '=' + encodeURIComponent(obj[k]);

			return code;
		}

		return obj;
	}
}

XHR.get = function(url, data, callback)
{
	(new XHR()).get(url, data, callback);
}

XHR.poll = function(interval, url, data, callback)
{
	if (isNaN(interval) || interval < 1)
		interval = 5;

	if (!XHR._q)
	{
		XHR._t = 0;
		XHR._q = [ ];
		XHR._r = function() {
			for (var i = 0, e = XHR._q[0]; i < XHR._q.length; e = XHR._q[++i])
			{
				if (!(XHR._t % e.interval) && !e.xhr.busy())
					e.xhr.get(e.url, e.data, e.callback);
			}

			XHR._t++;
		};
	}

	XHR._q.push({
		interval: interval,
		callback: callback,
		url:      url,
		data:     data,
		xhr:      new XHR()
	});

	XHR.run();
}

XHR.halt = function()
{
	if (XHR._i)
	{
		/* show & set poll indicator */
		try {
			document.getElementById('xhr_poll_status').style.display = '';
			document.getElementById('xhr_poll_status_on').style.display = 'none';
			document.getElementById('xhr_poll_status_off').style.display = '';
		} catch(e) { }

		window.clearInterval(XHR._i);
		XHR._i = null;
	}
}

XHR.run = function()
{
	if (XHR._r && !XHR._i)
	{
		/* show & set poll indicator */
		try {
			document.getElementById('xhr_poll_status').style.display = '';
			document.getElementById('xhr_poll_status_on').style.display = '';
			document.getElementById('xhr_poll_status_off').style.display = 'none';
		} catch(e) { }

		/* kick first round manually to prevent one second lag when setting up
		 * the poll interval */
		XHR._r();
		XHR._i = window.setInterval(XHR._r, 1000);
	}
}

XHR.running = function()
{
	return !!(XHR._r && XHR._i);
}
