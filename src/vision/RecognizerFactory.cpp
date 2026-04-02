// ============================================================
// RecognizerFactory.cpp — 人脸识别器工厂实现
// ============================================================
// [Design Pattern: Factory Method]
// 根据枚举或字符串创建具体的识别器实例，注入 FaceDatabase 依赖。
// ============================================================

#include "vision/RecognizerFactory.h"
#include "vision/recognizers/DlibFaceRecognizer.h"
#include "vision/recognizers/OpenCvLbphRecognizer.h"
#include "core/Logger.h"

namespace smart_classroom {

// [Design Pattern: Factory Method] 根据枚举创建识别器
std::unique_ptr<IFaceRecognizer> RecognizerFactory::create(
    RecognizerType type,
    std::shared_ptr<FaceDatabase> database) {

    switch (type) {
        case RecognizerType::DLIB_RESNET:
            LOG_INFO("Factory: creating DlibFaceRecognizer (ResNet-128D)");
            return std::make_unique<DlibFaceRecognizer>(database);

        case RecognizerType::OPENCV_LBPH:
            LOG_INFO("Factory: creating OpenCvLbphRecognizer");
            return std::make_unique<OpenCvLbphRecognizer>(database);

        default:
            LOG_ERROR("Factory: unknown recognizer type, falling back to DlibFaceRecognizer");
            return std::make_unique<DlibFaceRecognizer>(database);
    }
}

// [Design Pattern: Factory Method] 根据字符串创建
std::unique_ptr<IFaceRecognizer> RecognizerFactory::createFromConfig(
    const std::string& typeName,
    std::shared_ptr<FaceDatabase> database) {

    if (typeName == "DLIB_RESNET") {
        return create(RecognizerType::DLIB_RESNET, database);
    } else if (typeName == "OPENCV_LBPH") {
        return create(RecognizerType::OPENCV_LBPH, database);
    } else {
        LOG_WARN("Factory: unknown recognizer type name '" + typeName
                 + "', falling back to DLIB_RESNET");
        return create(RecognizerType::DLIB_RESNET, database);
    }
}

} // namespace smart_classroom
