#pragma once
// ============================================================
// PipeProcessor.h — 管道帧处理器（stdin/stdout 模式）
// ============================================================
// 通过标准输入接收 JPEG 帧，经过 Pipeline 处理后通过标准输出
// 返回处理后的帧。用于与 Python 服务器进程间通信。
//
// 二进制协议：
//   帧数据：  [4 字节: uint32 大端序 长度][N 字节: JPEG 数据]
//   控制消息：[4 字节: 0xFFFFFFFF 哨兵][4 字节: JSON长度][JSON 数据]
//
// stderr 输出：
//   "READY\n"            — 初始化完成信号
//   "EVENT:{json}\n"     — 识别事件通知
// ============================================================

#include "pipeline/Pipeline.h"
#include "pipeline/filters/FaceDetectionFilter.h"
#include "pipeline/filters/FaceRecognitionFilter.h"
#include "vision/FaceDatabase.h"
#include "observer/NotificationObserver.h"
#include <memory>
#include <atomic>

namespace smart_classroom {

class PipeProcessor {
public:
    PipeProcessor(std::shared_ptr<Pipeline> pipeline,
                  std::shared_ptr<NotificationObserver> notificationObserver,
                  std::shared_ptr<FaceDetectionFilter> detectionFilter,
                  std::shared_ptr<FaceRecognitionFilter> recognitionFilter,
                  std::shared_ptr<FaceDatabase> faceDatabase,
                  int inputFd, int outputFd);
    ~PipeProcessor() = default;

    void run();
    void stop() { running_ = false; }

private:
    std::shared_ptr<Pipeline> pipeline_;
    std::shared_ptr<NotificationObserver> notificationObserver_;
    std::shared_ptr<FaceDetectionFilter> detectionFilter_;
    std::shared_ptr<FaceRecognitionFilter> recognitionFilter_;
    std::shared_ptr<FaceDatabase> faceDatabase_;
    int inputFd_;
    int outputFd_;
    std::atomic<bool> running_{false};
    int frameIndex_ = 0;

    static constexpr uint32_t CONTROL_SENTINEL = 0xFFFFFFFF;

    bool readExact(int fd, void* buf, size_t count);
    bool writeExact(int fd, const void* buf, size_t count);
    void flushNotifications();
    void sendReady();

    // [Design Pattern: Strategy] 处理算法切换控制消息
    void handleControlMessage(const std::string& json);
};

} // namespace smart_classroom
