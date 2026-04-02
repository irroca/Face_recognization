// ============================================================
// PipeProcessor.cpp — 管道帧处理器实现
// ============================================================
// 从 stdin 读取 JPEG 帧 → Pipeline 处理 → 写入 stdout
// 支持控制消息实现运行时算法切换 [Design Pattern: Strategy]
// ============================================================

#include "network/PipeProcessor.h"
#include "vision/DetectorFactory.h"
#include "vision/RecognizerFactory.h"
#include "core/Logger.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
#include <chrono>

namespace smart_classroom {

PipeProcessor::PipeProcessor(std::shared_ptr<Pipeline> pipeline,
                             std::shared_ptr<NotificationObserver> notificationObserver,
                             std::shared_ptr<FaceDetectionFilter> detectionFilter,
                             std::shared_ptr<FaceRecognitionFilter> recognitionFilter,
                             std::shared_ptr<FaceDatabase> faceDatabase,
                             int inputFd, int outputFd)
    : pipeline_(std::move(pipeline))
    , notificationObserver_(std::move(notificationObserver))
    , detectionFilter_(std::move(detectionFilter))
    , recognitionFilter_(std::move(recognitionFilter))
    , faceDatabase_(std::move(faceDatabase))
    , inputFd_(inputFd)
    , outputFd_(outputFd) {
}

void PipeProcessor::run() {
    running_ = true;
    sendReady();

    LOG_INFO("PipeProcessor: entering frame processing loop");

    while (running_) {
        // Step 1: 读取帧长度 (4 字节, 大端序)
        uint32_t netLen = 0;
        if (!readExact(inputFd_, &netLen, 4)) {
            LOG_INFO("PipeProcessor: input closed, exiting");
            break;
        }
        uint32_t frameLen = ntohl(netLen);

        // 检查是否为控制消息哨兵
        if (frameLen == CONTROL_SENTINEL) {
            // [Design Pattern: Strategy] 读取控制消息
            uint32_t jsonNetLen = 0;
            if (!readExact(inputFd_, &jsonNetLen, 4)) break;
            uint32_t jsonLen = ntohl(jsonNetLen);
            if (jsonLen == 0 || jsonLen > 1024 * 1024) continue;

            std::string jsonStr(jsonLen, '\0');
            if (!readExact(inputFd_, jsonStr.data(), jsonLen)) break;

            handleControlMessage(jsonStr);
            continue;
        }

        if (frameLen == 0 || frameLen > 10 * 1024 * 1024) {
            LOG_WARN("PipeProcessor: invalid frame length: " + std::to_string(frameLen));
            continue;
        }

        // Step 2: 读取 JPEG 数据
        std::vector<uint8_t> jpegData(frameLen);
        if (!readExact(inputFd_, jpegData.data(), frameLen)) {
            LOG_INFO("PipeProcessor: input closed during frame read");
            break;
        }

        // Step 3: 构建 VideoFrame 并通过 Pipeline 处理
        VideoFrame frame;
        frame.encodedData = std::move(jpegData);
        frame.format = FrameFormat::ENCODED_JPEG;
        frame.frameIndex = frameIndex_++;
        frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        // [Design Pattern: Pipeline-Filter] 执行完整的 6 级管道处理
        bool success = pipeline_->execute(frame);

        if (success && !frame.encodedData.empty()) {
            // Step 4: 写入处理后的帧到 stdout
            uint32_t outNetLen = htonl(static_cast<uint32_t>(frame.encodedData.size()));
            if (!writeExact(outputFd_, &outNetLen, 4) ||
                !writeExact(outputFd_, frame.encodedData.data(), frame.encodedData.size())) {
                LOG_ERROR("PipeProcessor: failed to write output frame");
                break;
            }
        }

        // Step 5: [Design Pattern: Observer] 刷新事件通知到 stderr
        flushNotifications();
    }

    running_ = false;
    LOG_INFO("PipeProcessor: processing loop ended");
}

// [Design Pattern: Strategy] 运行时切换检测/识别算法
void PipeProcessor::handleControlMessage(const std::string& jsonStr) {
    try {
        auto msg = nlohmann::json::parse(jsonStr);
        std::string action = msg.value("action", "");

        if (action == "switch_algorithm") {
            std::string detectorType = msg.value("detector", "");
            std::string recognizerType = msg.value("recognizer", "");

            // [Design Pattern: Factory + Strategy] 通过工厂创建新策略并注入
            if (!detectorType.empty() && detectionFilter_) {
                LOG_INFO("PipeProcessor: switching detector to " + detectorType);
                auto newDetector = DetectorFactory::createFromConfig(detectorType);
                if (newDetector) {
                    detectionFilter_->setDetector(
                        std::shared_ptr<IFaceDetector>(newDetector.release()));

                    std::string evt = "EVENT:[算法切换] 检测算法已切换为 " + detectorType + "\n";
                    ::write(STDERR_FILENO, evt.c_str(), evt.size());
                } else {
                    std::string evt = "EVENT:[错误] 检测算法 " + detectorType
                        + " 初始化失败（可能需要 GPU 支持）\n";
                    ::write(STDERR_FILENO, evt.c_str(), evt.size());
                }
            }

            if (!recognizerType.empty() && recognitionFilter_ && faceDatabase_) {
                LOG_INFO("PipeProcessor: switching recognizer to " + recognizerType);
                auto newRecognizer = RecognizerFactory::createFromConfig(
                    recognizerType, faceDatabase_);
                if (newRecognizer) {
                    recognitionFilter_->setRecognizer(
                        std::shared_ptr<IFaceRecognizer>(newRecognizer.release()));

                    std::string evt = "EVENT:[算法切换] 识别算法已切换为 " + recognizerType + "\n";
                    ::write(STDERR_FILENO, evt.c_str(), evt.size());
                }
            }
        } else {
            LOG_WARN("PipeProcessor: unknown control action: " + action);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("PipeProcessor: failed to parse control message: " + std::string(e.what()));
    }
}

bool PipeProcessor::readExact(int fd, void* buf, size_t count) {
    size_t totalRead = 0;
    auto* ptr = static_cast<char*>(buf);
    while (totalRead < count) {
        ssize_t n = ::read(fd, ptr + totalRead, count - totalRead);
        if (n <= 0) return false;  // EOF or error
        totalRead += n;
    }
    return true;
}

bool PipeProcessor::writeExact(int fd, const void* buf, size_t count) {
    size_t totalWritten = 0;
    auto* ptr = static_cast<const char*>(buf);
    while (totalWritten < count) {
        ssize_t n = ::write(fd, ptr + totalWritten, count - totalWritten);
        if (n <= 0) return false;
        totalWritten += n;
    }
    return true;
}

void PipeProcessor::flushNotifications() {
    if (!notificationObserver_) return;

    auto notifications = notificationObserver_->fetchPendingNotifications();
    for (const auto& notification : notifications) {
        // 写到 stderr: "EVENT:{...}\n"
        std::string line = "EVENT:" + notification + "\n";
        ::write(STDERR_FILENO, line.c_str(), line.size());
    }
}

void PipeProcessor::sendReady() {
    // 先 flush 所有文件日志，确保初始化日志已写入
    const char* ready = "READY\n";
    ssize_t written = ::write(STDERR_FILENO, ready, 6);
    // 确保 READY 被立即写出
    ::fsync(STDERR_FILENO);
    (void)written;
}

} // namespace smart_classroom
