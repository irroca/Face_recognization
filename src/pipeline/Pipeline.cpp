// ============================================================
// Pipeline.cpp — 视频处理管道编排器实现
// ============================================================
// [Design Pattern: Pipeline and Filter Architecture]
// ============================================================

#include "pipeline/Pipeline.h"
#include "core/Logger.h"
#include <chrono>

namespace smart_classroom {

Pipeline::~Pipeline() {
    // 释放 shared filter 的 unique_ptr 而不删除对象
    for (auto idx : sharedIndices_) {
        if (idx < filters_.size()) {
            filters_[idx].release();  // 放弃所有权，不调用 delete
        }
    }
}

// [Design Pattern: Pipeline-Filter] 向管道添加过滤器
void Pipeline::addFilter(std::unique_ptr<IFilter> filter) {
    if (filter) {
        LOG_DEBUG("Pipeline: adding filter '" + filter->name() + "' at position "
                  + std::to_string(filters_.size()));
        filters_.push_back(std::move(filter));
    }
}

// 添加由外部管理生命周期的过滤器（不释放）
void Pipeline::addSharedFilter(IFilter* filter) {
    if (filter) {
        LOG_DEBUG("Pipeline: adding shared filter '" + filter->name() + "' at position "
                  + std::to_string(filters_.size()));
        // 使用空删除器的 unique_ptr，Pipeline 不负责释放此对象
        filters_.push_back(std::unique_ptr<IFilter>(filter));
        sharedIndices_.insert(filters_.size() - 1);
    }
}

// [Design Pattern: Pipeline-Filter] 串行执行所有过滤器
bool Pipeline::execute(VideoFrame& frame) {
    LOG_DEBUG("Pipeline: executing " + std::to_string(filters_.size())
              + " filter(s) on frame #" + std::to_string(frame.frameIndex));

    for (size_t i = 0; i < filters_.size(); ++i) {
        auto& filter = filters_[i];
        auto startTime = std::chrono::steady_clock::now();

        // 调用当前过滤器的 process() 方法
        bool success = filter->process(frame);

        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();

        LOG_DEBUG("Pipeline: filter '" + filter->name() + "' took "
                  + std::to_string(elapsed) + " us, result: "
                  + (success ? "OK" : "FAIL"));

        // 短路机制：任一 Filter 失败则终止管道
        if (!success) {
            LOG_WARN("Pipeline: filter '" + filter->name()
                     + "' failed, aborting pipeline");
            return false;
        }
    }

    return true;
}

void Pipeline::printPipeline() const {
    LOG_INFO("Pipeline structure (" + std::to_string(filters_.size()) + " filters):");
    for (size_t i = 0; i < filters_.size(); ++i) {
        std::string arrow = (i < filters_.size() - 1) ? " --> " : " [END]";
        LOG_INFO("  [" + std::to_string(i) + "] " + filters_[i]->name() + arrow);
    }
}

} // namespace smart_classroom
