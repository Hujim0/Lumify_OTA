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
    }
    else {
        ChangeMode(prev_mode_id);
        SendJson(prev_mode_args);
    }

    popup_holder.classList.toggle("hidden");
}