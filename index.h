const static char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<body>
  <div id="encoder">
    <h1>The ESP8266 NodeMCU + Encoder</h1>
  </div>
  <div>
    Encoder count : <span id="EncoderValues">0</span><br>
  </div>
  <div>
    <form action="/action_page">
      Hold on count: <input type="text" name="holdOnCount">
      <input type="submit" value="Submit">
    </form><br>
  </div>
  <div>
    <form action="/action_page">
      Hold off count: <input type="text" name="holdOffCount">
      <input type="submit" value="Submit">
    </form><br>
  </div>
  <div>
    <button onclick="voluemUpJS()">Volume Up</button><br>
  </div>
  <br>
  <div>
    <button onclick="voluemDownJS()">Volume Down</button><br>
  </div>
  <br>
  <div>
    <input type="text" id="volumeAmount" name="volumeAmount" value="1">
    <input type="checkbox" id="upOrDown" name="upOrDown"checked> <label for="upOrDown">Volume Up (or Down)</label>
    <button onclick="voluemSetJS()">Volume Set</button><br>
  <br>
  </div>
  <script>

    setInterval(function () {
      getEncoder();
    }, 10000); //1000mSeconds update rate
    
    function getEncoder() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("EncoderValues").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "getEncoder", true);
      xhttp.send();
    }

    function voluemUpJS() {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "volume_up", false);
      xhttp.send();
    }

    function voluemDownJS(){
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "volume_down", false);
      xhttp.send();  
    }

    function voluemSetJS(){
      var xhttp = new XMLHttpRequest();
      var volumeAmount = document.getElementById("volumeAmount").value;
      var upOrDown = document.getElementById("upOrDown").checked;
      console.log("volumeAmount = " + volumeAmount + ", upOrDown = " + upOrDown);
      var params = "volumestep=" + volumeAmount + "&upOrDown=" + upOrDown;
      // var params = JSON.stringify({ volumestep:  volumeAmount});
      xhttp.open("POST", "volume_set", false);
      xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
      xhttp.send(params);
    }

  </script>

</body>

</html>
)=====";