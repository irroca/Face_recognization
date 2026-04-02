#pragma once
// ============================================================
// IFaceDetector.h — 人脸检测策略接口
// ============================================================
// [Design Pattern: Strategy]
// 定义人脸检测算法的抽象接口，实现"检测算法"与"使用检测的客户
// 端代码"之间的解耦。
//
// 动机：系统需要支持多种人脸检测算法（Dlib HOG、Dlib CNN with
// CUDA、OpenCV DNN），如果在检测逻辑中使用 if-else 或 switch
// 来选择算法，会导致以下坏味道：
//   - "过长的条件分支" (Long Method / Switch Statements)
//   - "违反开闭原则" (OCP) — 新增算法需修改已有代码
//
// 策略模式将每种检测算法封装为独立的策略类，通过接口多态实现
// 运行时算法切换，新增检测算法只需新增一个策略类，无需修改任何
// 已有代码。
// ============================================================

#include "core/Types.h"
#include <vector>
#include <string>
#include <opencv2/core.hpp>

namespace smart_classroom {

// ============================================================
// [Design Pattern: Strategy] 策略接口
// 所有人脸检测算法必须实现此接口
// ============================================================
class IFaceDetector {
public:
    virtual ~IFaceDetector() = default;

    // 核心检测方法：输入一帧图像，返回检测到的所有人脸信息
    virtual std::vector<FaceInfo> detect(const cv::Mat& frame) = 0;

    // 返回当前策略的名称（用于日志和调试）
    virtual std::string getName() const = 0;

    // 返回该策略是否需要 CUDA 支持
    virtual bool requiresCuda() const { return false; }
};

} // namespace smart_classroom
