// ============================================================
// test_observer.cpp — 观察者模式单元测试
// ============================================================
// 验证：
//   1. Subject 能正确注册和注销 Observer
//   2. notify() 能将事件分发给所有已注册的 Observer
//   3. Observer 的事件过滤逻辑（如按类型过滤）
//   4. 多个 Observer 独立处理同一事件
// ============================================================

#include "observer/IObserver.h"
#include "observer/ISubject.h"
#include "observer/RecognitionEventSubject.h"
#include "core/Types.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

using namespace smart_classroom;

// ============================================================
// Mock Observer：记录收到的事件
// ============================================================
class MockObserver : public IObserver {
public:
    explicit MockObserver(const std::string& name) : name_(name) {}

    void onEvent(const RecognitionEvent& event) override {
        receivedEvents_.push_back(event);
    }

    std::string getObserverName() const override { return name_; }

    size_t eventCount() const { return receivedEvents_.size(); }
    const RecognitionEvent& lastEvent() const { return receivedEvents_.back(); }
    const std::vector<RecognitionEvent>& allEvents() const { return receivedEvents_; }

private:
    std::string name_;
    std::vector<RecognitionEvent> receivedEvents_;
};

// ============================================================
// Mock Observer：只关心 IDENTIFIED 事件（模拟 AttendanceObserver）
// ============================================================
class IdentifiedOnlyObserver : public IObserver {
public:
    void onEvent(const RecognitionEvent& event) override {
        if (event.type == RecognitionEventType::FACE_IDENTIFIED) {
            identifiedCount_++;
            lastIdentity_ = event.faceInfo.identity;
        }
    }

    std::string getObserverName() const override { return "IdentifiedOnlyObserver"; }

    int identifiedCount() const { return identifiedCount_; }
    std::string lastIdentity() const { return lastIdentity_; }

private:
    int identifiedCount_ = 0;
    std::string lastIdentity_;
};

// ============================================================
// 测试用例
// ============================================================

void test_attach_detach() {
    std::cout << "[TEST] Attach and detach observers... ";
    RecognitionEventSubject subject;

    auto obs1 = std::make_shared<MockObserver>("Obs1");
    auto obs2 = std::make_shared<MockObserver>("Obs2");

    assert(subject.observerCount() == 0);

    subject.attach(obs1);
    assert(subject.observerCount() == 1);

    subject.attach(obs2);
    assert(subject.observerCount() == 2);

    // 重复注册应被忽略
    subject.attach(obs1);
    assert(subject.observerCount() == 2);

    subject.detach(obs1);
    assert(subject.observerCount() == 1);

    subject.detach(obs2);
    assert(subject.observerCount() == 0);

    std::cout << "PASSED\n";
}

void test_notify_all() {
    std::cout << "[TEST] Notify all observers... ";
    RecognitionEventSubject subject;

    auto obs1 = std::make_shared<MockObserver>("Obs1");
    auto obs2 = std::make_shared<MockObserver>("Obs2");
    auto obs3 = std::make_shared<MockObserver>("Obs3");

    subject.attach(obs1);
    subject.attach(obs2);
    subject.attach(obs3);

    RecognitionEvent event;
    event.type = RecognitionEventType::FACE_IDENTIFIED;
    event.faceInfo.identity = "2024001:张三";
    event.message = "Identified: 张三";

    subject.notify(event);

    // 所有观察者都应收到事件
    assert(obs1->eventCount() == 1);
    assert(obs2->eventCount() == 1);
    assert(obs3->eventCount() == 1);
    assert(obs1->lastEvent().faceInfo.identity == "2024001:张三");

    std::cout << "PASSED\n";
}

void test_selective_observation() {
    std::cout << "[TEST] Selective event handling... ";
    RecognitionEventSubject subject;

    auto allEventsObs = std::make_shared<MockObserver>("AllEvents");
    auto identifiedObs = std::make_shared<IdentifiedOnlyObserver>();

    subject.attach(allEventsObs);
    subject.attach(identifiedObs);

    // 发送 UNKNOWN 事件
    RecognitionEvent unknownEvent;
    unknownEvent.type = RecognitionEventType::FACE_UNKNOWN;
    unknownEvent.message = "Unregistered face";
    subject.notify(unknownEvent);

    // 发送 IDENTIFIED 事件
    RecognitionEvent identifiedEvent;
    identifiedEvent.type = RecognitionEventType::FACE_IDENTIFIED;
    identifiedEvent.faceInfo.identity = "2024002:李四";
    identifiedEvent.message = "Identified: 李四";
    subject.notify(identifiedEvent);

    // AllEvents 应收到 2 个事件
    assert(allEventsObs->eventCount() == 2);

    // IdentifiedOnly 应只处理 1 个 IDENTIFIED 事件
    assert(identifiedObs->identifiedCount() == 1);
    assert(identifiedObs->lastIdentity() == "2024002:李四");

    std::cout << "PASSED\n";
}

void test_detach_stops_notification() {
    std::cout << "[TEST] Detach stops notification... ";
    RecognitionEventSubject subject;

    auto obs = std::make_shared<MockObserver>("TempObs");
    subject.attach(obs);

    RecognitionEvent event;
    event.type = RecognitionEventType::FACE_DETECTED;
    subject.notify(event);
    assert(obs->eventCount() == 1);

    // 注销后不应再收到通知
    subject.detach(obs);
    subject.notify(event);
    assert(obs->eventCount() == 1);  // 仍然是 1

    std::cout << "PASSED\n";
}

void test_multiple_events() {
    std::cout << "[TEST] Multiple events... ";
    RecognitionEventSubject subject;

    auto obs = std::make_shared<MockObserver>("MultiObs");
    subject.attach(obs);

    for (int i = 0; i < 5; ++i) {
        RecognitionEvent event;
        event.type = RecognitionEventType::FACE_IDENTIFIED;
        event.faceInfo.identity = "Student_" + std::to_string(i);
        subject.notify(event);
    }

    assert(obs->eventCount() == 5);
    assert(obs->lastEvent().faceInfo.identity == "Student_4");

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Observer Pattern Tests\n";
    std::cout << "========================================\n";

    test_attach_detach();
    test_notify_all();
    test_selective_observation();
    test_detach_stops_notification();
    test_multiple_events();

    std::cout << "========================================\n";
    std::cout << "All observer tests PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
