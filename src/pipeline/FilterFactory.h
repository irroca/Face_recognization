#pragma once
// ============================================================
// FilterFactory.h — 过滤器工厂
// ============================================================
// [Design Pattern: Abstract Factory / Factory Method]
// 提供创建各种 Pipeline Filter 的统一入口，隐藏具体 Filter
// 的构造细节和依赖注入逻辑。
//
// 动机：各 Filter 的创建需要不同的依赖：
//   - FaceDetectionFilter 需要注入 IFaceDetector 策略对象
//   - FaceRecognitionFilter 需要注入 IFaceRecognizer + FaceDatabase
//   - 其他 Filter 可能需要配置参数
// 如果在客户端代码中手动创建每个 Filter，会导致：
//   - "过长方法" (Long Method) — main 函数中充斥创建代码
//   - "特性依恋" (Feature Envy) — 客户端了解太多 Filter 内部细节
//
// 工厂模式将所有创建逻辑集中管理，同时支持一键创建默认管道。
// ============================================================

#include "pipeline/IFilter.h"
#include "pipeline/Pipeline.h"
#include "vision/IFaceDetector.h"
#include "vision/IFaceRecognizer.h"
#include "vision/FaceDatabase.h"
#include "core/Types.h"
#include <memory>

namespace smart_classroom {

// 前向声明 Observer 接口
class ISubject;

class FilterFactory {
public:
    // [Design Pattern: Factory Method] 根据类型创建单个 Filter
    static std::unique_ptr<IFilter> createFilter(FilterType type,
        std::shared_ptr<IFaceDetector> detector = nullptr,
        std::shared_ptr<IFaceRecognizer> recognizer = nullptr,
        std::shared_ptr<FaceDatabase> database = nullptr,
        std::shared_ptr<ISubject> eventSubject = nullptr);

    // [Design Pattern: Abstract Factory] 一键创建默认视频处理管道
    // 组装完整的 Filter 链：Decode -> Preprocess -> Detection ->
    //                       Recognition -> Drawing -> Encode
    static std::unique_ptr<Pipeline> createDefaultPipeline(
        std::shared_ptr<IFaceDetector> detector,
        std::shared_ptr<IFaceRecognizer> recognizer,
        std::shared_ptr<FaceDatabase> database,
        std::shared_ptr<ISubject> eventSubject = nullptr);

    FilterFactory() = delete;
};

} // namespace smart_classroom
