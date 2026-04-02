// ============================================================
// main.cpp — 智慧教室人脸跟踪识别系统入口
// ============================================================
// 本文件展示了所有设计模式的集成与协同：
//   - [Singleton] ConfigManager / Logger / CudaResourceManager
//   - [Factory]   DetectorFactory / RecognizerFactory / FilterFactory
//   - [Strategy]  IFaceDetector / IFaceRecognizer 的动态选择
//   - [Pipeline]  六级过滤器管道的组装与执行
//   - [Observer]  识别事件的分发与响应
// ============================================================

#include <iostream>
#include <memory>
#include <unistd.h>

// Core（单例）
#include "core/ConfigManager.h"
#include "core/Logger.h"
#include "core/CudaResourceManager.h"
#include "core/Types.h"

// Vision（策略 + 工厂）
#include "vision/DetectorFactory.h"
#include "vision/RecognizerFactory.h"
#include "vision/FaceDatabase.h"

// Pipeline（管道-过滤器 + 工厂）
#include "pipeline/FilterFactory.h"
#include "pipeline/Pipeline.h"
#include "pipeline/filters/DecodeFilter.h"
#include "pipeline/filters/PreprocessFilter.h"
#include "pipeline/filters/FaceDetectionFilter.h"
#include "pipeline/filters/FaceRecognitionFilter.h"
#include "pipeline/filters/DrawingFilter.h"
#include "pipeline/filters/EncodeFilter.h"

// Observer（观察者）
#include "observer/RecognitionEventSubject.h"
#include "observer/AttendanceObserver.h"
#include "observer/NotificationObserver.h"

// Network（WebRTC 骨架）
#include "network/WebRTCEndpoint.h"
#include "network/SignalingHandler.h"
#include "network/PipeProcessor.h"

