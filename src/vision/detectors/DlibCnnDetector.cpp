// ============================================================
// DlibCnnDetector.cpp — Dlib CNN (CUDA) 人脸检测器实现
// ============================================================
// [Design Pattern: Strategy] 具体策略实现
// 使用 Dlib 的 MMOD CNN 模型进行人脸检测。当系统编译时启用
// CUDA 且 RTX 4060 可用时，Dlib 会自动使用 GPU 加速推理。
// ============================================================

#include "vision/detectors/DlibCnnDetector.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "core/CudaResourceManager.h"
#include <dlib/opencv.h>
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

DlibCnnDetector::DlibCnnDetector(const std::string& modelPath) {
    std::string path = modelPath;
    if (path.empty()) {
        // [Design Pattern: Singleton] 通过 ConfigManager 单例获取模型路径
        path = ConfigManager::getInstance().getString("model.dlib_cnn",
            "models/mmod_human_face_detector.dat");
    }

    try {
        // [Design Pattern: Singleton] 检查 CUDA 可用性——CNN 检测器必须有 GPU
        auto& cudaMgr = CudaResourceManager::getInstance();
        if (!cudaMgr.isCudaAvailable()) {
            LOG_ERROR("DlibCnnDetector requires CUDA but no GPU available. "
                      "Please use DLIB_HOG or OPENCV_DNN instead.");
            initialized_ = false;
            return;
        }

        dlib::deserialize(path) >> net_;
        initialized_ = true;
        LOG_INFO("DlibCnnDetector initialized with CUDA acceleration on "
                 + cudaMgr.getDeviceName());
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load DlibCnnDetector model: " + std::string(e.what()));
        initialized_ = false;
    }
}

// [Design Pattern: Strategy] 具体策略的算法实现（CUDA 加速版本）
std::vector<FaceInfo> DlibCnnDetector::detect(const cv::Mat& frame) {
    std::vector<FaceInfo> results;

    if (!initialized_) {
        LOG_ERROR("DlibCnnDetector: model not initialized");
        return results;
    }

    if (frame.empty()) {
        LOG_WARN("DlibCnnDetector: received empty frame");
        return results;
    }

    // 将 OpenCV BGR 转换为 Dlib RGB 图像
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    dlib::matrix<dlib::rgb_pixel> dlibImage;
    dlib::assign_image(dlibImage, dlib::cv_image<dlib::rgb_pixel>(rgb));

    // 执行 CNN 推理（Dlib 在编译时若检测到 CUDA，会自动使用 GPU）
    auto detections = net_(dlibImage);

    for (const auto& det : detections) {
        FaceInfo face;
        face.bbox = BoundingBox(
            static_cast<int>(det.rect.left()),
            static_cast<int>(det.rect.top()),
            static_cast<int>(det.rect.width()),
            static_cast<int>(det.rect.height())
        );
        face.detectionConfidence = static_cast<float>(det.detection_confidence);
        results.push_back(face);
    }

    LOG_DEBUG("DlibCnnDetector: detected " + std::to_string(results.size()) + " face(s)");
    return results;
}

} // namespace smart_classroom
