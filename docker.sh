#!/bin/bash

sudo docker build -t quac:dev .
sudo docker run -e DISABLE_ARM=true -e DISABLE_LIDAR=true -it --rm --network host --privileged quac:dev