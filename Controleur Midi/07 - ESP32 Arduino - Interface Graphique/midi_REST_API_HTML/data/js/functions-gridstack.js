//###########################################################
// Funtions specifiques a la page DashboardGridstack
//###########################################################

var editMode = false;
var items;
  
  
var options = {
	staticGrid: true
};

var grid = GridStack.init(options);

function loadItems() {
	var lclItems = localStorage.getItem('itemsDashboard');
	if (!lclItems) {
		items = [
			{id: 1, w: 1, h:1, x:0, y:0, content: 'Mayo', color: '#FCF3CF', note:'G2', channel: 1},
			{id: 2, w: 1, h:1, x:1, y:0, content: 'Ketchup', color: '#F5B7B1', note:'GS2', channel: 1}, 
			{id: 3, w: 1, h:1, x:2, y:0, content: 'New Question', color: '#AED6F1', note:'GS3', channel: 1},
			{id: 4, w: 1, h:1, x:3, y:0, content: 'Buzzer Time', color: '#E8DAEF', note:'G3', channel: 1}
		]
	}
	else {
		items = JSON.parse(lclItems);
	}
}
function toggleEditMode() {
        customizeGrid();
		document.querySelectorAll('.remove-item').forEach(function(button) {
            button.style.display = editMode ? 'block' : 'none';
        });
		document.querySelectorAll('.edit-item').forEach(function(button) {
            button.style.display = editMode ? 'block' : 'none';
        });
		document.querySelectorAll('.play-item').forEach(function(button) {
            button.style.display = editMode ? 'block' : 'none';
        });
		document.getElementById("playButton").style.display = editMode ? 'inline' : 'none';
		document.getElementById("editButton").style.display = editMode ? 'none' : 'inline';
		document.getElementById("saveButton").style.display = editMode ? 'inline' : 'none';
		document.getElementById("addButton").style.display = editMode ? 'inline' : 'none';
		document.getElementById("editConfButton").style.display = editMode ? 'inline' : 'none';
}
		
function loadPage() {
	  
	grid.load(items);
	customizeGrid();
	  
	toggleEditMode()
	
	addNewWidget = function () {
	  
	  var maxId = items.reduce((max, item) => Math.max(max, parseInt(item.id)), 0);
	  var newId = maxId + 1;

	  var node = {
		id: newId,
		w: 1,
		h: 1,
		content: 'Nouveau ' + newId,
		color: '#EEEEEE',
		note: '',
		channel: 1
	  };
	  grid.addWidget(node);
	
	  customizeGrid();
	  
	  return false;
	};	
} 

function fromHTML(html, trim = true) {
  // Process the HTML string.
  html = trim ? html.trim() : html;
  if (!html) return null;

  // Then set up a new template element.
  const template = document.createElement('template');
  template.innerHTML = html;
  const result = template.content.children;

  // Then return either an HTMLElement or HTMLCollection,
  // based on whether the input HTML had one or more roots.
  if (result.length === 1) return result[0];
  return result;
}

