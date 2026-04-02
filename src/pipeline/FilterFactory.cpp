// ============================================================
// FilterFactory.cpp — 过滤器工厂实现
// ============================================================
// [Design Pattern: Abstract Factory / Factory Method]
// ============================================================

#include "pipeline/FilterFactory.h"
#include "pipeline/filters/DecodeFilter.h"
#include "pipeline/filters/PreprocessFilter.h"
#include "pipeline/filters/FaceDetectionFilter.h"
#include "pipeline/filters/FaceRecognitionFilter.h"
#include "pipeline/filters/DrawingFilter.h"
#include "pipeline/filters/EncodeFilter.h"
#include "core/Logger.h"

namespace smart_classroom {

// [Design Pattern: Factory Method] 创建单个 Filter
std::unique_ptr<IFilter> FilterFactory::createFilter(FilterType type,
    std::shared_ptr<IFaceDetector> detector,
    std::shared_ptr<IFaceRecognizer> recognizer,
    std::shared_ptr<FaceDatabase> database,
    std::shared_ptr<ISubject> eventSubject) {

    switch (type) {
        case FilterType::DECODE:
            LOG_INFO("FilterFactory: creating DecodeFilter");
            return std::make_unique<DecodeFilter>();

        case FilterType::PREPROCESS:
            LOG_INFO("FilterFactory: creating PreprocessFilter");
            return std::make_unique<PreprocessFilter>();

        case FilterType::FACE_DETECTION:
            LOG_INFO("FilterFactory: creating FaceDetectionFilter");
            return std::make_unique<FaceDetectionFilter>(detector);

        case FilterType::FACE_RECOGNITION:
            LOG_INFO("FilterFactory: creating FaceRecognitionFilter");
            return std::make_unique<FaceRecognitionFilter>(
                recognizer, database, eventSubject);

        case FilterType::DRAWING:
            LOG_INFO("FilterFactory: creating DrawingFilter");
            return std::make_unique<DrawingFilter>();

        case FilterType::ENCODE:
            LOG_INFO("FilterFactory: creating EncodeFilter");
            return std::make_unique<EncodeFilter>();

        default:
            LOG_ERROR("FilterFactory: unknown filter type");
            return nullptr;
    }
}

// [Design Pattern: Abstract Factory] 一键组装完整管道
std::unique_ptr<Pipeline> FilterFactory::createDefaultPipeline(
    std::shared_ptr<IFaceDetector> detector,
    std::shared_ptr<IFaceRecognizer> recognizer,
    std::shared_ptr<FaceDatabase> database,
    std::shared_ptr<ISubject> eventSubject) {

    LOG_INFO("FilterFactory: assembling default video processing pipeline");

    auto pipeline = std::make_unique<Pipeline>();

    // [Design Pattern: Pipeline-Filter] 按照处理顺序依次添加 Filter
    // 解码 → 预处理 → 人脸检测 → 身份识别 → 绘制 → 编码
    pipeline->addFilter(std::make_unique<DecodeFilter>());
    pipeline->addFilter(std::make_unique<PreprocessFilter>());
    pipeline->addFilter(std::make_unique<FaceDetectionFilter>(detector));
    pipeline->addFilter(std::make_unique<FaceRecognitionFilter>(
        recognizer, database, eventSubject));
    pipeline->addFilter(std::make_unique<DrawingFilter>());
    pipeline->addFilter(std::make_unique<EncodeFilter>());

    pipeline->printPipeline();
    return pipeline;
}

} // namespace smart_classroom
