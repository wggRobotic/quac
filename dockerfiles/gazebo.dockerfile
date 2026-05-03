FROM osrf/ros:humble-desktop-full
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-ros-gz ros-humble-gz-ros2-control
RUN apt install -y ros-humble-ros2-controllers ros-humble-xacro

WORKDIR /quac

COPY ./src/quac-controllers /quac/src/quac-controllers
RUN . /opt/ros/humble/setup.bash && colcon build