#!/bin/bash

ENV_OVERRIDE=$(mktemp)
echo "services:" >> $ENV_OVERRIDE
echo "  quac:" >> $ENV_OVERRIDE
echo "    environment:" >> $ENV_OVERRIDE

DEVICE_OVERRIDE=$(mktemp)
echo "services:" >> $DEVICE_OVERRIDE
echo "  quac:" >> $DEVICE_OVERRIDE
echo "    devices:" >> $DEVICE_OVERRIDE

cmd=(
  docker compose up -f docker/docker-compose.yaml
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