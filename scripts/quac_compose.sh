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

[[ " $@ " =~ " -dimu "    ]] \
  && cmd+=(DISABLE_IMU=true)

[[ " $@ " =~ " -dmag "    ]] \
  && cmd+=(DISABLE_MAGNETOMETER=true)

[[ " $@ " =~ " -dekf "    ]] \
  && cmd+=(DISABLE_EKF=true)

cmd+=(docker compose )

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose.yaml -f docker-compose.remote.yaml)

cmd+=(up rsp cmd_vel_mux control inverse_kinematics)

[[ $dlidar == false ]] \
  && cmd+=(lidar)

[[ ! " $@ " =~ " -dsen "  ]] \
  && cmd+=(sensors)

[[ ! " $@ " =~ " -dekf "    ]] \
  && cmd+=(ekf)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -drsf " ]]) \
  && cmd+=(realsense_streamer_front)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -drsb " ]]) \
  && cmd+=(realsense_streamer_back)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dhazmat " ]]) \
  && cmd+=(hazmat_detection_server)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dpaint " ]]) \
  && cmd+=(paintroller_detection_server)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dqrcode " ]]) \
  && cmd+=(qrcode_detection_server)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -dnav " ]]) \
  && cmd+=(nav2)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -ohm " ]]) \
  && cmd+=(slam_toolbox)

([[ $dslam == false ]] && [[ " $@ " =~ " -ohm " ]]) \
  && cmd+=(ohm_slam)

echo "${cmd[@]}"
"${cmd[@]}"

docker compose down