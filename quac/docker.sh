#!/bin/bash

docker build -t quac:dev .

cmd=(
  docker run
  -it
  --rm
  --network host
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

cmd+=(quac:dev)

"${cmd[@]}"