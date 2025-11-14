cmake_minimum_required(VERSION 3.5)
project(quac)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(gazebo_dev REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(controller_interface REQUIRED)

include_directories(${GAZEBO_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)

add_library(twist_joint_plugin SHARED src/gazebo_plugin.cpp)
ament_target_dependencies(
  twist_joint_plugin
  rclcpp
  geometry_msgs
  sensor_msgs
  nav_msgs
  tf2
  tf2_ros  
)
target_link_libraries(twist_joint_plugin ${GAZEBO_LIBRARIES})

add_library(ddsm_hardware SHARED
  src/ddsm_hardware.cpp
)
ament_target_dependencies(ddsm_hardware
  rclcpp
  hardware_interface
  pluginlib
  controller_interface
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
  TARGETS twist_joint_plugin
  DESTINATION lib
)

pluginlib_export_plugin_description_file(hardware_interface quac_hardware.xml)

install(FILES
  quac_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS ddsm_hardware
  LIBRARY DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include/)

ament_package()
