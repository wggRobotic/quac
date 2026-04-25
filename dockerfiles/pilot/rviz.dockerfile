FROM osrf/ros:humble-desktop-full
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

# quac

WORKDIR /quac

RUN git clone https://github.com/realsenseai/realsense-ros.git src/realsense_ros
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select realsense2_camera_msgs realsense2_description

COPY ./src/quac_interfaces /quac/src/quac_interfaces
COPY ./src/quac_rviz_plugins /quac/src/quac_rviz_plugins
RUN . /opt/ros/humble/setup.bash && colcon build