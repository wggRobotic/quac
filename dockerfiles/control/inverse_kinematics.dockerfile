FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

COPY ./src/quac-inverse-kinematics src/quac-inverse-kinematics
RUN . /opt/ros/humble/setup.bash && colcon build