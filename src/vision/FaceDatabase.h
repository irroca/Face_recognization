#pragma once
// ============================================================
// FaceDatabase.h — 人脸特征数据库
// ============================================================
// 管理已注册学生的人脸特征库，提供特征注册 (enroll)、比对
// (match)、文件持久化 (load/save) 功能。
//
// 识别流程：IFaceRecognizer 提取出 128D 特征向量后，调用
// FaceDatabase::match() 在已注册特征中查找最近邻，返回身份。
// ============================================================

#include "core/Types.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace smart_classroom {

class FaceDatabase {
public:
    FaceDatabase();
    ~FaceDatabase() = default;

    // 注册新人脸（学生 ID + 姓名 + 特征向量）
    void enrollFace(const FaceIdentity& identity);

    // 在已注册库中查找最匹配的身份
    // 返回匹配的身份字符串（"学号:姓名"格式），distance 输出特征距离
    // 若距离超过阈值，返回空字符串表示未识别
    std::string match(const std::vector<float>& feature, float& distance) const;

    // 从 JSON 文件加载已注册人脸库
    bool loadFromFile(const std::string& dbPath);

    // 保存到 JSON 文件
    bool saveToFile(const std::string& dbPath) const;

    // 获取已注册人脸数量
    size_t size() const;

    // 获取所有已注册身份
    std::vector<FaceIdentity> getAllIdentities() const;

    // 设置匹配距离阈值
    void setMatchThreshold(float threshold) { matchThreshold_ = threshold; }

private:
    // studentId -> FaceIdentity (支持同一人注册多个特征)
    std::unordered_map<std::string, FaceIdentity> identities_;
    mutable std::mutex mutex_;
    float matchThreshold_ = 0.6f;  // 欧氏距离阈值

    // 计算两个特征向量的欧氏距离
    static float euclideanDistance(const std::vector<float>& a,
                                   const std::vector<float>& b);
};

} // namespace smart_classroom
