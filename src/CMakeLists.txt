# ============================================================
# 源文件收集
# ============================================================
set(CORE_SOURCES
    core/ConfigManager.cpp
    core/Logger.cpp
    core/CudaResourceManager.cpp
)

set(VISION_SOURCES
    vision/DetectorFactory.cpp
    vision/RecognizerFactory.cpp
    vision/FaceDatabase.cpp
    vision/FaceTracker.cpp
    vision/detectors/DlibHogDetector.cpp
    vision/detectors/DlibCnnDetector.cpp
    vision/detectors/OpenCvDnnDetector.cpp
    vision/recognizers/DlibFaceRecognizer.cpp
    vision/recognizers/OpenCvLbphRecognizer.cpp
)

set(PIPELINE_SOURCES
    pipeline/Pipeline.cpp
    pipeline/FilterFactory.cpp
    pipeline/filters/DecodeFilter.cpp
    pipeline/filters/PreprocessFilter.cpp
    pipeline/filters/FaceDetectionFilter.cpp
    pipeline/filters/FaceRecognitionFilter.cpp
    pipeline/filters/DrawingFilter.cpp
    pipeline/filters/EncodeFilter.cpp
)

set(OBSERVER_SOURCES
    observer/RecognitionEventSubject.cpp
    observer/AttendanceObserver.cpp
    observer/NotificationObserver.cpp
)

set(NETWORK_SOURCES
    network/WebRTCEndpoint.cpp
    network/SignalingHandler.cpp
    network/TrackFactory.cpp
    network/PipeProcessor.cpp
)

# ============================================================
# 静态库: 核心模块
# ============================================================
add_library(smart_classroom_lib STATIC
    ${CORE_SOURCES}
    ${VISION_SOURCES}
    ${PIPELINE_SOURCES}
    ${OBSERVER_SOURCES}
    ${NETWORK_SOURCES}
)

target_include_directories(smart_classroom_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(smart_classroom_lib PUBLIC
    ${OpenCV_LIBS}
    dlib::dlib
    nlohmann_json::nlohmann_json
)

# 可选链接 CUDA
if(ENABLE_CUDA AND CUDAToolkit_FOUND)
    target_link_libraries(smart_classroom_lib PUBLIC
        CUDA::cudart
    )
endif()

# ============================================================
# 可执行文件: 主入口
# ============================================================
add_executable(smart_classroom main.cpp)
target_link_libraries(smart_classroom PRIVATE smart_classroom_lib)