int main(int argc, char* argv[]) {
    using namespace smart_classroom;

    // ============================================================
    // 检测运行模式
    // --pipe-mode: 从 stdin 读取帧，处理后写入 stdout（用于与 Python 服务器协作）
    // 默认模式: 展示设计模式初始化流程
    // ============================================================
    bool pipeMode = false;
    std::string configFile;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--pipe-mode") {
            pipeMode = true;
        } else {
            configFile = arg;
        }
    }

    // ============================================================
    // Phase 1: 初始化单例基础设施
    // [Design Pattern: Singleton] — 全局唯一实例的初始化
    // ============================================================

    // 1.1 初始化日志系统
    auto& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);
    logger.setLogFile("smart_classroom.log");
    if (pipeMode) {
        // pipe 模式下禁用控制台输出，stdout/stderr 保留给帧协议
        logger.setConsoleEnabled(false);
    }
    LOG_INFO("=== Smart Classroom Face Recognition System Starting ===");
    LOG_INFO(std::string("Mode: ") + (pipeMode ? "pipe-processing" : "standalone"));

    // 1.2 加载全局配置
    auto& config = ConfigManager::getInstance();
    if (!configFile.empty()) {
        if (config.loadFromFile(configFile)) {
            LOG_INFO("Configuration loaded from: " + configFile);
        } else {
            LOG_WARN("Failed to load config file, using defaults");
        }
    } else {
        LOG_INFO("No config file specified, using default configuration");
    }

    // 1.3 初始化 CUDA 资源
    auto& cudaMgr = CudaResourceManager::getInstance();
    int cudaDeviceId = config.getInt("cuda.device_id", 0);
    if (config.getBool("cuda.enabled", true)) {
        if (cudaMgr.initialize(cudaDeviceId)) {
            LOG_INFO("CUDA acceleration enabled on " + cudaMgr.getDeviceName());
        } else {
            LOG_WARN("CUDA initialization failed, using CPU mode");
        }
    }

    // ============================================================
    // Phase 2: 创建人脸数据库并加载已注册学生
    // ============================================================

    auto faceDatabase = std::make_shared<FaceDatabase>();
    std::string dbPath = config.getString("database.face_db_path",
                                          "data/face_database.json");
    if (faceDatabase->loadFromFile(dbPath)) {
        LOG_INFO("Face database loaded: " + std::to_string(faceDatabase->size())
                 + " registered face(s)");
    } else {
        LOG_WARN("No face database found at " + dbPath
                 + ", starting with empty database");
    }

    // ============================================================
    // Phase 3: 通过工厂创建检测器和识别器
    // [Design Pattern: Factory Method] — 隐藏创建复杂性
    // [Design Pattern: Strategy] — 通过配置选择算法
    // ============================================================

    // 3.1 [Factory + Strategy] 创建人脸检测器
    std::string detectorTypeName = config.getString("detector.type", "DLIB_HOG");
    auto detector = std::shared_ptr<IFaceDetector>(
        DetectorFactory::createFromConfig(detectorTypeName).release());
    LOG_INFO("Face detector created: " + detector->getName());

    // 3.2 [Factory + Strategy] 创建人脸识别器
    std::string recognizerTypeName = config.getString("recognizer.type", "DLIB_RESNET");
    auto recognizer = std::shared_ptr<IFaceRecognizer>(
        RecognizerFactory::createFromConfig(recognizerTypeName, faceDatabase).release());
    LOG_INFO("Face recognizer created: " + recognizer->getName());

    // ============================================================
    // Phase 4: 创建观察者并注册到事件主题
    // [Design Pattern: Observer] — 识别事件的订阅与分发
    // ============================================================

    // 4.1 创建事件主题（被观察者）
    auto eventSubject = std::make_shared<RecognitionEventSubject>();

    // 4.2 [Observer] 创建并注册考勤观察者
    auto attendanceObserver = std::make_shared<AttendanceObserver>("attendance.log");
    eventSubject->attach(attendanceObserver);

    // 4.3 [Observer] 创建并注册通知观察者
    auto notificationObserver = std::make_shared<NotificationObserver>();
    eventSubject->attach(notificationObserver);

    LOG_INFO("Observer pattern initialized: " 
             + std::to_string(eventSubject->observerCount()) + " observer(s) registered");

    // ============================================================
    // Phase 5: 组装视频处理管道
    // [Design Pattern: Pipeline-Filter] — 六级过滤器管道
    // 检测和识别 Filter 保留 shared_ptr 引用，以便 pipe 模式下运行时切换算法
    // ============================================================

    auto detectionFilter = std::make_shared<FaceDetectionFilter>(detector);
    auto recognitionFilter = std::make_shared<FaceRecognitionFilter>(
        recognizer, faceDatabase, eventSubject);

    auto pipeline = std::make_shared<Pipeline>();
    pipeline->addFilter(std::make_unique<DecodeFilter>());
    pipeline->addFilter(std::make_unique<PreprocessFilter>());
    pipeline->addSharedFilter(detectionFilter.get());
    pipeline->addSharedFilter(recognitionFilter.get());
    pipeline->addFilter(std::make_unique<DrawingFilter>());
    pipeline->addFilter(std::make_unique<EncodeFilter>());

    pipeline->printPipeline();
    LOG_INFO("Video processing pipeline assembled with "
             + std::to_string(pipeline->filterCount()) + " filters");

    // ============================================================
    // Phase 6: 根据运行模式启动不同的处理逻辑
    // ============================================================

    if (pipeMode) {
        // ---- Pipe 模式：实际帧处理，支持运行时算法切换 ----
        LOG_INFO("=== Pipe Mode: ready to process frames from stdin ===");

        auto pipeProcessor = std::make_unique<PipeProcessor>(
            pipeline, notificationObserver,
            detectionFilter, recognitionFilter, faceDatabase,
            STDIN_FILENO, STDOUT_FILENO);
        pipeProcessor->run();

    } else {
        // ---- 展示模式：启动 WebRTC 骨架，展示设计模式初始化流程 ----
        auto webrtcEndpoint = std::make_shared<WebRTCEndpoint>(pipeline);
        webrtcEndpoint->setStateCallback([](WebRTCState state) {
            switch (state) {
                case WebRTCState::CONNECTING:
                    LOG_INFO("WebRTC: connecting...");
                    break;
                case WebRTCState::CONNECTED:
                    LOG_INFO("WebRTC: connected — video streaming active");
                    break;
                case WebRTCState::DISCONNECTED:
                    LOG_INFO("WebRTC: disconnected");
                    break;
                case WebRTCState::FAILED:
                    LOG_ERROR("WebRTC: connection failed");
                    break;
            }
        });

        WebRTCConfig rtcConfig;
        rtcConfig.stunServer = config.getString("webrtc.stun_server",
                                                "stun:stun.l.google.com:19302");
        webrtcEndpoint->initialize(rtcConfig);

        auto signalingHandler = std::make_shared<SignalingHandler>(webrtcEndpoint);
        std::string signalingUrl = "ws://localhost:"
            + config.getString("signaling.ws_port", "8765");
        signalingHandler->connect(signalingUrl);

        LOG_INFO("=== System Ready — Waiting for WebRTC connections ===");
        LOG_INFO("Design patterns in use:");
        LOG_INFO("  [Singleton]  ConfigManager, Logger, CudaResourceManager");
        LOG_INFO("  [Strategy]   " + detector->getName() + " + " + recognizer->getName());
        LOG_INFO("  [Factory]    DetectorFactory, RecognizerFactory, FilterFactory");
        LOG_INFO("  [Pipeline]   6-stage video processing pipeline");
        LOG_INFO("  [Observer]   " + std::to_string(eventSubject->observerCount())
                 + " event observers");

        std::cout << "\nPress Enter to exit...\n";
        std::cin.get();
    }

    LOG_INFO("=== Smart Classroom System Shutting Down ===");
    return 0;
}
