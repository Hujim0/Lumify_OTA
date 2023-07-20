window.addEventListener("load", () => {
    sendGetRequest("/wifi", (status, responseText) => {
        console.log(responseText);
    });
});

function sendGetRequest(uri, _onload = null) {
    const xhr = new XMLHttpRequest();
    xhr.open("GET", uri, true);
    xhr.send();
    if (_onload != null) {
        xhr.onload = () => {
            _onload(xhr.status, xhr.responseText);
        };
    }

    return xhr;
}

function sendPostRequest(uri, body) {
    const xhr = new XMLHttpRequest();

    xhr.open("POST", uri);
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.send(body);

    return xhr;
}
