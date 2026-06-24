cmd=(docker compose)

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose.yaml -f docker-compose.remote.yaml)

cmd+=(
build --pull=false \
cam_streamer_front
cam_streamer_gripper \
cam_streamer_back \
qrcode_detection_server \
landolt_c_detection_server \
hazmat_detection_server \
paintroller_detection_server \
control \
ekf \
arm_target_controller \
inverse_kinematics \
rsp \
cmd_vel_mux \
lidar \
sensors \
nav2 \
slam_toolbox \
ohm_slam
)

"${cmd[@]}"