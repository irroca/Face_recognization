// ============================================================
// FaceTracker.cpp — 人脸跟踪器实现
// ============================================================
// 使用 IoU (Intersection over Union) 进行帧间人脸匹配跟踪。
// ============================================================

#include "vision/FaceTracker.h"
#include "core/Logger.h"
#include <algorithm>
#include <cmath>

namespace smart_classroom {

std::vector<TrackedFace> FaceTracker::update(const std::vector<FaceInfo>& detections) {
    // 标记所有轨迹为"未匹配"
    std::vector<bool> trackMatched(tracks_.size(), false);
    std::vector<bool> detMatched(detections.size(), false);

    // 计算所有 (track, detection) 对的 IoU，进行贪心匹配
    for (size_t t = 0; t < tracks_.size(); ++t) {
        float bestIoU = 0.3f;  // 最低匹配阈值
        int bestDet = -1;

        for (size_t d = 0; d < detections.size(); ++d) {
            if (detMatched[d]) continue;
            float iou = computeIoU(tracks_[t].lastFaceInfo.bbox, detections[d].bbox);
            if (iou > bestIoU) {
                bestIoU = iou;
                bestDet = static_cast<int>(d);
            }
        }

        if (bestDet >= 0) {
            // 匹配成功：更新轨迹
            tracks_[t].lastFaceInfo = detections[bestDet];
            tracks_[t].framesTracked++;
            tracks_[t].framesLost = 0;
            trackMatched[t] = true;
            detMatched[bestDet] = true;
        }
    }

    // 处理未匹配的轨迹（目标丢失）
    for (size_t t = 0; t < tracks_.size(); ++t) {
        if (!trackMatched[t]) {
            tracks_[t].framesLost++;
        }
    }

    // 移除丢失时间过长的轨迹
    tracks_.erase(
        std::remove_if(tracks_.begin(), tracks_.end(),
            [](const TrackedFace& t) {
                return t.framesLost > TrackedFace::MAX_LOST_FRAMES;
            }),
        tracks_.end()
    );

    // 为未匹配的检测结果创建新轨迹
    for (size_t d = 0; d < detections.size(); ++d) {
        if (!detMatched[d]) {
            TrackedFace newTrack;
            newTrack.trackId = nextTrackId_++;
            newTrack.lastFaceInfo = detections[d];
            newTrack.framesTracked = 1;
            newTrack.framesLost = 0;
            tracks_.push_back(newTrack);
        }
    }

    return tracks_;
}

void FaceTracker::reset() {
    tracks_.clear();
    nextTrackId_ = 1;
}

float FaceTracker::computeIoU(const BoundingBox& a, const BoundingBox& b) {
    int x1 = std::max(a.x, b.x);
    int y1 = std::max(a.y, b.y);
    int x2 = std::min(a.x + a.width, b.x + b.width);
    int y2 = std::min(a.y + a.height, b.y + b.height);

    if (x2 <= x1 || y2 <= y1) return 0.0f;

    float intersection = static_cast<float>((x2 - x1) * (y2 - y1));
    float areaA = static_cast<float>(a.width * a.height);
    float areaB = static_cast<float>(b.width * b.height);
    float unionArea = areaA + areaB - intersection;

    return (unionArea > 0) ? (intersection / unionArea) : 0.0f;
}

} // namespace smart_classroom
