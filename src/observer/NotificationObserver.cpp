// ============================================================
// NotificationObserver.cpp — 通知推送观察者实现
// ============================================================
// [Design Pattern: Observer] 具体观察者实现
// 处理所有类型的识别事件，生成前端通知消息。
// ============================================================

#include "observer/NotificationObserver.h"
#include "core/Logger.h"

namespace smart_classroom {

// [Design Pattern: Observer] 事件回调——生成通知消息
void NotificationObserver::onEvent(const RecognitionEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string notification;

    switch (event.type) {
        case RecognitionEventType::FACE_IDENTIFIED:
            notification = "[签到] " + event.faceInfo.identity + " 已签到";
            break;

        case RecognitionEventType::FACE_UNKNOWN:
            notification = "[警告] 检测到未注册人脸";
            break;

        case RecognitionEventType::FACE_DETECTED:
            notification = "[检测] 检测到人脸";
            break;
    }

    if (!notification.empty()) {
        notifications_.push(notification);
        LOG_DEBUG("NotificationObserver: queued notification: " + notification);
    }
}

std::vector<std::string> NotificationObserver::fetchPendingNotifications() {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> result;
    while (!notifications_.empty()) {
        result.push_back(notifications_.front());
        notifications_.pop();
    }

    return result;
}

size_t NotificationObserver::pendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return notifications_.size();
}

} // namespace smart_classroom
