#!/bin/bash

sudo docker build -t quac:dev .
sudo docker run -e DISABLE_WHEELS=$DISABLE_WHEELS -e DISABLE_LIDAR=$DISABLE_LIDAR -e DISABLE_ARM=$DISABLE_ARM -it --rm --network host --device=/dev/quac/wheels_rs485:/dev/quac/wheels_rs485 quac:dev