const popup_cross = document.getElementById("popup-cross");
const popup_holder = document.getElementById("popup-holder");

const add_button = document.getElementById("add_button");

const discard_button = document.getElementById("discard-button");
const apply_button = document.getElementById("apply-button");

const brightness_holder = document.getElementById("brightness-holder");
const mode_holder = document.getElementById("mode-holder");

const time_event_template = document.getElementById("time-event-template");
const time_events_list = document.getElementById("time-events-list");

var prev_brightness = 0;
var prev_mode_id = 0;
var prev_mode_args = "";

var current_time_events = null;

var current_time_event = null;
var editing_time_event_id = -1;

var time_input;
var transition_input;
var type_input;
var brightness_input;

var mode_id_buffer = 0;
var mode_args_buffer = "";
var brightness_buffer = 0;

var info_button_list = [];

function get_default_time_event() {
    return {
        epoch_time: 12 * 3600,
        transition: 0,
        event_type: 0,
        value: 128,
        args: "",
    };
}

window.addEventListener("load", () => {
    resetValues();

    //------------------------------------------
    const settings_list = document.getElementById("settings_list");

    time_input = new listElement().ConstructInput(
        settings_list.children[0],
        "epoch_time",
        ["type=time"],
        "12:00",
        info_button_list
    ).input_element;

    time_input.addEventListener("change", (event) => {
        let time = event.target.value.split(":");

        epoch_time = parseInt(time[0]) * 3600 + parseInt(time[1]) * 60;

        current_time_event["epoch_time"] = epoch_time;

        updateTimeEventValueAndArgs();
    });

    transition_input = new listElement().ConstructInput(
        settings_list.children[1],
        "transition",
        ["type=range", "min=0", "max=10", "step=0.1"],
        "0",
        info_button_list
    ).input_element;

    transition_input.addEventListener("change", (event) => {
        current_time_event["transition"] = Number(event.target.value); //in millis

        updateTimeEventValueAndArgs();
    });

    type_input = new listElement().ConstructInput(
        settings_list.children[2],
        "event_type",
        ["name=event_type_selector"],
        0,
        info_button_list
    ).input_element;

    type_input.addEventListener("change", (event) => {
        // pushValueToEvent(event);
        current_time_event["event_type"] = Number(event.target.value);

        changeEventType(current_time_event);

        updateTimeEventValueAndArgs();
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
        current_time_event["value"] = Number(event.target.value);

        updateTimeEventValueAndArgs();
    });
});

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    prev_brightness = event.detail.brightness;

    sendGetRequest("../data/time_events.json", (status, responseText) => {
        current_time_events = JSON.parse(responseText)["events"];
        SortAndCreateCurrentTimeEvents();
    });

    modeList.save_modes = false;
    document.body.className = "";

    modeList.eventSource.addEventListener("mode_changed", (event) => {
        let id_and_args = event.data.split(",");
        prev_mode_id = Number(id_and_args[0]);
        prev_mode_args = id_and_args[1];
    });

    modeList.eventSource.addEventListener("brightness_changed", (event) => {
        prev_brightness = Number(event.data);
    });

    modeList.eventSource.removeEventListener(
        "mode_changed",
        modeList.onModeChanged
    );
});

add_button.addEventListener("click", () => {
    editing_time_event_id = -1;

    current_time_event = get_default_time_event();

    TogglePopup(current_time_event);
});

popup_cross.addEventListener("click", (event) => {
    TogglePopup(null);
});
discard_button.addEventListener("click", (event) => {
    TogglePopup(null);
});

apply_button.addEventListener("click", (event) => {
    TogglePopup(null);
    updateTimeEventValueAndArgs(true);

    if (editing_time_event_id == -1) {
        current_time_events.push(current_time_event);
        if (CheckForDuplicates(current_time_events) == false) {
            TogglePopup(current_time_event);
            current_time_events.pop();
        }
    } else {
        current_time_events[editing_time_event_id] = current_time_event;
        if (CheckForDuplicates(current_time_events) == false) {
            TogglePopup(current_time_event);
        }
    }

    SortAndCreateCurrentTimeEvents();
    PostCurrentEvents();
});

function TogglePopup(_time_event) {
    if (prev_mode_args == "") {
        prev_mode_id = modeList.current_mode_id;
        prev_mode_args = JSON.stringify(modeList.current_mode);
    }

    if (!popup_holder.classList.contains("active")) {
        updateUI(_time_event);

        if (_time_event["event_type"] == 0) {
            sendGetRequest("/brightness?value=" + _time_event["value"]);
            brightness_input.value = _time_event["value"];
        } else {
            modeList.ChangeMode(
                _time_event["value"],
                _time_event["args"],
                false,
                true
            );
        }
    } else {
        sendGetRequest("/brightness?value=" + String(prev_brightness));
        modeList.ChangeMode(prev_mode_id, prev_mode_args, false, false);
        resetValues();
    }

    popup_holder.classList.toggle("active");
}

function resetValues() {
    mode_id_buffer = 0;
    brightness_buffer = 0;

    sendGetRequest("/mode?id=0", (status, responseText) => {
        mode_args_buffer = responseText;
    });
}

function updateTimeEventValueAndArgs(force_update = false) {
    if (current_time_event.event_type == 1 || force_update) {
        current_time_event.value = Number(modeList.current_mode_id);
        current_time_event.args = JSON.stringify(modeList.current_mode);
    }
}

function pushValueToEvent(event) {
    current_time_event[event.target.json_arg_name] = event.target.value;
}

function updateUI(_time_event) {
    time_input.value = FormatTime(_time_event["epoch_time"]);
    transition_input.value = _time_event["transition"];
    type_input.value = _time_event["event_type"];

    updateEventTypeUI(_time_event);
}

