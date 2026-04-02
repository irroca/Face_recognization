#pragma once
// ============================================================
// CudaResourceManager.h — CUDA 资源管理器
// ============================================================
// [Design Pattern: Singleton]
// 使用 Meyers' Singleton 管理全局唯一的 CUDA 资源。
// 动机：CUDA 设备的初始化和资源管理具有全局唯一性——系统中
// 只有一块 RTX 4060 GPU，多个模块（DlibCnnDetector、
// OpenCvDnnDetector、DlibFaceRecognizer）共享同一个 CUDA
// 上下文。如果分散初始化，会导致"平行继承体系"和"重复代码"
// 的坏味道。单例模式确保 CUDA 资源集中管理、一次初始化、
// 全局共享。
// ============================================================

#include <string>
#include <mutex>

namespace smart_classroom {

class CudaResourceManager {
public:
    // [Design Pattern: Singleton] 全局唯一访问点
    static CudaResourceManager& getInstance() {
        static CudaResourceManager instance;
        return instance;
    }

    // 禁止拷贝和移动
    CudaResourceManager(const CudaResourceManager&) = delete;
    CudaResourceManager& operator=(const CudaResourceManager&) = delete;
    CudaResourceManager(CudaResourceManager&&) = delete;
    CudaResourceManager& operator=(CudaResourceManager&&) = delete;

    // 初始化 CUDA 设备
    bool initialize(int deviceId = 0);

    // 查询 CUDA 状态
    bool isCudaAvailable() const { return cudaAvailable_; }
    int  getDeviceId() const { return deviceId_; }
    std::string getDeviceName() const { return deviceName_; }
    size_t getTotalMemory() const { return totalMemory_; }
    size_t getFreeMemory() const;

    // 打印 CUDA 设备信息
    void printDeviceInfo() const;

private:
    // [Design Pattern: Singleton] 私有构造函数
    CudaResourceManager();
    ~CudaResourceManager() = default;

    bool cudaAvailable_ = false;
    int deviceId_ = -1;
    std::string deviceName_;
    size_t totalMemory_ = 0;
    mutable std::mutex mutex_;
};

} // namespace smart_classroom
