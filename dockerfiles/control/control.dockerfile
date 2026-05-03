FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-ros2-control ros-humble-ros2-controllers

COPY ./src/quac-hardware src/quac-hardware
RUN . /opt/ros/humble/setup.bash && colcon build

COPY ./src/quac-controllers src/quac-controllers
RUN . /opt/ros/humble/setup.bash && colcon build