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
find_package(std_msgs REQUIRED)

#-----------------------------------------------
#----------------- application -----------------
#-----------------------------------------------

add_executable(stream_app SHARED
  application/main_test.cpp
)

# amen_package
ament_package()
