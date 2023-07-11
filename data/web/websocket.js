var gateway = "ws:" + window.location.host + "/ws";
var websocket = new WebSocket(gateway);

console.log("Trying to open a WebSocket connection...");

const STREAM_DELAY = 16;
var canSendWebsocket = true;
var buffer;

var sentStreamEvent = false;

var lostConnection = true;

window.addEventListener("load", BeginWebSocket);

function BeginWebSocket() {
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;

    websocket.onerror = () => {
        location.reload();
    };
}

function onClose(event) {
    current_mode = null;
    console.log("Lost connection!");
    lostConnection = true;
    location.reload();
}

function onOpen(event) {
    console.log("Connection opened");
}

function onMessage(event) {}

function EndStream() {
    if (lostConnection) return;
    websocket.send("]");
    console.log("closing stream");
    sentStreamEvent = false;
}

function SendJson(object) {
    if (lostConnection) return;
    websocket.send(JSON.stringify(object));
}

function sendStream(value) {
    if (canSendWebsocket == true) {
        canSendWebsocket = false;
        setTimeout(function () {
            canSendWebsocket = true;

            if (buffer != null) {
                sendStream(buffer);
            }
        }, STREAM_DELAY);

        websocket.send(value);
        console.log(value);
        buffer = null;
    } else {
        buffer = value;
    }
}
