#!/bin/bash

echo DISABLE_WHEELS=$DISABLE_WHEELS
echo DISABLE_LIDAR=$DISABLE_LIDAR
echo DISABLE_ARM=$DISABLE_ARM
source /opt/ros/jazzy/setup.bash
source /ros2_ws/install/setup.bash

exec ros2 launch quac quac.launch.py disable_wheels:=${DISABLE_WHEELS} disable_lidar:=${DISABLE_LIDAR} disable_arm:=${DISABLE_ARM}