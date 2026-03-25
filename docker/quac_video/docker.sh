#!/bin/bash

sudo docker build -t quac_video:dev .

docker run -it \
  --name quac_video \
  --network host \
  --runtime nvidia \
  --ipc=host \
  --init \
  --privileged \
  -e NVIDIA_VISIBLE_DEVICES=all \
  -v quac_video_trt_engines:/workspace/ros2_ws/engines \
  quac_video:dev