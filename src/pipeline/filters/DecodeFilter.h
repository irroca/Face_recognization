#pragma once
// ============================================================
// DecodeFilter.h — 解码过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 1 个过滤器
// 将编码的视频数据（JPEG/H.264/VP8）解码为 OpenCV Mat 格式。
// ============================================================

#include "pipeline/IFilter.h"

namespace smart_classroom {

class DecodeFilter : public IFilter {
public:
    DecodeFilter() = default;
    ~DecodeFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "DecodeFilter"; }
};

} // namespace smart_classroom
