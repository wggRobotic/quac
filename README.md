# For more information, go to the [wiki](https://github.com/wggRobotic/Team-Docs/wiki/Quac-%E2%80%93-All-Terrain-Crawler)

# External code used in this repo
https://github.com/htchr/waveshare_servos

# Setup

## On the robot
```
sudo apt update
sudo apt install ros-jazzy-ros2-control ros-jazzy-ros2-controlllers ros-jazzy-xacro
git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git
```
## On the pilot's machine
```
sudo apt update
sudo apt install ros-jazzy-twist-mux
sudo apt install ros-jazzy-ros-gz ros-jazzy-gz-ros2-control 
sudo apt install ros-jazzy-slam-toolbox
sudo apt install ros-jazzy-navigation2 ros-jazzy-nav2-bringup
```

# Docker

## Build
```
docker build -t quac:stable .
docker build -t quac:dev .
```

## Run
```
docker run -it --rm --network host --privileged quac:stable
docker run -it --rm --network host --privileged quac:dev
```

To disable individual components, do
```
sudo docker run -e DISABLE_LIDAR=true -e DISABLE_ARM=true -e DISABLE_WHEELS=true -it --rm --network host --privileged quac:dev
```