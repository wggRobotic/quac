cmd=(docker compose)

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose.yaml -f docker-compose.remote.yaml)

cmd+=(
build \
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