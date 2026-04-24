# Bringup

## Docker

To start the docker containers, in the root of this repo, run
```
. scripts/quac_compose.sh
```
The script accepts the following arguments:
`-remote`: pulls the prebuilt images from the github container registry. Before you can do this for the first time, you need to do
```
docker login --username <github-benutzername> --password <personal-access-token> ghcr.io
```
`-dnav`: disables the nav2 stack
`-dslam`: disables any slam system + effects of disable_nav
`-ohm`: uses ohm_tsd_slam instead of slam_toolbox
`-dlidar`: disables the lidar + effects of disable_slam
`-dwheels`: replaces the wheels hardware interface with a mock version
`-darm`: replaces the arms hardware interface with a mock version
`-dvid`: disable all nodes related to video streaming and processing
`-dsen`: disables all the sensors connected via I2C
`-dmag`: disables the magnometer
`-dtcam`: disables the thermal cam
`-dimu`: disables the imu


## Ros2 native

If this is the first setup. you need to install some dependencies.

[ros2](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html)

```
sudo apt update && sudo apt upgrade && sudo apt install -y \
  build-essential cmake git pkg-config \
  libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libudev-dev libv4l-dev \
  libusb-1.0-0-dev libssl-dev freeglut3-dev mesa-utils mesa-common-dev 
```
[librealsense](https://github.com/realsenseai/librealsense)

```
sudo apt install -y \
  unzip git \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  libgstreamer-plugins-good1.0-dev \
  libgstreamer-plugins-bad1.0-dev \
  libgstrtspserver-1.0-dev \
  libyaml-cpp-dev \
  libssl-dev \
  wget \
  build-essential \
  pkg-config \
  python3-pip \
  libann-dev libflann-dev libboost-thread-dev liblz4-dev
```

```
sudo apt install -y \
  ros-humble-rmw-cyclonedds-cpp \
  ros-humble-image-transport ros-humble-image-transport-plugins \
  ros-humble-ros2-control ros-humble-ros2-controllers \
  ros-humble-navigation2 ros-humble-nav2-bringup \
  ros-humble-opennav-docking \
  ros-humble-xacro \
  ros-humble-slam-toolbox ros-humble-twist-mux
```

```
pip3 install \
  adafruit-circuitpython-tlv493d \
  adafruit-circuitpython-lsm6ds \
  adafruit-circuitpython-mlx90640 \
  'numpy<2' Jetson.GPIO
```

```
mkdir ros2_ws && cd ros2_ws
git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git src/rplidar_ros
git clone https://github.com/autonohm/ohm_tsd_slam.git src/ohm_tsd_slam
colcon build
```


Add these lines to `~/.bashrc`:
```
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
source ~/quac/install/setup.bash
export ROS_DOMAIN_ID 187
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
```

In the root of this repo, run
```
git submodule init
git submodule update
colcon build
```
Then you can run the launch script
```
ros2 launch quac quac.launch.py
```
you can additionally specify arguments like `<arg>:=<val>` to include / exclude certain features. All arguments are set to`false` by default \
`disable_nav`: disables the nav2 stack
`disable_slam`: disables any slam system + effects of disable_nav
`ohm_slam`: uses ohm_tsd_slam instead of slam_toolbox
`disable_lidar`: disables the lidar + effects of disable_slam
`disable_wheels`: replaces the wheels hardware interface with a mock version
`disable_arm`: replaces the arms hardware interface with a mock version
`disable_video`: disable all nodes related to video streaming and processing
`disable_sensors`: disables all the sensors connected via I2C
`disable_magnometer`: disables the magnometer
`disable_thermal_cam`: disables the thermal cam
`disable_imu`: disables the imu