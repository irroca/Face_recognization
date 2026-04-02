#pragma once
// ============================================================
// SignalingHandler.h — WebSocket 信令消息处理器
// ============================================================
// 负责解析和处理来自信令服务器的 WebSocket 消息，协调
// WebRTC 的 SDP/ICE 交换。
// ============================================================

#include "network/WebRTCEndpoint.h"
#include <string>
#include <functional>
#include <memory>

namespace smart_classroom {

class SignalingHandler {
public:
    explicit SignalingHandler(std::shared_ptr<WebRTCEndpoint> endpoint);
    ~SignalingHandler() = default;

    // 连接到信令服务器
    bool connect(const std::string& wsUrl);

    // 处理收到的 WebSocket 消息（JSON 格式）
    void onMessage(const std::string& message);

    // 发送消息到信令服务器
    void sendMessage(const std::string& message);

    // 设置消息发送回调（由 WebSocket 实现层提供）
    using SendCallback = std::function<void(const std::string&)>;
    void setSendCallback(SendCallback callback) { sendCallback_ = callback; }

private:
    std::shared_ptr<WebRTCEndpoint> endpoint_;
    SendCallback sendCallback_;
    std::string roomId_;

    // 消息处理方法
    void handleOffer(const std::string& sdp);
    void handleAnswer(const std::string& sdp);
    void handleIceCandidate(const std::string& candidate,
                            const std::string& sdpMid);
};

} // namespace smart_classroom
