#pragma once
// ============================================================
// ConfigManager.h — 全局配置管理器
// ============================================================
// [Design Pattern: Singleton]
// 使用 Meyers' Singleton 实现全局唯一的配置管理器。
// 动机：系统中存在大量可配置参数（检测阈值、模型路径、CUDA设备号
// 等），如果在各模块中分散管理配置，会导致"散弹式修改"的坏味道
// (Shotgun Surgery)。通过单例模式将所有配置集中到一个全局访问点，
// 消除配置管理的分散与不一致。
//
// C++11 保证局部静态变量初始化的线程安全性（§6.7 [stmt.dcl] p4），
// 因此 Meyers' Singleton 天然线程安全，无需额外加锁。
// ============================================================

#include <string>
#include <unordered_map>
#include <mutex>

namespace smart_classroom {

class ConfigManager {
public:
    // [Design Pattern: Singleton] 全局唯一访问点
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // 禁止拷贝和移动 —— 单例的核心约束
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;

    // 加载配置文件（JSON 格式）
    bool loadFromFile(const std::string& filePath);

    // 类型安全的配置读取接口
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int         getInt(const std::string& key, int defaultValue = 0) const;
    double      getDouble(const std::string& key, double defaultValue = 0.0) const;
    bool        getBool(const std::string& key, bool defaultValue = false) const;

    // 动态设置配置项（用于运行时切换策略等）
    void set(const std::string& key, const std::string& value);

private:
    // [Design Pattern: Singleton] 私有构造函数，防止外部实例化
    ConfigManager();
    ~ConfigManager() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> config_;

    // 设置默认配置
    void setDefaults();
};

} // namespace smart_classroom
