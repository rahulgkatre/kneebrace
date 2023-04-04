var websocket;
window.addEventListener('load', initWebSocket);
const maxPointsDisplayed = 30;
const d = new Date();
var data = {};
var charts = {};
const labels = [...Array(maxPointsDisplayed).keys()];

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
                const canvas = document.createElement("canvas");
                canvas.id = label;
                document.getElementById('charts').appendChild(canvas);
                var datasets = [];
                Object.entries(message.series[i].data).forEach(([k, v]) => {
                    datasets.push({
                        label: k,
                        data: [v]
                    });
                });
                
                charts[label] = new Chart(canvas, {
                    type: 'line',
                    data: {
                        labels: labels,
                        datasets: datasets
                    },
                    options: {
                        reponsive: true,
                        plugins: {
                            legend: {
                                position: 'bottom',
                            },
                            title: {
                                display: true,
                                text: label
                            },
                        },
                    }
                });
            } else {
                for (let j = 0; j < charts[label].data.datasets.length; j++) {
                    const key = charts[label].data.datasets[j].label;
                    const dest = charts[label].data.datasets[j].data;
                    dest.push(message.series[i].data[key]);
                    if (dest.length > maxPointsDisplayed) {
                        labels.push(labels[maxPointsDisplayed-1]+1);
                        labels.shift();
                        dest.shift();
                    }
                }
                
                charts[label].update();
            }  
        }
    } else if (message.type == 'text') {
        // Access the DOM using the keys and set the inner HTML to the value
    } else if (message.type == 'connected') {
        // Wait for the connected message before scheduling requests
        setInterval(requestPlotData, 100);
        setInterval(requestTextData, 600);
    }
}
function requestPlotData() {
    websocket.send('getNewPlotData');
}
function requestTextData() {
    websocket.send('getNewTextData');
}
