// ============================================================
// DlibFaceRecognizer.cpp — Dlib ResNet 人脸识别器实现
// ============================================================
// [Design Pattern: Strategy] 具体策略实现
// 使用 Dlib ResNet 提取 128D 人脸特征描述符，通过欧氏距离
// 与 FaceDatabase 中已注册特征进行比对以识别身份。
// ============================================================

#include "vision/recognizers/DlibFaceRecognizer.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include <dlib/opencv.h>
#include <dlib/image_processing/render_face_detections.h>
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

DlibFaceRecognizer::DlibFaceRecognizer(std::shared_ptr<FaceDatabase> database,
                                       const std::string& recognitionModelPath,
                                       const std::string& shapePredictorPath) 
    : database_(std::move(database)) {
    // [Design Pattern: Singleton] 从 ConfigManager 获取模型路径
    auto& config = ConfigManager::getInstance();

    std::string recModelPath = recognitionModelPath.empty()
        ? config.getString("model.dlib_resnet",
            "models/dlib_face_recognition_resnet_model_v1.dat")
        : recognitionModelPath;

    std::string spPath = shapePredictorPath.empty()
        ? config.getString("model.shape_predictor",
            "models/shape_predictor_68_face_landmarks.dat")
        : shapePredictorPath;

    try {
        // 加载 68 点人脸关键点检测模型（用于人脸对齐）
        dlib::deserialize(spPath) >> shapePredictor_;

        // 加载 ResNet 128D 特征提取模型
        dlib::deserialize(recModelPath) >> net_;

        initialized_ = true;
        LOG_INFO("DlibFaceRecognizer initialized successfully");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load DlibFaceRecognizer models: "
                  + std::string(e.what()));
        initialized_ = false;
    }
}

// [Design Pattern: Strategy] 提取 128D 特征向量
std::vector<float> DlibFaceRecognizer::extractFeature(const cv::Mat& faceChip) {
    std::vector<float> result;

    if (!initialized_ || faceChip.empty()) {
        return result;
    }

    try {
        // OpenCV BGR → Dlib RGB
        cv::Mat rgb;
        cv::cvtColor(faceChip, rgb, cv::COLOR_BGR2RGB);
        dlib::matrix<dlib::rgb_pixel> dlibFace;
        dlib::assign_image(dlibFace, dlib::cv_image<dlib::rgb_pixel>(rgb));

        // 将人脸图像 resize 到模型要求的 150x150
        dlib::matrix<dlib::rgb_pixel> faceChipResized(150, 150);
        dlib::resize_image(dlibFace, faceChipResized);

        // 提取 128D 特征描述符
        dlib::matrix<float, 0, 1> faceDescriptor = net_(faceChipResized);

        // 转换为 std::vector<float>
        result.resize(128);
        for (int i = 0; i < 128; ++i) {
            result[i] = faceDescriptor(i);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Feature extraction failed: " + std::string(e.what()));
    }

    return result;
}

// [Design Pattern: Strategy] 识别身份
std::string DlibFaceRecognizer::recognize(const cv::Mat& faceChip, float& confidence) {
    confidence = -1.0f;

    if (!initialized_ || !database_) {
        return "";
    }

    // 提取特征
    auto feature = extractFeature(faceChip);
    if (feature.empty()) {
        return "";
    }

    // 在 FaceDatabase 中匹配
    float distance = 0.0f;
    std::string identity = database_->match(feature, distance);

    confidence = distance;

    if (!identity.empty()) {
        LOG_DEBUG("DlibFaceRecognizer: identified '" + identity
                  + "' with distance " + std::to_string(distance));
    }

    return identity;
}

} // namespace smart_classroom
