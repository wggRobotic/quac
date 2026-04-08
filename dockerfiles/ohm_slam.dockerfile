FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y git libgsl-dev libann-dev libflann-dev libboost-thread-dev liblz4-dev
RUN apt install -y ros-humble-opennav-docking

RUN git clone https://github.com/autonohm/ohm_tsd_slam.git src/ohm_tsd_slam
RUN . /opt/ros/humble/setup.bash && colcon build