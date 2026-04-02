#pragma once
// ============================================================
// DlibHogDetector.h — Dlib HOG + SVM 人脸检测器
// ============================================================
// [Design Pattern: Strategy] 具体策略类
// 实现基于 Dlib HOG (Histogram of Oriented Gradients) 特征
// + SVM 分类器的人脸检测算法。该方法为纯 CPU 实现，速度较快
// 但精度相对 CNN 略低，适用于实时性要求高的场景。
// ============================================================

#include "vision/IFaceDetector.h"
#include <dlib/image_processing/frontal_face_detector.h>

namespace smart_classroom {

class DlibHogDetector : public IFaceDetector {
public:
    DlibHogDetector();
    ~DlibHogDetector() override = default;

    // [Design Pattern: Strategy] 实现策略接口的检测方法
    std::vector<FaceInfo> detect(const cv::Mat& frame) override;
    std::string getName() const override { return "DlibHogDetector"; }
    bool requiresCuda() const override { return false; }

private:
    dlib::frontal_face_detector detector_;
};

} // namespace smart_classroom
