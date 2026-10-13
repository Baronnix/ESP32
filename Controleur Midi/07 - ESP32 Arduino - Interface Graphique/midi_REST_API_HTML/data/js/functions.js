//##################################################
// Funtions generiques et specifiques au Wifi
//##################################################

function includeHTML() {
  var z, i, elmnt, file, xhttp;
  /*loop through a collection of all HTML elements:*/
  z = document.getElementsByTagName("*");
  for (i = 0; i < z.length; i++) {
    elmnt = z[i];
    /*search for elements with a certain atrribute:*/
    file = elmnt.getAttribute("w3-include-html");
    if (file) {
      /*make an HTTP request using the attribute value as the file name:*/
      xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {elmnt.innerHTML = this.responseText;}
          if (this.status == 404) {elmnt.innerHTML = "Page not found.";}
          /*remove the attribute, and call this function once more:*/
          elmnt.removeAttribute("w3-include-html");
          includeHTML();
        }
      }      
      xhttp.open("GET", file, true);
      xhttp.send();
      /*exit the function:*/
      return;
    }
  }
};

function hideMsg() {
	document.getElementById('successMsg').hidden = true;
	document.getElementById('warningMsg').hidden = true;
	document.getElementById('errorMsg').hidden = true;
	document.getElementById('infoMsg').hidden = true;
}

function displayMsg(msg, msgType) {
	hideMsg();
	if (msgType == 'success' || msgType == 'warning' || msgType == 'error' || msgType == 'info') {
		document.getElementById(msgType + 'Msg').hidden = false;
		document.getElementById(msgType + 'Msg').innerHTML = msg;
	}
}	

function populateWifiAvailable() {
	var result, xhttp;
	xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
			  result = this.responseText;
			  buildOptions(result);
		  }
          else {result = "ERROR";}
        }
		else { result = "ERROR";}
      }      
      xhttp.open("GET", "/getAvailableWifi", true);
      xhttp.send();
	  
}

function buildOptions(wifiList) {
	  var json = JSON.parse(wifiList);
	  //console.log(json);
	  var selElt = document.getElementById("wifiName");
	  for (i = selElt.options.length-1; i >= 0; i--) {
	    selElt.options[i] = null;
	  }
	  for (i = 0; i < json.length; i++) { 
		var opt = document.createElement('option');
		opt.appendChild( document.createTextNode(json[i].ssid) );
		opt.value = json[i].ssid; 
		selElt.appendChild(opt);
	  }
}

function registerToWifi(){
	document.getElementById('registerWifi').disabled = true;
	var wifiNameValue = document.getElementById('wifiName').value;
	var wifiPasswordValue = document.getElementById('wifiPassword').value;
	var result;
	var xmlHttp = null;
	var theUrl = "/setWifi?ssid=" + wifiNameValue + "&password=" + wifiPasswordValue;
	xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
        if (this.readyState == 4) {
			if (this.status == 200) {
				result = this.responseText;
				displayMsg(result, "info");
				document.getElementById('registerWifi').disabled = false;
			}
			else {
				displayMsg("Erreur lors du traitement de la demande", "error");
				document.getElementById('registerWifi').disabled = false;
			}
        }
    }   
	xmlHttp.open( "GET", theUrl );
	xmlHttp.send();	
}

function forgetWifi() {
	var result;
	var xmlHttp = null;
	var theUrl = "/actions/?actionCode=reinitialisation";
	xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
        if (this.readyState == 4) {
			if (this.status == 200) {
				result = this.responseText;
				displayMsg(result, "info");
			}
			else {
				displayMsg("Erreur lors du traitement de la demande", "error");
			}
        }
    }   
	xmlHttp.open( "GET", theUrl );
	xmlHttp.send();	
}

function reboot() {
	displayMsg("Redemarrage en cours. Cela peut prendre plus ou moins une minute.\nVeuillez rafraichir dune fois cet elapse de temps passe." , "info");
	var responseReceived;
	var xmlHttp = null;
	var theUrl = "/actions/?actionCode=reboot";
	xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function(event) {
	};
	xmlHttp.open( "GET", theUrl );
	xmlHttp.send();
}

//######################################
// Funtions specifiques a la page Home
//######################################

var notes = [["C-2", "CS-2", "D-2", "DS-2", "E-2", "F-2", "FS-2", "G-2", "GS-2", "A-2", "AS-2", "B-2"],["C-1", "CS-1", "D-1", "DS-1", "E-1", "F-1", "FS-1", "G-1", "GS-1", "A-1", "AS-1", "B-1"],["C-0", "CS-0", "D-0", "DS-0", "E-0", "F-0", "FS-0", "G-0", "GS-0", "A-0", "AS-0", "B-0"],["C1", "CS1", "D1", "DS1", "E1", "F1", "FS1", "G1", "GS1", "A1", "AS1", "B1"],["C2", "CS2", "D2", "DS2", "E2", "F2", "FS2", "G2", "GS2", "A2", "AS2", "B2"],["C3", "CS3", "D3", "DS3", "E3", "F3", "FS3", "G3", "GS3", "A3", "AS3", "B3"],["C4", "CS4", "D4", "DS4", "E4", "F4", "FS4", "G4", "GS4", "A4", "AS4", "B4"],["C5", "CS5", "D5", "DS5", "E5", "F5", "FS5", "G5", "GS5", "A5", "AS5", "B5"],["C6", "CS6", "D6", "DS6", "E6", "F6", "FS6", "G6", "GS6", "A6", "AS6", "B6"],["C7", "CS7", "D7", "DS7", "E7", "F7", "FS7", "G7"]];

