// ============================================================
// RecognitionEventSubject.cpp — 识别事件主题实现
// ============================================================
// [Design Pattern: Observer] 具体主题实现
// 维护观察者列表，在事件发生时遍历通知所有观察者。
// ============================================================

#include "observer/RecognitionEventSubject.h"
#include "core/Logger.h"
#include <algorithm>

namespace smart_classroom {

// [Design Pattern: Observer] 注册观察者到通知列表
void RecognitionEventSubject::attach(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 避免重复注册
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it == observers_.end()) {
        observers_.push_back(observer);
        LOG_INFO("RecognitionEventSubject: attached observer '"
                 + observer->getObserverName() + "' (total: "
                 + std::to_string(observers_.size()) + ")");
    }
}

// [Design Pattern: Observer] 从通知列表中移除观察者
void RecognitionEventSubject::detach(std::shared_ptr<IObserver> observer) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        LOG_INFO("RecognitionEventSubject: detached observer '"
                 + (*it)->getObserverName() + "'");
        observers_.erase(it);
    }
}

// [Design Pattern: Observer] 遍历通知所有已注册的观察者
void RecognitionEventSubject::notify(const RecognitionEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    LOG_DEBUG("RecognitionEventSubject: notifying "
              + std::to_string(observers_.size()) + " observer(s)");

    for (auto& observer : observers_) {
        try {
            observer->onEvent(event);
        } catch (const std::exception& e) {
            LOG_ERROR("RecognitionEventSubject: observer '"
                      + observer->getObserverName()
                      + "' threw exception: " + e.what());
        }
    }
}

size_t RecognitionEventSubject::observerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return observers_.size();
}

} // namespace smart_classroom
