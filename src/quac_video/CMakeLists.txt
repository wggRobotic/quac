cmake_minimum_required(VERSION 3.5)
project(quac_video LANGUAGES C CXX CUDA)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)
find_package(CUDA REQUIRED)
find_package(sensor_msgs)
find_package(image_transport REQUIRED)
find_package(CUDAToolkit REQUIRED)


set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Jetson Orin = sm_87, Xavier = sm_72
set(CMAKE_CUDA_ARCHITECTURES "72;87")

set(TENSORRT_INCLUDE_DIR /usr/include)
link_directories(/usr/lib/aarch64-linux-gnu)

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

add_executable(camera_app
  src/camera_app.cpp
)

target_link_libraries(camera_app
  ${GST_LIBRARIES}
  ${GST_APP_LIBRARIES}
  ${realsense2_LIBRARY}
  ${CUDA_LIBRARIES}
  ${OpenCV_LIBS}
  nvinfer
  CUDA::cudart
  cuda_preprocessing
  quirc
)

ament_target_dependencies(camera_app
  rclcpp
  sensor_msgs
  image_transport
)

install(
  TARGETS camera_app
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
