// ============================================================
// WebRTCEndpoint.cpp — WebRTC 端点骨架实现
// ============================================================
// 展示 WebRTC 视频流的接收、Pipeline 处理和回传的整体架构。
// 实际项目中需要集成 libdatachannel 或 libwebrtc 的完整 API。
// ============================================================

#include "network/WebRTCEndpoint.h"
#include "core/Logger.h"
#include <chrono>

namespace smart_classroom {

WebRTCEndpoint::WebRTCEndpoint(std::shared_ptr<Pipeline> pipeline)
    : pipeline_(std::move(pipeline)) {
    LOG_INFO("WebRTCEndpoint: created");
}

WebRTCEndpoint::~WebRTCEndpoint() {
    LOG_INFO("WebRTCEndpoint: destroyed");
}

bool WebRTCEndpoint::initialize(const WebRTCConfig& config) {
    config_ = config;

    LOG_INFO("WebRTCEndpoint: initializing with STUN server: " + config_.stunServer);

    // ---- 骨架代码：实际项目中此处初始化 libdatachannel ----
    //
    // rtc::Configuration rtcConfig;
    // rtcConfig.iceServers.emplace_back(config_.stunServer);
    //
    // peerConnection_ = std::make_shared<rtc::PeerConnection>(rtcConfig);
    //
    // // 设置轨道回调
    // peerConnection_->onTrack([this](std::shared_ptr<rtc::Track> track) {
    //     onRemoteTrack(track);
    // });
    //
    // // 设置状态变化回调
    // peerConnection_->onStateChange([this](rtc::PeerConnection::State state) {
    //     // 映射 libdatachannel 状态到自定义状态
    //     setState(mapState(state));
    // });

    setState(WebRTCState::DISCONNECTED);
    LOG_INFO("WebRTCEndpoint: initialized successfully");
    return true;
}

void WebRTCEndpoint::handleOffer(const std::string& sdpOffer) {
    LOG_INFO("WebRTCEndpoint: received SDP offer (" 
             + std::to_string(sdpOffer.size()) + " bytes)");

    // ---- 骨架代码 ----
    // peerConnection_->setRemoteDescription(
    //     rtc::Description(sdpOffer, rtc::Description::Type::Offer));

    setState(WebRTCState::CONNECTING);
}

std::string WebRTCEndpoint::createAnswer() {
    LOG_INFO("WebRTCEndpoint: creating SDP answer");

    // ---- 骨架代码 ----
    // auto description = peerConnection_->localDescription();
    // return std::string(description.value());

    // 返回骨架 SDP
    return "v=0\r\n"
           "o=- 0 0 IN IP4 127.0.0.1\r\n"
           "s=SmartClassroom\r\n"
           "t=0 0\r\n";
}

void WebRTCEndpoint::addIceCandidate(const std::string& candidate,
                                      const std::string& sdpMid) {
    LOG_DEBUG("WebRTCEndpoint: adding ICE candidate: " + candidate);

    // ---- 骨架代码 ----
    // peerConnection_->addRemoteCandidate(
    //     rtc::Candidate(candidate, sdpMid));
}

void WebRTCEndpoint::onRemoteTrack(/* rtc::Track */) {
    LOG_INFO("WebRTCEndpoint: remote video track received");

    // ---- 骨架代码 ----
    // track->onMessage([this](rtc::message_variant message) {
    //     auto data = std::get<rtc::binary>(message);
    //     onVideoFrame(data.data(), data.size());
    // });

    setState(WebRTCState::CONNECTED);
}

void WebRTCEndpoint::onVideoFrame(const uint8_t* data, size_t size) {
    // 构建 VideoFrame 并送入 Pipeline 处理
    VideoFrame frame;
    frame.encodedData.assign(data, data + size);
    frame.format = FrameFormat::ENCODED_JPEG;  // 或 H.264/VP8
    frame.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    // [Design Pattern: Pipeline-Filter] 将帧送入处理管道
    if (pipeline_ && pipeline_->execute(frame)) {
        sendProcessedFrame(frame);
    }
}

void WebRTCEndpoint::sendProcessedFrame(const VideoFrame& frame) {
    if (frame.encodedData.empty()) {
        LOG_WARN("WebRTCEndpoint: no encoded data to send");
        return;
    }

    // ---- 骨架代码 ----
    // localTrack_->send(
    //     reinterpret_cast<const std::byte*>(frame.encodedData.data()),
    //     frame.encodedData.size());

    LOG_DEBUG("WebRTCEndpoint: sent processed frame ("
              + std::to_string(frame.encodedData.size()) + " bytes)");
}

void WebRTCEndpoint::setState(WebRTCState newState) {
    if (state_ != newState) {
        state_ = newState;
        if (stateCallback_) {
            stateCallback_(newState);
        }
    }
}

} // namespace smart_classroom
