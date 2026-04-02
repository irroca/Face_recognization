// ============================================================
// FaceDatabase.cpp — 人脸特征数据库实现
// ============================================================

#include "vision/FaceDatabase.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <limits>

namespace smart_classroom {

void FaceDatabase::enrollFace(const FaceIdentity& identity) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (identity.studentId.empty() || identity.featureDescriptor.empty()) {
        LOG_WARN("FaceDatabase: cannot enroll with empty studentId or feature");
        return;
    }

    identities_[identity.studentId] = identity;
    LOG_INFO("FaceDatabase: enrolled face for student '"
             + identity.name + "' (ID: " + identity.studentId + ")");
}

std::string FaceDatabase::match(const std::vector<float>& feature, float& distance) const {
    std::lock_guard<std::mutex> lock(mutex_);

    distance = std::numeric_limits<float>::max();
    std::string bestMatch;

    if (feature.empty() || identities_.empty()) {
        return "";
    }

    // 遍历所有已注册特征，找到最近邻
    for (const auto& [id, identity] : identities_) {
        float dist = euclideanDistance(feature, identity.featureDescriptor);
        if (dist < distance) {
            distance = dist;
            bestMatch = identity.studentId + ":" + identity.name;
        }
    }

    // [Design Pattern: Singleton] 使用 ConfigManager 获取动态阈值
    float threshold = static_cast<float>(
        ConfigManager::getInstance().getDouble(
            "recognition.distance_threshold", matchThreshold_));

    if (distance > threshold) {
        LOG_DEBUG("FaceDatabase: best match distance " + std::to_string(distance)
                  + " exceeds threshold " + std::to_string(threshold));
        return "";  // 超过阈值，视为未识别
    }

    return bestMatch;
}

bool FaceDatabase::loadFromFile(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ifstream file(dbPath);
    if (!file.is_open()) {
        LOG_WARN("FaceDatabase: cannot open file " + dbPath);
        return false;
    }

    try {
        nlohmann::json jsonDb;
        file >> jsonDb;

        identities_.clear();
        for (const auto& entry : jsonDb["faces"]) {
            FaceIdentity identity;
            identity.studentId = entry["student_id"].get<std::string>();
            identity.name = entry["name"].get<std::string>();
            identity.featureDescriptor = entry["feature"].get<std::vector<float>>();
            identities_[identity.studentId] = identity;
        }

        LOG_INFO("FaceDatabase: loaded " + std::to_string(identities_.size())
                 + " face(s) from " + dbPath);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("FaceDatabase: failed to parse " + dbPath + ": " + e.what());
        return false;
    }
}

bool FaceDatabase::saveToFile(const std::string& dbPath) const {
    std::lock_guard<std::mutex> lock(mutex_);

    nlohmann::json jsonDb;
    jsonDb["faces"] = nlohmann::json::array();

    for (const auto& [id, identity] : identities_) {
        nlohmann::json entry;
        entry["student_id"] = identity.studentId;
        entry["name"] = identity.name;
        entry["feature"] = identity.featureDescriptor;
        jsonDb["faces"].push_back(entry);
    }

    std::ofstream file(dbPath);
    if (!file.is_open()) {
        LOG_ERROR("FaceDatabase: cannot write to " + dbPath);
        return false;
    }

    file << jsonDb.dump(2);
    LOG_INFO("FaceDatabase: saved " + std::to_string(identities_.size())
             + " face(s) to " + dbPath);
    return true;
}

size_t FaceDatabase::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return identities_.size();
}

std::vector<FaceIdentity> FaceDatabase::getAllIdentities() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<FaceIdentity> result;
    result.reserve(identities_.size());
    for (const auto& [id, identity] : identities_) {
        result.push_back(identity);
    }
    return result;
}

float FaceDatabase::euclideanDistance(const std::vector<float>& a,
                                      const std::vector<float>& b) {
    if (a.size() != b.size()) return std::numeric_limits<float>::max();

    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

} // namespace smart_classroom
