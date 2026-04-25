FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-slam-toolbox
RUN apt install -y ros-humble-navigation2 ros-humble-nav2-bringup
RUN apt install -y ros-humble-ros2-control ros-humble-ros2-controllers
