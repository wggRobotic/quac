FROM ros:humble
SHELL ["/bin/bash", "-c"]

WORKDIR /ros2_ws
RUN apt update

RUN git clone https://github.com/introlab/rtabmap.git src/rtabmap
RUN git clone --branch ros2 https://github.com/introlab/rtabmap_ros.git src/rtabmap_ros
RUN rosdep update && rosdep install --from-paths src --ignore-src -r -y
RUN export MAKEFLAGS="-j2" && colcon build --symlink-install --cmake-args -DRTABMAP_SYNC_MULTI_RGBD=ON -DRTABMAP_SYNC_USER_DATA=ON -DCMAKE_BUILD_TYPE=Release

WORKDIR /quac

RUN apt install -y ros-humble-rmw-cyclonedds-cpp

ENV RCUTILS_LOGGING_USE_STDOUT=1
ENV RCUTILS_LOGGING_BUFFERED_STREAM=1