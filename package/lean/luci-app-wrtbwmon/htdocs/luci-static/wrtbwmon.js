var wrt = {
    // variables for auto-update, interval is in seconds
    scheduleTimeout: undefined,
    updateTimeout: undefined,
    isScheduled: true,
    interval: 5,
    // option on whether to show per host sub-totals
    perHostTotals: false,
    // variables for sorting
    sortData: {
        column: 7,
        elId: 'thTotal',
        dir: 'desc',
        cache: {}
    }
};

(function () {
    var oldDate, oldValues = [];

    // find base path
    var re = /(.*?admin\/nlbw\/[^/]+)/;
    var basePath = window.location.pathname.match(re)[1];

    //----------------------
    // HELPER FUNCTIONS
    //----------------------

    /**
     * Human readable text for size
     * @param size
     * @returns {string}
     */
    function getSize(size) {
        var prefix = [' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z'];
        var precision, base = 1000, pos = 0;
        while (size > base) {
            size /= base;
            pos++;
        }
        if (pos > 2) precision = 1000; else precision = 1;
        return (Math.round(size * precision) / precision) + ' ' + prefix[pos] + 'B';
    }

    /**
     * Human readable text for date
     * @param date
     * @returns {string}
     */
    function dateToString(date) {
        return date.toString().substring(0, 24);
    }

    /**
     * Gets the string representation of the date received from BE
     * @param value
     * @returns {*}
     */
    function getDateString(value) {
        var tmp = value.split('_'),
            str = tmp[0].split('-').reverse().join('-') + 'T' + tmp[1];
        return dateToString(new Date(str));
    }

    /**
     * Create a `tr` element with content
     * @param content
     * @returns {string}
     */
    function createTR(content) {
        var res = '<tr';
        res += ' class="tr">';
        res += content;
        res += '</tr>';

        return res;
    }

    /**
     * Create a `th` element with content and options
     * @param content
     * @param opts
     * @returns {string}
     */
    function createTH(content, opts) {
        opts = opts || {};
        var res = '<th';
        if (opts.right) {
            res += ' align="right"';
        }
        if (opts.title) {
            res += ' title="' + opts.title + '"';
        }
        if (opts.id) {
            res += ' id="' + opts.id + '"';
        }
        res += ' class="th">';
        res += content;
        res += '</th>';
        return res;
    }

    /**
     * Create a `td` element with content and options
     * @param content
     * @param opts
     * @returns {string}
     */
    function createTD(content, opts) {
        opts = opts || {};
        var res = '<td';
        if (opts.right) {
            res += ' align="right"';
        }
        if (opts.title) {
            res += ' title="' + opts.title + '"';
        }
        res += ' class="td">';
        res += content;
        res += '</td>';
        return res;
    }

    /**
     * Returns true if obj is instance of Array
     * @param obj
     * @returns {boolean}
     */
    function isArray(obj) {
        return obj instanceof Array;
    }

    //----------------------
    // END HELPER FUNCTIONS
    //----------------------

    /**
     * Handle the error that happened during the call to the BE
     */
    function handleError() {
        // TODO handle errors
        // var message = 'Something went wrong...';
    }

    /**
     * Handle the new `values` that were received from the BE
     * @param values
     * @returns {string}
     */
    function handleValues(values) {
        if (!isArray(values)) return '';

        // find data and totals
        var res = parseValues(values);
        var data = res[0];
        var totals = res[1];

        // aggregate (sub-total) by hostname (or MAC address) after the global totals are computed, before sort and display
        aggregateHostTotals(data);

        // store them in cache for quicker re-rendering
        wrt.sortData.cache.data = data;
        wrt.sortData.cache.totals = totals;

        renderTableData(data, totals);
    }

    /**
     * Renders the table body
     * @param data
     * @param totals
     */
    function renderTableData(data, totals) {
        // sort data
        data.sort(sortingFunction);

        // display data
        document.getElementById('tableBody').innerHTML = getDisplayData(data, totals);

        // set sorting arrows
        var el = document.getElementById(wrt.sortData.elId);
        if (el) {
            el.innerHTML = el.innerHTML + (wrt.sortData.dir === 'desc' ? '&#x25BC' : '&#x25B2');
        }

        // register table events
        registerTableEventHandlers();
    }

    /**
     * Parses the values and returns a data array, where each element in the data array is an array with two elements,
     * and a totals array, that holds aggregated values for each column.
     * The first element of each row in the data array, is the HTML output of the row as a `tr` element
     * and the second is the actual data:
     *  [ result, data ]
     * @param values The `values` array
     * @returns {Array}
     */
    function parseValues(values) {
        var data = [], totals = [0, 0, 0, 0, 0];
        for (var i = 0; i < values.length; i++) {
            var d = parseValueRow(values[i]);
            if (d[1]) {
                data.push(d);
                // get totals
                for (var j = 0; j < totals.length; j++) {
                    totals[j] += d[1][3 + j];
                }
            }
        }

        return [data, totals];
    }

    /**
     * Parse each row in the `values` array and return an array with two elements.
     * The first element is the HTML output of the row as a `tr` element and the second is the actual data
     *    [ result, data ]
     * @param data A row from the `values` array
     * @returns {[ string, [] ]}
     */
    function parseValueRow(data) {
        // check if data is array
        if (!isArray(data)) return [''];

        // find old data
        var oldData;
        for (var i = 0; i < oldValues.length; i++) {
            var cur = oldValues[i];
            // compare mac addresses and ip addresses
            if (oldValues[i][1] === data[1] && oldValues[i][2] === data[2]) {
                oldData = cur;
                break;
            }
        }

        // find download and upload speeds
        var dlSpeed = 0, upSpeed = 0;
        if (oldData) {
            var now = new Date(),
                seconds = (now - oldDate) / 1000;
            dlSpeed = (data[3] - oldData[3]) / seconds;
            upSpeed = (data[4] - oldData[4]) / seconds;
        }

        // create rowData
        var rowData = [];
        for (var j = 0; j < data.length; j++) {
            rowData.push(data[j]);
            if (j === 2) {
                rowData.push(dlSpeed, upSpeed);
            }
        }

        // create displayData
        var displayData = [
            createTD(data[0] + '<br />' + data[2], {title: data[1]}),
            createTD(getSize(dlSpeed) + '/s', {right: true}),
            createTD(getSize(upSpeed) + '/s', {right: true}),
            createTD(getSize(data[3]), {right: true}),
            createTD(getSize(data[4]), {right: true}),
            createTD(getSize(data[5]), {right: true}),
            createTD(getDateString(data[6])),
            createTD(getDateString(data[7]))
        ];

        // display row data
        var result = '';
        for (var k = 0; k < displayData.length; k++) {
            result += displayData[k];
        }
        result = createTR(result);
        return [result, rowData];
    }

    /**
     * Creates the HTML output based on the `data` and `totals` inputs
     * @param data
     * @param totals
     * @returns {string} HTML output
     */
    function getDisplayData(data, totals) {
        var result =
            createTH('客户端', {id: 'thClient'}) +
            createTH('下载带宽', {id: 'thDownload'}) +
            createTH('上传带宽', {id: 'thUpload'}) +
            createTH('总下载流量', {id: 'thTotalDown'}) +
            createTH('总上传流量', {id: 'thTotalUp'}) +
            createTH('流量合计', {id: 'thTotal'}) +
            createTH('首次上线时间', {id: 'thFirstSeen'}) +
            createTH('最后上线时间', {id: 'thLastSeen'});
        result = createTR(result);
        for (var k = 0; k < data.length; k++) {
            result += data[k][0];
        }
        var totalsRow = createTH('总计');
        for (var m = 0; m < totals.length; m++) {
            var t = totals[m];
            totalsRow += createTD(getSize(t) + (m < 2 ? '/s' : ''), {right: true});
        }
        result += createTR(totalsRow);
        return result;
    }

    /**
     * Calculates per host sub-totals and adds them in the data input
     * @param data The data input
     */
    function aggregateHostTotals(data) {
        if (!wrt.perHostTotals) return;

        var curHost = 0, insertAt = 1;
        while (curHost < data.length && insertAt < data.length) {
            // grab the current hostname/mac, and walk the data looking for rows with the same host/mac
            var hostName = data[curHost][1][0].toLowerCase();
            for (var k = curHost + 1; k < data.length; k++) {
                if (data[k][1][0].toLowerCase() === hostName) {
                    // this is another row for the same host, group it with any other rows for this host
                    data.splice(insertAt, 0, data.splice(k, 1)[0]);
                    insertAt++;
                }
            }

            // if we found more than one row for the host, add a subtotal row
            if (insertAt > curHost + 1) {
                var hostTotals = [data[curHost][1][0], '', '', 0, 0, 0, 0, 0];
                for (var i = curHost; i < insertAt && i < data.length; i++) {
                    for (var j = 3; j < hostTotals.length; j++) {
                        hostTotals[j] += data[i][1][j];
                    }
                }
                var hostTotalRow = createTH(data[curHost][1][0] + '<br/> (host total)', {title: data[curHost][1][1]});
                for (var m = 3; m < hostTotals.length; m++) {
                    var t = hostTotals[m];
                    hostTotalRow += createTD(getSize(t) + (m < 5 ? '/s' : ''), {right: true});
                }
                hostTotalRow = createTR(hostTotalRow);
                data.splice(insertAt, 0, [hostTotalRow, hostTotals]);
            }
            curHost = insertAt;
            insertAt = curHost + 1;
        }
    }

    /**
     * Sorting function used to sort the `data`. Uses the global sort settings
     * @param x first item to compare
     * @param y second item to compare
     * @returns {number} 1 for desc, -1 for asc, 0 for equal
     */
    function sortingFunction(x, y) {
        // get data from global variable
        var sortColumn = wrt.sortData.column, sortDirection = wrt.sortData.dir;
        var a = x[1][sortColumn];
        var b = y[1][sortColumn];
        if (a === b) {
            return 0;
        } else if (sortDirection === 'desc') {
            return a < b ? 1 : -1;
        } else {
            return a > b ? 1 : -1;
        }
    }

    /**
     * Sets the relevant global sort variables and re-renders the table to apply the new sorting
     * @param elId
     * @param column
     */
    function setSortColumn(elId, column) {
        if (column === wrt.sortData.column) {
            // same column clicked, switch direction
            wrt.sortData.dir = wrt.sortData.dir === 'desc' ? 'asc' : 'desc';
        } else {
            // change sort column
            wrt.sortData.column = column;
            // reset sort direction
            wrt.sortData.dir = 'desc';
        }
        wrt.sortData.elId = elId;

        // render table data from cache
        renderTableData(wrt.sortData.cache.data, wrt.sortData.cache.totals);
    }

    /**
     * Registers the table events handlers for sorting when clicking the column headers
     */
    function registerTableEventHandlers() {
        // note these ordinals are into the data array, not the table output
        document.getElementById('thClient').addEventListener('click', function () {
            setSortColumn(this.id, 0); // hostname
        });
        document.getElementById('thDownload').addEventListener('click', function () {
            setSortColumn(this.id, 3); // dl speed
        });
        document.getElementById('thUpload').addEventListener('click', function () {
            setSortColumn(this.id, 4); // ul speed
        });
        document.getElementById('thTotalDown').addEventListener('click', function () {
            setSortColumn(this.id, 5); // total down
        });
        document.getElementById('thTotalUp').addEventListener('click', function () {
            setSortColumn(this.id, 6); // total up
        });
        document.getElementById('thTotal').addEventListener('click', function () {
            setSortColumn(this.id, 7); // total
        });
    }

    /**
     * Fetches and handles the updated `values` from the BE
     * @param once If set to true, it re-schedules itself for execution based on selected interval
     */
    function receiveData(once) {
        var ajax = new XMLHttpRequest();
        ajax.onreadystatechange = function () {
            // noinspection EqualityComparisonWithCoercionJS
            if (this.readyState == 4 && this.status == 200) {
                var re = /(var values = new Array[^;]*;)/,
                    match = ajax.responseText.match(re);
                if (!match) {
                    handleError();
                } else {
                    // evaluate values
                    eval(match[1]);
                    //noinspection JSUnresolvedVariable
                    var v = values;
                    if (!v) {
                        handleError();
                    } else {
                        handleValues(v);
                        // set old values
                        oldValues = v;
                        // set old date
                        oldDate = new Date();
                        document.getElementById('updated').innerHTML = '数据更新时间 ' + dateToString(oldDate);
                    }
                }
                var int = wrt.interval;
                if (!once && int > 0) reschedule(int);
            }
        };
        ajax.open('GET', basePath + '/usage_data', true);
        ajax.send();
    }

    /**
     * Registers DOM event listeners for user interaction
     */
    function addEventListeners() {
        document.getElementById('intervalSelect').addEventListener('change', function () {
            var int = wrt.interval = this.value;
            if (int > 0) {
                // it is not scheduled, schedule it
                if (!wrt.isScheduled) {
                    reschedule(int);
                }
            } else {
                // stop the scheduling
                stopSchedule();
            }
        });

        document.getElementById('resetDatabase').addEventListener('click', function () {
            if (confirm('This will delete the database file. Are you sure?')) {
                var ajax = new XMLHttpRequest();
                ajax.onreadystatechange = function () {
                    // noinspection EqualityComparisonWithCoercionJS
                    if (this.readyState == 4 && this.status == 204) {
                        location.reload();
                    }
                };
                ajax.open('GET', basePath + '/usage_reset', true);
                ajax.send();
            }
        });

        document.getElementById('perHostTotals').addEventListener('change', function () {
            wrt.perHostTotals = !wrt.perHostTotals;
        });
    }

    //----------------------
    // AUTO-UPDATE
    //----------------------

    /**
     * Stop auto-update schedule
     */
    function stopSchedule() {
        window.clearTimeout(wrt.scheduleTimeout);
        window.clearTimeout(wrt.updateTimeout);
        setUpdateMessage('');
        wrt.isScheduled = false;
    }

    /**
     * Start auto-update schedule
     * @param seconds
     */
    function reschedule(seconds) {
        wrt.isScheduled = true;
        seconds = seconds || 60;
        updateSeconds(seconds);
        wrt.scheduleTimeout = window.setTimeout(receiveData, seconds * 1000);
    }

    /**
     * Sets the text of the `#updating` element
     * @param msg
     */
    function setUpdateMessage(msg) {
        document.getElementById('updating').innerHTML = msg;
    }

    /**
     * Updates the 'Updating in X seconds' message
     * @param start
     */
    function updateSeconds(start) {
        setUpdateMessage('倒数 <b><font color="blue">' + start + '</font></b> 秒后刷新.');
        if (start > 0) {
            wrt.updateTimeout = window.setTimeout(function () {
                updateSeconds(start - 1);
            }, 1000);
        }
    }

    //----------------------
    // END AUTO-UPDATE
    //----------------------

    /**
     * Check for dependency, and if all is well, run callback
     * @param cb Callback function
     */
    function checkForDependency(cb) {
        var ajax = new XMLHttpRequest();
        ajax.onreadystatechange = function () {
            // noinspection EqualityComparisonWithCoercionJS
            if (this.readyState == 4 && this.status == 200) {
                // noinspection EqualityComparisonWithCoercionJS
                if (ajax.responseText == "1") {
                    cb();
                } else {
                    alert("wrtbwmon is not installed!");
                }
            }
        };
        ajax.open('GET', basePath + '/check_dependency', true);
        ajax.send();
    }

    checkForDependency(function () {
        // register events
        addEventListeners();
        // Main entry point
        receiveData();
    });

})();
