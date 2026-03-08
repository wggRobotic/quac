FROM ros:jazzy

RUN sudo apt update && apt install git

RUN sudo apt install -y \
    ros-jazzy-ros2-control \
    ros-jazzy-ros2-controllers \
    ros-jazzy-xacro \
    ros-jazzy-rmw-cyclonedds-cpp

RUN rm -rf /var/lib/apt/lists/*

WORKDIR /ros2_ws/src

RUN git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git

WORKDIR /ros2_ws
SHELL ["/bin/bash", "-c"]

RUN . /opt/ros/jazzy/setup.bash && colcon build --packages-select rplidar_ros

COPY ./quac_control /ros2_ws/src/quac/quac_control
RUN . /opt/ros/jazzy/setup.bash && colcon build --packages-select quac_control

COPY ./quac /ros2_ws/src/quac/quac
RUN . /opt/ros/jazzy/setup.bash && colcon build --packages-select quac

COPY . /ros2_ws/src/quac

RUN chmod +x /ros2_ws/src/quac/run.sh

ENV DISABLE_LIDAR=false
ENV DISABLE_WHEELS=false
ENV DISABLE_ARM=false
CMD ["/ros2_ws/src/quac/run.sh"]