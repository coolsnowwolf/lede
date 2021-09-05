/*
Copyright (c) 2006, Lorenz Schori <lo@znerol.ch>
All rights reserved (Naja: Ich hab' trotzdem was geaendert. Sven-Ola). (Naja:
diese Rechte garantiert dir die BSD-Lizenz ja ausdrücklich. Lorenz)

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
- Neither the name of the <ORGANIZATION> nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

OLSR-Viz is inspired by Wi-viz: http://wiviz.natetrue.com

Changes:
2007-10-04: Added hostname display option -- Stefan Katerkamp <stefan@katerkamp.de>.
2007-10-04: Optimized display by moving presentation css out of js -- lo
2010-12-11: Changed some paths to make it work with Kamikaze and Luci -- soma
*/

var cgi_url = "/cgi-bin/vizdata.sh";

var maxmetric = 3;
var iconvariant = "-mini";
var nodes = new Array();
var ncount = 0;
var newnodes = new Array();
var edges = new Array();
var iel = 220; // ideal edge length
var optsize = 10; // boundingbox around nodes

var vwidth = 0;
var vheight = 0;

var xoff = 0;
var yoff = 0;
var scale = 1.0;

var idle_timeout = 15;
var erase_timeout = 60;
var dcl_timeout = 250;
var dcllow_timeout = 500;
var auto_declump = true;
var showdesc = true;
var auto_save = 1;
var now_secs = 5;

// dom elements
var IFrameObj;
var maindiv;
var nodediv;
var edgediv;

/******* CALL TO SERVER ********/
function callToServer(URL) {
	var IFrameDoc;

	if (IFrameObj.document) {
		// For IE5 + opera
		IFrameDoc = IFrameObj.document;
	}
	else if (IFrameObj.contentDocument) {
		// For NS6
		IFrameDoc = IFrameObj.contentDocument;
	}
	else if (IFrameObj.contentWindow) {
		// For IE5.5 and IE6
		IFrameDoc = IFrameObj.contentWindow.document;
	}
 	else {
		// opera? hmmmm
		return true;
	}

	IFrameDoc.location.replace(URL);
	return false;
}


/******** EDGE CLASS ********/
function edge(n1,n2){
	this.getHTML = function()
	{
		var nh = "";

		if(this.n1.metric > maxmetric || this.n2.metric > maxmetric) {
			return "";
		}

		x = this.n1.x*scale;
		y = this.n1.y*scale;
		dx = this.n2.x*scale - x;
		dy = this.n2.y*scale - y;

		x += xoff*scale + 75;
		y += yoff*scale + 15;

		imgtag = "<img src='/luci-static/resources/olsr-viz/dot_"
		if (this.etx > 0 && this.etx < 2) {
			imgtag += "good.gif'";
		}
		else if(this.etx > 2 && this.etx < 5) {
			imgtag += "ok.gif'";
		}
		else if(this.etx > 5 && this.etx < 10) {
			imgtag += "weak.gif'";
		}
		else {
			imgtag += "down.gif'";
		}
		imgtag += " alt='ETX: " + this.etx + "' title='ETX: " + this.etx + "' ";

		d = Math.sqrt(dx*dx+dy*dy);

		for (j = 0; j < d; j += 15) {
			nh += imgtag + "style='top:"
			+ parseInt(y+dy * j / d) + "px; left:"
			+ parseInt(x+dx * j / d) + "px; "
			+ "width: 4px; height: 4px; position: absolute; z-index: 2' >";
		}

		nh += "<div style='top:"
		+ parseInt(y+dy * 0.5 - 5) + "px; left:"
		+ parseInt(x+dx * 0.5 - 24) + "px; "
		+ "position: absolute; z-index: 3; width: 48px; text-align: center;' >"
		+ "<span class='label etx' >" + this.etx + "</span></div>";

		return nh;
	}

	this.isIdle = function()
	{
		return (now_secs - this.lastseen > idle_timeout);
	}

	this.isDead = function()
	{
		return (now_secs - this.lastseen > erase_timeout);
	}

	this.cleanup = function()
	{
		if(this.n1 && this.n1.weight) {
			this.n1.weight--;
		}
		if(this.n2 && this.n2.weight) {
			this.n2.weight--;
		}
		if(this.n1 && this.n2) {
			delete this.n1.edges[n2.ip];
			delete this.n2.edges[n1.ip];
		}
	}

	this.n1 = n1;
	this.n2 = n2;

	// setup edges within node objects
	this.n1.weight++;
	this.n1.edges[n2.ip] = this;
	this.n2.weight++;
	this.n2.edges[n1.ip] = this;

	return this;
}

