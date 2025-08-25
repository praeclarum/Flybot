let flybotRoot = "";

let state = {
    rollDegrees: 0.0,
    pitchDegrees: 0.0,
    yawDegrees: 0.0,
    throttlePercent: 0.0,
    rcRollDegrees: 0.0,
    rcPitchDegrees: 0.0,
    armed: false,
    errorPitchDegrees: 0.0,
    errorRollDegrees: 0.0,
    motor1Command: 0.0,
    motor2Command: 0.0,
    motor3Command: 0.0,
    motor4Command: 0.0,
    motor5Command: 0.0,
    motor6Command: 0.0
};

let config = {
    numMotors: 0
};
let configDefaults = {
    numMotors: 0
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
                state.flightStatus = data.fs;
                state.hardwareFlags = data.hf;
                state.errorPitchDegrees = data.ep * rad2deg;
                state.errorRollDegrees = data.er * rad2deg;
                state.motor1Command = data.m1;
                state.motor2Command = data.m2;
                state.motor3Command = data.m3;
                state.motor4Command = data.m4;
                state.motor5Command = data.m5;
                state.motor6Command = data.m6;
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
            // console.error("WebSocket not connected, cannot send message: " + message);
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
    const font = "sans-serif";
    ctx.font = `16px ${font}`;

    const cx = x + width / 2;
    const cy = y + height / 2;
    ctx.clearRect(0, 0, width, height);
    const rx = Math.max(width, height) * 10;
    const ry = Math.max(width, height) * 2;

    ctx.save();
    
    ctx.translate(cx, cy);
    ctx.rotate(-state.rollDegrees * Math.PI / 180);
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
    const yawHeight = height * 0.075;
    ctx.font = `${yawHeight*0.5}px ${font}`;
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
        ctx.lineTo(yawX, (i % 10 === 0) ? yawHeight*0.5 : yawHeight*0.25);
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
    const yawTextSize = ctx.measureText(yawText);
    const yawTextWidth = yawTextSize.width;
    ctx.fillStyle = "rgba(0, 0, 0, 1.0)";
    ctx.fillRect(cx - yawTextWidth / 2 - 5, yawHeight - 10 - yawTextSize.actualBoundingBoxAscent, yawTextWidth + 10, yawTextSize.actualBoundingBoxAscent + 10);
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
    ctx.rotate(-state.rcRollDegrees * Math.PI / 180);
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

    //
    // Draw motor commands
    //
    const droneSize = height * 0.25;
    const droneX = x + width - droneSize;
    const droneY = y + height - droneSize;
    drawDrone(ctx, droneX, droneY, droneSize, droneSize);

    ctx.fillStyle = "#000000";
    ctx.font = "16px sans-serif";
    ctx.fillText("Pitch: " + state.pitchDegrees.toFixed(1) + "°", 10, 60);
    ctx.fillText("Roll: " + state.rollDegrees.toFixed(1) + "°", 10, 80);
    ctx.fillText("Error Pitch: " + state.errorPitchDegrees.toFixed(1) + "°", 10, 100);
    ctx.fillText("Error Roll: " + state.errorRollDegrees.toFixed(1) + "°", 10, 120);
    ctx.fillText("Throttle: " + state.throttlePercent.toFixed(1) + "%", 10, 140);
    ctx.fillText("Flight Status: " + flightStatusToString(state.flightStatus), 10, 160);
    ctx.fillText("Hardware Flags: " + hardwareFlagsToString(state.hardwareFlags), 10, 180);
    ctx.fillText("Error: " + getFlightErrorMessage(), 10, 200);
}

const FS_Disarmed = 0;
const FS_Arming = 1;
const FS_Landed = 2;
const FS_Flying = 3;

function flightStatusToString(status) {
    switch (status) {
        case FS_Disarmed:
            return "Disarmed";
        case FS_Arming:
            return "Arming";
        case FS_Landed:
            return "Landed";
        case FS_Flying:
            return "Flying";
        default:
            return "Unknown";
    }
}

const HF_MPU_OK = 1 << 0;
const HF_RC_OK = 1 << 1;

