var websocket;
window.addEventListener('load', initWebSocket);
const maxPointsDisplayed = 40;
const plotFrequency = 100;
const plotPeriod = 1000 / plotFrequency;
var charts = {};

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    const message = JSON.parse(event.data);
    if (message.type == 'plot') {
        // Plot the data in realtime
        for (let i = 0; i < message.series.length; i++) {
            var label = message.series[i].label;
            if (! (label in charts) ) {
                const div = document.createElement("div");
                div.id = label;
                document.getElementById('charts').appendChild(div);
                var series = [];
                Object.entries(message.series[i].data).forEach(([k, v]) => {
                    series.push({
                        name: k,
                        showInLegend: true,
                        data: [],
                    });
                });
                charts[label] = new Highcharts.Chart({
                    chart: { renderTo: div },
                    title: { text: label },
                    series: series,
                    plotOptions: {
                        line: {
                            animation: false,
                            dataLabels: { enabled: false },
                            marker: { enabled: false }
                        }
                    },
                    xAxis: {
                        type: 'datetime',
                        dateTimeLabelFormats: { second: '%H:%M:%S'}
                    },
                    credits: { enabled: false }
                });
            }
            for (let j = 0; j < charts[label].series.length; j++) {
                const key = charts[label].series[j].name;
                if (charts[label].series[j].data.length > maxPointsDisplayed) {
                    charts[label].series[j].addPoint([(new Date()).getTime(), message.series[i].data[key]], true, true);
                } else {
                    charts[label].series[j].addPoint([(new Date()).getTime(), message.series[i].data[key]], true, false);
                }
            }
        }
    } else if (message.type == 'text') {
        // Access the DOM using the keys and set the inner HTML to the value
    } else if (message.type == 'connected') {
        // Wait for the connected message before scheduling requests
        setInterval(requestPlotData, plotPeriod);
        setInterval(requestTextData, 600);
    }
}
function requestPlotData() {
    websocket.send('getNewPlotData');
}
function requestTextData() {
    websocket.send('getNewTextData');
}
