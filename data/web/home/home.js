//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var brightness_slider = document.getElementById("brightness_slider");

    brightness_slider.value = event.detail.brightness;

    brightness_slider.addEventListener("change", EndStream);
    brightness_slider.addEventListener("change", (brightness_slider_event) => {
        SendJson(
            new espEvent(BRIGHTNESS, brightness_slider_event.target.value)
        );
    });
    brightness_slider.addEventListener("input", handleBrightnessInput);
});

function handleBrightnessInput() {
    if (!sentStreamEvent) {
        console.log("starting slider event");
        SendJson(new espEvent(OPEN_STREAM, BRIGHTNESS));
        sentStreamEvent = true;
    }

    sendStream(event.target.value);
}

//---------------------------------------------------------------------------

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var checkbox = document.getElementById("light_switch_checkbox");

    checkbox.checked = event.detail[LIGHT_SWITCH];

    checkbox.addEventListener("click", (checkbox_event) => {
        SendJson(new espEvent(LIGHT_SWITCH, checkbox_event.target.checked));
    });
});
