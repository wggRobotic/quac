#!/bin/bash

#docker build -t quac:dev -f docker/quac/dockerfile .

cmd=(
  docker run
  -it
  --rm
  --network host
  --runtime nvidia
  --device /dev/i2c-0
  --privileged
  -e NVIDIA_VISIBLE_DEVICES=all
  -e ROS_DOMAIN_ID=187
  -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
  --ipc=host
  -v quac_nvinfer:/quac/.nvinfer
)

([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar_uart ]) \
  && cmd+=(-e DISABLE_WHEELS="true") \
  || cmd+=(--device=/dev/quac/lidar_uart:/dev/quac/lidar_uart)

([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels_rs485 ]) \
  && cmd+=(-e DISABLE_LIDAR="true") \
  || cmd+=(--device=/dev/quac/wheels_rs485:/dev/quac/wheels_rs485)

([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/arm_servos ]) \
  && cmd+=(-e DISABLE_ARM="true") \
  || cmd+=(--device=/dev/quac/arm_servos:/dev/quac/arm_servos)

[[ " $@ " =~ " -dvid " ]] \
  && cmd+=(-e DISABLE_VIDEO="true")

[[ " $@ " =~ " -rhm " ]] \
  && cmd+=(-e REGENERATE_HAZMAT="true")

[[ " $@ " =~ " -rpr " ]] \
  && cmd+=(-e REGENERATE_PAINTROLLER="true")

[[ " $@ " =~ " -dnav " ]] \
  && cmd+=(-e DISABLE_NAV="true")

[[ " $@ " =~ " -ohm "    ]] \
  && cmd+=(-e OHM_SLAM="true")

[[ " $@ " =~ " -dslam "    ]] \
  && cmd+=(-e DISABLE_SLAM="true")

[[ " $@ " =~ " -dsen "    ]] \
  && cmd+=(-e DISABLE_SENSORS="true")

[[ " $@ " =~ " -dtmc"    ]] \
  && cmd+=(-e DISABLE_THERMAL_CAM="true")

[[ " $@ " =~ " -dimu "    ]] \
  && cmd+=(-e DISABLE_IMU="true")

[[ " $@ " =~ " -dmag "    ]] \
  && cmd+=(-e DISABLE_MAGNETOMETER="true")

[[ " $@ " =~ " -dent "    ]] \
  || cmd+=(--entrypoint "./entrypoint.sh")

cmd+=(quac:dev)

"${cmd[@]}"