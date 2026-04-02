#pragma once
// ============================================================
// WebRTCEndpoint.h — WebRTC 端点（骨架实现）
// ============================================================
// 基于 libdatachannel 的 WebRTC 端点骨架代码。
// 负责接收前端推送的视频流、通过 Pipeline 处理、将处理结果
// 推回前端。
//
// 注：WebRTC 的底层实现非常复杂，本文件侧重展示体系结构和
// 回调逻辑，而非完全可编译的 WebRTC 握手代码。
// ============================================================

#include "pipeline/Pipeline.h"
#include "core/Types.h"
#include <memory>
#include <string>
#include <functional>

namespace smart_classroom {

// WebRTC 连接状态
enum class WebRTCState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    FAILED
};

// WebRTC 端点配置
struct WebRTCConfig {
    std::string stunServer = "stun:stun.l.google.com:19302";
    int maxBitrate = 2000000;  // 2 Mbps
};

class WebRTCEndpoint {
public:
    explicit WebRTCEndpoint(std::shared_ptr<Pipeline> pipeline);
    ~WebRTCEndpoint();

    // 初始化 WebRTC 端点
    bool initialize(const WebRTCConfig& config = WebRTCConfig());

    // 处理信令消息（SDP offer/answer, ICE candidate）
    void handleOffer(const std::string& sdpOffer);
    std::string createAnswer();
    void addIceCandidate(const std::string& candidate,
                         const std::string& sdpMid);

    // 获取当前连接状态
    WebRTCState getState() const { return state_; }

    // 设置状态变化回调
    using StateCallback = std::function<void(WebRTCState)>;
    void setStateCallback(StateCallback callback) { stateCallback_ = callback; }

private:
    std::shared_ptr<Pipeline> pipeline_;
    WebRTCState state_ = WebRTCState::DISCONNECTED;
    WebRTCConfig config_;
    StateCallback stateCallback_;

    // ---- 以下为 WebRTC 回调骨架 ----

    // 当接收到远端视频轨道时的回调
    void onRemoteTrack(/* rtc::Track */);

    // 当接收到远端视频帧时的回调
    void onVideoFrame(const uint8_t* data, size_t size);

    // 将处理后的帧发送回前端
    void sendProcessedFrame(const VideoFrame& frame);

    // 更新连接状态
    void setState(WebRTCState newState);
};

} // namespace smart_classroom
