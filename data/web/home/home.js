//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var brightness_slider = document.getElementById("brightness_slider");

    brightness_slider.value = event.detail.brightness;

    addStreamEvent(brightness_slider, BRIGHTNESS);

    brightness_slider.addEventListener("change", (event) => {
        // SendJson(new espEvent(BRIGHTNESS, event.target.value));
        sendGetRequest(
            "/brightness?value=" + String(event.target.value) + "&save"
        );
    });
});

//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var checkbox = document.getElementById("light_switch_checkbox");

    checkbox.checked = event.detail[LIGHT_SWITCH];

    checkbox.addEventListener("click", (checkbox_event) => {
        // SendJson(new espEvent(LIGHT_SWITCH, checkbox_event.target.checked));
        sendGetRequest(
            "/light_switch?value=" +
                (checkbox_event.target.checked ? "true" : "false") +
                "&save"
        );
    });
});
