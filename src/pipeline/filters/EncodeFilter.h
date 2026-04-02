#pragma once
// ============================================================
// EncodeFilter.h — 编码过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 6 个（最后一个）过滤器
// 将处理后的 cv::Mat 编码为指定格式（JPEG 等），准备通过
// WebRTC 推回给前端。
// ============================================================

#include "pipeline/IFilter.h"

namespace smart_classroom {

class EncodeFilter : public IFilter {
public:
    EncodeFilter();
    ~EncodeFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "EncodeFilter"; }

private:
    int jpegQuality_;
    std::string encodeFormat_;
};

} // namespace smart_classroom
