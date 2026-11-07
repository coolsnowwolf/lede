'use strict';
'require view';
'require dom';
'require form';
'require rpc';
'require poll';
'require ui';
'require uci';
'require network';

var callStatus = rpc.declare({
	object: 'luci.zte-mesh',
	method: 'status',
	expect: { '': {} }
});

var callRestart = rpc.declare({
	object: 'luci.zte-mesh',
	method: 'restart',
	expect: { '': {} }
});

var callRebootNode = rpc.declare({
	object: 'luci.zte-mesh',
	method: 'reboot_node',
	params: [ 'mac' ],
	expect: { '': {} }
});

var callLightNode = rpc.declare({
	object: 'luci.zte-mesh',
	method: 'light_node',
	params: [ 'mac', 'on' ],
	expect: { '': {} }
});

/* The controller answers a node action with a status code (see the plugin): null = the message for "sent", otherwise the reason. */
function actionProblem(res) {
	var r = res ? res.result : undefined;
	if (r === 0)
		return null;
	if (r === null || r === undefined)
		return _('No answer from the controller yet.');
	return {
		'-2': _('The request was malformed.'),
		'-3': _('That router is not online or is not a ZTE router.'),
		'-4': _('The controller is restarting; try again in a moment.'),
		'-5': _('The controller is not running.')
	}[String(r)] || _('The controller could not send the request (%s).').format(r);
}

function normMac(mac) {
	return (mac || '').toLowerCase();
}

/* Node 1905 AL MAC -> IPv4 address, using whatever the router has learned (luci-rpc keys its hosts by upper-case MAC). */
function lookupAddress(hints, mac) {
	var host = hints && hints.hosts ? hints.hosts[(mac || '').toUpperCase()] : null;
	if (host && host.ipaddrs && host.ipaddrs.length)
		return host.ipaddrs[0];
	return null;
}

function badge(text, cls) {
	return E('span', { 'class': 'ifacebadge', 'style': cls ? 'background:' + cls : null }, text);
}

/*
 * A table section whose edit dialog says why it does not close. LuCI saves the
 * dialog silently and drops the error, so a missing required field only shows
 * as a red frame, often on a field that is out of sight.
 */
var ReportingGridSection = form.GridSection.extend({
	handleModalSave: function(modalMap, ev) {
		var self = this, args = arguments;
		var mapNode = this.getActiveModalMap();
		var activeMap = dom.findClassInstance(mapNode);

		return activeMap.parse().then(function() {
			return self.super('handleModalSave', args);
		}).catch(function(e) {
			var box = mapNode.querySelector('.zte-mesh-modal-error');

			if (!box) {
				box = E('div', { 'class': 'alert-message warning zte-mesh-modal-error' });
				mapNode.insertBefore(box, mapNode.firstChild);
			}
			dom.content(box, E('p', [ _('Not saved: '), e.message ]));
		});
	}
});

