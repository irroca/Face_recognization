#pragma once
// ============================================================
// IObserver.h — 观察者接口
// ============================================================
// [Design Pattern: Observer]
// 定义事件观察者的统一接口。当系统中发生特定事件（如识别到
// 学生人脸、检测到未注册人脸等）时，所有注册的观察者都会收到
// 通知。
//
// 动机：当识别到一张人脸后，可能需要执行多种不同的响应：
//   - 记录考勤日志
//   - 发送前端通知/弹幕
//   - 触发安全告警
//   - 更新统计数据
// 如果在识别代码中直接调用这些响应，会导致：
//   - "过大的类" (Large Class)
//   - 识别模块与各业务模块紧耦合
//   - 新增响应需修改识别代码，违反 OCP
//
// 观察者模式让事件源（Subject）和事件处理者（Observer）完全
// 解耦，新增业务响应只需新增一个 Observer 类并注册到 Subject。
// ============================================================

#include "core/Types.h"

namespace smart_classroom {

class IObserver {
public:
    virtual ~IObserver() = default;

    // [Design Pattern: Observer] 接收事件通知的回调方法
    virtual void onEvent(const RecognitionEvent& event) = 0;

    // 返回观察者名称（用于日志）
    virtual std::string getObserverName() const = 0;
};

} // namespace smart_classroom