function hardwareFlagsToString(flags) {
    let parts = [];
    const report = (f, n) => {
        if (flags & f) {
            parts.push("+" + n);
        }
        else {
            parts.push("-" + n);
        }
    };
    report(HF_RC_OK, "RC");
    report(HF_MPU_OK, "MPU");
    return parts.join(", ");
}

function getFlightErrorMessage() {
    const hf = state.hardwareFlags;
    // Is there a hardware failure?
    if (!(hf & HF_RC_OK)) {
        return "NO RC Signal";
    }
    if (!(hf & HF_MPU_OK)) {
        return "MPU Failure";
    }
    // Are we disarmed?
    if (state.flightStatus === FS_Disarmed) {
        return "Disarmed";
    }
    if (state.flightStatus === FS_Arming) {
        return "Arming";
    }
    return "";
}

function drawDrone(ctx, x, y, width, height) {
    const dim = Math.min(width, height);
    let maxXMM = 0;
    let maxYMM = 0;
    for (let i = 1; i <= config.numMotors; i++) {
        const key = `motor${i}`;
        const motorXMM = config[key + ".x"];
        const motorYMM = -config[key + ".y"];
        maxXMM = Math.max(maxXMM, Math.abs(motorXMM));
        maxYMM = Math.max(maxYMM, Math.abs(motorYMM));
    }
    if (maxXMM < 0.1 || maxYMM < 0.1) {
        return;
    }
    const motorRadius = width * 0.1;
    const padding = 16;
    const pxPerMMX = 0.4 * (width - 2*padding) / maxXMM;
    const pxPerMMY = 0.4 * (height - 2*padding) / maxYMM;
    const pxPerMM = Math.min(pxPerMMX, pxPerMMY);
    const thickness = Math.max(1, dim * 0.02);
    const cx = x + width / 2;
    const cy = y + height / 2;
    ctx.lineWidth = thickness;
    ctx.fillStyle = "rgba(255, 255, 255, 0.75)";
    ctx.font = `${motorRadius}px sans-serif`;
    for (let i = 1; i <= config.numMotors; i++) {
        const motorDir = config[`motor${i}.direction`];
        const key = `motor${i}`;
        const motorX = config[key + ".x"] * pxPerMM + cx;
        const motorY = -config[key + ".y"] * pxPerMM + cy;
        const motorCommand = state[`${key}Command`];
        if (motorCommand >= 0.0) {
            ctx.fillStyle = `rgba(0, 255, 0, ${motorCommand})`;
        }
        else {
            ctx.fillStyle = `rgba(255, 0, 0, ${-motorCommand})`;
        }
        ctx.beginPath();
        ctx.arc(motorX, motorY, motorRadius, 0, 2 * Math.PI);
        ctx.fill();
        ctx.strokeStyle = motorDir > 0 ? "#0F0" : "#F00";
        ctx.beginPath();
        ctx.arc(motorX, motorY, motorRadius, 0, 2 * Math.PI);
        ctx.stroke();
        const label = `M${i}`;
        const textSize = ctx.measureText(label);
        ctx.fillStyle = "rgba(255, 255, 255, 0.75)";
        ctx.fillText(label, motorX - textSize.width / 2, motorY + textSize.actualBoundingBoxAscent / 2);
        const value = motorCommand.toFixed(2);
        const valueSize = ctx.measureText(value);
        ctx.fillStyle = "rgba(255, 255, 255, 0.75)";
        ctx.fillText(value, motorX - valueSize.width / 2, motorY + motorRadius + thickness*2 + valueSize.actualBoundingBoxAscent);
    }
}

function flybotStart(root) {
    flybotRoot = root;
    flySocket.start();
    // Get config
    fetch("config.json")
    .then(response => response.json())
    .then(data => {
        console.log("Config data:", data);
        config = data;
    })
    .catch(error => {
        console.error("Error fetching config.json:", error);
    });

    // Initial draw
    const $hud = document.getElementById("hud");
    $hud.width = 800;
    $hud.height = 600;
    $hud.style.width = "800px";
    $hud.style.height = "600px";
    drawAll();
    // Update state at 2Hz
    setInterval(() => {
        getState();
    }, 1000/10);
}

