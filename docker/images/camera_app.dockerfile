FROM ultralytics/ultralytics:latest-jetson-jetpack6

SHELL ["/bin/bash", "-c"]

# librealsense

WORKDIR /workspace
RUN apt update && apt-get install -y \
    build-essential cmake git pkg-config \
    libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libxi-dev libudev-dev libv4l-dev \
    libusb-1.0-0-dev libssl-dev freeglut3-dev mesa-utils mesa-common-dev

RUN git clone https://github.com/realsenseai/librealsense.git
WORKDIR /workspace/librealsense/build
RUN cmake ..
RUN cmake --build . --config Release
RUN cmake --install .

# install ros2 humble

RUN apt update && apt install locales
RUN locale-gen en_US en_US.UTF-8
RUN update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
ENV LANG=en_US.UTF-8

RUN apt install -y software-properties-common
RUN add-apt-repository universe

RUN apt update && apt install curl -y
RUN export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F'"' '{print $4}') && \
    curl -L -o /tmp/ros2-apt-source.deb "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb" && \
    dpkg -i /tmp/ros2-apt-source.deb

RUN apt update
RUN apt upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt install -y tzdata && \
    ln -fs /usr/share/zoneinfo/Europe/Berlin /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata
RUN apt install -y ros-humble-ros-base
RUN apt install -y ros-dev-tools

# install some libs

RUN apt install -y unzip git
RUN apt install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-good1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    libgstrtspserver-1.0-dev \
    libyaml-cpp-dev \
    libssl-dev \
    wget \
    build-essential \
    pkg-config

# ros2 package dependencies

RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-image-transport ros-humble-image-transport-plugins

# quac

WORKDIR /quac

COPY ./src/quac_video /quac/src/quac_video
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select quac_video
