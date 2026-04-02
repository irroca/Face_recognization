#pragma once
// ============================================================
// ISubject.h — 被观察主题接口
// ============================================================
// [Design Pattern: Observer]
// 定义事件源（主题）的统一接口，支持动态注册/注销观察者，
// 并在事件发生时通知所有已注册的观察者。
// ============================================================

#include "observer/IObserver.h"
#include <memory>

namespace smart_classroom {

class ISubject {
public:
    virtual ~ISubject() = default;

    // [Design Pattern: Observer] 注册观察者
    virtual void attach(std::shared_ptr<IObserver> observer) = 0;

    // [Design Pattern: Observer] 注销观察者
    virtual void detach(std::shared_ptr<IObserver> observer) = 0;

    // [Design Pattern: Observer] 通知所有已注册的观察者
    virtual void notify(const RecognitionEvent& event) = 0;
};

} // namespace smart_classroom
