find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)

include_directories(include src)

add_library(quac_controllers SHARED
  src/wheel_monitor/wheel_monitor.cpp
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

install(FILES quac_controllers.xml
  DESTINATION share/${PROJECT_NAME}
)

pluginlib_export_plugin_description_file(controller_interface quac_controllers.xml)

set(AMENT_EXTENSIONS_ament_package ${AMENT_EXTENSIONS_ament_package} PARENT_SCOPE)
