#pragma once
// ============================================================
// Types.h — 系统公共类型定义
// ============================================================
// 定义了系统中所有模块共用的数据结构，包括视频帧、人脸信息、
// 识别事件等。这些类型在管道-过滤器架构中作为"数据流"在各
// Filter 之间传递。
// ============================================================

#include <cstdint>
#include <string>
#include <vector>
#include <opencv2/core.hpp>

namespace smart_classroom {

// ============================================================
// 帧格式枚举
// ============================================================
enum class FrameFormat {
    RAW_BGR,        // OpenCV 默认 BGR 格式
    RAW_RGB,        // RGB 格式
    ENCODED_JPEG,   // JPEG 编码
    ENCODED_H264,   // H.264 编码
    ENCODED_VP8     // VP8 编码
};

// ============================================================
// 人脸检测算法类型（用于策略模式切换）
// ============================================================
enum class DetectorType {
    DLIB_HOG,        // Dlib HOG + SVM (CPU)
    DLIB_CNN_CUDA,   // Dlib CNN (CUDA GPU 加速)
    OPENCV_DNN       // OpenCV DNN (支持 CUDA 后端)
};

// ============================================================
// 人脸识别算法类型（用于策略模式切换）
// ============================================================
enum class RecognizerType {
    DLIB_RESNET,     // Dlib ResNet 128D 特征提取
    OPENCV_LBPH      // OpenCV LBPH 方法
};

// ============================================================
// 识别事件类型（用于观察者模式）
// ============================================================
enum class RecognitionEventType {
    FACE_DETECTED,   // 检测到人脸（尚未识别身份）
    FACE_IDENTIFIED, // 成功识别出已注册身份
    FACE_UNKNOWN     // 检测到人脸但无法匹配已注册身份
};

// ============================================================
// 过滤器类型枚举（用于工厂模式）
// ============================================================
enum class FilterType {
    DECODE,
    PREPROCESS,
    FACE_DETECTION,
    FACE_RECOGNITION,
    DRAWING,
    ENCODE
};

// ============================================================
// 包围框
// ============================================================
struct BoundingBox {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    BoundingBox() = default;
    BoundingBox(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}

    cv::Rect toCvRect() const { return cv::Rect(x, y, width, height); }
};

// ============================================================
// 人脸信息（检测 + 识别结果）
// ============================================================
struct FaceInfo {
    BoundingBox bbox;                   // 人脸位置
    float detectionConfidence = 0.0f;   // 检测置信度
    std::string identity;               // 识别出的身份（学生姓名/ID），空表示未识别
    float recognitionDistance = -1.0f;   // 特征距离（越小越相似）
    std::vector<float> featureVector;   // 128D 特征向量
};

// ============================================================
// 人脸身份（数据库中注册的学生信息）
// ============================================================
struct FaceIdentity {
    std::string studentId;              // 学号
    std::string name;                   // 姓名
    std::vector<float> featureDescriptor; // 128D 特征向量
};

// ============================================================
// 识别事件（观察者模式中传递的事件数据）
// ============================================================
struct RecognitionEvent {
    RecognitionEventType type;          // 事件类型
    FaceInfo faceInfo;                  // 关联的人脸信息
    int64_t timestamp = 0;             // 事件时间戳 (毫秒)
    std::string message;               // 事件描述消息
};

// ============================================================
// 视频帧（管道-过滤器架构中流动的核心数据对象）
// ============================================================
struct VideoFrame {
    cv::Mat image;                      // 图像数据
    std::vector<uint8_t> encodedData;   // 编码后的数据
    FrameFormat format = FrameFormat::RAW_BGR;
    int64_t timestamp = 0;             // 帧时间戳 (毫秒)
    int frameIndex = 0;                // 帧序号

    // 检测和识别的结果 —— 在 Pipeline 中逐步填充
    std::vector<FaceInfo> detectedFaces;
};

} // namespace smart_classroom