function customizeGrid() {
	items = grid.save();
	document.querySelectorAll(".remove-item").forEach(el => el.remove());
	document.querySelectorAll(".edit-item").forEach(el => el.remove());
	document.querySelectorAll(".play-item").forEach(el => el.remove());
	items.forEach(item => {
			var itemElement = document.querySelector("[gs-id='"+item.id+"']");
			//itemElement.style.background = item.color;
			itemElement.style.color = getTextColor(item.color);
			itemElement.firstChild.style.backgroundColor = item.color;
			itemElement.onclick = function() { clickTile(item.note, item.channel); };
			
			const svgRemoveButton = fromHTML('<svg xmlns="http://www.w3.org/2000/svg" width="15px" shape-rendering="geometricPrecision" text-rendering="geometricPrecision" image-rendering="optimizeQuality" fill-rule="evenodd" clip-rule="evenodd" viewBox="0 0 456 511.82"><path fill="' + getTextColor(item.color) + '" d="M48.42 140.13h361.99c17.36 0 29.82 9.78 28.08 28.17l-30.73 317.1c-1.23 13.36-8.99 26.42-25.3 26.42H76.34c-13.63-.73-23.74-9.75-25.09-24.14L20.79 168.99c-1.74-18.38 9.75-28.86 27.63-28.86zM24.49 38.15h136.47V28.1c0-15.94 10.2-28.1 27.02-28.1h81.28c17.3 0 27.65 11.77 27.65 28.01v10.14h138.66c.57 0 1.11.07 1.68.13 10.23.93 18.15 9.02 18.69 19.22.03.79.06 1.39.06 2.17v42.76c0 5.99-4.73 10.89-10.62 11.19-.54 0-1.09.03-1.63.03H11.22c-5.92 0-10.77-4.6-11.19-10.38 0-.72-.03-1.47-.03-2.23v-39.5c0-10.93 4.21-20.71 16.82-23.02 2.53-.45 5.09-.37 7.67-.37zm83.78 208.38c-.51-10.17 8.21-18.83 19.53-19.31 11.31-.49 20.94 7.4 21.45 17.57l8.7 160.62c.51 10.18-8.22 18.84-19.53 19.32-11.32.48-20.94-7.4-21.46-17.57l-8.69-160.63zm201.7-1.74c.51-10.17 10.14-18.06 21.45-17.57 11.32.48 20.04 9.14 19.53 19.31l-8.66 160.63c-.52 10.17-10.14 18.05-21.46 17.57-11.31-.48-20.04-9.14-19.53-19.32l8.67-160.62zm-102.94.87c0-10.23 9.23-18.53 20.58-18.53 11.34 0 20.58 8.3 20.58 18.53v160.63c0 10.23-9.24 18.53-20.58 18.53-11.35 0-20.58-8.3-20.58-18.53V245.66z"/></svg>');
			const svgEditButton = fromHTML('<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" width="15px" x="0px" y="0px" viewBox="0 0 117.74 122.88" style="enable-background:new 0 0 117.74 122.88" xml:space="preserve"><style type="text/css">.st0{fill-rule:evenodd;clip-rule:evenodd;}</style><g><path fill="' + getTextColor(item.color) + '" class="st0" d="M94.62,2c-1.46-1.36-3.14-2.09-5.02-1.99c-1.88,0-3.56,0.73-4.92,2.2L73.59,13.72l31.07,30.03l11.19-11.72 c1.36-1.36,1.88-3.14,1.88-5.02s-0.73-3.66-2.09-4.92L94.62,2L94.62,2L94.62,2z M41.44,109.58c-4.08,1.36-8.26,2.62-12.35,3.98 c-4.08,1.36-8.16,2.72-12.35,4.08c-9.73,3.14-15.07,4.92-16.22,5.23c-1.15,0.31-0.42-4.18,1.99-13.6l7.74-29.61l0.64-0.66 l30.56,30.56L41.44,109.58L41.44,109.58L41.44,109.58z M22.2,67.25l42.99-44.82l31.07,29.92L52.75,97.8L22.2,67.25L22.2,67.25z"/></g></svg>');
			const svgPlayButton = fromHTML('<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" width="15px" x="0px" y="0px" viewBox="0 0 122.88 122.88" style="enable-background:new 0 0 122.88 122.88" xml:space="preserve"><style type="text/css">.st0{fill-rule:evenodd;clip-rule:evenodd;}</style><g><path fill="' + getTextColor(item.color) + '" class="st0" d="M61.44,0c33.93,0,61.44,27.51,61.44,61.44s-27.51,61.44-61.44,61.44S0,95.37,0,61.44S27.51,0,61.44,0L61.44,0z M84.91,65.52c3.41-2.2,3.41-4.66,0-6.61L49.63,38.63c-2.78-1.75-5.69-0.72-5.61,2.92l0.11,40.98c0.24,3.94,2.49,5.02,5.8,3.19 L84.91,65.52L84.91,65.52z"/></g></svg>');
			
			svgEditButton.classList.add("edit-item");
			svgRemoveButton.classList.add("remove-item");
			svgPlayButton.classList.add("play-item");
			
			itemElement.appendChild(svgEditButton);
			itemElement.appendChild(svgRemoveButton);
			itemElement.appendChild(svgPlayButton);
			
			let elementsResize = itemElement.getElementsByClassName('ui-resizable-se');
			if (elementsResize.length > 0) {
				
				Array.prototype.forEach.call(elementsResize, function(el) {
					// Do stuff here
					if (getTextColor(item.color) == "#ffffff") {
						el.style.filter = 'brightness(0) invert(1)';
					}
					else {
						el.style.filter = 'brightness(0)';
					}
				});
			}			
		}
	  );
	document.querySelectorAll('.remove-item').forEach(function(button) {
        button.addEventListener('click', function() {
            var item = button.closest('.grid-stack-item');
            grid.removeWidget(item);
        });
    });
	document.querySelectorAll('.edit-item').forEach(function(button) {
        button.addEventListener('click', function() {
			var item = retrieveItemById(button.closest('.grid-stack-item').getAttribute("gs-id"));
            console.log(item);
			var myModalEl = document.querySelector('#editWidgetModal');
			var modal = bootstrap.Modal.getOrCreateInstance(myModalEl);
			modal.show();
			openEditWidgetModal(item);
        });
    });
	document.querySelectorAll('.play-item').forEach(function(button) {
        button.addEventListener('click', function() {
			var item = retrieveItemById(button.closest('.grid-stack-item').getAttribute("gs-id"));
            playNote(item.note, item.channel);
        });
    });
	
}