return view.extend({
	load: function() {
		return Promise.all([
			callStatus(),
			network.getHostHints(),
			uci.load('zte_mesh')
		]);
	},

	renderStatus: function(info) {
		var state = info.state;
		var rows = [];
		var stale = info.age < 0 || info.age > 10;

		function row(label, value) {
			rows.push(E('div', { 'class': 'tr' }, [
				E('div', { 'class': 'td left', 'width': '33%' }, label),
				E('div', { 'class': 'td left' }, value)
			]));
		}

		if (!info.installed) {
			return E('div', { 'class': 'alert-message warning' },
				_('The ZTE Mesh runtime is not installed on this router.'));
		}

		var running = info.running && state && state.running && !stale;
		row(_('Controller'), running
			? badge(_('Running'), '#46a546')
			: badge(info.running ? _('Starting or stalled') : _('Stopped'), '#b94a48'));

		if (state && state.running) {
			row(_('Controller address (AL MAC)'), state.al_mac || '-');
			row(_('Self-organizing network'), uci.get('zte_mesh', 'main', 'mesh_v2') === '1'
				? _('On (ZTE enhanced)') : _('Off (ZTE compatible)'));
			row(_('Wired port'), info.attached ? badge(_('Attached to the LAN bridge'), '#46a546')
			                                   : badge(_('Not attached'), '#b94a48'));
			row(_('Wi-Fi updates sent'), String(state.config_applied));
		}

		if (state && state.config_error)
			row(_('Last configuration problem'), E('span', { 'style': 'color:#b94a48' }, state.config_error));
		if (state && state.restart_required)
			row(_('Pending'), _('A setting changed that the controller reads only at start; restart the controller to apply it.'));
		if (stale && info.running)
			row(_('Report'), _('No fresh status for %d s').format(info.age));

		return E('div', { 'class': 'table' }, rows);
	},

	/* Restart and LED buttons of one child router (ZTE routers only; the controller tunnels them to the router). */
	renderNodeActions: function(node, title) {
		var done = function(res, okText) {
			var problem = actionProblem(res);
			ui.addNotification(null, E('p', problem || okText), problem ? 'warning' : 'info');
		};
		var led = function(on) {
			return ui.createHandlerFn(this, function() {
				return callLightNode(node.mac, on).then(function(res) {
					done(res, on ? _('Indicator light switched on.') : _('Indicator light switched off.'));
				});
			});
		};
		return E('div', { 'class': 'cbi-section-actions' }, [
			E('button', {
				'class': 'btn cbi-button cbi-button-negative',
				'click': ui.createHandlerFn(this, function() {
					ui.showModal(_('Restart router'), [
						E('p', _('Restart %s? Its Wi-Fi and the devices connected through it are offline for a few minutes.').format(title)),
						E('div', { 'class': 'right' }, [
							E('button', { 'class': 'btn', 'click': ui.hideModal }, _('Cancel')), ' ',
							E('button', {
								'class': 'btn cbi-button cbi-button-negative important',
								'click': ui.createHandlerFn(this, function() {
									ui.hideModal();
									return callRebootNode(node.mac).then(function(res) {
										done(res, _('The restart request was sent.'));
									});
								})
							}, _('Restart'))
						])
					]);
				})
			}, _('Restart')), ' ',
			E('button', { 'class': 'btn cbi-button', 'click': led(true) }, _('Light on')), ' ',
			E('button', { 'class': 'btn cbi-button', 'click': led(false) }, _('Light off'))
		]);
	},

	renderNodes: function(info, hints) {
		var state = info.state;
		var nodes = (state && state.nodes) || [];
		var table = E('table', { 'class': 'table cbi-section-table' }, [
			E('tr', { 'class': 'tr table-titles' }, [
				E('th', { 'class': 'th' }, _('Name')),
				E('th', { 'class': 'th' }, _('Model')),
				E('th', { 'class': 'th' }, _('Address')),
				E('th', { 'class': 'th' }, _('MAC')),
				E('th', { 'class': 'th' }, _('Software')),
				E('th', { 'class': 'th' }, _('Type')),
				E('th', { 'class': 'th' }, _('Light')),
				E('th', { 'class': 'th' }, _('Clients')),
				E('th', { 'class': 'th' }, _('State')),
				E('th', { 'class': 'th' }, _('Actions'))
			])
		]);
		var self = this;

		/* names the administrator gave to child routers (the controller keeps them, like the factory topology page; the router itself is not renamed) */
		var aliases = {};
		uci.sections('zte_mesh', 'zte_mesh_node').forEach(function(sec) {
			if (sec.mac && sec.name)
				aliases[normMac(sec.mac)] = sec.name;
		});

		nodes.forEach(function(node) {
			var ip = lookupAddress(hints, node.mac);
			var title = aliases[normMac(node.mac)] || node.name || node.model || node.mac;
			table.appendChild(E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td' }, title),
				E('td', { 'class': 'td' }, node.model || '-'),
				E('td', { 'class': 'td' }, ip
					? E('a', { 'href': 'http://' + ip + '/', 'target': '_blank', 'rel': 'noreferrer' }, ip)
					: '-'),
				E('td', { 'class': 'td' }, node.mac),
				E('td', { 'class': 'td' }, node.software || '-'),
				E('td', { 'class': 'td' }, node.zte ? 'ZTE' : _('Standard')),
				E('td', { 'class': 'td' }, node.light === 1 ? _('On') : (node.light === 0 ? _('Off') : '-')),
				E('td', { 'class': 'td' }, (node.online && node.stations >= 0) ? String(node.stations) : '-'),
				E('td', { 'class': 'td' }, node.online ? badge(_('Online'), '#46a546') : badge(_('Offline'), '#999')),
				E('td', { 'class': 'td' }, (node.online && node.zte) ? self.renderNodeActions(node, title) : '-')
			]));
		});

		if (!nodes.length)
			table.appendChild(E('tr', { 'class': 'tr placeholder' }, [
				E('td', { 'class': 'td', 'colspan': 10 },
					E('em', _('No child routers have been discovered yet.')))
			]));

		return table;
	},

	render: function(data) {
		var info = data[0], hints = data[1];
		var statusBox = E('div', { 'id': 'zte-mesh-status' }, this.renderStatus(info));
		var nodesBox = E('div', { 'id': 'zte-mesh-nodes' }, this.renderNodes(info, hints));
		var self = this;

		poll.add(function() {
			return Promise.all([ callStatus(), network.getHostHints() ]).then(function(res) {
				dom.content(statusBox, self.renderStatus(res[0]));
				dom.content(nodesBox, self.renderNodes(res[0], res[1]));
			});
		}, 5);

		var m = new form.Map('zte_mesh', _('ZTE Mesh'),
			_('Manage ZTE child routers over the wired LAN. This router acts as the mesh controller; the child routers receive the Wi-Fi profile below.'));
		var s = m.section(form.NamedSection, 'main', 'zte_mesh');
		s.addremove = false;
		var o;

		s.tab('general', _('General'));
		s.tab('wifi', _('Wi-Fi profile'));
		s.tab('policy', _('Roaming and steering'));

		o = s.taboption('general', form.Flag, 'enabled', _('Enable'));
		o.default = '1';
		o.rmempty = false;

		o = s.taboption('general', form.Value, 'bridge', _('LAN bridge'));
		o.default = 'br-lan';
		o.datatype = 'maxlength(15)';
		o.rmempty = false;
		o.description = _('The bridge the child routers are cabled to (up to 15 characters, default br-lan).');

		o = s.taboption('general', form.ListValue, 'mesh_role', _('Device role'));
		o.value('', _('Default (master)'));
		o.value('master', _('Main router'));
		o.value('auto', _('Adaptive'));
		o.optional = true;
		o.description = _('The factory "main router / adaptive" switch. Only the controller role is supported. Changing from main router to adaptive restarts the mesh daemon twice and the child routers drop out of the topology for a short while; adding or removing this option restarts the service.');

		o = s.taboption('general', form.Flag, 'mesh_v2', _('Self-organizing network'));
		o.default = '0';
		o.rmempty = false;
		o.description = _('The factory MESH switch. On (ZTE enhanced): only ZTE child routers with the same market code are accepted. Off (ZTE compatible, default): any child router is accepted. Switching restarts the mesh daemon (the child routers drop out of the topology for up to a minute).');

		o = s.taboption('general', form.Flag, 'mlo_enable', _('Wi-Fi 7 MLO'));
		o.optional = true;
		o.default = '1';
		o.description = _('Tell child routers whether multi-link operation is on (factory default: on). Only Wi-Fi 7 children act on it.');

		o = s.taboption('wifi', form.Flag, 'wifi_dual_band_sync', _('Same name and password on both bands'));
		o.default = '1';
		o.rmempty = false;
		o.description = _('On: the 5 GHz band uses the 2.4 GHz name, password, security, hidden setting and MAC filter policy. Off: the 5 GHz name is the 2.4 GHz name plus the suffix below unless a 5 GHz name is given.');

		function profile(band, label) {
			var id = 'wifi_' + band;
			var five = band === '5g';

			o = s.taboption('wifi', form.Flag, id + '_enabled', _('%s enabled').format(label));
			o.default = '1';
			o.rmempty = false;
			o.description = _('Off switches the main network of this band off on the child routers.');

			o = s.taboption('wifi', form.Value, id + '_ssid', _('%s network name (SSID)').format(label));
			o.datatype = 'and(minlength(1),maxlength(31))';
			if (five) {
				o.rmempty = true;
				o.depends('wifi_dual_band_sync', '0');
				o.placeholder = _('2.4 GHz name + suffix');
				o.description = _('1 to 31 characters. Empty: the 2.4 GHz name plus the suffix below.');
			} else {
				o.rmempty = false;
				o.default = 'ImmortalWrt';
				o.description = _('1 to 31 characters. Required.');
			}

			o = s.taboption('wifi', form.Value, id + '_key', _('%s password').format(label));
			o.password = true;
			o.datatype = 'and(minlength(8),maxlength(63))';
			if (five) {
				o.rmempty = true;
				o.depends('wifi_dual_band_sync', '0');
				o.placeholder = _('Same as 2.4 GHz');
				o.description = _('8 to 63 characters. Empty: the 2.4 GHz password.');
			} else {
				o.rmempty = false;
				o.default = '00000000';
				o.placeholder = _('8 to 63 characters');
				o.description = _('8 to 63 characters, default 00000000. Required: the controller does not start without one.');
			}

			if (five) {
				o = s.taboption('wifi', form.Value, 'wifi_5g_suffix', _('5 GHz name suffix'));
				o.datatype = 'and(minlength(1),maxlength(8))';
				o.default = '-5G';
				o.optional = true;
				o.depends('wifi_dual_band_sync', '0');
				o.description = _('1 to 8 characters, appended to the 2.4 GHz name when no 5 GHz name is given (factory default -5G). The whole name must stay within 31 characters.');
			}

			o = s.taboption('wifi', form.Flag, id + '_hidden', _('%s hidden network name').format(label));
			o.optional = true;
			o.default = '0';
			if (five)
				o.depends('wifi_dual_band_sync', '0');

			o = s.taboption('wifi', form.ListValue, id + '_security', _('%s security').format(label));
			o.value('psk2', 'WPA2-PSK');
			o.value('psk-mixed', 'WPA/WPA2-PSK');
			o.value('sae-mixed', 'WPA2/WPA3');
			o.value('sae', 'WPA3');
			o.value('none', _('None (open network)'));
			o.default = five ? 'psk-mixed' : 'sae-mixed';
			o.optional = true;
			o.description = _('Child routers without WPA3 treat WPA2/WPA3 as WPA2 and ignore WPA3-only. "None" makes the main network open.');
			if (five)
				o.depends('wifi_dual_band_sync', '0');

			o = s.taboption('wifi', form.Value, id + '_beacon_interval', _('%s beacon interval (ms)').format(label));
			o.datatype = 'range(20,1000)';
			o.default = '100';
			o.optional = true;
			o.description = _('20 to 1000 ms, factory default 100. Changing it restarts the radio of the child routers for a few seconds.');

			o = s.taboption('wifi', form.Value, id + '_channel', _('%s channel').format(label));
			o.datatype = five ? 'or(range(0,0),range(36,165))' : 'range(0,13)';
			o.default = '0';
			o.optional = true;
			o.description = five
				? _('0 = automatic (factory default). Fixed: 36, 40, 44, 48, 52 to 64, 100 to 144 and 149 to 165 in steps of 4. Empty: keep the child routers\' setting.')
				: _('0 = automatic (factory default). Fixed: 1 to 13. Empty: keep the child routers\' setting.');

			o = s.taboption('wifi', form.Value, id + '_tx_power', _('%s transmit power (dBm, EIRP)').format(label));
			o.datatype = 'range(0,30)';
			o.default = five ? '23' : '20';
			o.optional = true;
			o.description = five
				? _('0 to 30 dBm. The child routers round it to one of five levels: above 21 is full power (factory default), 20 to 21, 16 to 19, 10 to 15, below 10. Empty: keep the child routers\' setting.')
				: _('0 to 30 dBm. The child routers round it to one of five levels: above 19 is full power (factory default), 16 to 19, 12 to 15, 8 to 11, below 8. Empty: keep the child routers\' setting.');

			o = s.taboption('wifi', form.ListValue, id + '_acl_policy', _('%s MAC filter policy').format(label));
			o.value('', _('Keep the child routers\' setting'));
			o.value('disabled', _('Off'));
			o.value('ban', _('Block listed devices'));
			o.value('allow', _('Allow only listed devices'));
			o.optional = true;
			if (five)
				o.depends('wifi_dual_band_sync', '0');
		}
		profile('24g', _('2.4 GHz'));
		profile('5g', _('5 GHz'));

		o = s.taboption('policy', form.Flag, 'band_steer', _('Band steering'));
		o.optional = true;
		o.default = '1';
		o.description = _('Move capable devices to the better band (factory default: on while both bands share a name). The factory turns this off together with the dual-band switch; here it is a separate choice.');

		o = s.taboption('policy', form.Value, 'band_steer_rssi_24g', _('Band steering threshold, 2.4 GHz (dBm)'));
		o.datatype = 'range(-110,-30)';
		o.default = '-60';
		o.optional = true;
		o.description = _('-110 to -30 dBm, factory default -60. The controller\'s own decision threshold; set both bands or neither.');
		o = s.taboption('policy', form.Value, 'band_steer_rssi_5g', _('Band steering threshold, 5 GHz (dBm)'));
		o.datatype = 'range(-110,-30)';
		o.default = '-110';
		o.optional = true;
		o.description = _('-110 to -30 dBm, factory default -110. Set both bands or neither.');

		o = s.taboption('policy', form.Value, 'roam_rssi_24g', _('Roaming threshold, 2.4 GHz (dBm)'));
		o.datatype = 'range(-110,-30)';
		o.default = '-60';
		o.optional = true;
		o.description = _('-110 to -30 dBm, factory default -60 (the factory page allows -110 to -40). A device weaker than this is moved to a better router. Sent to the child routers; some models, the Z506 among them, ignore it. Set both bands or neither.');
		o = s.taboption('policy', form.Value, 'roam_rssi_5g', _('Roaming threshold, 5 GHz (dBm)'));
		o.datatype = 'range(-110,-30)';
		o.default = '-70';
		o.optional = true;
		o.description = _('-110 to -30 dBm, factory default -70. Set both bands or neither.');

		o = s.taboption('policy', form.Flag, 'roam_force', _('Force roaming on weak signal'));
		o.optional = true;
		o.default = '0';
		o.description = _('Kick devices that stay on a weak signal (factory default: off).');

		var b = m.section(ReportingGridSection, 'zte_mesh_bss', _('Extra Wi-Fi networks'),
			_('Guest, spare and hidden backhaul networks of the child routers. The slots of one band must be 1, 2, 3 in order without a gap: removing slot 1 while slot 2 exists is refused. Adding or removing a slot makes the child routers drop out of the topology for up to a minute; switching one on or off does not.'));
		b.addremove = true;
		b.anonymous = true;
		b.nodescriptions = true;

		o = b.option(form.ListValue, 'band', _('Band'));
		o.value('24g', '2.4 GHz');
		o.value('5g', '5 GHz');
		o.default = '24g';
		o.rmempty = false;

		o = b.option(form.ListValue, 'slot', _('Slot'));
		o.value('1', _('1 (guest)'));
		o.value('2', _('2 (spare)'));
		o.value('3', _('3 (hidden backhaul)'));
		o.default = '1';
		o.rmempty = false;

		o = b.option(form.Flag, 'enabled', _('Enabled'));
		o.default = '1';
		o.rmempty = false;

		o = b.option(form.Value, 'ssid', _('Network name (SSID)'));
		o.datatype = 'and(minlength(1),maxlength(31))';
		o.rmempty = false;
		o.placeholder = _('1 to 31 characters');
		o.description = _('1 to 31 characters. Required.');

		o = b.option(form.Value, 'key', _('Password'));
		o.password = true;
		o.datatype = 'and(minlength(8),maxlength(63))';
		o.rmempty = false;
		o.modalonly = true;
		o.placeholder = _('8 to 63 characters');
		o.description = _('8 to 63 characters. Required, also for an open network (it is kept for when security is switched on).');

		o = b.option(form.Flag, 'hidden', _('Hidden network name'));
		o.optional = true;
		o.modalonly = true;

		o = b.option(form.ListValue, 'security', _('Security'));
		o.value('psk2', 'WPA2-PSK');
		o.value('psk-mixed', 'WPA/WPA2-PSK');
		o.value('sae-mixed', 'WPA2/WPA3');
		o.value('sae', 'WPA3');
		o.value('none', _('None (open network)'));
		o.default = 'psk2';
		o.optional = true;
		o.modalonly = true;

		o = b.option(form.Value, 'access_minutes', _('Switch off after (minutes)'));
		o.datatype = 'range(1,10080)';
		o.optional = true;
		o.modalonly = true;
		o.placeholder = _('Empty: no time limit');
		o.description = _('1 to 10080 minutes (7 days); empty = no time limit. Guest time limit: the network is switched off this long after it came on. Switching it off and on again, or changing this value, restarts the countdown. Not kept across a controller restart.');

		o = b.option(form.Flag, 'force_wifi5', _('Wi-Fi 5 compatible network'));
		o.optional = true;
		o.modalonly = true;
		o.description = _('Announces the network as Wi-Fi 5 only to child routers that understand it; many models ignore it.');

		o = b.option(form.ListValue, 'acl_policy', _('MAC filter policy'));
		o.value('', _('Keep the child routers\' setting'));
		o.value('disabled', _('Off'));
		o.value('ban', _('Block listed devices'));
		o.value('allow', _('Allow only listed devices'));
		o.optional = true;
		o.modalonly = true;

		var nm = m.section(ReportingGridSection, 'zte_mesh_node', _('Router names'),
			_('Names for child routers, shown in the list above instead of the name the router reports. They are kept on this router only; the child routers keep their own names.'));
		nm.addremove = true;
		nm.anonymous = true;
		nm.nodescriptions = true;

		o = nm.option(form.Value, 'mac', _('MAC address'));
		o.datatype = 'macaddr';
		o.rmempty = false;
		o.placeholder = 'aa:bb:cc:dd:ee:ff';
		o.description = _('The MAC address shown for the router in the list above.');

		o = nm.option(form.Value, 'name', _('Name'));
		o.datatype = 'and(minlength(1),maxlength(63))';
		o.rmempty = false;
		o.description = _('1 to 63 characters.');

		var t = m.section(ReportingGridSection, 'zte_mesh_timer', _('Wi-Fi schedule'),
			_('Switch Wi-Fi off during these windows on every child router. "All Wi-Fi" turns off the main networks and the guest and spare networks (the hidden backhaul network stays on); "Guest network" turns off guest network 1 only. A window that ends before it starts runs past midnight. Times are in the router\'s time zone (the zone in effect when the settings were saved).'));
		t.addremove = true;
		t.anonymous = true;
		t.nodescriptions = true;

		function timeOfDay(section_id, value) {
			return /^([01][0-9]|2[0-3]):[0-5][0-9]$/.test(value) || _('Use the form HH:MM, for example 23:00');
		}

		o = t.option(form.Value, 'start', _('Off from'));
		o.default = '23:00';
		o.rmempty = false;
		o.validate = timeOfDay;
		o.description = _('HH:MM, 00:00 to 23:59.');

		o = t.option(form.Value, 'end', _('Back on at'));
		o.default = '07:00';
		o.rmempty = false;
		o.validate = timeOfDay;
		o.description = _('HH:MM, 00:00 to 23:59. Earlier than the start: the window runs past midnight.');

		o = t.option(form.MultiValue, 'days', _('On the days that start on'));
		o.value('mon', _('Mon'));
		o.value('tue', _('Tue'));
		o.value('wed', _('Wed'));
		o.value('thu', _('Thu'));
		o.value('fri', _('Fri'));
		o.value('sat', _('Sat'));
		o.value('sun', _('Sun'));
		o.default = 'mon tue wed thu fri sat sun';
		o.rmempty = false;

		o = t.option(form.ListValue, 'scope', _('Switch off'));
		o.value('all', _('All Wi-Fi'));
		o.value('guest', _('Guest network'));
		o.default = 'all';

		var a = m.section(ReportingGridSection, 'zte_mesh_acl', _('MAC filter list'),
			_('Up to 16 devices. A rule applies to the main network of the chosen band or to one of its extra networks; the policy above decides whether listed devices are blocked or the only ones allowed.'));
		a.addremove = true;
		a.anonymous = true;
		a.nodescriptions = true;

		o = a.option(form.Value, 'mac', _('MAC address'));
		o.datatype = 'macaddr';
		o.rmempty = false;
		o.placeholder = 'aa:bb:cc:dd:ee:ff';
		o.description = _('The address the device uses now: phones often use a private MAC per network.');

		o = a.option(form.Value, 'name', _('Name'));
		o.datatype = 'maxlength(20)';
		o.optional = true;
		o.description = _('Up to 20 characters: letters, digits and _ . -. Empty: acl1, acl2 and so on.');
		o.validate = function(section_id, value) {
			if (value === '' || /^[A-Za-z0-9_.-]+$/.test(value))
				return true;
			return _('Only letters, digits and _ . - are allowed');
		};

		o = a.option(form.ListValue, 'band', _('Band'));
		o.value('', _('Both'));
		o.value('24g', '2.4 GHz');
		o.value('5g', '5 GHz');
		o.optional = true;

		o = a.option(form.ListValue, 'bss', _('Network'));
		o.value('', _('Main network'));
		o.value('1', _('Extra network 1'));
		o.value('2', _('Extra network 2'));
		o.value('3', _('Extra network 3'));
		o.optional = true;

		var restart = E('button', {
			'class': 'btn cbi-button cbi-button-action',
			'click': ui.createHandlerFn(this, function() {
				return callRestart().then(function() {
					ui.addNotification(null, E('p', _('The controller is restarting.')), 'info');
				});
			})
		}, _('Restart controller'));

		return m.render().then(function(formNode) {
			return E([
				E('div', { 'class': 'cbi-section' }, [
					E('h3', _('Status')), statusBox,
					E('div', { 'class': 'cbi-page-actions', 'style': 'text-align:left' }, restart)
				]),
				E('div', { 'class': 'cbi-section' }, [ E('h3', _('Child routers')), nodesBox ]),
				formNode
			]);
		});
	}
});
