var websocket;
window.addEventListener('load', initWebSocket);
let maxPointsDisplayed = 20;
var charts = {};
var counts = {};

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
            label = message.series[i].label;
            if (! (label in charts) ) {
                const div = document.createElement("div");
                div.id = label;
                document.getElementById('charts').appendChild(div);
                counts[label] = 1;
                var columns = Object.entries(message.series[i].data);
                charts[label] = c3.generate({
                    bindto: document.getElementById(label),
                    data: {
                        columns: columns
                    },
                    axis: {
                        x: {
                            tick: {
                                format: function(x) { return ''; }
                            }
                        }
                    }
                });
            } else {              
                // TODO: Fix this
                counts[label]++;
                var length;
                if (counts[label] > maxPointsDisplayed) {
                    length = 1;
                } else {
                    length = 0;                    
                }

                var columns = Object.entries(message.series[i].data);
                charts[label].flow({
                    columns: columns,
                    duration: 0,
                    length: length,
                });
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
