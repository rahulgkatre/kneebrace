const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <link rel='icon' href='data:,'>
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
  <title>ESP Web Server</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
</head>
<body>
  <script src='https://cdn.jsdelivr.net/npm/apexcharts'></script>
  <div class='topnav'>
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class='content'>
    <div class='card'>
      <div id='chart'></div>
    </div>
  </div>
  <script>
    
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
     
  </script>
</body>
</html>)rawliteral";
