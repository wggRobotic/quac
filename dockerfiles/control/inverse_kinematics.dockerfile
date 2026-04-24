FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

COPY ./src/quac_control src/quac_control
RUN . /opt/ros/humble/setup.bash && colcon build --cmake-args -DBUILD_INVERSE_KINEMATICS=ON