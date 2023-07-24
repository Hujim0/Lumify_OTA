var gotPreferences = false;

const OPEN_STREAM = "open_stream";

const ON_GET_ELEMENTS_EVENT = "onGetModeElements";
const ON_GET_PREFERENCES_EVENT = "onGetPreferences";

class ModeList {
    eventSource = new EventSource("/events");

    list_container = document.getElementById("list-container");
    list_template = document.getElementById("list-template");
    skeleton_template = document.getElementById("skeleton-template");

    localization = null;

    current_mode = null;
    current_mode_id = 0;
    current_elements = null;

    current_info_inputs = [];

    debug = true;
    save_modes = true;

    mode_carousel_elements = document.getElementById("mode_list").children;

    onModeChanged = (event) => {
        let id_and_args = event.data.split(",");
        this.current_mode_id = Number(id_and_args[0]);
        this.current_mode = JSON.parse(id_and_args[1]);

        sendGetRequest(
            "/elements?id=" + this.current_mode_id,
            (status, responseText) => {
                if (status != 200) return;

                this.CreateModeElements(JSON.parse(responseText));
            }
        );
    };

    constructor() {
        this.SetUpModeElements();
        this.CreateSkeleton();

        // this.eventSource.onmessage = function (event) {
        //     alert(event.data);
        // };
        // this.save_modes = true;

        this.eventSource.addEventListener("mode_changed", this.onModeChanged);
    }

    SetUpModeElements() {
        for (let i = 0; i < this.mode_carousel_elements.length; i++) {
            this.mode_carousel_elements[i].addEventListener(
                "input",
                (event) => {
                    modeList.GetMode(
                        event.target.getAttribute("mode-id"),
                        true,
                        true
                    );
                }
            );
        }

        sendGetRequest("/localization", (status, responseText) => {
            if (status == 200) {
                this.localization = JSON.parse(responseText);
            }
        });
    }

    CreateModeElements(_elements) {
        this.list_container.innerHTML = "";

        this.current_info_inputs = [];

        for (let i = 0; i < _elements.length; i++) {
            var list_element = new listElement().ConstructFull(
                this.list_template.content.cloneNode(true).children[0],
                _elements[i].name,
                _elements[i].desc,
                _elements[i].arg_name,
                _elements[i].input_props,
                this.current_mode[_elements[i].arg_name],
                this.current_info_inputs
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
                modeList.current_mode[event.target.json_arg_name] =
                    event.target[event.target.input_type_getter];

                console.log(JSON.stringify(modeList.current_mode));

                modeList.ChangeMode(
                    this.current_mode_id,
                    JSON.stringify(modeList.current_mode),
                    modeList.save_modes,
                    false
                );
            });

            this.list_container.appendChild(list_element.div);
        }
    }

    ChangeMode(id, args_string, save = true, update_ui = true) {
        let uri = "/mode?id=" + id;
        if (save && modeList.save_modes) {
            uri += "&save";
        }

        sendPostRequest(uri, args_string);

        if (!update_ui) return;

        this.setModesCarouselCheckedID(id);

        sendGetRequest("/elements?id=" + id, (status, responseText) => {
            if (status == 404) {
                alert("cant get args - file not found!");
                return;
            }
            modeList.current_elements = JSON.parse(responseText)["elems"];
            modeList.CreateModeElements(modeList.current_elements);
        });
    }

    setModesCarouselCheckedID(id) {
        this.mode_carousel_elements[id].children[0].checked = true;
    }

    //---------------------------------------------------------------------------

    GetMode(id, change = true, save = true, onLoaded = null) {
        this.CreateSkeleton();

        this.setModesCarouselCheckedID(id);

        this.current_mode_id = id;

        let loaded_first = "";

        let get_string = "/mode?id=" + id;

        if (change) {
            get_string += "&change";
        }
        if (save && this.save_modes) {
            get_string += "&save";
        }

        sendGetRequest(get_string, (status, responseText) => {
            if (status == 404) {
                alert("cant get args - file not found!");
                return;
            }
            modeList.current_mode = JSON.parse(responseText);
            // console.log(responseText);

            if (loaded_first == "") {
                loaded_first = "args";
            } else {
                modeList.CreateModeElements(modeList.current_elements);
                if (onLoaded != null) {
                    onLoaded(responseText);
                }
            }
        });

        sendGetRequest("/elements?id=" + id, (status, responseText) => {
            if (status == 404) {
                alert("cant get elems - file not found!");
                return;
            }
            // console.log(xhr_elems.responseText);
            modeList.current_elements = JSON.parse(responseText)["elems"];
            if (loaded_first == "") {
                loaded_first = "elems";
            } else {
                modeList.CreateModeElements(modeList.current_elements);
                if (onLoaded != null) {
                    onLoaded(responseText);
                }
            }
        });
    }

    CreateSkeleton() {
        this.list_container.innerHTML = "";
        let count = this.current_info_inputs.length;
        if (count == 0 || count == null) {
            count = 2;
        }

        for (let i = 0; i < count; i++) {
            this.list_container.appendChild(
                this.skeleton_template.content.cloneNode(true)
            );
        }
    }

    FormatModeName(id) {
        return this.localization.mode + this.localization.mode_list[id];
    }

    FormatModeArgs(elements, args) {
        let res = "";

        for (let i = 0; i < elements.length; i++) {
            if (elements[i].input_props[0] == "type=color") {
                res +=
                    elements[i].name +
                    ": " +
                    '<label style="color: ' +
                    args[elements[i].arg_name] +
                    ';">(!)</label>, ';
            } else if (elements[i].input_props[0] == "type=range") {
                let max = 0;

                for (let j = 1; j < elements[i].input_props.length; j++) {
                    let key_value_pair = elements[i].input_props[j].split("=");

                    if (key_value_pair[0] == "max") {
                        max = key_value_pair[1];
                        break;
                    }
                }

                res +=
                    elements[i].name +
                    ": " +
                    args[elements[i].arg_name] +
                    "/" +
                    max +
                    ", ";
            } else if (elements[i].input_props[0] == "type=checkbox") {
                res +=
                    elements[i].name +
                    ": " +
                    this.localization[String(args[elements[i].arg_name])] +
                    ", ";
            } else {
                res +=
                    elements[i].name + ": " + args[elements[i].arg_name] + ", ";
            }
        }

        return res.substring(0, res.length - 2);
    }
}

