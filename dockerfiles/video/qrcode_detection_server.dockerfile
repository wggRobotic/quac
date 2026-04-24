FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt update
RUN apt install libopencv-dev build-essential -y

# ros2 package dependencies

RUN apt install -y ros-humble-rmw-cyclonedds-cpp

# quac

WORKDIR /quac

COPY ./src/quac_interfaces /quac/src/quac_interfaces
RUN . /opt/ros/humble/setup.bash && colcon build
COPY ./src/quac_video /quac/src/quac_video
RUN . /opt/ros/humble/setup.bash && . /quac/install/setup.bash && colcon build --packages-select quac_video --cmake-args -DBUILD_QRCODE_DETECTION_SERVER=ON