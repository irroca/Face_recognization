// ============================================================
// AttendanceObserver.cpp — 考勤记录观察者实现
// ============================================================
// [Design Pattern: Observer] 具体观察者实现
// 仅处理 FACE_IDENTIFIED 类型的事件，记录考勤。
// ============================================================

#include "observer/AttendanceObserver.h"
#include "core/Logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace smart_classroom {

AttendanceObserver::AttendanceObserver(const std::string& logFilePath)
    : logFilePath_(logFilePath) {
    LOG_INFO("AttendanceObserver: logging to " + logFilePath_);
}

// [Design Pattern: Observer] 事件回调——仅关心 IDENTIFIED 事件
void AttendanceObserver::onEvent(const RecognitionEvent& event) {
    // 只处理成功识别的事件
    if (event.type != RecognitionEventType::FACE_IDENTIFIED) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // 去重：同一学生在同一次会话中只签到一次
    const std::string& identity = event.faceInfo.identity;
    if (checkedIn_.count(identity)) {
        return;  // 已签到，忽略
    }

    checkedIn_.insert(identity);

    // 生成签到时间戳
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto* tm = std::localtime(&time);
    if (!tm) {
        LOG_ERROR("AttendanceObserver: localtime() failed");
        return;
    }
    std::ostringstream timestamp;
    timestamp << std::put_time(tm, "%Y-%m-%d %H:%M:%S");

    // 记录到日志文件
    std::ofstream logFile(logFilePath_, std::ios::app);
    if (logFile.is_open()) {
        logFile << "[CHECK-IN] " << timestamp.str()
                << " | " << identity
                << " | confidence: " << event.faceInfo.recognitionDistance
                << std::endl;
    } else {
        LOG_WARN("AttendanceObserver: cannot open log file " + logFilePath_);
    }

    LOG_INFO("AttendanceObserver: student '" + identity
             + "' checked in at " + timestamp.str());
}

std::unordered_set<std::string> AttendanceObserver::getCheckedInStudents() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return checkedIn_;
}

} // namespace smart_classroom
