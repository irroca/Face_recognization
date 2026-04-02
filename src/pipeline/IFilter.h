#pragma once
// ============================================================
// IFilter.h — 管道过滤器接口
// ============================================================
// [Design Pattern: Pipeline and Filter Architecture]
// 定义视频帧处理管道中每个"过滤器"的统一接口。
//
// 管道-过滤器架构将视频处理分解为一系列独立的处理步骤
// (Filter)，每个 Filter 只关注单一职责——解码、预处理、
// 检测、识别、绘制、编码——通过标准接口串联成处理管道
// (Pipeline)。
//
// 动机：如果将所有视频处理逻辑写在一个大函数中，会导致：
//   - "过长方法" (Long Method)
//   - "发散式变化" (Divergent Change) — 修改任何一步都要
//     改动同一个大函数
//   - 极低的可测试性和可复用性
//
// 管道-过滤器架构通过标准化的 IFilter 接口实现：
//   - 各 Filter 之间高度解耦，可独立开发和测试
//   - Pipeline 中的 Filter 可灵活增删和重排
//   - 符合"单一职责原则" (SRP) 和"开闭原则" (OCP)
// ============================================================

#include "core/Types.h"
#include <string>

namespace smart_classroom {

// ============================================================
// [Design Pattern: Pipeline-Filter] 过滤器抽象接口
// 每个过滤器接收一个 VideoFrame，对其进行处理后传递给下游
// ============================================================
class IFilter {
public:
    virtual ~IFilter() = default;

    // 核心处理方法：对 frame 进行就地处理
    // 返回 true 表示处理成功，frame 应传递给下游
    // 返回 false 表示处理失败或应丢弃此帧
    virtual bool process(VideoFrame& frame) = 0;

    // 返回过滤器名称（用于日志和调试）
    virtual std::string name() const = 0;
};

} // namespace smart_classroom
