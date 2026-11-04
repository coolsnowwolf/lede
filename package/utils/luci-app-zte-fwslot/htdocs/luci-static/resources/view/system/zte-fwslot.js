'use strict';
'require view';
'require rpc';
'require ui';

var callInfo = rpc.declare({
	object: 'luci.zte-fwslot',
	method: 'info',
	expect: { '': {} }
});

var callSwitch = rpc.declare({
	object: 'luci.zte-fwslot',
	method: 'switch',
	params: [ 'reboot' ],
	expect: { '': {} }
});

/* Keep the firmware name for LEDE and Wrt derivatives. */
function systemName(version) {
	var m = /^(LEDE|\w*Wrt)\b/i.exec(version || '');

	return m ? m[1] : _('Vendor firmware');
}

return view.extend({
	load: function() {
		return callInfo();
	},

	handleSwitch: function(other) {
		ui.showModal(_('Switch firmware slot'), [
			E('p', _('The router checks the firmware in slot %d (%s). If it is intact, the router restarts into it. Nothing is erased.')
				.format(other.slot, other.version)),
			E('p', _('The web interface of that system may be at a different address.')),
			E('div', { 'class': 'right' }, [
				E('button', {
					'class': 'btn cbi-button cbi-button-neutral',
					'click': ui.hideModal
				}, _('Cancel')),
				' ',
				E('button', {
					'class': 'btn cbi-button cbi-button-negative important',
					'click': ui.createHandlerFn(this, 'doSwitch', other)
				}, _('Switch and reboot'))
			])
		]);
	},

	doSwitch: function(other) {
		ui.showModal(_('Switch firmware slot'), [
			E('p', { 'class': 'spinning' }, _('Checking the firmware…'))
		]);

		return callSwitch(true).then(function(res) {
			if (res.result !== 0) {
				ui.hideModal();
				ui.addNotification(null, E('p', _('The router did not switch: %s').format(res.message || _('unknown error'))), 'danger');
				return;
			}

			ui.showModal(_('Rebooting…'), [
				E('p', { 'class': 'spinning' }, _('The router is restarting into slot %d. This page does not reload by itself, because the other system may use a different address.').format(other.slot))
			]);
		}).catch(function(err) {
			ui.hideModal();
			ui.addNotification(null, E('p', _('The router did not switch: %s').format(err.message || err)), 'danger');
		});
	},

	render: function(info) {
		var slots = info.slots || [];

		if (info.error || slots.length < 2)
			return E('div', { 'class': 'cbi-section' }, [
				E('h2', _('Firmware Slots')),
				E('p', { 'class': 'alert-message warning' },
					_('The firmware slots cannot be read: %s').format(info.error || _('no slot information')))
			]);

		var running = info.running;
		var other = slots.filter(function(s) { return s.slot !== running; })[0];
		var rows = slots.map(function(s) {
			var state = !s.valid ? _('No firmware')
				: s.slot === running ? _('Running')
				: s.slot === info.next ? _('Starts next')
				: '';

			return E('tr', { 'class': 'tr' }, [
				E('td', { 'class': 'td', 'data-title': _('Slot') }, String(s.slot)),
				E('td', { 'class': 'td', 'data-title': _('System') }, s.valid ? systemName(s.version) : '–'),
				E('td', { 'class': 'td', 'data-title': _('Version') }, s.valid ? s.version : '–'),
				E('td', { 'class': 'td', 'data-title': _('Serial') }, s.valid ? String(s.serial) : '–'),
				E('td', { 'class': 'td', 'data-title': _('State') }, state)
			]);
		});

		var action;

		if (running < 0)
			action = E('p', { 'class': 'alert-message warning' }, _('The running slot is unknown, so switching is not available.'));
		else if (!other || !other.valid)
			action = E('p', { 'class': 'alert-message warning' }, _('The other slot holds no firmware.'));
		else
			action = E('div', { 'class': 'cbi-page-actions' }, [
				E('button', {
					'class': 'btn cbi-button cbi-button-action important',
					'click': ui.createHandlerFn(this, 'handleSwitch', other)
				}, _('Switch to slot %d (%s) and reboot').format(other.slot, systemName(other.version)))
			]);

		return E('div', { 'class': 'cbi-map' }, [
			E('h2', _('Firmware Slots')),
			E('div', { 'class': 'cbi-map-descr' },
				_('The router has two firmware slots. Each holds a complete system, for example this one and the vendor firmware. The router starts the slot that was installed or selected last. Switching erases nothing.')),
			E('div', { 'class': 'cbi-section' }, [
				E('table', { 'class': 'table' }, [
					E('tr', { 'class': 'tr table-titles' }, [
						E('th', { 'class': 'th' }, _('Slot')),
						E('th', { 'class': 'th' }, _('System')),
						E('th', { 'class': 'th' }, _('Version')),
						E('th', { 'class': 'th' }, _('Serial')),
						E('th', { 'class': 'th' }, _('State'))
					])
				].concat(rows))
			]),
			action
		]);
	},

	handleSave: null,
	handleSaveApply: null,
	handleReset: null
});
