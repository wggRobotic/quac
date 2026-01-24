# EXTERNAL CODE USED IN THIS REPO
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
docker build -t quac:latest .
docker build -t quac_test:latest .
```

## Run
```
docker run -it --rm --network host --privileged quac:latest
docker run -it --rm --network host --privileged quac_test:latest
```