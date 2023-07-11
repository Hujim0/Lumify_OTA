var current_mode = null;
var current_mode_id = 0;
var current_elements = null;

var gotPreferences = false;

const OPEN_STREAM = "open_stream";

const LIGHT_SWITCH = "light_switch";
const BRIGHTNESS = "brightness";
const MODE_SWITCH = "mode_switch";
const REQUEST_ARGS = "mode_args_request";

const ON_GET_ELEMENTS_EVENT = "onElementsMode";
const ON_GET_PREFERENCES_EVENT = "onGetPreferences";

class espEvent {
    event;
    value;

    constructor(event, value) {
        this.event = event;
        this.value = value;
    }
}

window.addEventListener("load", GetPreferences);
window.addEventListener("load", SendTime);

function GetPreferences() {
    const xhr = new XMLHttpRequest();

    xhr.open("GET", "/preferences");

    xhr.onload = () => {
        lostConnection = false;

        window.dispatchEvent(
            new CustomEvent(ON_GET_PREFERENCES_EVENT, {
                detail: JSON.parse(xhr.responseText),
            })
        );
        gotPreferences = true;

        console.log("[AJAX] - got preferences!");
        // console.log(xhr.responseText);
    };

    xhr.send();
}

function SendTime() {
    var date = new Date();

    var _event = new espEvent(
        "epoch_time",
        date.getHours() * 3600 + date.getMinutes() * 60 + date.getSeconds()
    );

    _event.dayOfTheWeek = date.getDay();

    const xhr = new XMLHttpRequest();

    xhr.open(
        "GET",
        "/time?epoch=" +
            String(
                date.getHours() * 3600 +
                    date.getMinutes() * 60 +
                    date.getSeconds()
            ) +
            "&dayoftheweek=" +
            String(date.getDay())
    );
    xhr.send();

    // SendJson(_event);
}

//---------------------------------------------------------------------------

var modes = document.getElementById("mode_list").children;
for (var i = 0; i < modes.length; i++) {
    modes[i].addEventListener("input", (event) => {
        GetMode(event.target.getAttribute("mode-id"), false);
        CreateSkeleton();
    });
}

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var mode_id = event.detail.mode;

    modes[mode_id].children[0].checked = true;
    GetMode(mode_id, true);
});

//---------------------------------------------------------------------------

var list_container = document.getElementById("list-container");
var list_template = document.getElementById("list-template");
var skeleton_template = document.getElementById("skeleton-template");

var loaded_first = "";

function ChangeMode(id) {
    const xhr_args = new XMLHttpRequest();

    var get_string = "/mode?id=" + id;

    get_string += "&change=true";

    xhr_args.open("GET", get_string, true);

    xhr_args.send();
}

function GetMode(id, RequestMode = false) {
    // SendJson(new espEvent(REQUEST_ARGS, id));

    current_mode_id = id;

    loaded_first = "";

    const xhr_args = new XMLHttpRequest();

    var get_string = "/mode?id=" + id;

    if (RequestMode) {
        get_string += "&request=true";
    }

    xhr_args.open("GET", get_string, true);

    xhr_args.onload = () => {
        if (xhr_args.status == 404) {
            alert("cant get args - file not found!");
            return;
        }
        current_mode = JSON.parse(xhr_args.responseText);
        // console.log(xhr_args.responseText);

        if (loaded_first == "") {
            loaded_first = "args";
        } else {
            CreateModeElements(current_elements);
        }
    };
    xhr_args.send();

    const xhr_elements = new XMLHttpRequest();

    xhr_elements.open("GET", "/elements?id=" + id, true);

    xhr_elements.onload = () => {
        if (xhr_args.status == 404) {
            alert("cant get elems - file not found!");
            return;
        }
        // console.log(xhr_elems.responseText);

        current_elements = JSON.parse(xhr_elements.responseText)["elems"];

        if (loaded_first == "") {
            loaded_first = "elems";
        } else {
            CreateModeElements(current_elements);
        }
    };

    xhr_elements.send();
}

var current_info_inputs = [];

function CreateSkeleton() {
    list_container.innerHTML = "";
    var count = current_info_inputs.length;
    if (count == 0 || count == null) {
        count = 2;
    }

    for (let i = 0; i < count; i++) {
        list_container.appendChild(skeleton_template.content.cloneNode(true));
    }
}
CreateSkeleton();

function CreateModeElements(_elements) {
    list_container.innerHTML = "";

    current_info_inputs = [];

    for (var i = 0; i < _elements.length; i++) {
        var child_element = list_template.content.cloneNode(true).children[0];

        child_element.children[0].innerText = _elements[i]["name"];

        child_element.children[1] = SetUpInputChild(
            child_element.children[1],
            _elements[i]
        );

        if (_elements[i].desc == null) {
            child_element.children[2].hidden = true;
            list_container.appendChild(child_element);
            return;
        }
        child_element.children[2].children[0]["arg_id"] = i;
        child_element.children[2].children[0].addEventListener(
            "input",
            (event) => {
                // window.dispatchEvent(new CustomEvent("ARG_INFO_ID_UPDATE", {
                //     detail: {
                //         state: event.target.checked,
                //         arg_info_id: event.target["arg_id"]
                //     }
                // }));

                current_info_inputs[event.target["arg_id"]].classList.toggle(
                    "active"
                );
            }
        );

        current_info_inputs.push(child_element.children[3]);

        child_element.children[3].innerText = _elements[i].desc;
        list_container.appendChild(child_element);
    }
}

function SetUpInputChild(input_element, _element) {
    for (var x = 0; x < _element.input_props.length; x++) {
        var attribute = _element.input_props[x].split("=");

        input_element[attribute[0]] = attribute[1];
    }

    input_element.mode_arg_name = _element.arg_name;

    var input_type_getter = "value";
    if (input_element.type == "checkbox") input_type_getter = "checked";

    input_element.input_type_getter = input_type_getter;

    input_element[input_type_getter] = current_mode[_element.arg_name];

    if (input_element.type == "color" || input_element.type == "range") {
        input_element.addEventListener("input", (event) => {
            if (!sentStreamEvent) {
                console.log("starting stream event");

                SendJson(new espEvent(OPEN_STREAM, event.target.mode_arg_name));

                sentStreamEvent = true;
            }
            sendStream(event.target.value);
        });

        input_element.addEventListener("change", EndStream);
    }

    input_element.addEventListener("change", (event) => {
        current_mode[event.target.mode_arg_name] =
            event.target[event.target.input_type_getter];

        console.log(JSON.stringify(current_mode));
        SendJson(current_mode);
    });

    return input_element;
}
