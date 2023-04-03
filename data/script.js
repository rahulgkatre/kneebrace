var websocket;
window.addEventListener('load', initWebSocket);
let maxPointsDisplayed = 30;
var data = {};
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
        for (let i = 0; i < message.labels.length; i++) {
            label = message.labels[i];
            if (! (label in charts) ) {
                const div = document.createElement("div");
                div.id = label;
                document.getElementById('charts').appendChild(div);
                data[label] = [];
                charts[label] = c3.generate({
                    bindto: document.getElementById(label),
                    data: {
                        columns: [[label]]
                    }
                });
                console.log('Created chart for ' + label);
            }

            data[label].push(message.data[i]);
            if (data[label].length > maxPointsDisplayed) {
                data[label].shift();
            }
            
            charts[label].flow({
                columns: [[label].concat(data[label])],
                duration: 100
            }); 
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
