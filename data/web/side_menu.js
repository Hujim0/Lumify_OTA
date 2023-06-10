const menu_bars = document.getElementById("menu_bars");
const menu_cross = document.getElementById("menu_cross");
const navbar = document.getElementById("navbar");

menu_bars.addEventListener("click", () => {
    navbar.style.width = "250px";
    navbar.style.transition = ".25s ease-out"
});

menu_cross.addEventListener("click", () => {
    navbar.style.width = "0px";
    navbar.style.transition = ".15s ease-in"
});