FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

RUN git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git src/rplidar_ros
RUN . /opt/ros/humble/setup.bash && colcon build