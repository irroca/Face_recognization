// ============================================================
// SignalingHandler.cpp — WebSocket 信令消息处理器实现
// ============================================================

#include "network/SignalingHandler.h"
#include "core/Logger.h"
#include <nlohmann/json.hpp>

namespace smart_classroom {

SignalingHandler::SignalingHandler(std::shared_ptr<WebRTCEndpoint> endpoint)
    : endpoint_(std::move(endpoint)) {
    LOG_INFO("SignalingHandler: created");
}

bool SignalingHandler::connect(const std::string& wsUrl) {
    LOG_INFO("SignalingHandler: connecting to signaling server at " + wsUrl);

    // ---- 骨架代码：实际项目中使用 WebSocket 客户端库 ----
    // 例如 libwebsockets、Beast (Boost)、或 ix::WebSocket
    //
    // wsClient_ = std::make_unique<ix::WebSocket>();
    // wsClient_->setUrl(wsUrl);
    // wsClient_->setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
    //     if (msg->type == ix::WebSocketMessageType::Message) {
    //         onMessage(msg->str);
    //     }
    // });
    // wsClient_->start();

    return true;
}

void SignalingHandler::onMessage(const std::string& message) {
    try {
        auto json = nlohmann::json::parse(message);
        std::string type = json.value("type", "");

        LOG_DEBUG("SignalingHandler: received message type: " + type);

        if (type == "offer") {
            handleOffer(json["data"]["sdp"].get<std::string>());
        } else if (type == "answer") {
            handleAnswer(json["data"]["sdp"].get<std::string>());
        } else if (type == "candidate") {
            handleIceCandidate(
                json["data"]["candidate"].get<std::string>(),
                json["data"]["sdpMid"].get<std::string>());
        } else {
            LOG_WARN("SignalingHandler: unknown message type: " + type);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("SignalingHandler: failed to parse message: "
                  + std::string(e.what()));
    }
}

void SignalingHandler::sendMessage(const std::string& message) {
    if (sendCallback_) {
        sendCallback_(message);
    } else {
        LOG_WARN("SignalingHandler: no send callback configured");
    }
}

void SignalingHandler::handleOffer(const std::string& sdp) {
    LOG_INFO("SignalingHandler: processing SDP offer");

    endpoint_->handleOffer(sdp);

    // 创建应答并发送回信令服务器
    std::string answer = endpoint_->createAnswer();

    nlohmann::json response;
    response["type"] = "answer";
    response["data"]["sdp"] = answer;

    sendMessage(response.dump());
}

void SignalingHandler::handleAnswer(const std::string& sdp) {
    LOG_INFO("SignalingHandler: processing SDP answer");
    // 通常后端作为 answer 端，不太会收到 answer
    // 但保留此接口以支持双向协商
    (void)sdp;
}

void SignalingHandler::handleIceCandidate(const std::string& candidate,
                                           const std::string& sdpMid) {
    LOG_DEBUG("SignalingHandler: adding ICE candidate");
    endpoint_->addIceCandidate(candidate, sdpMid);
}

} // namespace smart_classroom
