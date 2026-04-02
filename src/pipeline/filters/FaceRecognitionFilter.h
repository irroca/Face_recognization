#pragma once
// ============================================================
// FaceRecognitionFilter.h — 人脸身份识别过滤器
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道中的第 4 个过滤器
// [Design Pattern: Strategy] 内部持有 IFaceRecognizer 策略对象
// [Design Pattern: Observer] 识别完成后通过 ISubject 分发事件
//
// 这是三种设计模式的**汇合点**：
//   - Pipeline-Filter: 作为管道的组成部分
//   - Strategy: 识别算法可运行时切换
//   - Observer: 识别结果触发事件通知（考勤/弹幕/日志等）
//
// 处理流程：
//   1. 从 frame.detectedFaces 获取上游检测结果
//   2. 对每个检测到的人脸裁剪 face chip
//   3. 调用 IFaceRecognizer 策略提取特征 + 匹配身份
//   4. 将识别结果写回 FaceInfo.identity
//   5. 通过 ISubject 触发 IDENTIFIED / UNKNOWN 事件
// ============================================================

#include "pipeline/IFilter.h"
#include "vision/IFaceRecognizer.h"
#include "vision/FaceDatabase.h"
#include "observer/ISubject.h"
#include <memory>

namespace smart_classroom {

class FaceRecognitionFilter : public IFilter {
public:
    FaceRecognitionFilter(std::shared_ptr<IFaceRecognizer> recognizer,
                          std::shared_ptr<FaceDatabase> database,
                          std::shared_ptr<ISubject> eventSubject = nullptr);
    ~FaceRecognitionFilter() override = default;

    // [Design Pattern: Pipeline-Filter] 过滤器处理逻辑
    bool process(VideoFrame& frame) override;
    std::string name() const override { return "FaceRecognitionFilter"; }

    // [Design Pattern: Strategy] 运行时切换识别策略
    void setRecognizer(std::shared_ptr<IFaceRecognizer> recognizer);

private:
    std::shared_ptr<IFaceRecognizer> recognizer_;
    std::shared_ptr<FaceDatabase> database_;
    std::shared_ptr<ISubject> eventSubject_;  // [Design Pattern: Observer]

    // 裁剪人脸区域
    cv::Mat cropFaceChip(const cv::Mat& image, const BoundingBox& bbox) const;

    // 触发识别事件
    void emitRecognitionEvent(const FaceInfo& face, int64_t timestamp);

    // 识别节流：每 N 帧才执行一次完整识别，中间帧复用缓存
    int frameCounter_ = 0;
    static constexpr int RECOGNIZE_EVERY_N_FRAMES = 5;
    std::vector<FaceInfo> cachedIdentities_;  // 缓存的识别结果
};

} // namespace smart_classroom