window.addEventListener("load", GetPreferences);
window.addEventListener("load", SendTime);

var modeList = new ModeList();

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

window.addEventListener(ON_GET_PREFERENCES_EVENT, (event) => {
    var mode_id = event.detail.mode;
    modeList.debug = event.detail.debug;

    modeList.GetMode(mode_id, false, false);
});

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
            this.div.children[3].innerText == null || //description label
            this.div.children[3].innerText == ""
        ) {
            this.info_button_label.hidden = true;
        } else {
            this.ConfigureInfoButton(
                this.info_button_label,
                this.div.children[3],
                info_inputs_array
            );
        }
        //input-----------------------------------------

        this.input_props = input_props;
        this.input_name = input_name;
        this.input_element = this.div.children[1];

        for (var i = 0; i < this.input_props.length; i++) {
            let key_value_pair = this.input_props[i].split("=");

            this.input_element[key_value_pair[0]] = key_value_pair[1];
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
        info_descriptions_array
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
            info_descriptions_array
        );
    }

    ConfigureInfoButton(
        info_button_label_element,
        description_element,
        info_descriptions_array,
        onToggle = null,
        props_to_checkbox = ""
    ) {
        info_button_label_element.children[0]["info_button_id"] =
            info_descriptions_array.length;

        if (props_to_checkbox != "") {
            let key_value_pair = props_to_checkbox.split("=");
            let prop = key_value_pair[0];
            let val = key_value_pair[1];

            info_button_label_element.children[0][prop] = val;
        }
        info_button_label_element.children[0].addEventListener(
            "input",
            (event) => {
                info_descriptions_array[
                    event.target["info_button_id"]
                ].classList.toggle("active");

                if (onToggle != null)
                    onToggle(
                        event,
                        info_descriptions_array[
                            event.target["info_button_id"]
                        ].classList.contains("active"),
                        info_descriptions_array[event.target["info_button_id"]]
                    );
            }
        );

        info_descriptions_array.push(description_element);
    }
}
