#pragma once
// ============================================================
// TrackFactory.h — WebRTC 轨道工厂
// ============================================================
// [Design Pattern: Factory Method]
// 封装 WebRTC 视频/音频轨道的创建逻辑。不同的编解码器
// 和传输参数需要不同的轨道配置，工厂方法隐藏这些细节。
// ============================================================

#include <string>
#include <memory>

namespace smart_classroom {

// 轨道配置
struct TrackConfig {
    std::string codec = "H264";       // H264 / VP8 / VP9
    int maxBitrate = 2000000;         // bps
    int maxFrameRate = 30;
    int width = 640;
    int height = 480;
};

class TrackFactory {
public:
    // [Design Pattern: Factory Method] 创建视频轨道
    // 返回 void* 作为骨架（实际应返回 std::shared_ptr<rtc::Track>）
    static void* createVideoTrack(const TrackConfig& config);

    // [Design Pattern: Factory Method] 创建数据通道（用于传输控制指令）
    static void* createDataChannel(const std::string& label);

    TrackFactory() = delete;
};

} // namespace smart_classroom
