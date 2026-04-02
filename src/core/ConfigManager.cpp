// ============================================================
// ConfigManager.cpp — 全局配置管理器实现
// ============================================================
// [Design Pattern: Singleton] Meyers' Singleton 实现
// ============================================================

#include "core/ConfigManager.h"
#include "core/Logger.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace smart_classroom {

// [Design Pattern: Singleton] 私有构造函数——仅在首次调用 getInstance() 时执行
ConfigManager::ConfigManager() {
    setDefaults();
}

void ConfigManager::setDefaults() {
    // 检测与识别相关默认配置
    config_["detector.type"] = "DLIB_HOG";
    config_["recognizer.type"] = "DLIB_RESNET";
    config_["detection.confidence_threshold"] = "0.6";
    config_["recognition.distance_threshold"] = "0.6";

    // 模型文件路径
    config_["model.dlib_cnn"] = "models/mmod_human_face_detector.dat";
    config_["model.shape_predictor"] = "models/shape_predictor_68_face_landmarks.dat";
    config_["model.dlib_resnet"] = "models/dlib_face_recognition_resnet_model_v1.dat";
    config_["model.opencv_dnn_config"] = "models/deploy.prototxt";
    config_["model.opencv_dnn_weights"] = "models/res10_300x300_ssd_iter_140000.caffemodel";

    // 人脸数据库路径
    config_["database.face_db_path"] = "data/face_database.json";

    // CUDA 配置
    config_["cuda.device_id"] = "0";
    config_["cuda.enabled"] = "true";

    // 预处理配置
    config_["preprocess.resize_width"] = "640";
    config_["preprocess.resize_height"] = "480";

    // 编码配置
    config_["encode.format"] = "JPEG";
    config_["encode.jpeg_quality"] = "85";

    // WebRTC 配置
    config_["webrtc.stun_server"] = "stun:stun.l.google.com:19302";
    config_["signaling.ws_port"] = "8765";

    // 日志配置
    config_["log.level"] = "INFO";
    config_["log.file"] = "smart_classroom.log";
}

bool ConfigManager::loadFromFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json jsonConfig;
        file >> jsonConfig;

        // 扁平化 JSON 配置到 key-value 映射
        // 例如: {"detector": {"type": "DLIB_CNN_CUDA"}} -> "detector.type" = "DLIB_CNN_CUDA"
        std::function<void(const nlohmann::json&, const std::string&)> flatten;
        flatten = [&](const nlohmann::json& j, const std::string& prefix) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
                if (it->is_object()) {
                    flatten(*it, key);
                } else if (it->is_string()) {
                    config_[key] = it->get<std::string>();
                } else {
                    // 将数值/布尔转为字符串存储
                    std::ostringstream oss;
                    oss << *it;
                    config_[key] = oss.str();
                }
            }
        };
        flatten(jsonConfig, "");

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.find(key);
    return (it != config_.end()) ? it->second : defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.find(key);
    if (it == config_.end()) return defaultValue;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return defaultValue;
    }
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.find(key);
    if (it == config_.end()) return defaultValue;
    try {
        return std::stod(it->second);
    } catch (...) {
        return defaultValue;
    }
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_.find(key);
    if (it == config_.end()) return defaultValue;
    return (it->second == "true" || it->second == "1" || it->second == "yes");
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_[key] = value;
}

} // namespace smart_classroom
