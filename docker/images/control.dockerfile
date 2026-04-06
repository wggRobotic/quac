FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-ros2-control ros-humble-ros2-controllers

COPY ./src/quac_hardware src/quac_hardware
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select quac_hardware

COPY ./src/quac_control src/quac_control
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select quac_control