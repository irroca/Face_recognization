// ============================================================
// DecodeFilter.cpp — 解码过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道第 1 级
// 职责：将编码数据解码为 cv::Mat，为后续过滤器提供原始像素。
// ============================================================

#include "pipeline/filters/DecodeFilter.h"
#include "core/Logger.h"
#include <opencv2/imgcodecs.hpp>

namespace smart_classroom {

// [Design Pattern: Pipeline-Filter] 过滤器处理实现
bool DecodeFilter::process(VideoFrame& frame) {
    // 如果已经是原始 BGR/RGB 格式，无需解码，直接透传
    if (frame.format == FrameFormat::RAW_BGR ||
        frame.format == FrameFormat::RAW_RGB) {
        LOG_DEBUG("DecodeFilter: frame already decoded, passing through");
        return true;
    }

    // 解码 JPEG 编码数据
    if (frame.format == FrameFormat::ENCODED_JPEG) {
        if (frame.encodedData.empty()) {
            LOG_WARN("DecodeFilter: no encoded data to decode");
            return false;
        }

        frame.image = cv::imdecode(frame.encodedData, cv::IMREAD_COLOR);
        if (frame.image.empty()) {
            LOG_ERROR("DecodeFilter: JPEG decode failed");
            return false;
        }

        frame.format = FrameFormat::RAW_BGR;
        frame.encodedData.clear();  // 释放编码数据
        LOG_DEBUG("DecodeFilter: decoded JPEG -> BGR ("
                  + std::to_string(frame.image.cols) + "x"
                  + std::to_string(frame.image.rows) + ")");
        return true;
    }

    // H.264 / VP8 解码需要借助 FFmpeg 或硬件解码器
    // 此处为骨架实现，实际项目中应集成 FFmpeg 或 NVDEC
    if (frame.format == FrameFormat::ENCODED_H264 ||
        frame.format == FrameFormat::ENCODED_VP8) {
        LOG_WARN("DecodeFilter: H.264/VP8 decoding not yet implemented "
                 "(requires FFmpeg/NVDEC integration)");
        // TODO: 集成 FFmpeg avcodec 或 NVIDIA Video Codec SDK
        return false;
    }

    LOG_ERROR("DecodeFilter: unsupported frame format");
    return false;
}

} // namespace smart_classroom
