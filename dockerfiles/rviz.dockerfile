FROM osrf/ros:humble-desktop-full
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp

# quac

WORKDIR /quac

COPY ./src/quac_interfaces /quac/src/quac_interfaces
COPY ./src/quac_rviz_plugins /quac/src/quac_rviz_plugins
RUN . /opt/ros/humble/setup.bash && colcon build