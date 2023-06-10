
//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var brigthness_slider = document.getElementById("brightness_slider");

    brigthness_slider.value = event.detail.brightness;

    brigthness_slider.addEventListener("change", EndStream);
    brigthness_slider.addEventListener("change", (brigthness_slider_event) => {
        SendJson(new espEvent(BRIGHTNESS, brigthness_slider_event.target.value))
    });
    brigthness_slider.addEventListener("input", handleBrigthnessInput);
});

function handleBrigthnessInput() {
    if (!sentStreamEvent) {
        console.log("starting slider event");
        SendJson(new espEvent(OPEN_STREAM, BRIGHTNESS));
        sentStreamEvent = true;
    }

    console.log(event.target.value);
    websocket.send(event.target.value);
}


//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var checkbox = document.getElementById("light_switch_checkbox");

    checkbox.checked = event.detail[LIGHT_SWITCH];

    checkbox.addEventListener("click", (checkbox_event) => {
        SendJson(new espEvent(LIGHT_SWITCH, checkbox_event.target.checked))
    });
});
