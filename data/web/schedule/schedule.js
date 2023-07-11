const popup_cross = document.getElementById("popup-cross");
const popup_holder = document.getElementById("popup-holder");

var prev_mode_id = 0;
var prev_mode_args = null;

var current_time_events = null;

popup_cross.addEventListener("click", (event) => {
    TogglePopup();
});

function TogglePopup() {
    if (popup_holder.classList.contains("hidden")) {
        prev_mode_id = current_mode_id;
        prev_mode_args = current_mode;
    } else {
        ChangeMode(prev_mode_id);
        SendJson(prev_mode_args);
    }

    popup_holder.classList.toggle("hidden");
}

window.addEventListener(ON_GET_PREFERENCES_EVENT, () => {
    const xhr = new XMLHttpRequest();

    xhr.open("GET", "../data/time_events.json");

    xhr.send();

    xhr.onload = () => {
        current_time_events = JSON.parse(xhr.responseText);
        console.log(current_time_events);
    };
});
