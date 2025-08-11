let flybotRoot = "";

let state = {
    rollDegrees: 0.0,
    pitchDegrees: 0.0,
    yawDegrees: 0.0,
    throttlePercent: 0.0,
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
            console.log("WebSocket not connected, cannot send message: " + message);
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

    ctx.fillStyle = "#000000";
    ctx.font = "16px Arial";
    ctx.fillText("Roll: " + state.rollDegrees.toFixed(1) + "°", 10, 40);
    ctx.fillText("Yaw: " + state.yawDegrees.toFixed(1) + "°", 10, 60);
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


