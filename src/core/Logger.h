#pragma once
// ============================================================
// Logger.h — 全局日志管理器
// ============================================================
// [Design Pattern: Singleton]
// 使用 Meyers' Singleton 实现全局唯一的日志管理器。
// 动机：如果各模块自行管理日志输出（有的写文件、有的写控制台、
// 有的不输出），会导致"发散式变化"的坏味道 (Divergent Change)。
// 通过单例模式提供统一的日志访问点，所有模块使用同一个 Logger
// 实例，确保日志格式和输出目标的一致性。
//
// 支持多级日志：DEBUG / INFO / WARN / ERROR
// 线程安全：所有日志输出通过 mutex 保护。
// ============================================================

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace smart_classroom {

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

class Logger {
public:
    // [Design Pattern: Singleton] 全局唯一访问点
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // 禁止拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // 设置日志级别
    void setLevel(LogLevel level) { level_ = level; }

    // 启用/禁用控制台输出（pipe 模式下需要禁用，因为 stdout/stderr 用于帧协议）
    void setConsoleEnabled(bool enabled) { consoleEnabled_ = enabled; }

    // 设置日志输出文件（同时输出到控制台和文件）
    bool setLogFile(const std::string& filePath);

    // 日志输出接口
    void debug(const std::string& message, const std::string& module = "");
    void info(const std::string& message, const std::string& module = "");
    void warn(const std::string& message, const std::string& module = "");
    void error(const std::string& message, const std::string& module = "");

    // 通用日志输出
    void log(LogLevel level, const std::string& message, const std::string& module = "");

private:
    // [Design Pattern: Singleton] 私有构造与析构
    Logger();
    ~Logger();

    LogLevel level_ = LogLevel::INFO;
    bool consoleEnabled_ = true;
    std::mutex mutex_;
    std::ofstream logFile_;

    // 格式化辅助
    static std::string levelToString(LogLevel level);
    static std::string currentTimestamp();
};

// ============================================================
// 便捷宏——带模块名的日志调用
// ============================================================
#define LOG_DEBUG(msg) smart_classroom::Logger::getInstance().debug(msg, __func__)
#define LOG_INFO(msg)  smart_classroom::Logger::getInstance().info(msg, __func__)
#define LOG_WARN(msg)  smart_classroom::Logger::getInstance().warn(msg, __func__)
#define LOG_ERROR(msg) smart_classroom::Logger::getInstance().error(msg, __func__)

} // namespace smart_classroom
