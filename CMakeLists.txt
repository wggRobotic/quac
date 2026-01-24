cmake_minimum_required(VERSION 3.5)
project(quac)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)

include_directories(${GAZEBO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/hardware/include ${CMAKE_SOURCE_DIR}/hardware/waveshare_servos)

add_library(quac_hardware SHARED
  hardware/ddsm115.cpp

  hardware/waveshare_servos.cpp
  hardware/waveshare_servos/SCSCL.cpp
  hardware/waveshare_servos/SCS.cpp
  hardware/waveshare_servos/SCSerial.cpp
  hardware/waveshare_servos/SMSBL.cpp
  hardware/waveshare_servos/SMSCL.cpp
  hardware/waveshare_servos/SMS_STS.cpp
)

ament_target_dependencies(quac_hardware
  rclcpp
  hardware_interface
  pluginlib
  controller_interface
)

add_executable(set_id hardware/waveshare_servos/set_id.cpp)
target_link_libraries(set_id quac_hardware)
ament_target_dependencies(set_id rclcpp)

add_executable(calibrate_midpoint hardware/waveshare_servos/calibrate_midpoint.cpp)
target_link_libraries(calibrate_midpoint quac_hardware)
ament_target_dependencies(calibrate_midpoint rclcpp)

pluginlib_export_plugin_description_file(hardware_interface quac_hardware.xml)

ament_export_libraries(
  quac_hardware
)

install(FILES
  quac_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS quac_hardware
  DESTINATION lib
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)

  ament_lint_auto_find_test_dependencies()
endif()

install(
  DIRECTORY config description launch worlds meshes
  DESTINATION share/${PROJECT_NAME}  
)

install(
  TARGETS set_id calibrate_midpoint
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
