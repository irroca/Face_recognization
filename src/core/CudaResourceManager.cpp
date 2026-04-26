// ============================================================
// CudaResourceManager.cpp — CUDA 资源管理器实现
// ============================================================
// [Design Pattern: Singleton] Meyers' Singleton 实现
// 集中管理 RTX 4060 GPU 的 CUDA 资源初始化与查询。
// ============================================================

#include "core/CudaResourceManager.h"
#include "core/Logger.h"

#ifdef CUDA_ENABLED
#include <cuda_runtime.h>
#endif

#include <sstream>

namespace smart_classroom {

// [Design Pattern: Singleton] 私有构造函数
CudaResourceManager::CudaResourceManager() {
    // 构造时不自动初始化 CUDA，由 initialize() 显式调用
}

bool CudaResourceManager::initialize(int deviceId) {
    std::lock_guard<std::mutex> lock(mutex_);

#ifdef CUDA_ENABLED
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    if (err != cudaSuccess || deviceCount == 0) {
        LOG_WARN("No CUDA devices found, falling back to CPU mode");
        cudaAvailable_ = false;
        return false;
    }

    if (deviceId < 0 || deviceId >= deviceCount) {
        LOG_ERROR("Invalid CUDA device ID: " + std::to_string(deviceId));
        cudaAvailable_ = false;
        return false;
    }

    err = cudaSetDevice(deviceId);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to set CUDA device: " + std::string(cudaGetErrorString(err)));
        cudaAvailable_ = false;
        return false;
    }

    cudaDeviceProp prop;
    err = cudaGetDeviceProperties(&prop, deviceId);
    if (err != cudaSuccess) {
        LOG_ERROR("Failed to get CUDA device properties: " + std::string(cudaGetErrorString(err)));
        cudaAvailable_ = false;
        return false;
    }

    deviceId_ = deviceId;
    deviceName_ = prop.name;
    totalMemory_ = prop.totalGlobalMem;
    cudaAvailable_ = true;

    LOG_INFO("CUDA initialized successfully on device: " + deviceName_);
    printDeviceInfo();
    return true;

#else
    LOG_WARN("CUDA support not compiled, running in CPU-only mode");
    cudaAvailable_ = false;
    return false;
#endif
}

size_t CudaResourceManager::getFreeMemory() const {
    std::lock_guard<std::mutex> lock(mutex_);

#ifdef CUDA_ENABLED
    if (!cudaAvailable_) return 0;
    size_t free = 0, total = 0;
    cudaMemGetInfo(&free, &total);
    return free;
#else
    return 0;
#endif
}

void CudaResourceManager::printDeviceInfo() const {
    if (!cudaAvailable_) {
        LOG_INFO("CUDA: Not available");
        return;
    }

    std::ostringstream oss;
    oss << "CUDA Device Info:"
        << "\n  Device ID:    " << deviceId_
        << "\n  Device Name:  " << deviceName_
        << "\n  Total Memory: " << (totalMemory_ / (1024 * 1024)) << " MB"
        << "\n  Free Memory:  " << (getFreeMemory() / (1024 * 1024)) << " MB";

    LOG_INFO(oss.str());
}

} // namespace smart_classroom
