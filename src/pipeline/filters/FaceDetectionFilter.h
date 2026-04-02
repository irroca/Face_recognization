#pragma once
// ============================================================
// FaceDetectionFilter.h — 人脸检测过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 3 个过滤器
// [Design Pattern: Strategy] 内部持有 IFaceDetector 策略对象
//
// 这是管道-过滤器架构与策略模式的**交叉点**：
// - 作为 Pipeline 中的一个 Filter，遵循 IFilter 接口
// - 内部将检测任务委托给 IFaceDetector 策略对象
// - 检测结果写入 VideoFrame::detectedFaces，供下游 Filter 使用
//
// 这种设计实现了 Pipeline 架构与算法选择的双重解耦：
//   1. Pipeline 不关心检测如何实现（IFilter 接口）
//   2. FaceDetectionFilter 不关心用哪个检测算法（IFaceDetector 接口）
// ============================================================

#include "pipeline/IFilter.h"
#include "vision/IFaceDetector.h"
#include <memory>

namespace smart_classroom {

class FaceDetectionFilter : public IFilter {
public:
    // [Design Pattern: Strategy] 通过构造函数注入检测策略
    explicit FaceDetectionFilter(std::shared_ptr<IFaceDetector> detector);
    ~FaceDetectionFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "FaceDetectionFilter"; }

    // [Design Pattern: Strategy] 运行时切换检测策略
    void setDetector(std::shared_ptr<IFaceDetector> detector);

private:
    // [Design Pattern: Strategy] 持有策略对象（通过接口引用）
    std::shared_ptr<IFaceDetector> detector_;
};

} // namespace smart_classroom
