#!/bin/bash

sudo docker build -t quac:dev .

cmd=(
  sudo docker run
  -it
  --rm
  --privilegedz
  --network host
)

  
  -e DISABLE_LIDAR="$DISABLE_LIDAR"
  -e DISABLE_ARM="$DISABLE_ARM"

([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar_uart ]) \
  && cmd+=(-e DISABLE_WHEELS="true") \
  || cmd+=(--device=/dev/quac/wheels_rs485:/dev/quac/lidar_uart)

([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels_rs485 ]) \
  && cmd+=(-e DISABLE_LIDAR="true") \
  || cmd+=(--device=/dev/quac/lidar_uart:/dev/quac/lidar_uart)

([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/arm_servos ]) \
  && cmd+=(-e DISABLE_ARM="true") \
  || cmd+=(--device=/dev/quac/arm_servos:/dev/quac/arm_servos)

cmd+=(quac:dev)

"${cmd[@]}"