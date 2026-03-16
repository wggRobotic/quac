cmake_minimum_required(VERSION 3.10)
project(realsenseinfer)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)
find_package(realsense2 REQUIRED)
find_package(CUDA REQUIRED)

# GStreamer + RealSense + CUDA
include_directories(
    ${GST_INCLUDE_DIRS}
    ${realsense2_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIRS}
    /opt/nvidia/deepstream/deepstream-7.1/includes
    /opt/nvidia/deepstream/deepstream-7.1/sources/includes
)

link_directories(
    ${GST_LIBRARY_DIRS}
    /opt/nvidia/deepstream/deepstream-7.1/lib
)

add_executable(application main_test.cpp)

target_link_libraries(application
    ${GST_LIBRARIES}
    ${realsense2_LIBRARY}
    ${CUDA_LIBRARIES}
    nvdsgst_meta
    nvdsgst_helper
    nvds_infer
)