# For more information, go to the [wiki](https://github.com/wggRobotic/Team-Docs/wiki/Quac-%E2%80%93-All-Terrain-Crawler)

# External code used in this repo
https://github.com/htchr/waveshare_servos

# On the robot
```
sudo apt update
sudo apt install ros-jazzy-ros2-control ros-jazzy-ros2-controlllers ros-jazzy-xacro
git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git
```

## udev rules
```
sudo nano /etc/udev/rules.d/99-quac.rules
```
add the following
```
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A50285BI", SYMLINK+="quac/wheels_rs485", MODE="0666"
```
reload the rules
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Docker
```
sudo chmod +x docker.sh
./docker.sh
```

# On the pilot's machine
```
sudo apt update
sudo apt install ros-jazzy-twist-mux
sudo apt install ros-jazzy-ros-gz ros-jazzy-gz-ros2-control 
sudo apt install ros-jazzy-slam-toolbox
sudo apt install ros-jazzy-navigation2 ros-jazzy-nav2-bringup
```