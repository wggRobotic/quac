export ROS_DOMAIN_ID=187
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

source /opt/ros/humble/setup.bash
colcon build --packages-select quac
source install/setup.bash