//
// CONFIG
//

const $motorUIs = {};

function buildConfigUI() {
    const $config = document.getElementById("config");
    $config.innerHTML = `
        <div><canvas id="droneCanvas" width="400" height="400"></canvas></div>
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

function drawConfig() {
    const canvas = document.getElementById("droneCanvas");
    const ctx = canvas.getContext("2d");
    const width = canvas.width;
    const height = canvas.height;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = "#000000";
    ctx.fillRect(0, 0, width, height);
    drawDrone(ctx, 0, 0, width, height);
}

function updateConfigUI() {
    for (let i = 1; i <= 6; i++) {
        const $motorUI = $motorUIs[i];
        $motorUI.style.display = (i <= config.numMotors) ? "block" : "none";
    }
    drawConfig();
}

function buildMotorConfigUI(motorId) {
    const $motorUI = document.createElement("div");
    const key = `motor${motorId}`;
    $motorUI.classList.add("motor-config");
    $motorUI.innerHTML = `
        <div id="${key}-config">
        <h3>Motor ${motorId}</h3>
        <table>
        <tr>
        <th><label for="${key}-x">X</label></th>
        <td><input type="number" id="${key}-x" value="${config[key+".x"]}" onchange="updateMotorConfig('${key}.x', this.value)">&nbsp;mm</td>
        <th><label for="${key}-y" style="margin-left: 1em;">Y</label></th>
        <td><input type="number" id="${key}-y" value="${config[key+".y"]}" onchange="updateMotorConfig('${key}.y', this.value)">&nbsp;mm</td>
        </tr>
        <tr>
        <th><label for="${key}-direction">Dir</label></th>
        <td><input type="number" id="${key}-direction" min="-1" max="1" value="${config[key+".direction"]}" onchange="updateMotorConfig('${key}.direction', this.value)"></td>
        </tr>
        </table>
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

function buildConfigDefaultsUI() {
    const keys = Object.keys(config).sort();
    const $config = document.getElementById("config");
    const $defaults = document.createElement("div");
    $defaults.id = "config-defaults";
    $defaults.innerHTML = `
    <h2>Config Defaults</h2>
    <table id="config-defaults-table">
    <tr>
    <th>Key</th>
    <th style="text-align: right;">Value</th>
    <th style="text-align: right;">Default Value</th>
    <th>Restore</th>
    </tr>
    </table>
    `;
    const $table = $defaults.querySelector("#config-defaults-table");
    $config.appendChild($defaults);
    keys.forEach(key => {
        const $row = document.createElement("tr");
        const display = configDefaults[key] === config[key] ? "none" : "inline";
        $row.innerHTML = `
        <td>${key}</td>
        <td style="text-align: right;">${config[key]}</td>
        <td style="text-align: right;">${configDefaults[key]}</td>
        <td><button onclick="restoreConfig('${key}')" style="display: ${display};">Restore</button></td>
        `;
        $table.appendChild($row);
    });
}

function restoreConfig(key) {
    console.log("Restoring config", key);
    const urlParams = new URLSearchParams();
    urlParams.append("key", key);
    const url = "config_restore?" + urlParams.toString();
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
        updateConfigDefaultsUI();
    })
    .catch(error => {
        console.error("Error updating config:", error);
    });
}

function updateConfigDefaultsUI() {
    // Delete old UI
    const $defaults = document.getElementById("config-defaults");
    if ($defaults) {
        $defaults.remove();
    }
    fetch("config.json")
    .then(response => response.json())
    .then(data => {
        console.log("Config data:", data);
        config = data;
        buildConfigDefaultsUI();
    })
    .catch(error => {
        console.error("Error fetching config.json:", error);
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

    fetch("config_defaults.json")
    .then(response => response.json())
    .then(data => {
        console.log("Config default data:", data);
        configDefaults = data;
        buildConfigDefaultsUI();
    })
    .catch(error => {
        console.error("Error fetching config_defaults.json:", error);
    });
}
