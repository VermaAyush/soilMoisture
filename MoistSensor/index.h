

const char MAIN_page[] PROGMEM = R"=====(
<html>
<head>
  <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<title>moisture Sensor</title>

<style>
.slider {
  -webkit-appearance: none;  /* Override default CSS styles */
  appearance: none;
  width: 100%; /* Full-width */
  height: 25px; /* Specified height */
  background: #d3d3d3; /* Grey background */
  outline: none; /* Remove outline */
  opacity: 0.7; /* Set transparency (for mouse-over effects on hover) */
  -webkit-transition: .2s; /* 0.2 seconds transition on hover */
  transition: opacity .2s;
}
/* Mouse-over effects */
.slider:hover {
  opacity: 1; /* Fully shown on mouse-over */
}
.gauge {
  width: 100%;
  max-width: 250px;
  margin: 100px auto;
  font-family: "Roboto", sans-serif;
  font-size: 32px;
  text-align: center;
  color: #004033;
}

.gauge__body {
  width: 100%;
  height: 0;
  padding-bottom: 50%;
  background: #b4c0be;
  position: relative;
  border-top-left-radius: 100% 200%;
  border-top-right-radius: 100% 200%;
  overflow: hidden;
}

.gauge__fill {
  position: absolute;
  top: 100%;
  left: 0;
  width: inherit;
  height: 100%;
  background: #009578;
  transform-origin: center top;
  transform: rotate(0.25turn);
  transition: transform 0.2s ease-out;
}

.gauge__cover {
  width: 75%;
  height: 150%;
  background: #ffffff;
  border-radius: 50%;
  position: absolute;
  top: 25%;
  left: 50%;
  transform: translateX(-50%);

  /* Text */
  display: flex;
  align-items: center;
  justify-content: center;
  padding-bottom: 25%;
  box-sizing: border-box;
   /* Text */
}
.dot{
    width: 25px;
    height: 25px;
    background-color: rgb(234, 233, 233);
    border-radius: 50%;
    display: inline-block;
  }
    body {font-family: Arial, Helvetica, sans-serif;}

/* Full-width input fields */
input[type=text], input[type=password] {
  width: 100%;
  padding: 12px 20px;
  margin: 8px 0;
  display: inline-block;
  border: 1px solid #ccc;
  box-sizing: border-box;
}

/* Set a style for all buttons */
button {
  background-color: #04AA6D;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  cursor: pointer;
  width: 100%;
}

button:hover {
  opacity: 0.8;
}

/* Extra styles for the cancel button */
.cancelbtn {
  width: auto;
  padding: 10px 18px;
  background-color: #f44336;
}





.container {
  padding: 16px;
}

span.psw {
  float: right;
  padding-top: 16px;
}

/* The Modal (background) */
.modal {
  display: none; /* Hidden by default */
  position: fixed; /* Stay in place */
  z-index: 1; /* Sit on top */
  left: 0;
  top: 0;
  width: 100%; /* Full width */
  height: 100%; /* Full height */
  overflow: auto; /* Enable scroll if needed */
  background-color: rgb(0,0,0); /* Fallback color */
  background-color: rgba(0,0,0,0.4); /* Black w/ opacity */
  padding-top: 60px;
}

/* Modal Content/Box */
.modal-content {
  background-color: #fefefe;
  margin: 5% auto 15% auto; /* 5% from the top, 15% from the bottom and centered */
  border: 1px solid #888;
  width: 80%; /* Could be more or less, depending on screen size */
}

/* The Close Button (x) */
.close {
  position: absolute;
  right: 25px;
  top: 0;
  color: #000;
  font-size: 35px;
  font-weight: bold;
}

.close:hover,
.close:focus {
  color: red;
  cursor: pointer;
}

/* Add Zoom Animation */
.animate {
  -webkit-animation: animatezoom 0.6s;
  animation: animatezoom 0.6s
}

@-webkit-keyframes animatezoom {
  from {-webkit-transform: scale(0)} 
  to {-webkit-transform: scale(1)}
}
  
@keyframes animatezoom {
  from {transform: scale(0)} 
  to {transform: scale(1)}
}

