#pragma once
// ============================================================
// RecognizerFactory.h — 人脸识别器工厂
// ============================================================
// [Design Pattern: Factory Method]
// 封装人脸身份识别器的创建逻辑。与 DetectorFactory 配合，
// 构成"双工厂"体系，分别负责检测器和识别器的实例化。
//
// 动机：识别器的初始化需要加载不同的模型文件，并且需要注入
// FaceDatabase 依赖。工厂方法隐藏了这些复杂性，客户端只需
// 指定所需的识别器类型。
// ============================================================

#include "vision/IFaceRecognizer.h"
#include "vision/FaceDatabase.h"
#include "core/Types.h"
#include <memory>

namespace smart_classroom {

class RecognizerFactory {
public:
    // [Design Pattern: Factory Method] 根据类型创建识别器
    // 需要注入 FaceDatabase 共享指针作为依赖
    static std::unique_ptr<IFaceRecognizer> create(
        RecognizerType type,
        std::shared_ptr<FaceDatabase> database);

    // 根据配置字符串创建
    static std::unique_ptr<IFaceRecognizer> createFromConfig(
        const std::string& typeName,
        std::shared_ptr<FaceDatabase> database);

    RecognizerFactory() = delete;
};

} // namespace smart_classroom
