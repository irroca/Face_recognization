// ============================================================
// OpenCvDnnDetector.cpp — OpenCV DNN 人脸检测器实现
// ============================================================
// [Design Pattern: Strategy] 具体策略实现
// 使用 OpenCV DNN 模块的 SSD (Single Shot MultiBox Detector)
// 进行人脸检测。支持 CUDA 后端加速。
// ============================================================

#include "vision/detectors/OpenCvDnnDetector.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include "core/CudaResourceManager.h"

namespace smart_classroom {

OpenCvDnnDetector::OpenCvDnnDetector(const std::string& configPath,
                                     const std::string& weightsPath) {
    // [Design Pattern: Singleton] 从 ConfigManager 获取路径和阈值
    auto& config = ConfigManager::getInstance();

    std::string config_file = configPath.empty()
        ? config.getString("model.opencv_dnn_config", "models/deploy.prototxt")
        : configPath;
    std::string weights_file = weightsPath.empty()
        ? config.getString("model.opencv_dnn_weights",
            "models/res10_300x300_ssd_iter_140000.caffemodel")
        : weightsPath;

    confidenceThreshold_ = static_cast<float>(
        config.getDouble("detection.confidence_threshold", 0.6));

    try {
        net_ = cv::dnn::readNetFromCaffe(config_file, weights_file);
        configureCudaBackend();
        initialized_ = true;
        LOG_INFO("OpenCvDnnDetector initialized successfully");
    } catch (const cv::Exception& e) {
        LOG_ERROR("Failed to load OpenCV DNN model: " + std::string(e.what()));
        initialized_ = false;
    }
}

void OpenCvDnnDetector::configureCudaBackend() {
    // [Design Pattern: Singleton] 检查 CUDA 可用性并配置后端
    auto& cudaMgr = CudaResourceManager::getInstance();
    if (cudaMgr.isCudaAvailable()) {
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        LOG_INFO("OpenCvDnnDetector: using CUDA backend on "
                 + cudaMgr.getDeviceName());
    } else {
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        LOG_INFO("OpenCvDnnDetector: using CPU backend");
    }
}

// [Design Pattern: Strategy] 具体策略的算法实现
std::vector<FaceInfo> OpenCvDnnDetector::detect(const cv::Mat& frame) {
    std::vector<FaceInfo> results;

    if (!initialized_) {
        LOG_ERROR("OpenCvDnnDetector: model not initialized");
        return results;
    }

    if (frame.empty()) {
        LOG_WARN("OpenCvDnnDetector: received empty frame");
        return results;
    }

    // 构建输入 blob：300x300，归一化，均值减除
    cv::Mat blob = cv::dnn::blobFromImage(
        frame, 1.0, cv::Size(300, 300),
        cv::Scalar(104.0, 177.0, 123.0), false, false);

    net_.setInput(blob);
    cv::Mat detections = net_.forward();

    // 解析 SSD 输出格式: [1, 1, N, 7]
    // 每行: [batchId, classId, confidence, x1, y1, x2, y2]
    cv::Mat detectionMat(detections.size[2], detections.size[3],
                         CV_32F, detections.ptr<float>());

    for (int i = 0; i < detectionMat.rows; ++i) {
        float confidence = detectionMat.at<float>(i, 2);
        if (confidence < confidenceThreshold_) continue;

        // 坐标从归一化 [0,1] 转为像素坐标
        int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
        int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
        int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
        int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

        // 边界裁剪
        x1 = std::max(0, x1);
        y1 = std::max(0, y1);
        x2 = std::min(frame.cols, x2);
        y2 = std::min(frame.rows, y2);

        FaceInfo face;
        face.bbox = BoundingBox(x1, y1, x2 - x1, y2 - y1);
        face.detectionConfidence = confidence;
        results.push_back(face);
    }

    LOG_DEBUG("OpenCvDnnDetector: detected " + std::to_string(results.size()) + " face(s)");
    return results;
}

} // namespace smart_classroom
