cmake_minimum_required(VERSION 3.5)
project(quac_video)

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
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)
find_package(realsense2 REQUIRED)
find_package(CUDA REQUIRED)
find_package(sensor_msgs)

#----------------- Deepstream-Yolo-------------

include(ExternalProject)

# Build DeepStream-Yolo library from its Makefile
include(ExternalProject)

# Paths
set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)
set(YOLO_PLUGIN_SRC_DIR ${DEPS_DIR}/DeepStream-Yolo/nvdsinfer_custom_impl_Yolo)
set(YOLO_PLUGIN_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/deepstream_yolo_build)
set(YOLO_PLUGIN_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/deepstream_yolo_install)

# Build DeepStream-Yolo library
ExternalProject_Add(
    deepstream_yolo_build
    SOURCE_DIR ${YOLO_PLUGIN_SRC_DIR}
    BINARY_DIR ${YOLO_PLUGIN_BUILD_DIR}      # build happens in workspace build dir
    CONFIGURE_COMMAND ""                     # skip configure
    BUILD_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${YOLO_PLUGIN_SRC_DIR} ${YOLO_PLUGIN_BUILD_DIR} &&
        export CUDA_VER=12.6 &&
        make -C ${YOLO_PLUGIN_BUILD_DIR} clean &&
        make -C ${YOLO_PLUGIN_BUILD_DIR}
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E make_directory ${YOLO_PLUGIN_INSTALL_DIR}/lib &&
        # copy only the .so
        ${CMAKE_COMMAND} -E copy ${YOLO_PLUGIN_BUILD_DIR}/libnvdsinfer_custom_impl_Yolo.so ${YOLO_PLUGIN_INSTALL_DIR}/lib/
)

# Imported library to link against
add_library(deepstream_yolo SHARED IMPORTED)
set_target_properties(deepstream_yolo PROPERTIES
    IMPORTED_LOCATION ${YOLO_PLUGIN_INSTALL_DIR}/lib/libnvdsinfer_custom_impl_Yolo.so
)

# Ensure library builds before your node
add_dependencies(deepstream_yolo deepstream_yolo_build)

install(FILES ${YOLO_PLUGIN_INSTALL_DIR}/lib/libnvdsinfer_custom_impl_Yolo.so
        DESTINATION lib/${PROJECT_NAME})


#-----------------------------------------------
#----------------- application -----------------
#-----------------------------------------------

include_directories(
    ${GST_INCLUDE_DIRS}
    ${realsense2_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIRS}
    /opt/nvidia/deepstream/deepstream-7.1/includes
    /opt/nvidia/deepstream/deepstream-7.1/sources/includes
    include/
)

link_directories(
    ${GST_LIBRARY_DIRS}
    /opt/nvidia/deepstream/deepstream-7.1/lib
)

add_executable(video_app
  src/video_app.cpp
  src/gst_app.cpp
)

target_link_libraries(video_app
    ${GST_LIBRARIES}
    ${realsense2_LIBRARY}
    ${CUDA_LIBRARIES}
    nvdsgst_meta
    nvdsgst_helper
    nvds_infer
)

ament_target_dependencies(video_app
  rclcpp
  sensor_msgs
  ament_index_cpp
)

install(
  DIRECTORY config models
  DESTINATION share/${PROJECT_NAME}  
)

install(
  TARGETS video_app
  DESTINATION lib/${PROJECT_NAME}
)

# amen_package
ament_package()
