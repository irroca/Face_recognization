#pragma once
// ============================================================
// DetectorFactory.h — 人脸检测器工厂
// ============================================================
// [Design Pattern: Factory Method]
// 封装人脸检测器的复杂创建逻辑，客户端代码无需关心具体检测器
// 类的构造细节（模型路径加载、CUDA 设备检查等），只需通过枚举
// 指定所需的检测器类型即可。
//
// 动机：各种检测器的初始化逻辑差异很大：
//   - DlibHogDetector 无需模型文件
//   - DlibCnnDetector 需要加载 ~700MB 模型 + CUDA 检查
//   - OpenCvDnnDetector 需要加载 prototxt + caffemodel
// 如果在客户端代码中直接用 new/make_unique 创建，会导致：
//   - "过长方法" (Long Method) — 大量初始化代码
//   - "重复代码" (Duplicated Code) — 多处创建逻辑重复
//   - 客户端与具体类紧耦合
//
// 工厂方法将这些复杂性封装在一处，客户端只依赖 IFaceDetector
// 接口和 DetectorType 枚举。
// ============================================================

#include "vision/IFaceDetector.h"
#include "core/Types.h"
#include <memory>

namespace smart_classroom {

class DetectorFactory {
public:
    // [Design Pattern: Factory Method] 根据类型创建具体检测器
    static std::unique_ptr<IFaceDetector> create(DetectorType type);

    // 根据配置字符串创建检测器（适用于从配置文件读取）
    static std::unique_ptr<IFaceDetector> createFromConfig(const std::string& typeName);

    DetectorFactory() = delete;  // 纯工具类，禁止实例化
};

} // namespace smart_classroom
