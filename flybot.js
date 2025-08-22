let flybotRoot = "";

let state = {
    rollDegrees: 0.0,
    pitchDegrees: 0.0,
    yawDegrees: 0.0,
    throttlePercent: 0.0,
    rcRollDegrees: 0.0,
    rcPitchDegrees: 0.0,
    armed: false,
};

const rad2deg = 180 / Math.PI;

//
// COMMUNICATIONS
//
class FlySocket {
    constructor() {
        this.ws = null;
        this.wsConnected = false;
    }
    start() {
        this.ws = new WebSocket(`ws://${flybotRoot}/ws`);
        this.wsConnected = false;
        this.ws.onopen = () => {
            console.log("WebSocket connected");
            this.wsConnected = true;
        };
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            if (data.type === "state") {
                state.rollDegrees = data.mr * rad2deg;
                state.pitchDegrees = data.mp * rad2deg;
                state.yawDegrees = data.my * rad2deg;
                state.rcRollDegrees = data.rr * rad2deg;
                state.rcPitchDegrees = data.rp * rad2deg;
                state.throttlePercent = data.rt * 100.0;
                state.armed = data.a;
                drawAll();
            }
        };
        this.ws.onclose = () => {
            this.wsConnected = false;
            console.log("WebSocket closed");
        };
        this.ws.onerror = (error) => {
            console.log("WebSocket error: " + error);
        };
    }
    send(message) {
        if (this.wsConnected) {
            this.ws.send(message);
        } else {
            console.error("WebSocket not connected, cannot send message: " + message);
        }
    }
}
let flySocket = new FlySocket();

function sendCommand() {
    const command = document.getElementById("command").value;
    flySocket.send("command " + command);
}
function getState() {
    flySocket.send("state");
}

//
// HUD
//
function drawAll() {
    const $hud = document.getElementById("hud");
    if (!$hud) return;
    const ctx = $hud.getContext("2d");
    const width = $hud.width;
    const height = $hud.height;
    drawHUD(ctx, 0, 0, width, height);
}

