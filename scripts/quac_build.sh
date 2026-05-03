cmd=(docker compose)

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose.yaml -f docker-compose.remote.yaml)

cmd+=(
build \
realsense_streamer_front \
realsense_streamer_back \
qrcode_detection_server \
hazmat_detection_server \
paintroller_detection_server \
control \
arm_target_controller \
inverse_kinematics \
rsp \
twist_mux \
lidar \
sensors \
nav2 \
slam_toolbox \
ohm_slam
)

"${cmd[@]}"