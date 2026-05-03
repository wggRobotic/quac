FROM osrf/ros:humble-desktop-full
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

# quac

WORKDIR /quac

RUN git clone https://github.com/realsenseai/realsense-ros.git src/realsense_ros
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select realsense2_camera_msgs realsense2_description

COPY ./src/quac-interfaces /quac/src/quac-interfaces
COPY ./src/quac-rviz-plugins /quac/src/quac-rviz-plugins
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select quac-interfaces quac-rviz-plugins