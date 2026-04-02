#pragma once
// ============================================================
// IFaceRecognizer.h — 人脸身份识别策略接口
// ============================================================
// [Design Pattern: Strategy]
// 定义人脸身份识别（"谁是谁"）的抽象接口，将识别算法与使用者
// 解耦。
//
// 动机：与 IFaceDetector（检测"哪里有脸"）不同，IFaceRecognizer
// 解决的是"这张脸属于谁"的问题。系统需要支持多种识别方法：
//   - Dlib ResNet 提取 128D 特征向量 + 欧氏距离比对
//   - OpenCV LBPH 局部二值模式直方图
//
// 如果将所有识别逻辑硬编码在一起，会导致"过大的类"(Large Class)
// 和"违反单一职责原则"(SRP)。策略模式让每种识别方法独立封装，
// 可在运行时灵活切换，新增识别方法无需修改已有代码。
//
// 与 IFaceDetector 共同构成"双层策略模式"，分别解耦检测和
// 识别两个正交维度的算法变化。
// ============================================================

#include "core/Types.h"
#include <vector>
#include <string>
#include <opencv2/core.hpp>

namespace smart_classroom {

// ============================================================
// [Design Pattern: Strategy] 身份识别策略接口
// ============================================================
class IFaceRecognizer {
public:
    virtual ~IFaceRecognizer() = default;

    // 从裁剪的人脸图像中提取特征向量（如 128D 描述符）
    virtual std::vector<float> extractFeature(const cv::Mat& faceChip) = 0;

    // 识别身份：输入人脸图像，返回最匹配的身份标识
    // confidence 输出匹配的距离（越小越相似）
    virtual std::string recognize(const cv::Mat& faceChip, float& confidence) = 0;

    // 返回当前策略的名称
    virtual std::string getName() const = 0;

    // 是否需要 CUDA 支持
    virtual bool requiresCuda() const { return false; }
};

} // namespace smart_classroom
