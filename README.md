# 智慧教室人脸跟踪识别系统

基于 WebRTC/OpenCV/Dlib 的智慧教室人脸跟踪识别系统。
C++17 后端实现 6 级管道-过滤器视频处理管道，支持 CUDA GPU 加速，综合运用 5 种经典设计模式。

> **课程设计文档**：[docs/final_report.md](docs/final_report.md) — 软件体系结构与设计模式分析报告

## 系统架构

```
浏览器摄像头 ──WebSocket──▶ Python服务器 ──stdin/stdout──▶ C++后端Pipeline
   │                          │                              │
   ├─ 帧捕获(JPEG)          ├─ 子进程管理                ├─ Decode → Preprocess
   ├─ 右侧canvas渲染       ├─ 帧转发/丢帧控制           ├─ FaceDetection [Strategy]
   └─ 算法切换UI            └─ 控制消息路由              ├─ FaceRecognition [Strategy+Observer]
                                                          ├─ Drawing → Encode
                                                          └─ 运行时算法切换 [Factory]
```

## 设计模式（155 处标注）

| 模式 | 代码位置 | 用途 |
|------|---------|------|
| 管道-过滤器 | `src/pipeline/` | 6 级视频处理流水线 |
| 策略模式 | `src/vision/` | 检测(3种) + 识别(2种)算法运行时可切换 |
| 观察者模式 | `src/observer/` | 识别事件 → 考勤/通知解耦分发 |
| 单例模式 | `src/core/` | 配置/日志/CUDA 资源全局管理 |
| 工厂方法模式 | `*Factory.*` | 封装检测器/识别器/管道的创建 |

## 环境要求

| 组件 | 版本 |
|------|------|
| Ubuntu (WSL2) | 24.04 |
| GCC | 13+ (C++17) |
| CMake | ≥ 3.18 |
| OpenCV | ≥ 4.5 |
| Dlib | 最新版 (需源码编译) |
| CUDA Toolkit | ≥ 12.0 |
| cuDNN | ≥ 9.x |
| Python (Conda) | 3.10 |
| GPU | NVIDIA RTX 4060 (8GB) |

## 快速开始

### 1. 安装依赖

```bash
# 系统依赖
sudo apt install -y build-essential cmake libopencv-dev libopencv-contrib-dev nlohmann-json3-dev

# cuDNN（通过 NVIDIA 官方仓库）
cd /tmp && wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb && sudo apt update
sudo apt install -y libcudnn9-dev-cuda-12 libcudnn9-cuda-12

# Dlib（源码编译，启用 CUDA）
git clone --depth 1 https://github.com/davisking/dlib.git
cd dlib && mkdir build && cd build
cmake .. -DDLIB_USE_CUDA=1 -DUSE_AVX_INSTRUCTIONS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_DEFAULT_CMP0146=OLD
make -j$(nproc) && sudo make install

# Python 环境
conda create -n smart_classroom python=3.10 -y
conda activate smart_classroom
pip install websockets pillow
```

> Dlib 配置时应输出 `DLIB WILL USE CUDA`。若显示 `WILL NOT USE CUDA`，检查 cuDNN：`ls /usr/lib/x86_64-linux-gnu/libcudnn.so`

### 2. 下载模型文件

```bash
mkdir -p models && cd models
wget http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2 && bzip2 -d shape_predictor_68_face_landmarks.dat.bz2
wget http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2 && bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2
wget http://dlib.net/files/mmod_human_face_detector.dat.bz2 && bzip2 -d mmod_human_face_detector.dat.bz2
wget https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt
wget https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel
cd ..
```

### 3. 编译

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_CUDA=ON -DBUILD_TESTS=ON
make -j$(nproc)
```

### 4. 运行

```bash
# 终端 1：启动处理服务器（自动启动 C++ 后端）
conda activate smart_classroom
PYTHONUNBUFFERED=1 python signaling/server.py --port 8765

# 终端 2：启动前端页面
cd frontend && python3 -m http.server 8080
```

浏览器打开 `http://localhost:8080`，点击 **"连接"**：
- **左侧**：本地摄像头实时画面
- **右侧**：C++ Pipeline 处理后的画面（人脸绿框/红框标注）
- **通知栏**：识别事件（"检测到未注册人脸"等）

### 5. 运行测试

```bash
conda deactivate
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ctest --test-dir build --output-on-failure
```

## 算法可用性

| 算法 | 要求 | 说明 |
|------|------|------|
| `DLIB_HOG` | 无 | CPU 检测，始终可用（默认） |
| `OPENCV_DNN` | 模型文件 | CPU/CUDA 可选 |
| `DLIB_CNN_CUDA` | GPU + Dlib CUDA 编译 | 需要 CUDA 可用 |
| `DLIB_RESNET` | 模型文件 | 128D 特征提取 |
| `OPENCV_LBPH` | opencv_face 模块 | 需 OpenCV contrib |

前端下拉框选择后点击"切换算法"即可运行时切换，无需重启。

## 项目结构

```
├── CMakeLists.txt
├── config.json                        # 运行配置
├── docs/final_report.md               # 课程设计报告
├── src/                               # C++ 后端源码
│   ├── core/                          # [Singleton] ConfigManager, Logger, CudaResourceManager
│   ├── pipeline/                      # [Pipeline-Filter] IFilter, Pipeline, FilterFactory
│   │   └── filters/                   # 6 个具体过滤器
│   ├── vision/                        # [Strategy + Factory] 检测器/识别器
│   │   ├── detectors/                 # DlibHog, DlibCnn, OpenCvDnn
│   │   └── recognizers/               # DlibResNet, OpenCvLbph
│   ├── observer/                      # [Observer] 事件主题 + 考勤/通知观察者
│   ├── network/                       # PipeProcessor, WebRTC 骨架, TrackFactory
│   └── main.cpp                       # 入口（支持 --pipe-mode）
├── tests/                             # 单元测试 (Pipeline / Strategy / Observer)
├── frontend/                          # 前端 (HTML + JS WebSocket 帧传输)
├── signaling/server.py                # Python 处理服务器（管理 C++ 子进程）
├── models/                            # 模型文件（需下载，.gitignore）
└── data/face_database.json            # 人脸特征库示例
```

## WSL2 注意事项

- **GPU 间歇断开**：`nvidia-smi` 偶尔失败。Windows PowerShell 执行 `wsl --shutdown` 重启恢复。程序自动降级为 CPU 模式。
- **Conda libstdc++ 冲突**：运行 C++ 程序/测试前须 `conda deactivate`。server.py 已自动清理子进程环境。
- **端口转发**：WSL2 端口自动转发，浏览器访问 `http://localhost:8080` 即可。

## License

MIT
