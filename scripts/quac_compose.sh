cmd=(env)

dwheels=false
([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels ]) && dwheels=true

darm=false
([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/servos ]) && darm=true

dlidar=false
([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar ]) && dlidar=true

dslam=false
([[ " $@ " =~ " -dslam " ]] || [[ $dwheels == true ]] || [[ $dlidar == true ]]) && dslam=true

[[ $dwheels == true ]] \
  && cmd+=(WHEELS_DEVICE=/dev/null DISABLE_WHEELS=true) \
  || cmd+=(WHEELS_DEVICE=/dev/quac/wheels)

[[ $darm == true ]] \
  && cmd+=(ARM_DEVICE=/dev/null DISABLE_ARM=true) \
  || cmd+=(ARM_DEVICE=/dev/quac/servos)

[[ " $@ " =~ " -dtcam"    ]] \
  && cmd+=(DISABLE_THERMAL_CAM=true)

[[ " $@ " =~ " -dmag "    ]] \
  && cmd+=(DISABLE_MAGNETOMETER=true)

[[ " $@ " =~ " -ekf " || " $@ " =~ " -dodomtf " ]] \
  && cmd+=(CONTROL_ODOM_TF=false)

cmd+=(docker compose )

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose.yaml -f docker-compose.remote.yaml)

cmd+=(up rsp cmd_vel_mux control inverse_kinematics)

[[ $dlidar == false ]] \
  && cmd+=(lidar)

[[ ! " $@ " =~ " -dsen "  ]] \
  && cmd+=(sensors)

[[ " $@ " =~ " -ekf "    ]] \
  && cmd+=(ekf)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dcsg " ]]) \
  && cmd+=(cam_streamer_gripper)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dcsb " ]]) \
  && cmd+=(cam_streamer_back)

([[ ! " $@ " =~ " -dvid " ]] && [[ " $@ " =~ " -hazmat " ]]) \
  && cmd+=(hazmat_detection_server)

([[ ! " $@ " =~ " -dvid " ]] && [[ " $@ " =~ " -paint " ]]) \
  && cmd+=(paintroller_detection_server)

([[ ! " $@ " =~ " -dvid " ]] && [[ " $@ " =~ " -qrcode " ]]) \
  && cmd+=(qrcode_detection_server)

([[ ! " $@ " =~ " -dvid " ]] && [[ " $@ " =~ " -landolt " ]]) \
&& cmd+=(landolt_c_detection_server)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -dnav " ]]) \
  && cmd+=(nav2)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -toolbox " ]]) \
  && cmd+=(ohm_slam)

([[ $dslam == false ]] && [[ " $@ " =~ " -toolbox " ]]) \
  && cmd+=(slam_toolbox)

echo "${cmd[@]}"
"${cmd[@]}"

docker compose down