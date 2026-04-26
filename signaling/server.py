#!/usr/bin/env python3
# ============================================================
# server.py — 智慧教室 WebSocket 处理服务器
# ============================================================
# 功能：
#   1. 启动 C++ 后端作为子进程（--pipe-mode）
#   2. 运行 WebSocket 服务器接收前端视频帧
#   3. 将帧转发给 C++ 后端 Pipeline 处理（6级过滤器）
#   4. 将处理后的帧和识别事件返回给前端
#
# 协议：
#   - 前端 → 服务器：binary（JPEG 帧）或 text（JSON 控制消息）
#   - 服务器 → 前端：binary（处理后的 JPEG）或 text（事件通知）
#   - 服务器 ↔ C++：stdin/stdout 二进制帧协议 [4字节长度][JPEG数据]
#
# 运行方式：
#   conda activate smart_classroom
#   python signaling/server.py --port 8765
# ============================================================

import asyncio
import json
import argparse
import logging
import os
import struct
import signal
import sys

try:
    from websockets.asyncio.server import serve
except ImportError:
    from websockets.server import serve

logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] [%(levelname)s] %(message)s',
    stream=sys.stderr  # 确保日志输出到 stderr，不受 stdout 缓冲影响
)
logger = logging.getLogger(__name__)

# ============================================================
# 全局状态
# ============================================================
backend_proc = None
connected_clients = set()
frame_lock = asyncio.Lock()
processing_busy = False  # 帧跳过标志：管道忙时丢弃新帧避免积压


# ============================================================
# C++ 后端子进程管理
# ============================================================
async def start_backend(backend_path, config_path):
    """启动 C++ 后端子进程（pipe 模式）"""
    global backend_proc

    # 清理 LD_LIBRARY_PATH：移除所有 conda/miniconda 路径，避免 libstdc++ 冲突
    env = os.environ.copy()
    ld_path = env.get('LD_LIBRARY_PATH', '')
    if ld_path:
        clean_paths = [p for p in ld_path.split(':')
                       if p and 'conda' not in p and 'miniconda' not in p]
        env['LD_LIBRARY_PATH'] = ':'.join(clean_paths) if clean_paths else ''
    # 同时清理 PATH 中的 conda lib 目录（某些系统会通过 PATH 影响加载）
    for key in ['CONDA_PREFIX', 'CONDA_DEFAULT_ENV']:
        env.pop(key, None)

    cmd = [os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                        'launch_backend.sh'),
           backend_path, '--pipe-mode']
    if config_path:
        cmd.append(config_path)

    logger.info(f"Starting C++ backend: {' '.join(cmd)}")

    # 使用 subprocess.Popen 同步启动，等待 READY 后再转为 asyncio
    import subprocess as sp
    sync_proc = sp.Popen(
        cmd,
        stdin=sp.PIPE,
        stdout=sp.PIPE,
        stderr=sp.PIPE,
        env=env
    )

    # 同步读取 READY（避免 asyncio 事件循环问题）
    logger.info("Waiting for C++ backend to initialize...")
    import select
    ready = False
    start_time = __import__('time').time()
    while __import__('time').time() - start_time < 30:
        # 用 select 检查 stderr 是否有数据
        rlist, _, _ = select.select([sync_proc.stderr], [], [], 1.0)
        if rlist:
            line = sync_proc.stderr.readline()
            if not line:
                logger.error(f"C++ backend exited (code={sync_proc.poll()})")
                sys.exit(1)
            text = line.decode().strip()
            if text == "READY":
                logger.info("C++ backend ready")
                ready = True
                break
            else:
                logger.info(f"[C++ backend] {text}")
        else:
            rc = sync_proc.poll()
            if rc is not None:
                err = sync_proc.stderr.read().decode()
                logger.error(f"C++ backend exited with code {rc}: {err[:500]}")
                sys.exit(1)

    if not ready:
        logger.error("C++ backend init timed out (30s)")
        sync_proc.kill()
        sys.exit(1)

    # 将同步 Popen 的 stdin/stdout/stderr 包装为 asyncio 流
    loop = asyncio.get_event_loop()

    # 创建 asyncio 子进程包装器（复用同步进程的管道）
    backend_proc = sync_proc
    logger.info("C++ backend subprocess connected")


async def read_backend_events():
    """后台任务：持续读取 C++ stderr 中的事件通知"""
    global backend_proc
    try:
        import select
        loop = asyncio.get_event_loop()
        while backend_proc and backend_proc.poll() is None:
            rlist = await loop.run_in_executor(
                None, lambda: select.select([backend_proc.stderr], [], [], 0.5)[0])
            if rlist:
                line = backend_proc.stderr.readline()
                if not line:
                    break
                text = line.decode().strip()
                if text.startswith("EVENT:"):
                    event_data = text[6:]
                    msg = json.dumps({
                        "type": "notification",
                        "data": {"message": event_data}
                    })
                    await broadcast_text(msg)
    except Exception as e:
        logger.error(f"Error reading backend events: {e}")


