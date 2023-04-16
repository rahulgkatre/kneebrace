var websocket;
window.addEventListener('load', onLoad);
const maxPointsDisplayed = 30;
const freq = 50;
const period = 1000 / freq;
var cards = {};
var csvRecording = false;

var plotEnabled = true;
var plotSetInterval;
var textSetInterval;

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onLoad() {
    initWebSocket();
    document.getElementById('togglePlots').addEventListener('click', togglePlots);
    document.getElementById('toggleHaptics').addEventListener('click', toggleHaptics);

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
    if (message.type == 'plot' && plotEnabled) {
        // Plot the data in realtime
        for (let i = 0; i < message.series.length; i++) {
            var label = message.series[i].label;
            if (! (label in cards) ) {
                const div = document.createElement("div");
                div.id = label;
                div.className = "card";
                document.getElementById('cards').appendChild(div);
                cards[label] = {};
                cards[label]['x'] = 0;
                cards[label]['data'] = {};
                cards[label]["div"] = div;
                var data = [];
                Object.entries(message.series[i].data).forEach(([k, v]) => {
                    cards[label]['data'][k] = [v];
                    data.push({
                        label: k,
                        type: "line",
                        dataPoints: cards[label]['data'][k],
                    });
                });
                
                var options = {
                    interactivityEnabled: false,
                    animationEnabled: false,
                    legend: {
                        horizontalAlign: "center",
                        verticalAlign: "top",
                        fontSize: 15
                    },
                    axisX:{
                        gridThickness: 0,
                        tickLength: 0,
                        labelFormatter: function(e) {
                          return "";
                        }
                    },
                    title: { 
                        fontFamily: "Helvetica",
                        text: label 
                    },
                    data: data,
                };

                if ("bounds" in message.series[i]) {
                    options["axisY"] = message.series[i].bounds;
                }

                cards[label]['chart'] = new CanvasJS.Chart(div, options);
            } else {
                Object.entries(message.series[i].data).forEach(([k, v]) => {
                    cards[label]['data'][k].push({x: cards[label]['x'], y: v});
                    if (cards[label]['data'][k].length > maxPointsDisplayed) {
                        cards[label]['data'][k].shift();
                    }
                });
                cards[label]['chart'].render();
                cards[label]['x']++;
            }
        }
    } else if (message.type == 'text') {
        for (let i = 0; i < message.series.length; i++) {
            // Access the DOM using the keys and set the inner HTML to the value
            var label = message.series[i].label;
            if (! (label in cards) ) {
                cards[label] = {};
                const div = document.createElement("div");
                div.id = label;
                div.className = "card";
                document.getElementById('cards').appendChild(div);
                cards[label]["div"] = div;
            }
            cards[label]["div"].innerHTML = "<div class=\"center\"><h3>" + label + "</h3><br/><h2>" + message.series[i].data["steps"] + "</h2></div>";    
        }    
    } else if (message.type == 'connected') {
        // Wait for the connected message before scheduling requests
        plotSetInterval = setInterval(requestPlotData, period);
        textSetInterval = setInterval(requestTextData, period);
    }
}
function requestPlotData() {
    websocket.send('getNewPlotData');
}
function requestTextData() {
    websocket.send('getNewTextData');
}
function toggleHaptics() {
    websocket.send('toggleHaptics');
}
function togglePlots() {
    cards = {};
    document.getElementById('cards').textContent = "";
    if (plotEnabled) {
        clearInterval(plotSetInterval);
        plotEnabled = false;
    } else {
        plotSetInterval = setInterval(requestPlotData, period);
        plotEnabled = true;
    }
}
