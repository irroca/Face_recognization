// ============================================================
// PreprocessFilter.cpp — 预处理过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道第 2 级
// 职责：将图像缩放到统一尺寸，便于后续检测/识别处理。
// ============================================================

#include "pipeline/filters/PreprocessFilter.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

PreprocessFilter::PreprocessFilter() {
    // [Design Pattern: Singleton] 从全局配置获取目标尺寸
    auto& config = ConfigManager::getInstance();
    targetWidth_ = config.getInt("preprocess.resize_width", 640);
    targetHeight_ = config.getInt("preprocess.resize_height", 480);
    LOG_INFO("PreprocessFilter: target size " + std::to_string(targetWidth_)
             + "x" + std::to_string(targetHeight_));
}

// [Design Pattern: Pipeline-Filter] 过滤器处理实现
bool PreprocessFilter::process(VideoFrame& frame) {
    if (frame.image.empty()) {
        LOG_WARN("PreprocessFilter: received empty image");
        return false;
    }

    // 如果图像尺寸与目标不同，进行缩放
    if (frame.image.cols != targetWidth_ || frame.image.rows != targetHeight_) {
        cv::Mat resized;
        cv::resize(frame.image, resized, cv::Size(targetWidth_, targetHeight_),
                   0, 0, cv::INTER_LINEAR);
        frame.image = std::move(resized);
        LOG_DEBUG("PreprocessFilter: resized to " + std::to_string(targetWidth_)
                  + "x" + std::to_string(targetHeight_));
    }

    // 确保颜色空间是 BGR（OpenCV 默认）
    if (frame.format == FrameFormat::RAW_RGB) {
        cv::cvtColor(frame.image, frame.image, cv::COLOR_RGB2BGR);
        frame.format = FrameFormat::RAW_BGR;
    }

    return true;
}

} // namespace smart_classroom
