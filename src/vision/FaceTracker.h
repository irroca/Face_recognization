#pragma once
// ============================================================
// FaceTracker.h — 人脸跟踪器
// ============================================================
// 基于帧间 IoU（交并比）匹配，对连续帧中的人脸进行跟踪，
// 避免每帧都重新识别已跟踪的人脸，提升性能。
// ============================================================

#include "core/Types.h"
#include <vector>
#include <unordered_map>

namespace smart_classroom {

struct TrackedFace {
    int trackId;
    FaceInfo lastFaceInfo;
    int framesTracked = 0;
    int framesLost = 0;
    static constexpr int MAX_LOST_FRAMES = 10;
};

class FaceTracker {
public:
    FaceTracker() = default;

    // 更新跟踪状态：输入当前帧的检测结果，返回带 trackId 的结果
    std::vector<TrackedFace> update(const std::vector<FaceInfo>& detections);

    // 重置跟踪器
    void reset();

private:
    std::vector<TrackedFace> tracks_;
    int nextTrackId_ = 1;

    // 计算两个 BoundingBox 的 IoU
    static float computeIoU(const BoundingBox& a, const BoundingBox& b);
};

} // namespace smart_classroom
