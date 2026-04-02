// ============================================================
// DrawingFilter.cpp — 绘制过滤器实现
// ============================================================
// [Design Pattern: Pipeline-Filter] 管道第 5 级
// 职责：在图像上绘制人脸框和身份信息，生成可视化结果。
// ============================================================

#include "pipeline/filters/DrawingFilter.h"
#include "core/Logger.h"
#include <opencv2/imgproc.hpp>

namespace smart_classroom {

// [Design Pattern: Pipeline-Filter] 过滤器处理实现
bool DrawingFilter::process(VideoFrame& frame) {
    if (frame.image.empty()) {
        LOG_WARN("DrawingFilter: received empty image");
        return false;
    }

    // 遍历所有检测/识别到的人脸，绘制包围框和标签
    for (const auto& face : frame.detectedFaces) {
        cv::Rect rect = face.bbox.toCvRect();

        // 根据识别状态选择颜色
        cv::Scalar color;
        std::string label;

        if (!face.identity.empty()) {
            // 已识别：绿色框 + 显示姓名
            color = cv::Scalar(0, 255, 0);
            label = face.identity;
        } else {
            // 未识别：红色框 + "Unknown"
            color = cv::Scalar(0, 0, 255);
            label = "Unknown";
        }

        // 绘制人脸包围框
        cv::rectangle(frame.image, rect, color, 2);

        // 绘制置信度信息
        std::string confStr = "(" + std::to_string(
            static_cast<int>(face.detectionConfidence * 100)) + "%)";
        label += " " + confStr;

        // 绘制标签背景
        int baseLine = 0;
        cv::Size textSize = cv::getTextSize(
            label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        cv::rectangle(frame.image,
            cv::Point(rect.x, rect.y - textSize.height - 10),
            cv::Point(rect.x + textSize.width, rect.y),
            color, cv::FILLED);

        // 绘制标签文字（白色）
        cv::putText(frame.image, label,
            cv::Point(rect.x, rect.y - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.5,
            cv::Scalar(255, 255, 255), 1);
    }

    LOG_DEBUG("DrawingFilter: drew " + std::to_string(frame.detectedFaces.size())
              + " face annotation(s)");
    return true;
}

} // namespace smart_classroom
