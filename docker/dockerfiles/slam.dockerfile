FROM ros:humble

SHELL ["/bin/bash", "-c"]

RUN apt update && apt install git

WORKDIR /ros2_ws/src

RUN git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git
RUN git clone https://github.com/autonohm/ohm_tsd_slam.git

WORKDIR /ros2_ws
RUN rosdep init && rosdep update
RUN rosdep install --from-paths src --ignore-src -y
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

RUN colcon build

WORKDIR /quac
COPY ./docker/quac_compose/control.entrypoint.sh entrypoint.sh