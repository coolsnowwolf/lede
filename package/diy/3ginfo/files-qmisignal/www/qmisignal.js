var data = [];
var g;
var min = [], max = [];

var colors = [];
colors['signal'] = '#333';
colors['rssi'] = '#333';
colors['rsrp'] = '#333';
colors['ecio'] = '#333';
colors['rsrq'] = '#333';
colors['snr'] = '#333';

window.intervalId = setInterval(function() {
	$.getJSON( "/cgi-bin/qmisignal.sh", function(data1) {

		var idx=0
		$.each(data1, function(i, val) {
			idx++;

			unit='';
			if (!min[i]) {min[i]=999;max[i]=-999;}

			switch(i) {
			case "type":
				val=(val).toUpperCase();
				break;
			case "signal":
			case "rssi":
			case "rsrp":
				if (val < min[i]) {min[i] = val;}
				if (val > max[i]) {max[i] = val;}
				unit='dBm'
				break;
			case "ecio":
			case "rsrq":
			case "snr":
				if (val < min[i]) {min[i] = val;}
				if (val > max[i]) {max[i] = val;}
				unit='dB'
				break;
			}

			$("#key"+idx).text(''+i);
			$("#val"+idx).text(''+val);
			$('#val'+idx).css('color', colors[i]);
			$("#unit"+idx).text(''+unit);
			$('#unit'+idx).css('color', colors[i]);
			if (i != 'type')
			{
				$("#min"+idx).text(''+min[i]);
				$('#min'+idx).css('color', colors[i]);
				$("#max"+idx).text(''+max[i]);
				$('#max'+idx).css('color', colors[i]);
			}
		});

		data.push([new Date(),
			data1["signal"],
			data1["rssi"],
			data1["rsrp"],
			data1["ecio"],
			data1["rsrq"],
			data1["snr"]
		]);

		if (data.length > 300) {data.shift();}
		if (g==null) {
			g = new Dygraph(document.getElementById("div_g"), data, {
				drawPoints: false,
				showRoller: false,
				labels: ['', 'Signal', 'RSSI', 'RSRP', 'ECIO', 'RSRQ', 'SNR'],
				labelsSeparateLines: true,
				ylabel: 'RSSI, RSRP [dBm]',
				y2label: 'ECIO, RSRQ, SNR [dB]',
				series : {
					'ECIO': {axis: 'y2'},
					'RSRQ': {axis: 'y2'},
					'SNR': {axis: 'y2'}
				},
				axes: {
					y: {independentTicks: true},
					y2: {
						independentTicks: true,
						ticker: Dygraph.numericLinearTicks,
						drawGrid: true,
						gridLineColor: "#ff0000",
						gridLinePattern: [4,4]
					}
				}
			});
			var tcolors = g.getColors();
			colors['signal'] = tcolors[0];
			colors['rssi'] = tcolors[1];
			colors['rsrp'] = tcolors[2];
			colors['ecio'] = tcolors[3];
			colors['rsrq'] = tcolors[4];
			colors['snr'] = tcolors[5];
		} else {
			g.updateOptions({'file':data});
		}
	});
}, 3000);
