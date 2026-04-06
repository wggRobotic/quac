# For more information, go to the [wiki](https://github.com/wggRobotic/Team-Docs/wiki/4.-Quac-%E2%80%93-All-Terrain-Crawler)

# External code used
https://github.com/htchr/waveshare_servos

https://github.com/Geekgineer/YOLOs-CPP-TensorRT

https://github.com/dlbeer/quirc.git

# On the robot
```
sudo apt update
sudo apt install ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-xacro
git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git
```

Set paths:
```
export NVINVER_YOLO_LIB_PATH=/home/pilot/DeepStream-Yolo/nvdsinfer_custom_impl_Yolo/libnvdsinfer_custom_impl_Yolo.so
```

## udev rules
```
sudo nano /etc/udev/rules.d/99-quac.rules
```
add the following
```
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A50285BI", SYMLINK+="quac/wheels_rs485", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d3", ATTRS{serial}=="58FA095462", SYMLINK+="quac/arm_servos", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="0e16a7441b82da48a5daed6b8d07ffde", SYMLINK+="quac/lidar_uart", MODE="0666"
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
sudo apt install ros-humble-twist-mux
sudo apt install ros-humble-ros-gz ros-humble-gz-ros2-control 
sudo apt install ros-humble-slam-toolbox
sudo apt install ros-humble-navigation2 ros-humble-nav2-bringup
```

# Quac_vide

convert to onnx to tensorrt

```
python3 export_yolo26.py -w hazmat_yolo26.pt --opset 18
/usr/src/tensorrt/bin/trtexec --onnx=hazmat_yolo26.onnx --saveEngine=hazmat_yolo26.engine --fp16 --verbose
```