const char MAIN_page[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
<head>
  <title>Homepage</title>
</head>
<style>
  @import url('Roboto-Regular.ttf');
  body{
    margin: 0px;
    font-family: 'Roboto', sans-serif;
  }
  .required{
    color: red;
  }
  #topbar{
    background-color: #2c2f33;
    width: 100vw;
    padding-top: 20px;
    padding-bottom: 20px;
    text-align: center;
    margin: 0px;
    color: white;
    font-size: 25px;
  }
  #secondbar{
    background-color: #82B590;
    width: 100vw;
    padding-top: 20px;
    padding-bottom: 20px;
    text-align: center;
    color:white;
    font-size: 20px;
  }
  .field-name{
    font-size: 20px;
    text-align: left;
    width: 100vw; 
  }
  #main-container input, select, textarea{
    padding: 8px;
    font-size: 20px;
    width: 70vw;
    margin-bottom: 10px;
  }
  #main-container{
    width: 100vw;
    text-align: center;
    margin-top: 15px;
  }
  #submit{
    width: 70vw;
    padding: 10px;
    font-size: 20px;
    font-weight: bold;
    margin-top: 20px;
    background-color: #82B590;
    color: white;
    border: none;
    -webkit-transition: all 0.3s ease-in-out;
    -moz-transition: all 0.3s ease-in-out;
    -o-transition: all 0.3s ease-in-out;
  }

  #submit:hover{
    background-color: #4eb36a;
  }
</style>
<body onload="getLocation()">
  <div id="topbar">
    Project MLAN
  </div>
  <div id="secondbar">
    <span class="required">*</span>Required Fields
  </div>
  <form method="get" action="">
    <div id="main-container">
      <span class="field-name">Name:</span><span class="required">*</span><br>
      <input type="text" name="name" id="name" required maxlength="15"><br>

      <span class="field-name">Phone Number:</span><span class="required">*</span><br>
      <input type="text" name="hpno" id="hp" required maxlength="11" onkeypress="return isNumberKey(event); limit(this)"><br>

      <span class="field-name">Latitude:</span><br>
      <input type="text" name="lati" id="lati" readonly required style="color: grey" value="3.054884329652521"><br>

      <span class="field-name">Longitude:</span><br>
      <input type="text" name="long" id="long" readonly required style="color: grey" value="101.7003967423641"><br>

      <span class="field-name">Message:</span><span class="required">*</span><br>
      <textarea maxlength="200" name="message"></textarea><br>

      <span class="field-name">Priority Level:</span><br>
      <select name="priority" style="width: 75vw;">
        <option value="null" disabled>Less Urgent</option>
        <option value="1">Level 1</option>
        <option value="2">Level 2</option>
        <option value="3">Level 3</option>
        <option value="null" disabled>Most Urgent</option>
      </select><br>
      <input type="submit" name="submit" id="submit" value="Submit">
    </div>
  </form>
<script>
var x = document.getElementById("demo");

function getLocation() {
  if (navigator.geolocation) {
    navigator.geolocation.getCurrentPosition(showPosition);
  } 
  else{ 
    x.innerHTML = "Geolocation is not supported by this browser.";
  }
}

function showPosition(position) {
  document.getElementById('lati').value=position.coords.latitude;
  document.getElementById('long').value=position.coords.longitude;
  console.log("Sucess");
}

function isNumberKey(evt){
    var charCode = (evt.which) ? evt.which : event.keyCode
    if (charCode > 31 && (charCode < 48 || charCode > 57))
      return false;

    return true;
  }
</script>
</html>

)=====";