function drawHUD(ctx, x, y, width, height) {
    ctx.font = "16px sans-serif";

    const cx = x + width / 2;
    const cy = y + height / 2;
    ctx.clearRect(0, 0, width, height);
    const rx = Math.max(width, height) * 10;
    const ry = Math.max(width, height) * 2;

    ctx.save();
    
    ctx.translate(cx, cy);
    ctx.rotate(state.rollDegrees * Math.PI / 180);
    ctx.translate(-cx, -cy);
    // Calculate the pitch vertical translation
    const pitchPxPerDeg = (height / 2) / 35;
    const pitchOffset = state.pitchDegrees * pitchPxPerDeg;
    ctx.translate(0, pitchOffset);
    // Draw the sky
    const skyGradient = ctx.createLinearGradient(cx, cy - height/2, cx, cy);
    skyGradient.addColorStop(0, "#3869F8");
    skyGradient.addColorStop(1, "#76E6FD");
    ctx.fillStyle = skyGradient;
    ctx.fillRect(cx-rx/2, cy-ry, rx, ry);
    // Draw the brown ground
    const groundGradient = ctx.createLinearGradient(cx, cy, cx, cy + height/2);
    groundGradient.addColorStop(0, "#C88C30");
    groundGradient.addColorStop(1, "#984428");
    ctx.fillStyle = groundGradient;
    ctx.fillRect(cx-rx/2, cy, rx, ry);
    // Draw the horizon line
    ctx.strokeStyle = "#FFFFFF";
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(cx - rx / 2, cy);
    ctx.lineTo(cx + rx / 2, cy);
    ctx.stroke();
    // Draw pitch lines
    ctx.translate(0, -pitchOffset);
    ctx.lineWidth = 1;
    const pitchLineLen = width * 0.25;
    for (let i = -50; i <= 50; i += 5) {
        const pitchY = cy + pitchPxPerDeg * i;
        const alpha = 1.0 - Math.max(0.0, Math.min(1.0, Math.abs(state.pitchDegrees - i) / 30));
        ctx.strokeStyle = "rgba(255, 255, 255, " + alpha + ")";
        ctx.fillStyle = "rgba(255, 255, 255, " + alpha + ")";
        ctx.beginPath();
        let len = (i % 10 === 0) ? pitchLineLen : pitchLineLen * 0.5;
        ctx.moveTo(cx - len / 2, pitchY);
        ctx.lineTo(cx + len / 2, pitchY);
        ctx.stroke();
        if (i % 10 === 0) {
            // ctx.fillText(i + "°", cx - len / 2 - 30, pitchY + 5);
            ctx.fillText(i + "°", cx + len / 2 + 5, pitchY + 5);
        }
    }
    ctx.restore();

    //
    // Draw the yaw indicator
    //
    const yawHeight = height * 0.1;
    const yawPxPerDeg = (width / 2) / 45;
    // yaw_x = yawPxPerDeg * deg + yawOffset
    // cx = yawPxPerDeg * yawDegrees + yawOffset
    const yawCompass = -state.yawDegrees;
    const yawOffset = cx - (yawCompass * yawPxPerDeg);
    const yawMinDeg = Math.floor((yawCompass - 110) / 10) * 10;
    ctx.fillStyle = "rgba(0, 0, 0, 0.5)";
    ctx.fillRect(0, 0, width, yawHeight);
    for (let i = yawMinDeg; i <= yawMinDeg + 220; i += 2) {
        const yawX = yawPxPerDeg * i + yawOffset;
        const dist = Math.abs(i - yawCompass);
        const alpha = 1.0 - Math.max(0.0, Math.min(1.0, dist / 45));
        ctx.strokeStyle = "rgba(255, 255, 255, " + alpha + ")";
        ctx.fillStyle = "rgba(255, 255, 255, " + alpha + ")";
        ctx.lineWidth = (i % 10 === 0) ? 3 : 2;
        ctx.beginPath();
        ctx.moveTo(yawX, 0);
        ctx.lineTo(yawX, (i % 10 === 0) ? 12 : 6);
        ctx.stroke();
        if (i % 10 === 0) {
            let displayI = i;
            while (displayI < 0) {
                displayI += 360;
            }
            while (displayI >= 360) {
                displayI -= 360;
            }
            let text = displayI + "°";
            if (displayI === 0 || displayI === 360) {
                text = "N";
            }
            else if (displayI === 45) {
                text = "NE";
            }
            else if (displayI === 90) {
                text = "E";
            }
            else if (displayI === 135) {
                text = "SE";
            }
            else if (displayI === 180) {
                text = "S";
            }
            else if (displayI === 225) {
                text = "SW";
            }
            else if (displayI === 270) {
                text = "W";
            }
            else if (displayI === 315) {
                text = "NW";
            }
            const textWidth = ctx.measureText(text).width;
            ctx.fillText(text, yawX - textWidth / 2, yawHeight - 5);
        }
    }
    let displayI = yawCompass;
    while (displayI < 0) {
        displayI += 360;
    }
    while (displayI >= 360) {
        displayI -= 360;
    }
    const yawText = displayI.toFixed(0) + "°";
    const yawTextWidth = ctx.measureText(yawText).width;
    ctx.fillStyle = "rgba(0, 0, 0, 1.0)";
    ctx.fillRect(cx - yawTextWidth / 2 - 5, yawHeight - 20, yawTextWidth + 10, 20);
    ctx.fillStyle = "#FFFFFF";
    ctx.fillText(yawText, cx - yawTextWidth / 2, yawHeight - 5);
    ctx.strokeStyle = "rgba(255, 255, 255, 1)";
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(x, yawHeight);
    ctx.lineTo(x + width, yawHeight);
    ctx.stroke();

    //
    // Draw command reticle
    //
    ctx.save();
    
    ctx.translate(cx, cy);
    ctx.rotate(state.rcRollDegrees * Math.PI / 180);
    ctx.translate(-cx, -cy);
    // Calculate the pitch vertical translation
    const rcPitchOffset = state.rcPitchDegrees * pitchPxPerDeg;
    ctx.translate(0, rcPitchOffset);
    const commandThickness = 10;
    const commandWidth = width * 0.175;
    const commandGap = width * 0.075;
    ctx.fillStyle = "#FCFF49";
    ctx.strokeStyle = "#000000";
    ctx.lineWidth = 0.5;
    const addReticle = (reflection) => {
        ctx.beginPath();
        ctx.moveTo(cx + reflection*(commandGap), cy - commandThickness/2);
        ctx.lineTo(cx + reflection*(commandGap + commandWidth), cy - commandThickness/2);
        ctx.lineTo(cx + reflection*(commandGap + commandWidth), cy + commandThickness/2);
        ctx.lineTo(cx + reflection*(commandGap + commandThickness), cy + commandThickness/2);
        ctx.lineTo(cx + reflection*(commandGap + commandThickness), cy + commandThickness/2 + commandThickness);
        ctx.lineTo(cx + reflection*(commandGap), cy + commandThickness/2 + commandThickness);
        ctx.closePath();
    };
    for (let reflection = -1; reflection <= 1; reflection += 2) {
        addReticle(reflection);
        ctx.shadowOffsetX = 0;
        ctx.shadowOffsetY = 0;
        ctx.shadowBlur = 10;
        ctx.shadowColor = "rgba(0, 0, 0, 0.25)";
        ctx.fill();
        addReticle(reflection);
        ctx.shadowColor = "rgba(0, 0, 0, 0)";
        ctx.stroke();
    }
    ctx.shadowOffsetX = 0;
    ctx.shadowOffsetY = 0;
    ctx.shadowBlur = 10;
    ctx.shadowColor = "rgba(0, 0, 0, 0.25)";
    ctx.fillRect(cx - commandThickness / 2, cy - commandThickness / 2, commandThickness, commandThickness);
    ctx.shadowColor = "rgba(0, 0, 0, 0)";
    ctx.strokeRect(cx - commandThickness / 2, cy - commandThickness / 2, commandThickness, commandThickness);

    ctx.restore();

    ctx.fillStyle = "#000000";
    ctx.font = "16px sans-serif";
    ctx.fillText("Roll: " + state.rollDegrees.toFixed(1) + "°", 10, 60);
    ctx.fillText("Throttle: " + state.throttlePercent.toFixed(1) + "%", 10, 80);
    ctx.fillText("Armed: " + (state.armed ? "Yes" : "No"), 10, 100);
    
}