function getEdgeKey(ip1,ip2)
{
	key = "";
	if(ip1 > ip2) {
		key = ip2 + "-" + ip1;
	}
	else {
		key = ip1 + "-" + ip2;
	}
	return key;
}

function touch_edge(n1,n2,etx)
{
	var key = getEdgeKey(n1.ip,n2.ip);
	var e = edges[key];
	if(!e) {
		e = new edge(n1,n2);
		edges[key] = e;
	}
	e.etx = etx;
	e.lastseen = now_secs;
	return e;
}

/******** NODE CLASS ********/
function node(ip) {
	this.getHTML = function()
	{
		var nh;

		if(this.metric > maxmetric) {
			return "";
		}
		var igw = 0;
		for(h in this.hna) {
			if(h == "0.0.0.0") {
				igw = 1;
				break;
			}
		}
		nh =
			"<div id='node_" + this.ip + "' onmousedown='dragstart(this)' style="
			+ "'top: " + parseInt((this.y+yoff)*scale) + "px; "
			+ "left: " + parseInt((this.x+xoff)*scale) + "px; "
			+ "width: 150px; height: 1px; z-index: 4; "
			+ "position: absolute; background-color: transparent;' >"
			+ "<div><img src='/luci-static/resources/olsr-viz/node"+(igw ? "-hna" : "")+iconvariant + ".gif'"
			+ " alt='node " + this.ip + "' style='border: none;'><br>"
			+ 	"<a href='http://" + this.ip + "/'>"
			+ 	"<span class='label ip'>" + this.ip + "</span></a>"
			+ 	(showdesc && this.desc != "" ? 
						"<br><span class='label desc'>" + this.desc + "</span>" : "")
			+ "</div></div>";
		return nh;
	}

	this.isIdle = function()
	{
		return (now_secs - this.lastseen > idle_timeout);
	}

	this.isDead = function()
	{
		return (now_secs - this.lastseen > erase_timeout);
	}

	this.cleanup = function()
	{
		ncount--;
	}
	
	this.set_metric = function(metric) {
		this.metric = metric;
		return this;
	}
	
	this.set_desc = function(desc) {
		this.desc = desc
		return this;
	}
	
	this.update = function() {
		this.lastseen = now_secs;
		return this;
	}
	
	this.ip = ip;
	this.x = 0;
	this.y = 0;
	this.dx_last=0;
	this.dy_last=0;
	this.placed = false;
	this.weight = 0;
	this.edges = new Array();
	this.hna = new Array();
	this.pinned = false;
	this.metric = 999;
	this.desc = "";

	ncount++;
	return this;
}

function touch_node(ip) {
	n = nodes[ip];
	if(!n) {
		n = new node(ip);
		nodes[ip] = n;
		// newnodes.push(n);
		// push and pop not supported in old ie. shit.
		newnodes[newnodes.length] = n;
	}
	return n;
}

function place_new_nodes() {
	var nc = 0;
	for(i = 0;i<newnodes.length;i++){
		n = newnodes[i];
		if(n.placed){continue;}
		if(sp = getCookie("node_"+n.ip)) {
			xy = sp.split("x");
			debug_writeln("sp: "+sp+" xy[0]: "+xy[0]+" xy[1]: "+xy[1]);
			n.x = parseFloat(xy[0]);
			n.y = parseFloat(xy[1]);
		}
		else if(n.weight>1){
			// see if we find allredy placed nodes
			ox=0,oy=0;dx=0,dy=0;c=0;
			for(e in n.edges){
				if(nodes[e] && nodes[e].placed){
					if(!ox && !oy) {
						ox = nodes[e].x;
						oy = nodes[e].y;
					}
					else {
						dx += nodes[e].x - ox;
						dy += nodes[e].y - oy;
					}
					c++;
				}
			}
			if(c>0) {
				n.x = ox + dx/c + Math.random()*iel/2-iel/4;
				n.y = oy + dy/c + Math.random()*iel/2-iel/4;
			}
		}
		else {
			// beginn somewhere
			n.x = Math.random()*400;
			n.y = Math.random()*400;
		}
		n.placed = true;
		nc++;
	}
	newnodes.length=0;
	return nc;
}

/******** HNA CLASS ********/
function hna(gw,net,mask) {
	this.gw = gw;
	this.net = net;
	this.mask = mask;
	return this;
}

function touch_hna(node,net,mask) {
	h = node.hna[net];
	if(!h) {
		h = new hna(node.ip,net,mask);
		node.hna[net] = h;
	}

	h.lastseen = now_secs;
	return h;
}