function retrieveItemById(id) {
	var result;
	grid.save().forEach(item => {
		if (id == item.id) {
			result = item;
		}
	})
	return result;
}

function clickTile(note, channel) {
	if (!editMode) {
		playNote(note, channel)
	}
}

function switchToEdit() {
	editMode = true;
	grid.setStatic(false);
	toggleEditMode();
}

function switchToPlay() {
	editMode = false;
	grid.setStatic(true);
	toggleEditMode();
}

function saveGrid() {
	items = grid.save();
	localStorage.setItem('itemsDashboard', JSON.stringify(items));
}

function openEditConfigModal() {
	saveGrid();
	document.getElementById('alertEditConfigModal').hidden = true;
	document.getElementById('configTextArea').value = JSON.stringify(items);
}

function openEditWidgetModal(item) {
	document.getElementById("inputGroupSelectNote").innerHTML = "";
	notes.forEach(octaveNotes => {
		octaveNotes.forEach(note => {
			new Option(note, note);
			document.getElementById("inputGroupSelectNote").appendChild(new Option(note, note));
		});
	});
	document.getElementById('textInput').value = item.content;
	document.getElementById('inputGroupSelectChannel').value = item.channel;
	document.getElementById('inputGroupSelectNote').value = item.note;
	document.getElementById('colorInput').value = item.color;
	document.getElementById('idInput').value = item.id;
}

function saveConfigGridstack(input) {
	var configItems;
	try {
		configItems = JSON.parse(input);
	}
	catch (error) {
		showConfigError();
		return;
	}

	items = configItems;
	loadPage();
	saveGrid();

	var myModalEl = document.querySelector('#editConfigModal');
	var modal = bootstrap.Modal.getOrCreateInstance(myModalEl);
	modal.hide();
	document.getElementById('alertEditConfigModal').hidden = true;
}

function showConfigError() {
	document.getElementById('alertEditConfigModal').innerHTML = "Invalide configuration";
	document.getElementById('alertEditConfigModal').hidden = false;
}

function applyWidgetConfig() {
	var item = retrieveItemById(document.getElementById('idInput').value);
	item.content = document.getElementById('textInput').value;
	item.channel = parseInt(document.getElementById('inputGroupSelectChannel').value);
	item.note = document.getElementById('inputGroupSelectNote').value;
	item.color = document.getElementById('colorInput').value;
	var myModalEl = document.querySelector('#editWidgetModal');
	var modal = bootstrap.Modal.getOrCreateInstance(myModalEl);
	modal.hide();
	
	var itemElement = document.querySelector("[gs-id='"+item.id+"']");
    grid.removeWidget(itemElement);
	grid.addWidget(item);
	customizeGrid();
}