FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y nano vim

RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-slam-toolbox
RUN apt install -y ros-humble-navigation2 ros-humble-nav2-bringup
RUN apt install -y ros-humble-ros2-control ros-humble-ros2-controllers
RUN apt install -y ros-humble-ros-gz ros-humble-gz-ros2-control
RUN apt install -y ros-humble-rtabmap ros-humble-rtabmap-ros

COPY ./src/quac-interfaces /quac/src/quac-interfaces
RUN . /opt/ros/humble/setup.bash && colcon build
