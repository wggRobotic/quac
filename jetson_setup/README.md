# System Setup

## Flashing OS and Jetpack
Use the [NVIDIA SDK Manager](https://developer.nvidia.com/sdk-manager) to flash the latest version of linux for tegra and jetpack6 for the jetson orin nano dev kit onto the jetson.

## Change Hostname
```
sudo hostnamectl set-hostname quac
```
```
sudo nano /etc/hosts
```
replace
```
127.0.1.1   ubuntu
```
with
```
127.0.1.1   quac
```
and reboot

## uart patch
apply the patch from https://github.com/jetsonhacks/jetson-orin-uart

## udev rules
```
sudo nano /etc/udev/rules.d/99-quac.rules
```
add the following
```
SUBSYSTEM=="tty", KERNEL=="ttyTHS1", SYMLINK+="quac/wheels", MODE="0666"
SUBSYSTEM=="tty", SUBSYSTEMS=="usb" ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d3", SYMLINK+="quac/servos", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="0e16a7441b82da48a5daed6b8d07ffde", SYMLINK+="quac/lidar", MODE="0666"
```
reload the rules
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## ros2 native

install [ros2 humble](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debs.html)

add this to ~/.bashrc for native usage
```
source /opt/ros/humble/setup.bash
source ~/quac/install/setup.bash

export ROS_DOMAIN_ID=187
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
```


## Docker
For gui docker apps, add this to ~/.bashrc
```
xhost +local:docker
```