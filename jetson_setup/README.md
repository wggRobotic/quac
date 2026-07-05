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
```
sudo groupadd docker
sudo usermod -aG docker $USER
newgrp docker
```

```
docker login ghcr.io --username <github-benutzername> --password <personal-access-token>
```