function flybotStart(root) {
    flybotRoot = root;
    flySocket.start();
    // Initial draw
    drawAll();
    // Update state at 2Hz
    setInterval(() => {
        getState();
    }, 1000/10);
}

//
// CONFIG
//

let config = {
    "numMotors": 0
};
const $motorUIs = {};

function buildConfigUI() {
    const $config = document.getElementById("config");
    $config.innerHTML = `
        <label for="numMotors">Num Motors:</label>
        <input type="number" id="numMotors" min="1" max="6" value="${config.numMotors}" onchange="updateMotorConfig('numMotors', this.value)">
    `;
    for (let i = 1; i <= 6; i++) {
        const $motorUI = buildMotorConfigUI(i);
        $motorUIs[i] = $motorUI;
        $config.appendChild($motorUI);
    }
    updateConfigUI();
}

function updateConfigUI() {
    for (let i = 1; i <= 6; i++) {
        const $motorUI = $motorUIs[i];
        $motorUI.style.display = (i <= config.numMotors) ? "block" : "none";
    }
}

function buildMotorConfigUI(motorId) {
    const $motorUI = document.createElement("div");
    const key = `motor${motorId}`;
    $motorUI.classList.add("motor-config");
    $motorUI.innerHTML = `
        <div id="${key}-config">
        <h3>Motor ${motorId}</h3>
        <label for="${key}-x">X:</label>
        <input type="number" id="${key}-x" value="${config[key+".x"]}" onchange="updateMotorConfig('${key}.x', this.value)">&nbsp;mm
        <label for="${key}-y">Y:</label>
        <input type="number" id="${key}-y" value="${config[key+".y"]}" onchange="updateMotorConfig('${key}.y', this.value)">&nbsp;mm
        </div>
    `;
    return $motorUI;
}

function updateMotorConfig(key, value) {
    config[key] = parseFloat(value);
    console.log("Updating motor config", {key, value});
    const urlParams = new URLSearchParams();
    urlParams.append("key", key);
    urlParams.append("value", value);
    const url = "config_value?" + urlParams.toString();
    fetch(url, {
        method: "POST",
        body: ""
    })
    .then(response => {
        if (!response.ok) {
            throw new Error("Network response was not ok");
        }
        return response.json();
    })
    .then(data => {
        console.log("Config updated successfully:", data);
        updateConfigUI();
    })
    .catch(error => {
        console.error("Error updating config:", error);
    });
}


function flybotConfigStart() {
    fetch("config.json")
    .then(response => response.json())
    .then(data => {
        console.log("Config data:", data);
        config = data;
        buildConfigUI();
        // const configElement = document.getElementById("raw_config");
        // configElement.textContent = JSON.stringify(data, null, 2);
    })
    .catch(error => {
        console.error("Error fetching config.json:", error);
    });
}
