#pragma once
// ============================================================
// PreprocessFilter.h — 预处理过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 2 个过滤器
// 对解码后的图像进行预处理：缩放、颜色空间转换、直方图均衡。
// ============================================================

#include "pipeline/IFilter.h"

namespace smart_classroom {

class PreprocessFilter : public IFilter {
public:
    PreprocessFilter();
    ~PreprocessFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "PreprocessFilter"; }

private:
    int targetWidth_;
    int targetHeight_;
};

} // namespace smart_classroom
