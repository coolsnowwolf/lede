'use strict';
'require fs';
'require ui';
'require form';
'require uci';
'require view';

return view.extend({
	load: function() {
		return Promise.all([
			uci.load('sqm-npu'),
		]);
	},

	applyService: function() {
		var enabled = uci.get('sqm-npu', 'wan', 'enabled') === '1';
		if (enabled)
			return fs.exec('/etc/init.d/sqm-npu', [ 'enable' ]).then(function() {
				return fs.exec('/etc/init.d/sqm-npu', [ 'reload' ]);
			});

		return fs.exec('/etc/init.d/sqm-npu', [ 'stop' ]).then(function() {
			return fs.exec('/etc/init.d/sqm-npu', [ 'disable' ]);
		});
	},

	handleSaveApply: function(ev, mode) {
		var self = this;

		return this.super('handleSaveApply', [ ev, mode ]).then(function() {
			return self.applyService();
		}).then(function() {
			ui.addNotification(null, E('p', _('SQM configuration applied.')));
		}).catch(function(err) {
			ui.addNotification(null, E('p', _('Configuration was saved, but applying the SQM policy failed: %s').format(err.message || err)));
		});
	},

	render: function(data) {
		var m = new form.Map('sqm-npu', _('ZTE SQM'));
		m.description = _('Manage WAN upload and download rates with the SR1010 hardware shaper.');

		var s = m.section(form.NamedSection, 'wan', 'wan', _('WAN SQM'));
		s.addremove = false;
		s.anonymous = false;

		var o = s.option(form.Flag, 'enabled', _('Enable SQM'));
		o.rmempty = false;
		o.default = '0';

		o = s.option(form.ListValue, 'wan_ifname', _('WAN interface'));
		o.value('eth0', _('eth0 (SR1010 physical WAN)'));
		o.default = 'eth0';
		o.rmempty = false;
		o.description = _('The NPU hardware shaper currently supports the physical WAN device eth0. PPPoE and VLAN interfaces are not valid shaper targets.');

		o = s.option(form.Value, 'upload', _('Upload limit (kbit/s)'), _('0 disables upload shaping. NPU hardware mode supports up to 2500000 kbit/s.'));
		o.datatype = 'and(uinteger,min(0),max(2500000))';
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.Value, 'download', _('Download limit (kbit/s)'), _('0 disables download shaping. In NPU mode the rate is shared across all queues on the LAN conduit.'));
		o.datatype = 'and(uinteger,min(0),max(2500000))';
		o.default = '0';
		o.rmempty = false;

		o = s.option(form.DummyValue, '_profile', _('Traffic priority mode'));
		o.default = _('Balanced');
		o.description = _('Balanced mode assigns each TCP/UDP hardware flow direction independently to NPU queues 1-7 from its flow tuple.');

		return m.render();
	}
});
