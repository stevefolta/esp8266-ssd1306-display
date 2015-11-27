function load() {
	document.getElementById('ok').onclick = function() {
		var text_area = document.getElementById('message');
		var message = text_area.value;
		var request = new XMLHttpRequest();
		request.open('POST', 'message', true);
		request.send(message);
		};
	}


