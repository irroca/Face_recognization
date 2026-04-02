#pragma once
// ============================================================
// DlibFaceRecognizer.h — Dlib ResNet 人脸身份识别器
// ============================================================
// [Design Pattern: Strategy] 具体策略类
// 使用 Dlib 的 ResNet 模型 (dlib_face_recognition_resnet_model_v1)
// 提取 128 维人脸特征描述符。该模型在 LFW 数据集上达到 99.38%
// 的准确率。特征描述符通过余弦距离/欧氏距离与 FaceDatabase
// 中已注册的特征进行比对，完成身份识别。
//
// 需要配合 shape_predictor_68_face_landmarks.dat 进行人脸对齐。
// ============================================================

#include "vision/IFaceRecognizer.h"
#include "vision/FaceDatabase.h"
#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <memory>

namespace smart_classroom {

class DlibFaceRecognizer : public IFaceRecognizer {
public:
    explicit DlibFaceRecognizer(std::shared_ptr<FaceDatabase> database,
                                const std::string& recognitionModelPath = "",
                                const std::string& shapePredictorPath = "");
    ~DlibFaceRecognizer() override = default;

    // [Design Pattern: Strategy] 实现策略接口
    std::vector<float> extractFeature(const cv::Mat& faceChip) override;
    std::string recognize(const cv::Mat& faceChip, float& confidence) override;
    std::string getName() const override { return "DlibFaceRecognizer (ResNet-128D)"; }
    bool requiresCuda() const override { return false; }  // 可选 CUDA 加速

private:
    // Dlib ResNet 人脸识别网络定义
    template <template <int,template<typename>class,int,typename> class block, int N,
              template<typename>class BN, typename SUBNET>
    using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

    template <template <int,template<typename>class,int,typename> class block, int N,
              template<typename>class BN, typename SUBNET>
    using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,
                          dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using block = BN<dlib::con<N,3,3,1,1,dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

    template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
    template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

    using anet_type = dlib::loss_metric<dlib::fc_no_bias<128,dlib::avg_pool_everything<
                      alevel0<alevel1<alevel2<alevel3<alevel4<
                      dlib::max_pool<3,3,2,2,dlib::relu<dlib::affine<dlib::con<32,7,7,2,2,
                      dlib::input_rgb_image_sized<150>>>>>>>>>>>>>;

    anet_type net_;
    dlib::shape_predictor shapePredictor_;
    std::shared_ptr<FaceDatabase> database_;
    bool initialized_ = false;
};

} // namespace smart_classroom