/******** VIZ SETUP AND SETTINGS ********/
function viz_setup(iframeid,maindivid,nodedivid,edgedivid) {
	// assign a reference to the
	// object to our global variable IFrameObj.
	IFrameObj=document.getElementById(iframeid);
	if (document.frames) {
		// this is for IE5 Mac, because it will only
		// allow access to the document object
		// of the IFrame if we access it through
		// the document.frames array
		IFrameObj = document.frames[iframeid];
	}

	draginit();

	maindiv=document.getElementById(maindivid);
	nodediv=document.getElementById(nodedivid);
	edgediv=document.getElementById(edgedivid);

	// autosave on exit?
	if((autosave = getCookie("prefs_autosave"))) {
		auto_save = parseInt(autosave);
	}
	viz_autosave(auto_save);

	// maximum metric of surrounding nodes
	if(mmx = getCookie("prefs_maxmetric")) {
		set_maxmetric(mmx,true,true);
	}

	// scale of view
	if((savescale = getCookie("prefs_scale")) &&
		 (savescale = parseFloat(savescale))) {
			set_scale(savescale,true);
	}

	// scroll - FIXME
	/*
	if(val = getCookie("prefs_innerview")) {
		iv = val.split("x");
		if (iv[0] && (iv[0] = parseInt(iv[0])) &&
				iv[1] && (iv[2] = parseInt(iv[2])) &&
				iv[3] && (iv[3] = parseInt(iv[3])) &&
				iv[4] && (iv[4] = parseInt(iv[4])))
		{
			maindiv.scrollLeft = iv[0] + "px";
			maindiv.scrollHeight = iv[1] + "px";
		}
	}
	*/
}

function viz_save()
{
	// let cookie survive a month
	exp = new Date();
	exp.setTime(exp.getTime() + 2592000000);
	// save node positions
	for(ip in nodes)
	{
		if(nodes[ip].metric > maxmetric) {
			continue;
		}
		setCookie("node_"+ip,nodes[ip].x+"x"+nodes[ip].y,exp);
	}

	// save maxmetric
	setCookie("prefs_maxmetric",maxmetric,exp);

	// save zooming
	setCookie("prefs_scale",scale,exp);

	// save scroll - FIXME
	setCookie("prefs_innerview",
		parseInt(maindiv.scrollLeft)+"x"+parseInt(maindiv.scrollTop)+"x"+
		parseInt(vwidth*scale)+"x"+parseInt(vheight*scale),exp);
}

function viz_autosave(autosave)
{
	auto_save = autosave;
	if(auto_save) {
		document.body.onunload=viz_save;
	}
	else {
		deleteCookie("prefs_autosave");
	}
}

function viz_reset()
{
	deleteAllCookies();
	for(ip in nodes) {
		delete nodes[ip];
	}
	for(e in edges) {
		delete edges[e];
	}
	viz_update();
}

var updateTimer = 0;
function viz_update() {
	if (updateTimer) {
		clearTimeout(updateTimer);
	}
	now_secs = new Date().getTime()/1000;
	callToServer(cgi_url);
}

function viz_callback() {
	if (updateTimer) {
		clearTimeout(updateTimer);
	}

	if(place_new_nodes() > 0 && auto_declump) {
		declump();
	}
	refresh();
	updateTimer = setTimeout('viz_update()', 5000);
}

var refresh_running = false;
function refresh() {
	if(refresh_running) {
		return;
	}
	refresh_running = true;

	var nh = "";

	// refresh nodes
	nh = "";
	for (var n in nodes) {
		if(nodes[n].isDead()) {
			nodes[n].cleanup();
			delete nodes[n];
		}
		else {
			nh += nodes[n].getHTML();
		}
	}
	nodediv.innerHTML = nh;

	// refresh edges

		nh = "";
	for (var e in edges) {
		if(edges[e].isDead()) {
			edges[e].cleanup();
			delete edges[e];
		}
		else {
			nh += edges[e].getHTML();
		}
	}
	edgediv.innerHTML = nh;
	refresh_running = false;
}

function set_showdesc(doit)
{
	showdesc = doit;
	if(!noupdate) refresh();
}

function set_autodeclump(doit)
{
	auto_declump = doit;
	if(doit) {
		declump();
	}
	else {
		clearTimeout(dclTimer);
	}
}

