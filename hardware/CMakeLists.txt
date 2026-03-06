find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)

include_directories(include src)

add_library(quac_hardware SHARED
  src/ddsm115/ddsm115.cpp
  src/ddsm115/DDSM115CMD.cpp

  src/waveshare_servos/waveshare_servos.cpp
  src/waveshare_servos/SCSCL.cpp
  src/waveshare_servos/SCS.cpp
  src/waveshare_servos/SCSerial.cpp
  src/waveshare_servos/SMSBL.cpp
  src/waveshare_servos/SMSCL.cpp
  src/waveshare_servos/SMS_STS.cpp
)

ament_target_dependencies(quac_hardware
  rclcpp
  hardware_interface
  pluginlib
  controller_interface
)

add_executable(set_id src/waveshare_servos/set_id.cpp)
target_link_libraries(set_id quac_hardware)
ament_target_dependencies(set_id rclcpp)

add_executable(calibrate_midpoint src/waveshare_servos/calibrate_midpoint.cpp)
target_link_libraries(calibrate_midpoint quac_hardware)
ament_target_dependencies(calibrate_midpoint rclcpp)

pluginlib_export_plugin_description_file(hardware_interface quac_hardware.xml)

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

set(AMENT_EXTENSIONS_ament_package ${AMENT_EXTENSIONS_ament_package} PARENT_SCOPE)
