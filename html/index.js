function load() {
	document.getElementById('ok').onclick = function() {
		var text_area = document.getElementById('message');
		var message = text_area.value;
		var request = new XMLHttpRequest();
		request.open('POST', 'message', true);
		request.send(message);
		};
	document.getElementById('set-word-delay').onclick = function() {
		var element = document.getElementById('word-delay');
		var request = new XMLHttpRequest();
		request.open('POST', 'word-delay', true);
		request.send(element.value);
		};
	}


