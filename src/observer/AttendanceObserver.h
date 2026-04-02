#pragma once
// ============================================================
// AttendanceObserver.h — 考勤记录观察者
// ============================================================
// [Design Pattern: Observer] 具体观察者
// 监听 FACE_IDENTIFIED 事件，自动记录学生考勤信息。
// 当 FaceRecognitionFilter 成功识别出某位学生时，本观察者
// 将收到通知并记录考勤日志（学号、姓名、签到时间）。
// ============================================================

#include "observer/IObserver.h"
#include <string>
#include <unordered_set>
#include <mutex>

namespace smart_classroom {

class AttendanceObserver : public IObserver {
public:
    explicit AttendanceObserver(const std::string& logFilePath = "attendance.log");
    ~AttendanceObserver() override = default;

    // [Design Pattern: Observer] 处理识别事件
    void onEvent(const RecognitionEvent& event) override;
    std::string getObserverName() const override { return "AttendanceObserver"; }

    // 获取已签到的学生列表
    std::unordered_set<std::string> getCheckedInStudents() const;

private:
    std::string logFilePath_;
    std::unordered_set<std::string> checkedIn_;  // 去重：同一学生只记录一次
    mutable std::mutex mutex_;
};

} // namespace smart_classroom
