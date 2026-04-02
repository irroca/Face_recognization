// ============================================================
// EncodeFilter.cpp — 编码过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道最后一级
// 职责：将标注完毕的图像编码，准备通过 WebRTC 传输。
// ============================================================

#include "pipeline/filters/EncodeFilter.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"
#include <opencv2/imgcodecs.hpp>

namespace smart_classroom {

EncodeFilter::EncodeFilter() {
    // [Design Pattern: Singleton] 从全局配置获取编码参数
    auto& config = ConfigManager::getInstance();
    encodeFormat_ = config.getString("encode.format", "JPEG");
    jpegQuality_ = config.getInt("encode.jpeg_quality", 85);
    LOG_INFO("EncodeFilter: format=" + encodeFormat_
             + ", quality=" + std::to_string(jpegQuality_));
}

// [Design Pattern: Pipeline-Filter] 过滤器处理实现
bool EncodeFilter::process(VideoFrame& frame) {
    if (frame.image.empty()) {
        LOG_WARN("EncodeFilter: received empty image");
        return false;
    }

    if (encodeFormat_ == "JPEG") {
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpegQuality_};
        bool success = cv::imencode(".jpg", frame.image, frame.encodedData, params);

        if (!success) {
            LOG_ERROR("EncodeFilter: JPEG encoding failed");
            return false;
        }

        frame.format = FrameFormat::ENCODED_JPEG;
        LOG_DEBUG("EncodeFilter: encoded to JPEG ("
                  + std::to_string(frame.encodedData.size()) + " bytes)");
        return true;
    }

    // H.264 / VP8 编码需要硬件编码器（NVENC for RTX 4060）
    // 此处为骨架实现
    if (encodeFormat_ == "H264" || encodeFormat_ == "VP8") {
        LOG_WARN("EncodeFilter: H.264/VP8 encoding not yet implemented "
                 "(requires NVENC/FFmpeg integration)");
        // TODO: 集成 NVENC 或 FFmpeg libavcodec
        // 对于 WebRTC 场景，应使用 H.264 Baseline Profile
        return false;
    }

    LOG_ERROR("EncodeFilter: unsupported encode format: " + encodeFormat_);
    return false;
}

} // namespace smart_classroom
