const popup_cross = document.getElementById("popup-cross");
const popup_holder = document.getElementById("popup-holder");

const add_button = document.getElementById("add_button");

const discard_button = document.getElementById("discard-button");
const apply_button = document.getElementById("apply-button");

const brightness_holder = document.getElementById("brightness-holder");
const mode_holder = document.getElementById("mode-holder");

var prev_brightness = 0;
var prev_mode_id = 0;
var prev_mode_args = null;

var current_time_events = null;

var current_time_event = null;
var editing_time_event_id = -1;

var info_button_list = [];

const default_time_event = {
    epoch_time: 12 * 3600,
    transition: 0,
    event_type: 0,
    value: 255,
    args: "",
};

popup_cross.addEventListener("click", (event) => {
    TogglePopup();
});
discard_button.addEventListener("click", (event) => {
    TogglePopup();
});
apply_button.addEventListener("click", (event) => {
    TogglePopup();
});

function TogglePopup() {
    if (popup_holder.classList.contains("hidden")) {
        prev_mode_id = current_mode_id;
        prev_mode_args = current_mode;

        updateUI(current_time_event);
    } else {
        sendGetRequest("/brightness?value=" + prev_brightness);
        ChangeMode(prev_mode_id);
        SendJson(prev_mode_args);
    }

    popup_holder.classList.toggle("hidden");
}

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    const xhr = new XMLHttpRequest();

    prev_brightness = event.detail.brightness;

    xhr.open("GET", "../data/time_events.json");

    xhr.send();

    xhr.onload = () => {
        current_time_events = JSON.parse(xhr.responseText);
        console.log(current_time_events);
    };
});

add_button.addEventListener("click", () => {
    editing_time_event_id = -1;

    current_time_event = default_time_event;

    TogglePopup();
});

var time_input;
var transition_input;
var type_input;
var brightness_input;

var mode_id_buffer = 0;
var mode_args_buffer = "";
var brightness_buffer = 0;

window.addEventListener("load", () => {
    const settings_list = document.getElementById("settings_list");

    time_input = new listElement().ConstructInput(
        settings_list.children[0],
        "epoch_time",
        ["type=time"],
        "12:00",
        info_button_list
    ).input_element;

    time_input.addEventListener("change", (event) => {
        var time = event.target.value.split(":");

        epoch_time = parseInt(time[0]) * 3600 + parseInt(time[1]) * 60;

        current_time_event["epoch_time"] = epoch_time;

        updateTimeEventValueAndArgs(event);
    });

    transition_input = new listElement().ConstructInput(
        settings_list.children[1],
        "transition",
        ["type=range", "min=0", "max=10", "step=0.1"],
        "0",
        info_button_list
    ).input_element;

    transition_input.addEventListener("change", (event) => {
        current_time_event["transition"] = event.target.value * 1000; //in millis

        updateTimeEventValueAndArgs(event);
    });

    type_input = new listElement().ConstructInput(
        settings_list.children[2],
        "event_type",
        ["name=event_type_selector"],
        0,
        info_button_list
    ).input_element;

    type_input.addEventListener("change", (event) => {
        pushValueToEvent(event);

        updateEventTypeUI(event.target.value);

        console.log("update!");

        if (event.target.value == 0) {
            mode_id_buffer = current_time_event["value"];
            mode_args_buffer = current_time_event["args"];

            ChangeMode(prev_mode_id);
            SendJson(prev_mode_args);

            brightness_input.value = brightness_buffer;

            sendGetRequest("/brightness?value=" + brightness_buffer);

            current_time_event["args"] = "";
        } else {
            brightness_buffer = current_time_event["value"];
            ChangeMode(mode_id_buffer, false, (args_string) => {
                current_time_event["args"] = args_string;
            });

            if (mode_args_buffer != "") {
                SendJson(mode_args_buffer);
            }

            sendGetRequest("/brightness?value=" + prev_brightness);
        }

        updateTimeEventValueAndArgs(event);
    });

    brightness_input = new listElement().ConstructInput(
        document.getElementById("brightness_input"),
        "value",
        ["type=range", "max=255", "min=0"],
        0,
        info_button_list
    ).input_element;

    addStreamEvent(brightness_input, BRIGHTNESS);

    brightness_input.addEventListener("change", (event) => {
        current_time_event["value"] = event.target.value;

        updateTimeEventValueAndArgs(event);
    });
});

function updateTimeEventValueAndArgs(event) {
    if (current_time_event.event_type == 1) {
        current_time_event.value = current_mode_id;
        current_time_event.args = JSON.stringify(current_mode);
    }

    console.log(current_time_event);
}

function pushValueToEvent(event) {
    current_time_event[event.target.json_arg_name] = event.target.value;
}

function updateUI(_time_event) {
    let hours = Math.floor(_time_event["epoch_time"] / 3600);
    let minutes = Math.floor((_time_event["epoch_time"] % 3600) / 60);
    if (hours < 10) {
        hours = "0" + String(hours);
    }
    if (minutes < 10) {
        minutes = "0" + String(minutes);
    }

    console.log(hours + ":" + minutes);
    time_input.value = hours + ":" + minutes;
    transition_input.value = _time_event["transition"];
    type_input.value = _time_event["event_type"];

    updateEventTypeUI(_time_event["event_type"]);

    if (_time_event["event_type"] == 0) {
        brightness_input.value = _time_event["value"];
    } else {
        mode_id_buffer = _time_event["value"];
        mode_args_buffer = _time_event["args"];

        ChangeMode(_time_event["value"]);
        SendJson(_time_event["args"]);
    }
}

function updateEventTypeUI(value) {
    if (value == 0) {
        brightness_holder.classList.remove("hidden");
        mode_holder.classList.add("hidden");
    } else {
        brightness_holder.classList.add("hidden");
        mode_holder.classList.remove("hidden");
    }
}
