// ============================================================
// test_strategy.cpp — 策略模式单元测试
// ============================================================
// 验证：
//   1. IFaceDetector 接口的多态性——不同检测策略可互换
//   2. IFaceRecognizer 接口的多态性——不同识别策略可互换
//   3. 运行时策略切换的正确性
//   4. 工厂方法创建的策略对象的有效性
// ============================================================

#include "vision/IFaceDetector.h"
#include "vision/IFaceRecognizer.h"
#include "core/Types.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

using namespace smart_classroom;

// ============================================================
// Mock 检测策略：用于测试策略模式的多态性
// ============================================================
class MockDetectorA : public IFaceDetector {
public:
    std::vector<FaceInfo> detect(const cv::Mat& frame) override {
        FaceInfo face;
        face.bbox = BoundingBox(10, 10, 100, 100);
        face.detectionConfidence = 0.95f;
        return {face};
    }
    std::string getName() const override { return "MockDetectorA"; }
};

class MockDetectorB : public IFaceDetector {
public:
    std::vector<FaceInfo> detect(const cv::Mat& frame) override {
        FaceInfo face1, face2;
        face1.bbox = BoundingBox(10, 10, 100, 100);
        face1.detectionConfidence = 0.90f;
        face2.bbox = BoundingBox(200, 50, 80, 80);
        face2.detectionConfidence = 0.85f;
        return {face1, face2};
    }
    std::string getName() const override { return "MockDetectorB"; }
};

// ============================================================
// Mock 识别策略
// ============================================================
class MockRecognizerA : public IFaceRecognizer {
public:
    std::vector<float> extractFeature(const cv::Mat& faceChip) override {
        return std::vector<float>(128, 0.5f);
    }
    std::string recognize(const cv::Mat& faceChip, float& confidence) override {
        confidence = 0.3f;
        return "StudentA";
    }
    std::string getName() const override { return "MockRecognizerA"; }
};

class MockRecognizerB : public IFaceRecognizer {
public:
    std::vector<float> extractFeature(const cv::Mat& faceChip) override {
        return std::vector<float>(128, 0.8f);
    }
    std::string recognize(const cv::Mat& faceChip, float& confidence) override {
        confidence = 0.5f;
        return "StudentB";
    }
    std::string getName() const override { return "MockRecognizerB"; }
};

// ============================================================
// 使用策略的客户端（模拟 FaceDetectionFilter 的角色）
// ============================================================
class DetectorClient {
public:
    void setDetector(std::unique_ptr<IFaceDetector> d) {
        detector_ = std::move(d);
    }
    std::vector<FaceInfo> performDetection(const cv::Mat& frame) {
        return detector_ ? detector_->detect(frame) : std::vector<FaceInfo>{};
    }
    std::string getDetectorName() {
        return detector_ ? detector_->getName() : "None";
    }
private:
    std::unique_ptr<IFaceDetector> detector_;
};

// ============================================================
// 测试用例
// ============================================================

void test_detector_polymorphism() {
    std::cout << "[TEST] Detector strategy polymorphism... ";
    cv::Mat dummyFrame(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));

    // 策略 A 返回 1 个人脸
    std::unique_ptr<IFaceDetector> detectorA = std::make_unique<MockDetectorA>();
    auto facesA = detectorA->detect(dummyFrame);
    assert(facesA.size() == 1);
    assert(detectorA->getName() == "MockDetectorA");

    // 策略 B 返回 2 个人脸
    std::unique_ptr<IFaceDetector> detectorB = std::make_unique<MockDetectorB>();
    auto facesB = detectorB->detect(dummyFrame);
    assert(facesB.size() == 2);
    assert(detectorB->getName() == "MockDetectorB");

    std::cout << "PASSED\n";
}

void test_runtime_strategy_switch() {
    std::cout << "[TEST] Runtime strategy switch... ";
    cv::Mat dummyFrame(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));

    DetectorClient client;

    // 初始使用策略 A
    client.setDetector(std::make_unique<MockDetectorA>());
    assert(client.getDetectorName() == "MockDetectorA");
    auto results1 = client.performDetection(dummyFrame);
    assert(results1.size() == 1);

    // 运行时切换到策略 B（无需重建客户端）
    client.setDetector(std::make_unique<MockDetectorB>());
    assert(client.getDetectorName() == "MockDetectorB");
    auto results2 = client.performDetection(dummyFrame);
    assert(results2.size() == 2);

    std::cout << "PASSED\n";
}

void test_recognizer_polymorphism() {
    std::cout << "[TEST] Recognizer strategy polymorphism... ";
    cv::Mat dummyFace(100, 100, CV_8UC3, cv::Scalar(128, 128, 128));

    // 识别策略 A
    std::unique_ptr<IFaceRecognizer> recA = std::make_unique<MockRecognizerA>();
    float conf = 0;
    std::string nameA = recA->recognize(dummyFace, conf);
    assert(nameA == "StudentA");
    assert(recA->getName() == "MockRecognizerA");

    auto featureA = recA->extractFeature(dummyFace);
    assert(featureA.size() == 128);

    // 识别策略 B
    std::unique_ptr<IFaceRecognizer> recB = std::make_unique<MockRecognizerB>();
    std::string nameB = recB->recognize(dummyFace, conf);
    assert(nameB == "StudentB");
    assert(recB->getName() == "MockRecognizerB");

    std::cout << "PASSED\n";
}

void test_dual_strategy_independence() {
    std::cout << "[TEST] Dual-layer strategy independence... ";
    cv::Mat dummyFrame(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));

    // 检测和识别策略可以独立切换
    std::unique_ptr<IFaceDetector> detector = std::make_unique<MockDetectorA>();
    std::unique_ptr<IFaceRecognizer> recognizer = std::make_unique<MockRecognizerB>();

    auto faces = detector->detect(dummyFrame);
    assert(faces.size() == 1);

    float conf = 0;
    std::string name = recognizer->recognize(dummyFrame, conf);
    assert(name == "StudentB");

    // 切换检测器不影响识别器
    detector = std::make_unique<MockDetectorB>();
    auto faces2 = detector->detect(dummyFrame);
    assert(faces2.size() == 2);

    // 识别器仍然返回 StudentB
    name = recognizer->recognize(dummyFrame, conf);
    assert(name == "StudentB");

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Strategy Pattern Tests\n";
    std::cout << "========================================\n";

    test_detector_polymorphism();
    test_runtime_strategy_switch();
    test_recognizer_polymorphism();
    test_dual_strategy_independence();

    std::cout << "========================================\n";
    std::cout << "All strategy tests PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}
