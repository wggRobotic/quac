#!/bin/bash

source /opt/ros/humble/setup.bash
source /ros2_ws/install/setup.bash
source /quac/install/setup.bash

echo SIM_MODE=$SIM_MODE
echo DISABLE_NAV=$DISABLE_NAV
echo OHM_SLAM=$OHM_SLAM
echo DISABLE_SLAM=$DISABLE_SLAM
echo DISABLE_GUINIVERSE=$DISABLE_GUINIVERSE

exec ros2 launch quac quac_pilot.launch.py \
    sim_mode:=${SIM_MODE} \
    disable_nav:=${DISABLE_NAV} \
    ohm_slam:=${OHM_SLAM} \
    disable_slam:=${DISABLE_SLAM} \
    disable_guiniverse=${DISABLE_GUINIVERSE}