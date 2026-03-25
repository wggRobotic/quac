cmake_minimum_required(VERSION 3.5)
project(quac)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY config description launch worlds meshes
  DESTINATION share/${PROJECT_NAME}  
)

ament_package()