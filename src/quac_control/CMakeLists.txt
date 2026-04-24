cmake_minimum_required(VERSION 3.18)
project(quac_control)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_CONTROLLERS "Build controllers" OFF)
option(BUILD_INVERSE_KINEMATICS "Build inverse_kinematics" OFF)

if(NOT BUILD_CONTROLLERS AND NOT BUILD_INVERSE_KINEMATICS)
  set(BUILD_CONTROLLERS ON)
  set(BUILD_INVERSE_KINEMATICS ON)
endif()

##################################################
# controllers
##################################################

if(BUILD_CONTROLLERS)

  find_package(ament_cmake REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(pluginlib REQUIRED)
  find_package(controller_interface REQUIRED)

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

endif()

##################################################
# inverse_kinematics
##################################################

if(BUILD_INVERSE_KINEMATICS)

  find_package(ament_cmake REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(trajectory_msgs REQUIRED)

  add_executable(ik_node nodes/src/ik_node.cpp)
  ament_target_dependencies(ik_node rclcpp geometry_msgs trajectory_msgs std_msgs)

  install(TARGETS
    ik_node
    DESTINATION lib/${PROJECT_NAME})
endif()

ament_package()
