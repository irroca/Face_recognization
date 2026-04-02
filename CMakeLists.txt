cmake_minimum_required(VERSION 3.18)
project(SmartClassroomFaceRecognition
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Smart Classroom Face Tracking & Recognition System"
)

# ============================================================
# C++17 标准
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# 构建选项
# ============================================================
option(ENABLE_CUDA    "Enable CUDA acceleration for Dlib/OpenCV" ON)
option(BUILD_TESTS    "Build unit tests"                         ON)

# ============================================================
# 查找依赖库
# ============================================================

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui dnn videoio objdetect imgcodecs)
# face 模块在 contrib 中，单独查找（可选）
find_package(OpenCV QUIET COMPONENTS face)
if(OpenCV_face_FOUND OR TARGET opencv_face)
    message(STATUS "OpenCV face module: found")
else()
    # 系统包中 face 模块可能不注册为单独 component，直接检查头文件
    find_path(OPENCV_FACE_INCLUDE opencv2/face.hpp HINTS ${OpenCV_INCLUDE_DIRS})
    if(OPENCV_FACE_INCLUDE)
        message(STATUS "OpenCV face module: found (via header)")
    else()
        message(WARNING "OpenCV face module not found, OpenCvLbphRecognizer will not be functional")
    endif()
endif()
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# Dlib
find_package(dlib REQUIRED)
message(STATUS "Dlib found: ${dlib_DIR}")

# CUDA Toolkit (可选)
if(ENABLE_CUDA)
    find_package(CUDAToolkit QUIET)
    if(CUDAToolkit_FOUND)
        message(STATUS "CUDA Toolkit version: ${CUDAToolkit_VERSION}")
        add_definitions(-DCUDA_ENABLED)
    else()
        message(WARNING "CUDA Toolkit not found, CUDA acceleration disabled")
        set(ENABLE_CUDA OFF)
    endif()
endif()

# nlohmann/json (JSON 解析)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
    )
    FetchContent_MakeAvailable(json)
endif()

# ============================================================
# 子目录
# ============================================================
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