async def send_frame_to_backend(jpeg_data):
    """将 JPEG 帧发送给 C++ 后端，忙时丢帧避免积压"""
    global backend_proc, processing_busy
    if not backend_proc or backend_proc.poll() is not None:
        return None

    # 帧跳过：如果上一帧还在处理，丢弃这一帧
    if processing_busy:
        return None

    processing_busy = True
    try:
        async with frame_lock:
            try:
                loop = asyncio.get_event_loop()
                length = len(jpeg_data)
                header = struct.pack('>I', length)

                def sync_io():
                    backend_proc.stdin.write(header)
                    backend_proc.stdin.write(jpeg_data)
                    backend_proc.stdin.flush()

                    len_bytes = backend_proc.stdout.read(4)
                    if len(len_bytes) < 4:
                        return None
                    out_length = struct.unpack('>I', len_bytes)[0]
                    processed = backend_proc.stdout.read(out_length)
                    if len(processed) < out_length:
                        return None
                    return processed

                processed = await asyncio.wait_for(
                    loop.run_in_executor(None, sync_io), timeout=5.0)
                return processed
            except asyncio.TimeoutError:
                logger.warning("Backend timeout")
                return None
            except Exception as e:
                logger.error(f"Backend error: {e}")
                return None
    finally:
        processing_busy = False


async def broadcast_text(message):
    for ws in list(connected_clients):
        try:
            await ws.send(message)
        except Exception:
            connected_clients.discard(ws)


async def send_control_to_backend(data):
    """发送控制消息给 C++ 后端（算法切换等）"""
    global backend_proc
    if not backend_proc or backend_proc.poll() is not None:
        return

    control_msg = json.dumps({
        "action": "switch_algorithm",
        "detector": data.get("detector", ""),
        "recognizer": data.get("recognizer", "")
    }).encode()

    async with frame_lock:
        try:
            loop = asyncio.get_event_loop()

            def sync_write():
                # 发送哨兵 0xFFFFFFFF 表示控制消息
                backend_proc.stdin.write(struct.pack('>I', 0xFFFFFFFF))
                backend_proc.stdin.write(struct.pack('>I', len(control_msg)))
                backend_proc.stdin.write(control_msg)
                backend_proc.stdin.flush()

            await loop.run_in_executor(None, sync_write)
            logger.info(f"Sent algorithm switch to backend: {data}")
        except Exception as e:
            logger.error(f"Failed to send control message: {e}")


# ============================================================
# WebSocket 连接处理
# ============================================================
async def handler(websocket):
    connected_clients.add(websocket)
    logger.info(f"Client connected (total: {len(connected_clients)})")

    try:
        async for message in websocket:
            if isinstance(message, bytes):
                processed = await send_frame_to_backend(message)
                if processed:
                    try:
                        await websocket.send(processed)
                    except Exception:
                        break
            else:
                try:
                    msg = json.loads(message)
                    msg_type = msg.get("type", "")
                    logger.info(f"Control: {msg_type}")

                    if msg_type == "switch_algorithm":
                        # 转发算法切换指令给 C++ 后端
                        await send_control_to_backend(msg.get("data", {}))
                except json.JSONDecodeError:
                    pass
    except Exception:
        pass
    finally:
        connected_clients.discard(websocket)
        logger.info(f"Client disconnected (total: {len(connected_clients)})")


# ============================================================
# 主入口
# ============================================================
async def async_main(host, port, backend_path, config_path):
    await start_backend(backend_path, config_path)

    # 启动事件读取后台任务
    asyncio.create_task(read_backend_events())

    logger.info(f"WebSocket server on ws://{host}:{port}")

    async with serve(handler, host, port, max_size=10*1024*1024):
        logger.info("Open http://localhost:8080 in browser")
        await asyncio.Future()


def cleanup(signum, frame):
    global backend_proc
    if backend_proc:
        logger.info("Terminating C++ backend...")
        backend_proc.kill()
        backend_proc.wait()
    sys.exit(0)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Smart Classroom Processing Server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--backend", default="./build/src/smart_classroom")
    parser.add_argument("--config", default="config.json")
    args = parser.parse_args()

    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGTERM, cleanup)

    try:
        asyncio.run(async_main(args.host, args.port, args.backend, args.config))
    except KeyboardInterrupt:
        cleanup(None, None)