/* Change styles for span and cancel button on extra small screens */
@media screen and (max-width: 300px) {
  span.psw {
     display: block;
     float: none;
  }
  .cancelbtn {
     width: 100%;
  }
}
</style>
<body>
 <center><span class="dot"></span></center>
<center>
    <div class="gauge">
        <div class="gauge__body">
          <div class="gauge__fill"></div>
          <div class="gauge__cover"></div>
        </div>
      </div>
</center>
<center><p>pump is  <span id="motor"></span></p></center>
<center>     
<div class="slidecontainer">
  <p><span id="curValue"></span></p>
  <input type="range" min="0" max="100" value="50" class="slider" id="myRange">
  <p>pump stop if moisture level is greater than : <span id="demo"></span> %</p>
</div>
</center>
<center>
<button onclick="document.getElementById('id01').style.display='block'" style="width:auto;">Connect to router</button>

<div id="id01" class="modal">
  
  <form class="modal-content animate" action="/action_page.php" method="post">


    <div class="container">
      <label for="uname"><b>ssid</b></label>
      <input id="ssid" type="text" placeholder="SSID" name="uname" required>

      <label for="psw"><b>Password</b></label>
      <input  id="password" type="password" placeholder="PASSWORD" name="psw" required>
        
      <button type="button" onclick="sendCrendital()">connect</button>
     
    </div>

    <div class="container" style="background-color:#f1f1f1">
      <button type="button" onclick="document.getElementById('id01').style.display='none'" class="cancelbtn">Cancel</button>
    </div>
  </form>
</div>
</center>
<script>
  window.addEventListener('load',onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket =new WebSocket('ws://'+window.location.hostname+':81/');
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }
  function onOpen(event) {
    console.log('Connection opened');
   // send();
  }
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 5000);
}

function onLoad(event){
  console.log("trying to load initWebsockets");
  setInterval("changecolor()",1600);
    initWebSocket();
} 
  var slider = document.getElementById("myRange");
  var output = document.getElementById("demo");
  var pump = document.getElementById("motor");
  var currentValue = document.getElementById("curValue");
  const status = document.querySelector(".dot");

  var online = false;
  
  currentValue.innerHTML = slider.value;
  
  function changecolor(){
    status.style.backgroundColor = 'grey';
  }
  const gaugeElement = document.querySelector(".gauge");
  function setGaugeValue(gauge, value) {
    if (value < 0) {
      value = 0;
    }
    if(value >1){
      value = 1;
    }
    gauge.querySelector(".gauge__cover").textContent = `${Math.round(
      value * 100
    )}%`;
    gauge.querySelector(".gauge__fill").style.transform = `rotate(${
      value / 2
    }turn)`;
    
  }
  slider.oninput = function() {
    currentValue.innerHTML = this.value;
  }
  slider.onchange = function() {
    currentValue.innerHTML = this.value;
    var data = new ArrayBuffer(1);
    var view = new Uint8Array(data);
    view[0]= this.value;
    console.log(data)
    websocket.send(data);
  }
  function onMessage(event){
   status.style.backgroundColor = 'green';
  console.log(event.data);
  var JSONobj = JSON.parse(event.data);
  var pot = parseFloat(JSONobj.POT)/100;
  var threshold = parseInt(JSONobj.THRESHOLD);
  var pumpStat = parseInt(JSONobj.PUMP);
  output.innerHTML = threshold;
  if(pumpStat === 0){
  pump.innerHTML = "ON";
  }else if(pumpStat === 1){pump.innerHTML = "OFF";}
  console.log(pumpStat);
  console.log(threshold);
  console.log(pot);
  setGaugeValue(gaugeElement,pot);
}
  var modal = document.getElementById('id01');

// When the user clicks anywhere outside of the modal, close it
window.onclick = function(event) {
    if (event.target == modal) {
        modal.style.display = "none";
    }
}
function sendCrendital(){
      var ssid = document.getElementById("ssid").value;
      var password = document.getElementById("password").value;
      var data = {ssid:ssid,password:password};
      websocket.send(JSON.stringify(data));
      console.log(data);
    }
  </script>
</body>
</html>
)=====";
