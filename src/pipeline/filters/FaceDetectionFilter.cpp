// ============================================================
// FaceDetectionFilter.cpp — 人脸检测过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道第 3 级
// [Design Pattern: Strategy] 委托给 IFaceDetector 策略执行检测
// ============================================================

#include "pipeline/filters/FaceDetectionFilter.h"
#include "core/Logger.h"
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

// [Design Pattern: Strategy] 构造时注入检测策略
FaceDetectionFilter::FaceDetectionFilter(std::shared_ptr<IFaceDetector> detector)
    : detector_(std::move(detector)) {
    if (detector_) {
        LOG_INFO("FaceDetectionFilter: using detector '" + detector_->getName() + "'");
    } else {
        LOG_WARN("FaceDetectionFilter: no detector provided");
    }
}

// [Design Pattern: Pipeline-Filter] 过滤器处理
// [Design Pattern: Strategy] 委托检测任务给策略对象
bool FaceDetectionFilter::process(VideoFrame& frame) {
    if (!detector_) {
        LOG_ERROR("FaceDetectionFilter: no detector configured");
        return false;
    }

    if (frame.image.empty()) {
        LOG_WARN("FaceDetectionFilter: received empty image");
        return false;
    }

    // [Design Pattern: Strategy] 调用当前策略的 detect() 方法
    // 性能优化：在缩小的图像上检测，然后将坐标映射回原始尺寸
    cv::Mat detectImage = frame.image;
    float scale = 1.0f;
    const int MAX_DETECT_DIM = 320;  // 检测用的最大分辨率

    if (frame.image.cols > MAX_DETECT_DIM || frame.image.rows > MAX_DETECT_DIM) {
        scale = static_cast<float>(MAX_DETECT_DIM) /
                static_cast<float>(std::max(frame.image.cols, frame.image.rows));
        cv::resize(frame.image, detectImage, cv::Size(),
                   scale, scale, cv::INTER_LINEAR);
    }

    frame.detectedFaces = detector_->detect(detectImage);

    // 将检测框坐标映射回原始图像尺寸
    if (scale < 1.0f) {
        float invScale = 1.0f / scale;
        for (auto& face : frame.detectedFaces) {
            face.bbox.x = static_cast<int>(static_cast<float>(face.bbox.x) * invScale);
            face.bbox.y = static_cast<int>(static_cast<float>(face.bbox.y) * invScale);
            face.bbox.width = static_cast<int>(static_cast<float>(face.bbox.width) * invScale);
            face.bbox.height = static_cast<int>(static_cast<float>(face.bbox.height) * invScale);
        }
    }

    LOG_DEBUG("FaceDetectionFilter: detected "
              + std::to_string(frame.detectedFaces.size()) + " face(s) using "
              + detector_->getName());

    // 即使未检测到人脸也继续管道（下游 Filter 需要处理零结果的情况）
    return true;
}

// [Design Pattern: Strategy] 运行时切换检测算法——无需重建管道
void FaceDetectionFilter::setDetector(std::shared_ptr<IFaceDetector> detector) {
    detector_ = std::move(detector);
    if (detector_) {
        LOG_INFO("FaceDetectionFilter: switched to detector '"
                 + detector_->getName() + "'");
    }
}

} // namespace smart_classroom
