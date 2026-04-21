cmake_minimum_required(VERSION 3.18)
project(quac_video LANGUAGES C CXX CUDA)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

add_compile_definitions(JETSON_PLATFORM)

# Jetson Orin = sm_87, Xavier = sm_72
set(CMAKE_CUDA_ARCHITECTURES "72;87")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)
find_package(CUDA REQUIRED)
find_package(CUDAToolkit REQUIRED)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(quac_interfaces REQUIRED)


if(NOT DEFINED TENSORRT_DIR)
    # Try common TensorRT locations
    set(TENSORRT_SEARCH_PATHS
        "/usr/lib/x86_64-linux-gnu"          # Ubuntu x86 system install
        "/usr/lib/aarch64-linux-gnu"          # Jetson / aarch64 system
        "/usr/local/tensorrt"                 # Manual install
        "$ENV{TENSORRT_DIR}"                  # Environment variable
        "${CMAKE_SOURCE_DIR}/TensorRT"        # Local copy
    )
    foreach(path ${TENSORRT_SEARCH_PATHS})
        if(EXISTS "${path}/libnvinfer.so" OR EXISTS "${path}/lib/libnvinfer.so")
            set(TENSORRT_DIR "${path}")
            break()
        endif()
    endforeach()
endif()

find_path(TENSORRT_INCLUDE_DIR NvInfer.h
    HINTS ${TENSORRT_DIR}/include /usr/include/x86_64-linux-gnu /usr/include/aarch64-linux-gnu /usr/include
)

# Find TensorRT libraries
find_library(NVINFER_LIB nvinfer
    HINTS ${TENSORRT_DIR}/lib ${TENSORRT_DIR} /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
)
find_library(NVONNXPARSER_LIB nvonnxparser
    HINTS ${TENSORRT_DIR}/lib ${TENSORRT_DIR} /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
)

# CUDA Preprocessing Library

add_library(cuda_preprocessing STATIC
  YOLOs-CPP-TensorRT/include/yolos/core/cuda_preprocessing.cu
)
target_include_directories(cuda_preprocessing PRIVATE
  YOLOs-CPP-TensorRT/include
  ${TENSORRT_INCLUDE_DIR}
)
target_link_libraries(cuda_preprocessing PRIVATE
  CUDA::cudart
)
set_target_properties(cuda_preprocessing PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  POSITION_INDEPENDENT_CODE ON
)

# quirc

add_library(quirc STATIC
  quirc/lib/decode.c
  quirc/lib/identify.c
  quirc/lib/quirc.c
  quirc/lib/version_db.c
)

#-----------------------------------------------
#----------------- application -----------------
#-----------------------------------------------

include_directories(
  ${GST_INCLUDE_DIRS}
  ${realsense2_INCLUDE_DIR}
  ${CUDA_INCLUDE_DIRS}
  ${OPENCV_INCLUDE_DIRS}
  ${TENSORRT_INCLUDE_DIR}
  include/
  YOLOs-CPP-TensorRT/include
  quirc/lib
)

# realsense_streamer
add_executable(realsense_streamer
  src/realsense_streamer.cpp
)

target_link_libraries(realsense_streamer
  ${GST_LIBRARIES}
  ${GST_APP_LIBRARIES}
  ${realsense2_LIBRARY}
)

ament_target_dependencies(realsense_streamer
  rclcpp
  std_msgs
  sensor_msgs
  quac_interfaces
)

# qrcode_detection_server

# add_executable(qrcode_detection_server
#   src/hazmat_detection_server.cpp
# )

# target_link_libraries(qrcode_detection_server
#   quirc
#   ${OpenCV_LIBS}
# )

# ament_target_dependencies(qrcode_detection_server
#   rclcpp
#   std_msgs
#   quac_interfaces
# )

# hazmat_detection_server

add_executable(hazmat_detection_server
  src/hazmat_detection_server.cpp
)

target_link_libraries(hazmat_detection_server
  ${CUDA_LIBRARIES}
  ${OpenCV_LIBS}
  nvinfer
  CUDA::cudart
  cuda_preprocessing
)

ament_target_dependencies(hazmat_detection_server
  rclcpp
  std_msgs
  quac_interfaces
  sensor_msgs
  geometry_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
)

if(NVONNXPARSER_LIB)
  target_link_libraries(hazmat_detection_server ${NVONNXPARSER_LIB})
endif()

target_link_options(hazmat_detection_server PRIVATE
  -Wl,--allow-shlib-undefined
)

install(
  TARGETS realsense_streamer hazmat_detection_server
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()