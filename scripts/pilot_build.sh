cmd=(docker compose build \
inverse_kinematics \
rsp \
cmd_vel_mux \
nav2 \
slam_toolbox \
ohm_slam \
rtabmap \
gazebo \
rviz
)

"${cmd[@]}"