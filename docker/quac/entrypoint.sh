#!/bin/bash

echo DISABLE_WHEELS=$DISABLE_WHEELS
echo DISABLE_LIDAR=$DISABLE_LIDAR
echo DISABLE_ARM=$DISABLE_ARM
echo DISABLE_VIDEO=$DISABLE_VIDEO
echo REGENERATE_HAZMAT=$REGENERATE_HAZMAT
echo REGENERATE_PAINTROLLER=$REGENERATE_PAINTROLLER
echo DISABLE_NAV=$DISABLE_NAV
echo OHM_SLAM=$OHM_SLAM
echo DISABLE_SLAM=$DISABLE_SLAM

source /opt/ros/jazzy/setup.bash
source /ros2_ws/install/setup.bash
source /quac/install/setup.bash

export NVINVER_YOLO_LIB_PATH=/DeepStream-Yolo/nvdsinfer_custom_impl_Yolo/libnvdsinfer_custom_impl_Yolo.so

exec ros2 launch quac quac.launch.py \
    disable_wheels:=${DISABLE_WHEELS} \
    disable_lidar:=${DISABLE_LIDAR} \
    disable_arm:=${DISABLE_ARM} \
    disable_video:=${DISABLE_VIDEO} \
    regenerate_hazmat:=${REGENERATE_HAZMAT} \
    regenerate_paintroller:=${REGENERATE_PAINTROLLER} \
    disable_nav:=${DISABLE_NAV} \
    ohm_slam:=${OHM_SLAM} \
    disable_slam:=${DISABLE_SLAM}