#pragma once
// ============================================================
// DlibCnnDetector.h — Dlib CNN (CUDA) 人脸检测器
// ============================================================
// [Design Pattern: Strategy] 具体策略类
// 实现基于 Dlib CNN (MMOD) 的人脸检测算法，利用 CUDA 在
// RTX 4060 GPU 上进行硬件加速。精度最高但需要 GPU 支持。
// ============================================================

#include "vision/IFaceDetector.h"
#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <memory>
#include <string>

namespace smart_classroom {

class DlibCnnDetector : public IFaceDetector {
public:
    explicit DlibCnnDetector(const std::string& modelPath = "");
    ~DlibCnnDetector() override = default;

    // [Design Pattern: Strategy] 实现策略接口的检测方法（CUDA 加速）
    std::vector<FaceInfo> detect(const cv::Mat& frame) override;
    std::string getName() const override { return "DlibCnnDetector (CUDA)"; }
    bool requiresCuda() const override { return true; }
    bool isInitialized() const { return initialized_; }

private:
    // Dlib CNN 人脸检测网络定义 (MMOD)
    template <long num_filters, typename SUBNET> using con5d = dlib::con<num_filters,5,5,2,2,SUBNET>;
    template <long num_filters, typename SUBNET> using con5  = dlib::con<num_filters,5,5,1,1,SUBNET>;
    template <typename SUBNET> using downsampler = dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<16,SUBNET>>>>>>>>>;
    template <typename SUBNET> using rcon5 = dlib::relu<dlib::affine<con5<45,SUBNET>>>;

    using net_type = dlib::loss_mmod<dlib::con<1,9,9,1,1,rcon5<rcon5<rcon5<downsampler<dlib::input_rgb_image_pyramid<dlib::pyramid_down<6>>>>>>>>;

    net_type net_;
    bool initialized_ = false;
};

} // namespace smart_classroom
