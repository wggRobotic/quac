FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-xacro

RUN git clone https://github.com/realsenseai/realsense-ros.git src/realsense_ros
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select realsense2_camera_msgs realsense2_description