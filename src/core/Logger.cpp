// ============================================================
// Logger.cpp — 全局日志管理器实现
// ============================================================
// [Design Pattern: Singleton] Meyers' Singleton 实现
// ============================================================

#include "core/Logger.h"

namespace smart_classroom {

// [Design Pattern: Singleton] 私有构造函数
Logger::Logger() {
    level_ = LogLevel::INFO;
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

bool Logger::setLogFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(filePath, std::ios::app);
    return logFile_.is_open();
}

void Logger::debug(const std::string& message, const std::string& module) {
    log(LogLevel::DEBUG, message, module);
}

void Logger::info(const std::string& message, const std::string& module) {
    log(LogLevel::INFO, message, module);
}

void Logger::warn(const std::string& message, const std::string& module) {
    log(LogLevel::WARN, message, module);
}

void Logger::error(const std::string& message, const std::string& module) {
    log(LogLevel::ERROR, message, module);
}

void Logger::log(LogLevel level, const std::string& message, const std::string& module) {
    if (level < level_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream oss;
    oss << "[" << currentTimestamp() << "]"
        << "[" << levelToString(level) << "]";

    if (!module.empty()) {
        oss << "[" << module << "]";
    }
    oss << " " << message;

    std::string formatted = oss.str();

    // 输出到控制台（ERROR 使用 stderr）
    if (consoleEnabled_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << formatted << std::endl;
        } else {
            std::cout << formatted << std::endl;
        }
    }

    // 同步输出到日志文件
    if (logFile_.is_open()) {
        logFile_ << formatted << std::endl;
        logFile_.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default:              return "?????";
    }
}

std::string Logger::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace smart_classroom
