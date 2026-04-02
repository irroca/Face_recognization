// ============================================================
// DlibHogDetector.cpp — Dlib HOG + SVM 人脸检测器实现
// ============================================================
// [Design Pattern: Strategy] 具体策略实现
// 使用 Dlib 的预训练 HOG 人脸检测器，纯 CPU 运算。
// ============================================================

#include "vision/detectors/DlibHogDetector.h"
#include "core/Logger.h"
#include <dlib/opencv.h>
#include <dlib/image_processing.h>

namespace smart_classroom {

DlibHogDetector::DlibHogDetector() {
    // Dlib 内置的 HOG 人脸检测器无需额外模型文件
    detector_ = dlib::get_frontal_face_detector();
    LOG_INFO("DlibHogDetector initialized (CPU mode)");
}

// [Design Pattern: Strategy] 具体策略的算法实现
std::vector<FaceInfo> DlibHogDetector::detect(const cv::Mat& frame) {
    std::vector<FaceInfo> results;

    if (frame.empty()) {
        LOG_WARN("DlibHogDetector: received empty frame");
        return results;
    }

    // 将 OpenCV Mat 转换为 Dlib 图像格式
    dlib::cv_image<dlib::bgr_pixel> dlibImage(frame);

    // 执行 HOG + SVM 检测，返回检测到的人脸矩形列表和置信度
    std::vector<std::pair<double, dlib::rectangle>> detections;
    detector_(dlibImage, detections, -0.5);  // 阈值可通过配置调整

    for (const auto& [confidence, rect] : detections) {
        FaceInfo face;
        face.bbox = BoundingBox(
            static_cast<int>(rect.left()),
            static_cast<int>(rect.top()),
            static_cast<int>(rect.width()),
            static_cast<int>(rect.height())
        );
        face.detectionConfidence = static_cast<float>(confidence);
        results.push_back(face);
    }

    LOG_DEBUG("DlibHogDetector: detected " + std::to_string(results.size()) + " face(s)");
    return results;
}

} // namespace smart_classroom
