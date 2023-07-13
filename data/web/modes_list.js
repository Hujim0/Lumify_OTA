var current_mode = null;
var current_mode_id = 0;
var current_elements = null;

var gotPreferences = false;

const OPEN_STREAM = "open_stream";

const ON_GET_ELEMENTS_EVENT = "onGetModeElements";
const ON_GET_PREFERENCES_EVENT = "onGetPreferences";

var debug = true;

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

    const query_string =
        "/time?epoch=" +
        String(
            date.getHours() * 3600 + date.getMinutes() * 60 + date.getSeconds()
        ) +
        "&dayoftheweek=" +
        String(date.getDay());

    sendGetRequest(query_string);
}

//---------------------------------------------------------------------------

var modes = document.getElementById("mode_list").children;
for (var i = 0; i < modes.length; i++) {
    modes[i].addEventListener("input", (event) => {
        GetMode(event.target.getAttribute("mode-id"), true);
    });
}

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var mode_id = event.detail.mode;
    debug = event.detail.debug;

    setModesCarouselCheckedID(mode_id);
    GetMode(mode_id, false);
});

function setModesCarouselCheckedID(id) {
    modes[id].children[0].checked = true;
}

//---------------------------------------------------------------------------

var list_container = document.getElementById("list-container");
var list_template = document.getElementById("list-template");
var skeleton_template = document.getElementById("skeleton-template");

var loaded_first = "";

function ChangeMode(id) {
    // sendGetRequest("/mode?id=" + id + "&change");
}

function GetMode(id, change = true, save = true, onLoaded = null) {
    CreateSkeleton();

    current_mode_id = id;

    loaded_first = "";

    var get_string = "/mode?id=" + id;

    if (change) {
        get_string += "&change";
    }
    if (save) {
        get_string += "&save";
    }

    var xhr_args = sendGetRequest(get_string, () => {
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
            if (onLoaded != null) {
                onLoaded(xhr_args.responseText);
            }
        }
    });

    var xhr_elements = sendGetRequest("/elements?id=" + id, () => {
        if (xhr_elements.status == 404) {
            alert("cant get elems - file not found!");
            return;
        }
        // console.log(xhr_elems.responseText);
        current_elements = JSON.parse(xhr_elements.responseText)["elems"];
        if (loaded_first == "") {
            loaded_first = "elems";
        } else {
            CreateModeElements(current_elements);
            if (onLoaded != null) {
                onLoaded(xhr_args.responseText);
            }
        }
    });
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
        var list_element = new listElement().ConstructFull(
            list_template.content.cloneNode(true).children[0],
            _elements[i].name,
            _elements[i].desc,
            _elements[i].arg_name,
            _elements[i].input_props,
            current_mode[_elements[i].arg_name],
            current_info_inputs
        );

        if (
            list_element.input_element.type == "color" ||
            list_element.input_element.type == "range"
        ) {
            addStreamEvent(
                list_element.input_element,
                list_element.input_element.json_arg_name
            );
        }

        list_element.input_element.addEventListener("change", (event) => {
            current_mode[event.target.json_arg_name] =
                event.target[event.target.input_type_getter];

            if (debug) console.log();
            SendJson(current_mode);
            sendPostRequest(
                "/mode?id=" + current_mode_id + "&save",
                current_mode
            );
        });

        list_container.appendChild(list_element.div);
    }
}

class listElement {
    // <label>Parameter</label>
    // <input type="number" />
    // <label class="info-button">
    //     Info
    //     <input type="checkbox"></input>
    //     <i class="fas fa-circle-info"></i>
    // </label>
    // <label class="text-light">Description</label>

    SetInputValue(value) {
        this.input_element[this.input_element.input_type_getter] = value;
    }

    ConstructInput(
        div,
        input_name,
        input_props,
        input_value,
        info_inputs_array
    ) {
        this.div = div;

        this.info_button_label = this.div.children[2];

        //description---------------------------------
        if (
            this.div.children[3].innerText == null ||
            this.div.children[3].innerText == ""
        ) {
            this.info_button_label.hidden = true;
        } else {
            this.info_button_label.children[0]["arg_id"] =
                info_inputs_array.length;
            this.info_button_label.children[0].addEventListener(
                "input",
                (event) => {
                    info_inputs_array[event.target["arg_id"]].classList.toggle(
                        "active"
                    );
                }
            );

            info_inputs_array.push(this.div.children[3]);
            //description label
        }
        //input-----------------------------------------

        this.input_props = input_props;
        this.input_name = input_name;
        this.input_element = this.div.children[1];

        for (var i = 0; i < this.input_props.length; i++) {
            var attribute = this.input_props[i].split("=");

            this.input_element[attribute[0]] = attribute[1];
        }

        this.input_element.json_arg_name = this.input_name;

        var input_type_getter = "value";
        if (this.input_element.type == "checkbox") {
            input_type_getter = "checked";
        }

        this.input_element.input_type_getter = input_type_getter;

        this.input_element[input_type_getter] = input_value;

        return this;
    }

    ConstructFull(
        div,
        setting_name,
        setting_description,
        input_name,
        input_props,
        input_value,
        info_inputs_array
    ) {
        this.setting_name = setting_name;

        div.children[0].innerText = this.setting_name;
        div.children[3].innerText = setting_description;
        this.setting_description = setting_description;

        return this.ConstructInput(
            div,
            input_name,
            input_props,
            input_value,
            info_inputs_array
        );
    }
}
