#pragma once
// ============================================================
// NotificationObserver.h — 通知推送观察者
// ============================================================
// [Design Pattern: Observer] 具体观察者
// 监听所有识别事件，将通知消息推送给前端或存储在队列中供
// 前端轮询。可用于实现弹幕通知（如"张三已签到"）或安全
// 告警（如"检测到未注册人脸"）。
// ============================================================

#include "observer/IObserver.h"
#include <vector>
#include <string>
#include <mutex>
#include <queue>

namespace smart_classroom {

class NotificationObserver : public IObserver {
public:
    NotificationObserver() = default;
    ~NotificationObserver() override = default;

    // [Design Pattern: Observer] 处理识别事件
    void onEvent(const RecognitionEvent& event) override;
    std::string getObserverName() const override { return "NotificationObserver"; }

    // 获取待推送的通知消息（消费后清空）
    std::vector<std::string> fetchPendingNotifications();

    // 获取待推送通知数量
    size_t pendingCount() const;

private:
    std::queue<std::string> notifications_;
    mutable std::mutex mutex_;
};

} // namespace smart_classroom
