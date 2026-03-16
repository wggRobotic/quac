#!/bin/bash

ENV_OVERRIDE=$(mktemp)
echo "services:" >> "$ENV_OVERRIDE"
echo "  quac:" >> "$ENV_OVERRIDE"
echo "    environment:" >> "$ENV_OVERRIDE"

DEVICE_OVERRIDE=$(mktemp)
echo "services:" >> $DEVICE_OVERRIDE
echo "  quac:" >> $DEVICE_OVERRIDE
echo "    devices:" >> $DEVICE_OVERRIDE

([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar_uart ]) \
  && echo "      DISABLE_LIDAR: true"  >> $ENV_OVERRIDE \
  || echo "      - /dev/quac/lidar_uart:/dev/quac/lidar_uart"     >> $DEVICE_OVERRIDE

([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels_rs485 ]) \
  && echo "      DISABLE_WHEELS: true" >> $ENV_OVERRIDE \
  || echo "      - /dev/quac/wheels_rs485:/dev/quac/wheels_rs485" >> $DEVICE_OVERRIDE

([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/arm_servos ]) \
  && echo "      DISABLE_ARM: true"    >> $ENV_OVERRIDE \
  || echo "      - /dev/quac/arm_servos:/dev/quac/arm_servos"     >> $DEVICE_OVERRIDE

docker compose build quac
docker compose -f docker-compose.yml -f "$ENV_OVERRIDE" -f "$DEVICE_OVERRIDE" up quac
