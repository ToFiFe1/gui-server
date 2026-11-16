const add = document.getElementById("add");
const sub = document.getElementById("sub");
const number = document.getElementById("number");

const stop = document.getElementById("stop");
const numberStop = document.getElementById("numberStop");

let updating = false;

// -------------------------
// GET CURRENT VALUE
// -------------------------

function updateNumber() {


if (updating) {
    return;
}

updating = true;

fetch("/api/getx", {
    cache: "no-store"
})
    .then(response => {

        if (!response.ok) {
            throw new Error("HTTP " + response.status);
        }

        return response.text();
    })
    .then(data => {

        console.log("SERVER:", data);

        number.textContent = data;
    })
    .catch(error => {

        console.error("GETX ERROR:", error);

    })
    .finally(() => {

        updating = false;

    });


}

// -------------------------
// ADD
// -------------------------

add.addEventListener("click", () => {


fetch("/api/add", {
    cache: "no-store"
})
    .then(response => response.text())
    .then(data => {

        console.log("ADD:", data);

        updateNumber();

    })
    .catch(error => {

        console.error("ADD ERROR:", error);

    });


});

// -------------------------
// SUB
// -------------------------

sub.addEventListener("click", () => {


fetch("/api/sub", {
    cache: "no-store"
})
    .then(response => response.text())
    .then(data => {

        console.log("SUB:", data);

        updateNumber();

    })
    .catch(error => {

        console.error("SUB ERROR:", error);

    });


});

// -------------------------
// STOPWATCH
// -------------------------

stop.addEventListener("click", () => {


fetch("/api/TrigStop", {
    cache: "no-store"
})
    .then(response => response.text())
    .then(data => {

        console.log("STOPWATCH:", data);

        numberStop.textContent = data;

    })
    .catch(error => {

        console.error("STOPWATCH ERROR:", error);

    });


});


setInterval(updateNumber, 50);

updateNumber()