function set_scale(inscale,noupdate)
{
	if(!inscale) {
		inscale = parseFloat(document.getElementById("zoom").value/2);
	}
	scale = Math.round(inscale*100)/100;
	if(!scale || scale<0.1) {
		scale = 0.1;
	}
	document.getElementById("zoom").value = scale*2;
	if(!noupdate) refresh();
}

function set_maxmetric(inmetric,noupdate,noconfirm)
{
	inmetric = parseInt(inmetric);
	if(inmetric > 0 || !noconfirm || confirm("warning. setting the maximum metric to zero can lead to expensive calculations if you are connected to a network with many nodes. do you want to proceed?")) {
		maxmetric = inmetric;
	}
	document.getElementById("maxmetric").value = maxmetric;
	if(!noupdate) refresh();
}

// k = area / nodes
function fr(x) {
	return Math.pow((iel*iel)/x,2);
}

function fa(x) {
	return Math.pow((x*x)/iel,2);
}

var dclTimer = 0;
var declump_running = false;
function declump(t) {
	// clear declump timer
	if(dclTimer) {
		clearTimeout(dclTimer);
	}
	if(declump_running) {
		return;
	}
	declump_running = true;

	// nodes
	nc = 0;
	for (var ip1 in nodes) {
		nodes[ip1].fr_x=0;
		nodes[ip1].fr_y=0;
		nodes[ip1].fa_x=0;
		nodes[ip1].fa_y=0;
		nodes[ip1].x_next = nodes[ip1].x;
		nodes[ip1].y_next = nodes[ip1].y;
		nodes[ip1].randdisplace = 0;
	}
	for (var ip1 in nodes) {
		if(nodes[ip1].metric > maxmetric || nodes[ip1].pinned) {
			continue;
		}
		for (ip2 in nodes) {
			if (nodes[ip2].metric > maxmetric || ip1 == ip2) {
				continue;
			}
			dx = (nodes[ip1].x_next - nodes[ip2].x_next);
			dy = (nodes[ip1].y_next - nodes[ip2].y_next);
			d = Math.sqrt(dx*dx+dy*dy);
			d = Math.max(d-optsize,(d+optsize)/optsize);

			nodes[ip1].fr_x += (dx/d) * fr(d);
			nodes[ip1].fr_y += (dy/d) * fr(d);
		}

		dx = nodes[ip1].fr_x;
		dy = nodes[ip1].fr_y;
		d = Math.sqrt(dx*dx+dy*dy);
		md = Math.min(d,iel/nodes[ip1].weight);
		nodes[ip1].x_next += (dx / d) * md;
		nodes[ip1].y_next += (dy / d) * md;
		nc++;
	}

	// edges
	ec = 0;
	for (var e in edges) {
		if (!edges[e].n1 || !edges[e].n2 ||
			edges[e].n1.metric > maxmetric || edges[e].n2.metric > maxmetric) {
			continue;
		}
		dx = (edges[e].n1.x_next - edges[e].n2.x_next);
		dy = (edges[e].n1.y_next - edges[e].n2.y_next);
		d = Math.sqrt(dx*dx+dy*dy);
//		d = Math.max(d-optsize,(d+optsize)/optsize);

		edges[e].n1.fa_x -= (dx/d) * fa(d);
		edges[e].n1.fa_y -= (dy/d) * fa(d);
		edges[e].n2.fa_x += (dx/d) * fa(d);
		edges[e].n2.fa_y += (dy/d) * fa(d);
		ec++;
	}

	// displacement
	xmin=-20;ymin=-20;xmax=20;ymax=20;dsum=0;
	for (var ip in nodes) {
		if(nodes[ip].metric > maxmetric || nodes[ip].pinned) {
			continue;
		}

		dx = nodes[ip].fa_x;
		dy = nodes[ip].fa_y;
		d = Math.sqrt(dx*dx+dy*dy);
		dx = (dx / d) * Math.min(d,iel/nodes[ip].weight) * 0.75 + nodes[ip].dx_last * 0.25;
		dy = (dy / d) * Math.min(d,iel/nodes[ip].weight) * 0.75 + nodes[ip].dy_last * 0.25;

		nodes[ip].dx_last = dx;
		nodes[ip].dy_last = dy;
		nodes[ip].x_next += dx;
		nodes[ip].y_next += dy;

		if(!nodes[ip].x_next || !nodes[ip].y_next) {
			continue;
		}

		dx = (nodes[ip].x - nodes[ip].x_next);
		dy = (nodes[ip].y - nodes[ip].y_next);
		dsum += Math.sqrt(dx*dx+dy*dy);

		nodes[ip].x = nodes[ip].x_next;
		nodes[ip].y = nodes[ip].y_next;

		xmin = Math.min(xmin,nodes[ip].x);
		xmax = Math.max(xmax,nodes[ip].x);
		ymin = Math.min(ymin,nodes[ip].y);
		ymax = Math.max(ymax,nodes[ip].y);
	}
	vwidth=(xmax-xmin);
	vheight=(ymax-ymin);

	xoff=-xmin;
	yoff=-ymin;
	/*
	document.getElementById('debug').innerHTML = "<br>" +
		"offset: " + xoff + "x" + yoff + " dsum: " + dsum + "<br>" +
		"nc: " + nc + " ec: " + ec + "xmax: " + xmax + " xmin: " + xmin +	"<br>" +
		"optsize: " + optsize + "<br>";
		*/
	refresh();
	if(auto_declump) {
		dclTimer = setTimeout("declump()", dsum>ncount ? dcl_timeout : dcllow_timeout );
	}
	declump_running = false;
}