function changeEventType(_time_event) {
    updateEventTypeUI(_time_event);

    if (_time_event["event_type"] == 0) {
        updateTimeEventValueAndArgs(true);

        mode_id_buffer = _time_event["value"];
        mode_args_buffer = _time_event["args"];

        brightness_input.value = brightness_buffer;

        modeList.ChangeMode(prev_mode_id, prev_mode_args, false, false);
        _time_event["args"] = "";
        _time_event["value"] = brightness_buffer;

        sendGetRequest("/brightness?value=" + brightness_buffer);
    } else {
        brightness_buffer = _time_event["value"];

        modeList.ChangeMode(mode_id_buffer, mode_args_buffer, false, true);
        _time_event["args"] = JSON.stringify(mode_args_buffer);
        _time_event["value"] = mode_id_buffer;

        sendGetRequest("/brightness?value=" + prev_brightness);
    }
}

function updateEventTypeUI(_time_event) {
    if (_time_event["event_type"] == 0) {
        brightness_holder.classList.remove("hidden");
        mode_holder.classList.add("hidden");
    } else {
        brightness_holder.classList.add("hidden");
        mode_holder.classList.remove("hidden");
    }
}

function FormatTime(epoch) {
    let hours = Math.floor(epoch / 3600);
    let minutes = Math.floor((epoch % 3600) / 60);
    if (hours < 10) {
        hours = "0" + String(hours);
    }
    if (minutes < 10) {
        minutes = "0" + String(minutes);
    }

    return hours + ":" + minutes;
}

/* <label>12:00</label>
<button class="edit-button">
    <i class="fa-solid fa-pen"></i>
</button>
<button class="trash-button">
    <i class="fa-regular fa-trash-can"></i>
</button>
<label class="info-button">Info
    <input type="checkbox"></input>
    <i class="fas fa-circle-info"></i>
</label>
<label class="text-light">Description</label> */

var info_descriptions_array = [];

function CreateTimeEvents(events) {
    time_events_list.innerHTML = "";
    info_descriptions_array = [];

    for (let i = 0; i < events.length; i++) {
        createTimeEventListElement(events[i], i);
    }
}

function createTimeEventListElement(time_event, id) {
    list_child = time_event_template.content.cloneNode(true).children[0];

    Object.assign(list_child, { time_event_id: String(id) });

    list_child.children[0].innerText = FormatTime(time_event["epoch_time"]);

    // list_child.children[1]["list_id"] = String(i);
    Object.assign(list_child.children[1], { time_event_id: String(id) });
    Object.assign(list_child.children[1].children[0], {
        time_event_id: String(id),
    });

    list_child.children[1].addEventListener("click", (event) => {
        let event_id = event.target.parentNode["time_event_id"];

        editing_time_event_id = event_id;
        current_time_event = current_time_events[event_id];
        TogglePopup(current_time_events[event_id]);
    });

    Object.assign(list_child.children[2], { time_event_id: String(id) });
    Object.assign(list_child.children[2].children[0], {
        time_event_id: String(id),
    });
    list_child.children[2].addEventListener("click", (event) => {
        deleteEvent(event.target.parentNode["time_event_id"]);
    });

    new listElement().ConfigureInfoButton(
        list_child.children[3],
        list_child.children[4],
        info_descriptions_array,
        (event, isActive, description_element) => {
            if (isActive) {
                if (description_element.innerHTML[0] != "*") return;

                description_element.innerHTML =
                    description_element.innerHTML.substring(1);

                let time_event =
                    current_time_events[event.target["time_event_id"]];

                description_element.innerHTML +=
                    FormatTime(time_event["epoch_time"]) + ".\n" + "- ";

                if (time_event["event_type"] == 0) {
                    description_element.innerHTML +=
                        modeList.localization.brightness +
                        String(Number(time_event["value"]) / 2.55) +
                        "%";
                } else {
                    description_element.innerHTML +=
                        modeList.FormatModeName(time_event["value"]) + ": ";

                    sendGetRequest(
                        "/elements?id=" + time_event["value"],
                        (status, responseText) => {
                            if (status != 200) return;

                            let elements = JSON.parse(responseText)["elems"];
                            let args = JSON.parse(time_event["args"]);
                            description_element.innerHTML +=
                                modeList.FormatModeArgs(elements, args) + ".";
                        }
                    );
                }
            }
        },
        "time_event_id=" + id
    );

    time_events_list.appendChild(list_child);
}

function deleteEvent(id) {
    current_time_events.splice(id, 1);
    SortAndCreateCurrentTimeEvents();
    PostCurrentEvents();
}

function SortAndCreateCurrentTimeEvents() {
    current_time_events.sort((a, b) => a.epoch_time - b.epoch_time);
    CreateTimeEvents(current_time_events);
}

function PostCurrentEvents() {
    sendPostRequest(
        "/time_events",
        JSON.stringify({ events: current_time_events })
    );
}

function CheckForDuplicates(_time_events) {
    if (_time_events == null || _time_events.length == 0) return true;

    var time_events = _time_events.toSorted(
        (a, b) => a.epoch_time - b.epoch_time
    );

    var last_epoch = time_events[0]["epoch_time"];
    var last_type = time_events[0]["event_type"];

    for (let i = 1; i < time_events.length; i++) {
        if (
            time_events[i]["epoch_time"] == last_epoch &&
            time_events[i]["event_type"] == last_type
        ) {
            alert(
                "Invalid time event at " +
                    FormatTime(time_events[i]["epoch_time"]) +
                    "! There cannot be more than one event with the same type and time!"
            );
            return false;
        }

        last_epoch = time_events[i]["epoch_time"];
        last_type = time_events[i]["event_type"];
    }

    return true;
}
