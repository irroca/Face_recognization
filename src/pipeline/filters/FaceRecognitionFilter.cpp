// ============================================================
// FaceRecognitionFilter.cpp — 人脸身份识别过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道第 4 级
// [Design Pattern: Strategy] 委托给 IFaceRecognizer 执行识别
// [Design Pattern: Observer] 识别完成后通知所有观察者
// ============================================================

#include "pipeline/filters/FaceRecognitionFilter.h"
#include "core/Logger.h"
#include <chrono>

namespace smart_classroom {

FaceRecognitionFilter::FaceRecognitionFilter(
    std::shared_ptr<IFaceRecognizer> recognizer,
    std::shared_ptr<FaceDatabase> database,
    std::shared_ptr<ISubject> eventSubject)
    : recognizer_(std::move(recognizer))
    , database_(std::move(database))
    , eventSubject_(std::move(eventSubject)) {

    if (recognizer_) {
        LOG_INFO("FaceRecognitionFilter: using recognizer '"
                 + recognizer_->getName() + "'");
    }
}

// [Design Pattern: Pipeline-Filter] 过滤器处理
// [Design Pattern: Strategy] 委托识别任务给策略对象
bool FaceRecognitionFilter::process(VideoFrame& frame) {
    if (!recognizer_) {
        LOG_DEBUG("FaceRecognitionFilter: no recognizer configured, skipping");
        return true;
    }

    if (frame.detectedFaces.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        cachedIdentities_.clear();
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    frameCounter_++;
    bool shouldRecognize = (frameCounter_ % RECOGNIZE_EVERY_N_FRAMES == 1)
                           || cachedIdentities_.empty();

    if (shouldRecognize) {
        // 完整识别：对每个检测到的人脸进行身份识别
        for (auto& face : frame.detectedFaces) {
            cv::Mat faceChip = cropFaceChip(frame.image, face.bbox);
            if (faceChip.empty()) continue;

            // [Design Pattern: Strategy] 调用识别策略
            float distance = 0.0f;
            std::string identity = recognizer_->recognize(faceChip, distance);

            face.identity = identity;
            face.recognitionDistance = distance;

            // [Design Pattern: Observer] 触发识别事件
            emitRecognitionEvent(face, frame.timestamp);
        }
        cachedIdentities_ = frame.detectedFaces;
    } else {
        // 节流帧：复用上一次识别的身份结果（仅更新位置框）
        for (size_t i = 0; i < frame.detectedFaces.size(); ++i) {
            if (i < cachedIdentities_.size()) {
                frame.detectedFaces[i].identity = cachedIdentities_[i].identity;
                frame.detectedFaces[i].recognitionDistance =
                    cachedIdentities_[i].recognitionDistance;
            }
        }
    }

    return true;
}

cv::Mat FaceRecognitionFilter::cropFaceChip(const cv::Mat& image,
                                             const BoundingBox& bbox) const {
    cv::Rect roi = bbox.toCvRect();

    // 边界安全检查
    roi.x = std::max(0, roi.x);
    roi.y = std::max(0, roi.y);
    roi.width = std::min(roi.width, image.cols - roi.x);
    roi.height = std::min(roi.height, image.rows - roi.y);

    if (roi.width <= 0 || roi.height <= 0) {
        return cv::Mat();
    }

    return image(roi).clone();
}

// [Design Pattern: Observer] 构建并分发识别事件
void FaceRecognitionFilter::emitRecognitionEvent(const FaceInfo& face,
                                                  int64_t timestamp) {
    if (!eventSubject_) return;

    RecognitionEvent event;
    event.faceInfo = face;
    event.timestamp = timestamp;

    if (face.identity.empty()) {
        event.type = RecognitionEventType::FACE_UNKNOWN;
        event.message = "Detected unregistered face";
    } else {
        event.type = RecognitionEventType::FACE_IDENTIFIED;
        event.message = "Identified: " + face.identity;
    }

    // [Design Pattern: Observer] 通知所有已注册的观察者
    eventSubject_->notify(event);
}

// [Design Pattern: Strategy] 运行时切换识别算法
void FaceRecognitionFilter::setRecognizer(
    std::shared_ptr<IFaceRecognizer> recognizer) {
    std::lock_guard<std::mutex> lock(mutex_);
    recognizer_ = std::move(recognizer);
    if (recognizer_) {
        LOG_INFO("FaceRecognitionFilter: switched to recognizer '"
                 + recognizer_->getName() + "'");
    }
}

} // namespace smart_classroom
