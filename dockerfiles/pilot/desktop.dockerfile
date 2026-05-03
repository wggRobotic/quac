FROM osrf/ros:humble-desktop-full
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-twist-mux
RUN apt install -y ros-humble-ros-gz ros-humble-gz-ros2-control
RUN apt install -y ros-humble-slam-toolbox
RUN apt install -y ros-humble-navigation2 ros-humble-nav2-bringup
RUN apt install -y ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-xacro

RUN apt install -y libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-good1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    libgstrtspserver-1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-good1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools

COPY ./src/quac-interfaces /quac/src/quac-interfaces
RUN . /opt/ros/humble/setup.bash && colcon build
