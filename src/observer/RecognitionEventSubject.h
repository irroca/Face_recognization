#pragma once
// ============================================================
// RecognitionEventSubject.h — 识别事件主题
// ============================================================
// [Design Pattern: Observer] 具体主题类
// 管理观察者列表，在人脸识别事件发生时通知所有观察者。
// ============================================================

#include "observer/ISubject.h"
#include <vector>
#include <mutex>

namespace smart_classroom {

class RecognitionEventSubject : public ISubject {
public:
    RecognitionEventSubject() = default;
    ~RecognitionEventSubject() override = default;

    // [Design Pattern: Observer] 注册观察者
    void attach(std::shared_ptr<IObserver> observer) override;

    // [Design Pattern: Observer] 注销观察者
    void detach(std::shared_ptr<IObserver> observer) override;

    // [Design Pattern: Observer] 通知所有观察者
    void notify(const RecognitionEvent& event) override;

    // 获取当前观察者数量
    size_t observerCount() const;

private:
    std::vector<std::shared_ptr<IObserver>> observers_;
    mutable std::mutex mutex_;
};

} // namespace smart_classroom
