// ============================================================
// ui.js — 前端 UI 交互逻辑
// ============================================================

// 更新连接状态指示器
function updateStatus(state, text) {
    const el = document.getElementById('connectionStatus');
    el.className = 'status ' + state;

    switch (state) {
        case 'connected':
            el.textContent = '● ' + (text || '已连接');
            document.getElementById('connectBtn').textContent = '断开';
            break;
        case 'connecting':
            el.textContent = '● ' + (text || '连接中...');
            break;
        case 'disconnected':
        default:
            el.textContent = '● ' + (text || '未连接');
            document.getElementById('connectBtn').textContent = '连接';
            break;
    }
}

// 添加通知到面板
function addNotification(data) {
    const list = document.getElementById('notificationList');
    const item = document.createElement('div');
    item.className = 'notification-item';

    const now = new Date().toLocaleTimeString('zh-CN');

    if (data.type === 'identified') {
        item.classList.add('identified');
        item.innerHTML = `<span class="time">[${now}]</span> ✅ ${data.message}`;
    } else if (data.type === 'unknown') {
        item.classList.add('unknown');
        item.innerHTML = `<span class="time">[${now}]</span> ⚠️ ${data.message}`;
    } else {
        item.innerHTML = `<span class="time">[${now}]</span> ${data.message}`;
    }

    list.insertBefore(item, list.firstChild);

    // 最多保留 50 条通知
    while (list.children.length > 50) {
        list.removeChild(list.lastChild);
    }
}

// FPS 计数器
let frameCount = 0;
let lastFpsTime = performance.now();
let fpsAnimationId = null;

function updateFps() {
    const now = performance.now();
    if (now - lastFpsTime >= 1000) {
        document.getElementById('fpsCounter').textContent =
            'FPS: ' + frameCount;
        frameCount = 0;
        lastFpsTime = now;
    }
    fpsAnimationId = requestAnimationFrame(updateFps);
}

fpsAnimationId = requestAnimationFrame(updateFps);