//Das Objekt, das gerade bewegt wird.
var dragip = null;

// Position, an der das Objekt angeklickt wurde.
var dragx = 0;
var dragy = 0;

// Mausposition
var posx = 0;
var posy = 0;

function draginit() {
	// Initialisierung der ãberwachung der Events

	document.onmousemove = drag;
	document.onmouseup = dragstop;
}


function dragstart(element) {
	//Wird aufgerufen, wenn ein Objekt bewegt werden soll.
	dragip = element.id.split("_")[1];
	dragx = posx - element.offsetLeft;
	dragy = posy - element.offsetTop;

	n = nodes[dragip];
	if(n) {
		n.pinned = true;
	}
}


function dragstop() {
	//Wird aufgerufen, wenn ein Objekt nicht mehr bewegt werden soll.

	n = nodes[dragip];
	if(n) {
		n.pinned = false;
	}
	refresh();
	dragip=null;
}


function drag(ereignis) {
	//Wird aufgerufen, wenn die Maus bewegt wird und bewegt bei Bedarf das Objekt.

	posx = document.all ? window.event.clientX : ereignis.pageX;
	posy = document.all ? window.event.clientY : ereignis.pageY;
	if(dragip != null) {
		n = nodes[dragip];
		if(n) {
			n.x = (posx - dragx)/scale - xoff;
			n.y = (posy - dragy)/scale - yoff;
		}
		e = document.getElementById('node_'+dragip);
		e.style.left = parseInt((n.x+xoff)*scale) + "px";
		e.style.top = parseInt((n.y+yoff)*scale) + "px";
	}
}

function debug_writeln(line)
{
	document.getElementById('debug').innerHTML = line + "<br>" + document.getElementById('debug').innerHTML;
}

/**
 * Sets a Cookie with the given name and value.
 *
 * name			 Name of the cookie
 * value			Value of the cookie
 * [expires]	Expiration date of the cookie (default: end of current session)
 * [path]		 Path where the cookie is valid (default: path of calling document)
 * [domain]	 Domain where the cookie is valid
 *							(default: domain of calling document)
 * [secure]	 Boolean value indicating if the cookie transmission requires a
 *							secure transmission
 */

function setCookie(name, value, expires, path, domain, secure) {
	document.cookie= name + "=" + escape(value) +
		((expires) ? "; expires=" + expires.toGMTString() : "") +
		((path) ? "; path=" + path : "") +
		((domain) ? "; domain=" + domain : "") +
		((secure) ? "; secure" : "");
}

/**
 * Gets the value of the specified cookie.
 *
 * name	Name of the desired cookie.
 *
 * Returns a string containing value of specified cookie,
 *	 or null if cookie does not exist.
 */

function getCookie(name)
{
	var results = document.cookie.match ( name + '=(.*?)(;|$)' );
	if (results) {
		return unescape(results[1]);
	}
	return null;
}

/**
 * Deletes the specified cookie.
 *
 * name		name of the cookie
 * [path]	path of the cookie (must be same as path used to create cookie)
 * [domain]	domain of the cookie (must be same as domain used to create cookie)
 */

function deleteCookie(name, path, domain) {
	if (getCookie(name)) {
		document.cookie = name + "=" +
			((path) ? "; path=" + path : "") +
			((domain) ? "; domain=" + domain : "") +
			"; expires=Thu, 01-Jan-70 00:00:01 GMT";
	}
}

function deleteAllCookies() {
	cookies = document.cookie.split("; ");
	for(i=0;i<cookies.length;i++) {
		deleteCookie(cookies[i].split("=")[0]);
	}
}
