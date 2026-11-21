
const dot = document.querySelector(".logo-dot");

let updating = false;
let online = false;

const updateButton = document.getElementById("update-now");

updateButton.addEventListener("click", () => {
    sentUpdateInfos();
});

async function sentUpdateInfos() {
    const project = Number(
        document.getElementById("project").value
    );

    const mode = Number(
        document.getElementById("update-mode").value
    );

    const updateInfos = {
        project: project,
        mode: mode
    };

    console.log("Update-Anfrage:", updateInfos);

try {
    const response = await fetch("/update", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(updateInfos)
    });

    const data = await response.text();

    console.log("HTTP-Status:", response.status);
    console.log("Serverantwort:", data);

} catch (error) {
    console.error("Update-Anfrage fehlgeschlagen:", error);
}
}

async function loop() {
    if (updating || !dot) {
        return;
    }

    updating = true;

    try {
        const response = await fetch("/api/status", {
            cache: "no-store"
        });

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const data = await response.text();

        online = data.trim() === "true";

        dot.style.backgroundColor = online ? "green" : "red";

        console.log("Server online:", online);



    } catch (error) {
        online = false;
        dot.style.backgroundColor = "red";

        console.error("Statusabfrage fehlgeschlagen:", error);

    } finally {
        updating = false;
    }
}

setInterval(loop, 1000);
loop();
