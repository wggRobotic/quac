cmake_minimum_required(VERSION 3.5)
project(quac_control)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

#-----------------------------------------------
#----------------- controllers -----------------
#-----------------------------------------------

include_directories(controllers/include controllers/src)

add_library(quac_controllers SHARED
  controllers/src/wheel_monitor/wheel_monitor.cpp
)

ament_target_dependencies(quac_controllers
  rclcpp
  pluginlib
  std_msgs
  controller_interface
)

ament_export_libraries(
  quac_controllers
)

install(TARGETS quac_controllers
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(FILES controllers/quac_controllers.xml
  DESTINATION share/${PROJECT_NAME}
)

pluginlib_export_plugin_description_file(controller_interface controllers/quac_controllers.xml)

#--------------------------------------------
#----------------- hardware -----------------
#--------------------------------------------

include_directories(hardware/include hardware/src)

add_library(quac_hardware SHARED
  hardware/src/ddsm115/ddsm115.cpp
  hardware/src/ddsm115/DDSM115CMD.cpp

  hardware/src/waveshare_servos/waveshare_servos.cpp
  hardware/src/waveshare_servos/SCSCL.cpp
  hardware/src/waveshare_servos/SCS.cpp
  hardware/src/waveshare_servos/SCSerial.cpp
  hardware/src/waveshare_servos/SMSBL.cpp
  hardware/src/waveshare_servos/SMSCL.cpp
  hardware/src/waveshare_servos/SMS_STS.cpp
)

ament_target_dependencies(quac_hardware
  rclcpp
  hardware_interface
  pluginlib
  controller_interface
)

add_executable(set_id hardware/src/waveshare_servos/set_id.cpp)
target_link_libraries(set_id quac_hardware)
ament_target_dependencies(set_id rclcpp)

add_executable(calibrate_midpoint hardware/src/waveshare_servos/calibrate_midpoint.cpp)
target_link_libraries(calibrate_midpoint quac_hardware)
ament_target_dependencies(calibrate_midpoint rclcpp)

pluginlib_export_plugin_description_file(hardware_interface hardware/quac_hardware.xml)

ament_export_libraries(
  quac_hardware
)

install(TARGETS quac_hardware
  DESTINATION lib
)

install(
  TARGETS set_id calibrate_midpoint
  DESTINATION lib/${PROJECT_NAME}
)

#-----------------------------------------
#----------------- nodes -----------------
#--------------------------------------------

add_executable(quac_ik_node nodes/src/quac_ik_node.cpp)
ament_target_dependencies(quac_ik_node rclcpp geometry_msgs trajectory_msgs)

install(TARGETS
  quac_ik_node
  DESTINATION lib/${PROJECT_NAME})

# amen_package
ament_package()
