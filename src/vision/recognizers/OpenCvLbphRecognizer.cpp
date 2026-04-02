// ============================================================
// OpenCvLbphRecognizer.cpp — OpenCV LBPH 人脸识别器实现
// ============================================================
// [Design Pattern: Strategy] 具体策略实现
// LBPH 是一种基于纹理特征的方法，不需要深度学习模型，
// 纯 CPU 即可运行。作为 DlibFaceRecognizer 的备选策略。
// ============================================================

#include "vision/recognizers/OpenCvLbphRecognizer.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

OpenCvLbphRecognizer::OpenCvLbphRecognizer(std::shared_ptr<FaceDatabase> database)
    : database_(std::move(database)) {
    recognizer_ = cv::face::LBPHFaceRecognizer::create(
        1,    // radius
        8,    // neighbors
        8,    // grid_x
        8,    // grid_y
        80.0  // threshold
    );
    LOG_INFO("OpenCvLbphRecognizer initialized");
}

bool OpenCvLbphRecognizer::train(const std::vector<cv::Mat>& faces,
                                  const std::vector<int>& labels,
                                  const std::vector<std::string>& labelNames) {
    if (faces.empty() || faces.size() != labels.size()) {
        LOG_ERROR("OpenCvLbphRecognizer: invalid training data");
        return false;
    }

    // 将图像转为灰度并统一尺寸
    std::vector<cv::Mat> grayFaces;
    for (const auto& face : faces) {
        cv::Mat gray;
        if (face.channels() > 1) {
            cv::cvtColor(face, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = face.clone();
        }
        cv::Mat resized;
        cv::resize(gray, resized, cv::Size(100, 100));
        grayFaces.push_back(resized);
    }

    recognizer_->train(grayFaces, labels);
    labelNames_ = labelNames;
    trained_ = true;

    LOG_INFO("OpenCvLbphRecognizer: trained with "
             + std::to_string(faces.size()) + " face images");
    return true;
}

// [Design Pattern: Strategy] 提取特征
// 注：LBPH 不直接生成可比较的特征向量，返回空
std::vector<float> OpenCvLbphRecognizer::extractFeature(const cv::Mat& faceChip) {
    // LBPH 的内部特征无法直接暴露为通用向量
    // 此方法返回空向量；识别通过 recognize() 完成
    (void)faceChip;
    return {};
}

// [Design Pattern: Strategy] 识别身份
std::string OpenCvLbphRecognizer::recognize(const cv::Mat& faceChip, float& confidence) {
    confidence = -1.0f;

    if (!trained_) {
        LOG_WARN("OpenCvLbphRecognizer: model not trained yet");
        return "";
    }

    if (faceChip.empty()) return "";

    cv::Mat gray;
    if (faceChip.channels() > 1) {
        cv::cvtColor(faceChip, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = faceChip;
    }
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(100, 100));

    int label = -1;
    double dist = 0.0;
    recognizer_->predict(resized, label, dist);

    confidence = static_cast<float>(dist);

    // [Design Pattern: Singleton] 获取识别阈值
    double threshold = ConfigManager::getInstance().getDouble(
        "recognition.distance_threshold", 80.0);

    if (label >= 0 && dist < threshold
        && label < static_cast<int>(labelNames_.size())) {
        std::string identity = labelNames_[label];
        LOG_DEBUG("OpenCvLbphRecognizer: identified '" + identity
                  + "' with distance " + std::to_string(dist));
        return identity;
    }

    return "";
}

} // namespace smart_classroom
