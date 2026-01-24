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

COPY . /ros2_ws/src/quac

WORKDIR /ros2_ws
SHELL ["/bin/bash", "-c"]

RUN . /opt/ros/jazzy/setup.bash && colcon build --symlink-install

CMD ["/bin/bash", "-c", "source /opt/ros/jazzy/setup.bash && source /ros2_ws/install/setup.bash && ros2 launch quac quac.launch.py"]