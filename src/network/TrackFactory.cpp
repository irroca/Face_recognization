// ============================================================
// TrackFactory.cpp — WebRTC 轨道工厂实现
// ============================================================
// [Design Pattern: Factory Method]
// 骨架实现——展示工厂方法如何隐藏轨道创建的复杂性。
// ============================================================

#include "network/TrackFactory.h"
#include "core/Logger.h"

namespace smart_classroom {

// [Design Pattern: Factory Method] 创建视频轨道
void* TrackFactory::createVideoTrack(const TrackConfig& config) {
    LOG_INFO("TrackFactory: creating video track (codec: " + config.codec
             + ", " + std::to_string(config.width) + "x"
             + std::to_string(config.height) + "@"
             + std::to_string(config.maxFrameRate) + "fps)");

    // ---- 骨架代码 ----
    // 实际项目中此处创建 libdatachannel 的 Track 对象：
    //
    // rtc::Description::Video media("video", rtc::Description::Direction::SendRecv);
    // if (config.codec == "H264") {
    //     media.addH264Codec(96);  // payload type 96
    // } else if (config.codec == "VP8") {
    //     media.addVP8Codec(96);
    // }
    //
    // auto track = peerConnection_->addTrack(media);
    // return track;

    return nullptr;  // 骨架返回
}

// [Design Pattern: Factory Method] 创建数据通道
void* TrackFactory::createDataChannel(const std::string& label) {
    LOG_INFO("TrackFactory: creating data channel '" + label + "'");

    // ---- 骨架代码 ----
    // auto dc = peerConnection_->createDataChannel(label);
    // return dc;

    return nullptr;
}

} // namespace smart_classroom
