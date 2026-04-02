#pragma once
// ============================================================
// OpenCvDnnDetector.h — OpenCV DNN 人脸检测器
// ============================================================
// [Design Pattern: Strategy] 具体策略类
// 使用 OpenCV DNN 模块加载预训练的 SSD 人脸检测模型，支持
// 通过 cv::dnn::DNN_BACKEND_CUDA 和 cv::dnn::DNN_TARGET_CUDA
// 启用 GPU 加速推理。
// ============================================================

#include "vision/IFaceDetector.h"
#include <opencv2/dnn.hpp>

namespace smart_classroom {

class OpenCvDnnDetector : public IFaceDetector {
public:
    OpenCvDnnDetector(const std::string& configPath = "",
                      const std::string& weightsPath = "");
    ~OpenCvDnnDetector() override = default;

    // [Design Pattern: Strategy] 实现策略接口
    std::vector<FaceInfo> detect(const cv::Mat& frame) override;
    std::string getName() const override { return "OpenCvDnnDetector"; }
    bool requiresCuda() const override { return false; }  // 可选 CUDA，非必需

private:
    cv::dnn::Net net_;
    float confidenceThreshold_ = 0.6f;
    bool initialized_ = false;

    // 配置 CUDA 后端
    void configureCudaBackend();
};

} // namespace smart_classroom
