cmake_minimum_required(VERSION 3.5)
project(quac_video LANGUAGES CXX CUDA)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ament_index_cpp REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
find_package(OpenCV REQUIRED)
find_package(realsense2 REQUIRED)
find_package(CUDA REQUIRED)
find_package(sensor_msgs)
find_package(image_transport REQUIRED)
find_package(CUDAToolkit REQUIRED)

add_subdirectory(YOLOs-CPP-TensorRT)

#-----------------------------------------------
#----------------- application -----------------
#-----------------------------------------------

include_directories(
  ${GST_INCLUDE_DIRS}
  ${realsense2_INCLUDE_DIR}
  ${CUDA_INCLUDE_DIRS}
  ${OPENCV_INCLUDE_DIRS}
  include/
  YOLOs-CPP-TensorRT/include
)

link_directories(
    ${GST_LIBRARY_DIRS}
)

add_executable(camera_app
  src/camera_app.cpp
)

target_include_directories(camera_app PRIVATE
    "${TENSORRT_INCLUDE_DIR}"
)

if(UNIX)
  set_target_properties(camera_app PROPERTIES
      BUILD_RPATH "$ORIGIN"
      INSTALL_RPATH "$ORIGIN"
  )
endif()

target_link_libraries(camera_app
  ${GST_LIBRARIES}
  ${GST_APP_LIBRARIES}
  ${realsense2_LIBRARY}
  ${CUDA_LIBRARIES}
  ${OpenCV_LIBS}
  ${NVINFER_LIB}
  CUDA::cudart
  cuda_preprocessing
  Threads::Threads
)

ament_target_dependencies(camera_app
  rclcpp
  sensor_msgs
  ament_index_cpp
  image_transport
)

install(
  TARGETS camera_app
  DESTINATION lib/${PROJECT_NAME}
)

# amen_package
ament_package()
