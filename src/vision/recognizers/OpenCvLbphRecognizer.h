#pragma once
// ============================================================
// OpenCvLbphRecognizer.h — OpenCV LBPH 人脸识别器
// ============================================================
// [Design Pattern: Strategy] 具体策略类
// 使用 OpenCV 的 LBPH (Local Binary Patterns Histograms) 方法
// 进行人脸身份识别。相比 Dlib ResNet，LBPH 更轻量但精度较低，
// 适用于不需要 GPU 的场景或作为备选策略。
// ============================================================

#include "vision/IFaceRecognizer.h"
#include "vision/FaceDatabase.h"
#include <opencv2/face.hpp>
#include <memory>

namespace smart_classroom {

class OpenCvLbphRecognizer : public IFaceRecognizer {
public:
    explicit OpenCvLbphRecognizer(std::shared_ptr<FaceDatabase> database);
    ~OpenCvLbphRecognizer() override = default;

    // [Design Pattern: Strategy] 实现策略接口
    std::vector<float> extractFeature(const cv::Mat& faceChip) override;
    std::string recognize(const cv::Mat& faceChip, float& confidence) override;
    std::string getName() const override { return "OpenCvLbphRecognizer"; }
    bool requiresCuda() const override { return false; }

    // LBPH 需要训练步骤
    bool train(const std::vector<cv::Mat>& faces,
               const std::vector<int>& labels,
               const std::vector<std::string>& labelNames);

private:
    cv::Ptr<cv::face::LBPHFaceRecognizer> recognizer_;
    std::shared_ptr<FaceDatabase> database_;
    std::vector<std::string> labelNames_;
    bool trained_ = false;
};

} // namespace smart_classroom
