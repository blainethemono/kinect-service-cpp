/**
 * WebSocket specs http://www.w3.org/TR/websockets/
 */

// get elements

var button = document.getElementById('button');
button.addEventListener('click', clickHandler);

var log = document.getElementById('log');

// - - - create and subscribing sockets

var socket = new WebSocket('ws://127.0.0.1:7681', 'kinect_app');

//for Kinect need use ws://127.0.0.1:6001

socket.onopen = _connectHandler;
socket.onclose = _disconnectHandler;
socket.onerror = _errorHandler;
socket.onmessage = _messageHandler;
	
function _connectHandler() {
	log.value += 'connected\n';
};

function _disconnectHandler() {
	log.value += 'disconnected\n';
};

function _errorHandler(err) {
	log.value += JSON.stringify(err) + '\n';
};

function _messageHandler(msg) {
	log.value += msg + '\n';
};

// - - - kinect request

var request = {protocol: 'MS', entity: 'skeleton'},
	msg = JSON.stringify(request);

// - - - send on click

function clickHandler() {
	socket.send(msg);
}