function playNote(note,channel) {
	var responseReceived;
	var xmlHttp = null;
	var theUrl = "/playNote?note=" + note + "&channel=" + channel;
	xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function(event) {
		if (this.readyState === XMLHttpRequest.DONE) {
			if (this.status === 200) {
				responseReceived = this.responseText;
				displayMsg(responseReceived, "info");
			} else {
				displayMsg("Erreur de connection.", "error");
			}
		}
	};
	xmlHttp.open( "GET", theUrl );
	xmlHttp.send();
	hideMsg();
}

function buildNotesButtons() {
	for (let octaveIndex in notes) {
		for (let noteIndex in notes[octaveIndex]) {
			newbutton = document.createElement("button");
			newbutton.setAttribute("class", "btn btn-primary");
			newbutton.setAttribute("id", "note_" + notes[octaveIndex][noteIndex]);
			newbutton.setAttribute("onclick", "playNote('" + notes[octaveIndex][noteIndex] + "',1);");
			newbutton.innerHTML = notes[octaveIndex][noteIndex];
			document.getElementById("notesDiv").append(newbutton);
		}
		newbr = document.createElement("br");
		document.getElementById("notesDiv").append(newbr);
	}
}

function isJsonString(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

function getTextColor(hexcolor){
    var r = parseInt(hexcolor.substr(1,2),16);
    var g = parseInt(hexcolor.substr(3,2),16);
    var b = parseInt(hexcolor.substr(5,2),16);
    var yiq = ((r*299)+(g*587)+(b*114))/1000;
	//var yiq =(0.2126*r) + (0.7152*g) + (0.0722*b);
	//var yiq =Math.sqrt(0.299*r*r + 0.587*g*g + 0.114*b*b);
    return (yiq >= 128) ? '#000000' : '#ffffff';
  }

//##############################################
// Funtions specifiques a la page Dashboard
//##############################################
var configDashboard;
function loadConfig() {
	var lclConfig = localStorage.getItem('configDashboard');
	if (!lclConfig) {
		configDashboard = JSON.stringify([{"nam":"Ketchup","chn":1,"not":"GS2","row":1,"col":1,"clr":"#F5B7B1"},{"nam":"Mayo","chn":1,"not":"G2","row":1,"col":2,"clr":"#FCF3CF"},{"nam":"New Question","chn":1,"not":"GS3","row":2,"col":1,"clr":"#AED6F1"},{"nam":"Buzzer Time","chn":1,"not":"G3","row":2,"col":2,"clr":"#E8DAEF"}]);
	}
	else {
		configDashboard = lclConfig;
	}
	document.getElementById('configTextArea').value = configDashboard;
	document.getElementById('alertEditConfigModal').innerHTML = "";
	document.getElementById('alertEditConfigModal').hidden = true;
	document.getElementById('dashboardDiv').innerHTML = "";
	var configObject = JSON.parse(configDashboard); 
	var numRows = 0;
	configObject.forEach((element) => {
		if (element['row'] > numRows) {numRows = element['row']};
	});
	for (let i = 1; i <= numRows; i++) {
	  var newRow = document.createElement("div");
	  newRow.classList.add('row');
	  newRow.classList.add('justify-content-md-center');
	  newRow.classList.add('g-2');
	  newRow.setAttribute("id", "row_" + i);
	  document.getElementById('dashboardDiv').appendChild(newRow);
		configObject.forEach((element) => {
			if ((element['row'] == i)) {
				var newCol = document.createElement("div");
				newCol.classList.add('col-md-auto');
				newCol.classList.add('p-2');
				var newCard = document.createElement("button");
				newCard.classList.add('card');
				newCard.classList.add('btn');
				newCard.classList.add('p-3');
				newCard.innerHTML = element['nam'];
				newCard.style.backgroundColor = element['clr'];
				newCard.style.color = getTextColor(element['clr']);
				newCard.setAttribute("onclick", "playNote('" + element['not'] + "'," + element['chn'] + ")");
				newCol.appendChild(newCard);
				if (document.getElementById('row_' + i).childElementCount < element['col']) {
					document.getElementById('row_' + i).appendChild(newCol);
				}
				else {
					document.getElementById('row_' + i).insertBefore(newCol, document.getElementById('row_' + i).childNodes[element['col']-1]);
				}
			};			
		});
		
	}
}
  
function saveConfig(input) {
	if (isJsonString(input)) {
		configDashboard = input;	
		localStorage.setItem('configDashboard', configDashboard);
		loadConfig();
	}
	else {
		document.getElementById('alertEditConfigModal').innerHTML = "Invalide configuration";
		document.getElementById('alertEditConfigModal').hidden = false;
	}
}

