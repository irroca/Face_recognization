// ============================================================
// webrtc-client.js — WebSocket 帧传输客户端
// ============================================================
// 功能：
//   1. 获取本地摄像头视频流
//   2. 定时捕获帧 → JPEG → 发送到 Python 服务器
//   3. 接收处理后的帧渲染到 canvas
//   4. 接收识别事件显示为通知
// ============================================================

const WS_URL = 'ws://localhost:8765';
const CAPTURE_FPS = 5;  // 帧率（每秒发送帧数，降低以减少管道压力）

let localStream = null;
let ws = null;
let isConnected = false;
let captureInterval = null;
let captureCanvas = null;
let captureCtx = null;

// ============================================================
// 获取本地摄像头
// ============================================================
async function startLocalCamera() {
    try {
        localStream = await navigator.mediaDevices.getUserMedia({
            video: { width: 640, height: 480, frameRate: 30 },
            audio: false
        });
        document.getElementById('localVideo').srcObject = localStream;
        console.log('[Camera] Started');
    } catch (err) {
        console.error('[Camera] Failed:', err);
        updateStatus('disconnected', '摄像头访问失败');
    }
}

// ============================================================
// 连接 WebSocket 服务器并开始帧传输
// ============================================================
function connectToServer() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        return;
    }

    updateStatus('connecting', '连接中...');
    ws = new WebSocket(WS_URL);
    ws.binaryType = 'arraybuffer';

    ws.onopen = () => {
        console.log('[WS] Connected');
        updateStatus('connected', '已连接 - 视频处理中');
        isConnected = true;
        startFrameCapture();
    };

    ws.onmessage = (event) => {
        if (event.data instanceof ArrayBuffer) {
            // 二进制消息 = 处理后的 JPEG 帧
            renderProcessedFrame(event.data);
        } else {
            // 文本消息 = JSON 事件通知
            try {
                const msg = JSON.parse(event.data);
                if (msg.type === 'notification') {
                    addNotification(msg.data);
                }
            } catch (e) {
                console.warn('[WS] Invalid text message');
            }
        }
    };

    ws.onclose = () => {
        console.log('[WS] Disconnected');
        updateStatus('disconnected', '已断开');
        isConnected = false;
        stopFrameCapture();
    };

    ws.onerror = (err) => {
        console.error('[WS] Error:', err);
        updateStatus('disconnected', '连接失败');
    };
}

// ============================================================
// 帧捕获和发送
// ============================================================
function startFrameCapture() {
    if (captureInterval) return;

    // 创建离屏 canvas 用于帧捕获
    captureCanvas = document.createElement('canvas');
    captureCanvas.width = 640;
    captureCanvas.height = 480;
    captureCtx = captureCanvas.getContext('2d');

    const localVideo = document.getElementById('localVideo');

    captureInterval = setInterval(() => {
        if (!ws || ws.readyState !== WebSocket.OPEN) return;
        if (!localVideo.videoWidth) return;  // 视频尚未就绪

        // 从 video 元素捕获当前帧到 canvas
        captureCtx.drawImage(localVideo, 0, 0, 640, 480);

        // 将 canvas 内容编码为 JPEG blob
        captureCanvas.toBlob(blob => {
            if (blob && ws && ws.readyState === WebSocket.OPEN) {
                blob.arrayBuffer().then(buffer => {
                    ws.send(buffer);
                });
            }
        }, 'image/jpeg', 0.75);

    }, 1000 / CAPTURE_FPS);

    console.log(`[Capture] Started at ${CAPTURE_FPS} FPS`);
}

function stopFrameCapture() {
    if (captureInterval) {
        clearInterval(captureInterval);
        captureInterval = null;
        console.log('[Capture] Stopped');
    }
}

// ============================================================
// 渲染处理后的帧到右侧 canvas
// ============================================================
function renderProcessedFrame(arrayBuffer) {
    const blob = new Blob([arrayBuffer], { type: 'image/jpeg' });
    const url = URL.createObjectURL(blob);
    const img = new Image();

    img.onload = () => {
        const canvas = document.getElementById('remoteCanvas');
        const ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
        URL.revokeObjectURL(url);
        frameCount++;  // FPS 计数（ui.js 中定义）
    };

    img.src = url;
}

// ============================================================
// 连接/断开切换
// ============================================================
async function toggleConnection() {
    if (isConnected) {
        disconnect();
    } else {
        if (!localStream) {
            await startLocalCamera();
        }
        connectToServer();
    }
}

function disconnect() {
    stopFrameCapture();
    if (ws) {
        ws.close();
        ws = null;
    }
    isConnected = false;
    updateStatus('disconnected', '已断开');
}

// ============================================================
// 算法切换
// ============================================================
function switchAlgorithm() {
    const detector = document.getElementById('detectorSelect').value;
    const recognizer = document.getElementById('recognizerSelect').value;

    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            type: 'switch_algorithm',
            data: { detector, recognizer }
        }));
        console.log(`[Control] Switch: detector=${detector}, recognizer=${recognizer}`);
    }
}

// 页面加载时自动启动摄像头预览
window.addEventListener('load', () => {
    startLocalCamera();
});
