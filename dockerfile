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

ENV DISABLE_WHEELS=false
ENV DISABLE_LIDAR=false
ENV DISABLE_ARM=false
CMD echo DISABLE_WHEELS=$DISABLE_WHEELS && \
    echo DISABLE_LIDAR=$DISABLE_LIDAR && \
    echo DISABLE_ARM=$DISABLE_ARM && \
    source /opt/ros/jazzy/setup.bash && \
    source /ros2_ws/install/setup.bash && \
    ros2 launch quac quac.launch.py disable_wheels:=${DISABLE_WHEELS} disable_lidar:=${DISABLE_LIDAR} disable_arm:=${DISABLE_ARM}