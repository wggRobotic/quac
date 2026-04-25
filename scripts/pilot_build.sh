cmd=(docker compose build \
inverse_kinematics \
rsp \
twist_mux \
nav2 \
slam_toolbox \
ohm_slam \
gazebo \
rviz
)

"${cmd[@]}"