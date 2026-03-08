#!/bin/bash

DISABLE_WHEELS=${DISABLE_WHEELS:-false}
DISABLE_ARM=${DISABLE_ARM:-false}
DISABLE_LIDAR=${DISABLE_LIDAR:-false}

sudo docker build -t quac:dev .

cmd=(
  sudo docker run
  -e DISABLE_WHEELS="$DISABLE_WHEELS"
  -e DISABLE_LIDAR="$DISABLE_LIDAR"
  -e DISABLE_ARM="$DISABLE_ARM"
  -it
  --rm
  --network host
)

[ "$DISABLE_WHEELS" != "true" ] && cmd+=(--device=/dev/quac/wheels_rs485:/dev/quac/wheels_rs485)
[ "$DISABLE_ARM" != "true" ] && cmd+=(--device=/dev/quac/arm_servos:/dev/quac/arm_servos)

cmd+=(quac:dev)

"${cmd[@]}"