FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update

# ros2 package dependencies

RUN apt install -y ros-humble-rmw-cyclonedds-cpp

# quac

WORKDIR /quac

COPY ./src/quac_interfaces /quac/src/quac_interfaces
COPY ./src/quac_video /quac/src/quac_video
RUN . /opt/ros/humble/setup.bash && colcon build --cmake-args -DBUILD_QRCODE_DETECTION_SERVER=ON