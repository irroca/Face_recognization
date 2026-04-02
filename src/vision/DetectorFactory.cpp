// ============================================================
// DetectorFactory.cpp — 人脸检测器工厂实现
// ============================================================
// [Design Pattern: Factory Method]
// 集中封装所有检测器的创建逻辑，隐藏初始化复杂性。
// ============================================================

#include "vision/DetectorFactory.h"
#include "vision/detectors/DlibHogDetector.h"
#include "vision/detectors/DlibCnnDetector.h"
#include "vision/detectors/OpenCvDnnDetector.h"
#include "core/Logger.h"
#include "core/CudaResourceManager.h"

namespace smart_classroom {

// [Design Pattern: Factory Method] 根据枚举类型创建对应的检测器实例
std::unique_ptr<IFaceDetector> DetectorFactory::create(DetectorType type) {
    switch (type) {
        case DetectorType::DLIB_HOG:
            LOG_INFO("Factory: creating DlibHogDetector (CPU)");
            return std::make_unique<DlibHogDetector>();

        case DetectorType::DLIB_CNN_CUDA: {
            // 创建前检查 CUDA 可用性
            auto& cudaMgr = CudaResourceManager::getInstance();
            if (!cudaMgr.isCudaAvailable()) {
                LOG_ERROR("Factory: DLIB_CNN_CUDA requires GPU but CUDA not available");
                return nullptr;
            }
            LOG_INFO("Factory: creating DlibCnnDetector (CUDA)");
            auto detector = std::make_unique<DlibCnnDetector>();
            if (!detector->isInitialized()) {
                LOG_ERROR("Factory: DlibCnnDetector initialization failed");
                return nullptr;
            }
            return detector;
        }

        case DetectorType::OPENCV_DNN:
            LOG_INFO("Factory: creating OpenCvDnnDetector");
            return std::make_unique<OpenCvDnnDetector>();

        default:
            LOG_ERROR("Factory: unknown detector type, falling back to DlibHogDetector");
            return std::make_unique<DlibHogDetector>();
    }
}

// [Design Pattern: Factory Method] 根据字符串名称创建（配置文件友好）
std::unique_ptr<IFaceDetector> DetectorFactory::createFromConfig(const std::string& typeName) {
    if (typeName == "DLIB_HOG") {
        return create(DetectorType::DLIB_HOG);
    } else if (typeName == "DLIB_CNN_CUDA") {
        return create(DetectorType::DLIB_CNN_CUDA);
    } else if (typeName == "OPENCV_DNN") {
        return create(DetectorType::OPENCV_DNN);
    } else {
        LOG_WARN("Factory: unknown detector type name '" + typeName
                 + "', falling back to DLIB_HOG");
        return create(DetectorType::DLIB_HOG);
    }
}

} // namespace smart_classroom
