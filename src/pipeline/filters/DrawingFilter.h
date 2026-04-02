#pragma once
// ============================================================
// DrawingFilter.h — 绘制过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 5 个过滤器
// 根据 frame.detectedFaces 中的检测/识别结果，在图像上绘制
// 人脸包围框和身份标签（姓名/学号）。
// ============================================================

#include "pipeline/IFilter.h"

namespace smart_classroom {

class DrawingFilter : public IFilter {
public:
    DrawingFilter() = default;
    ~DrawingFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "DrawingFilter"; }
};

} // namespace smart_classroom
