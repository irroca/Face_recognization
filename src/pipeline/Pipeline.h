#pragma once
// ============================================================
// Pipeline.h — 视频处理管道编排器
// ============================================================
// [Design Pattern: Pipeline and Filter Architecture]
// Pipeline 是管道-过滤器架构中的"管道"部分，负责将多个
// IFilter 串联起来，依次对 VideoFrame 进行处理。
//
// 设计要点：
//   - 使用 std::vector<unique_ptr<IFilter>> 管理过滤器链
//   - execute() 方法依次调用每个 Filter 的 process()
//   - 任一 Filter 返回 false 时立即终止管道（短路）
//   - Pipeline 本身不关心 Filter 的具体类型（多态）
// ============================================================

#include "pipeline/IFilter.h"
#include <vector>
#include <memory>
#include <set>

namespace smart_classroom {

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();

    // [Design Pattern: Pipeline-Filter] 向管道末端添加过滤器
    void addFilter(std::unique_ptr<IFilter> filter);

    // 添加由 shared_ptr 管理的过滤器（Pipeline 不拥有其生命周期）
    void addSharedFilter(IFilter* filter);

    // [Design Pattern: Pipeline-Filter] 执行完整的管道处理
    // 依次调用每个 Filter 的 process() 方法
    // 返回 true 表示所有 Filter 都处理成功
    bool execute(VideoFrame& frame);

    // 获取管道中的过滤器数量
    size_t filterCount() const { return filters_.size(); }

    // 清空管道中的所有过滤器
    void clear() { filters_.clear(); }

    // 打印管道结构（用于调试）
    void printPipeline() const;

private:
    std::vector<std::unique_ptr<IFilter>> filters_;
    std::set<size_t> sharedIndices_;  // 不由 Pipeline 释放的 filter 索引
};

} // namespace smart_classroom
