# For more information, go to the [wiki](https://github.com/wggRobotic/Team-Docs/wiki/4.-Quac-%E2%80%93-All-Terrain-Crawler)

# External code used
https://github.com/htchr/waveshare_servos

https://github.com/Geekgineer/YOLOs-CPP-TensorRT

https://github.com/dlbeer/quirc.git

# Quac - Software

## System Setup

### Flashing OS and Jetpack
Use the [NVIDIA SDK Manager](https://developer.nvidia.com/sdk-manager) to flash the latest version of linux for tegra and jetpack6 for the jetson orin nano dev kit onto the jetson.

### uart patch
apply the patch from https://github.com/jetsonhacks/jetson-orin-uart

### udev rules
```
sudo nano /etc/udev/rules.d/99-quac.rules
```
add the following
```
SUBSYSTEM=="tty", KERNEL=="ttyTHS1", SYMLINK+="quac/ddsm115", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d3", ATTRS{serial}=="58FA095462", SYMLINK+="quac/servos", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="0e16a7441b82da48a5daed6b8d07ffde", SYMLINK+="quac/lidar", MODE="0666"
```

```
# old
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A50285BI", SYMLINK+="quac/ddsm115", MODE="0666"
```
reload the rules
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

# Docker
For gui docker apps, add this to ~/.bashrc
```
xhost +local:docker
```