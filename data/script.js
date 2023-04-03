var websocket;
window.addEventListener('load', initWebSocket);
function onOpen(event) {
    console.log('Connection opened');
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    const message = JSON.parse(event.data);     
    if (message.type == 'plot' ) {
    // Plot the data in realtime using Apexcharts
    console.log(message);
    } else if (message.type == 'text') {
    // Access the DOM using the keys and set the inner HTML to the value
    console.log(message);
    } else if (message.type == 'connected') {
    setInterval(requestPlottingData, 100);
    setInterval(requestTextData, 600);
    }
}
function requestPlottingData(){
    websocket.send('getNewPlottingData');
}
function requestTextData(){
    websocket.send('getNewTextData');
}
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}