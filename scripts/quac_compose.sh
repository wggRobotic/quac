cmd=(env)

dwheels=false
([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels_rs485 ]) && dwheels=true

darm=false
([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/arm_servos ]) && darm=true

dlidar=false
([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar_uart ]) && dlidar=true

dslam=false
([[ " $@ " =~ " -dslam " ]] || [[ $dwheels == true ]] || [[ $dlidar == true ]]) dslam=true

[[ $dwheels == true ]] \
  && cmd+=(WHEELS_DEVICE=/dev/null DISABLE_WHEELS=true) \
  || cmd+=(WHEELS_DEVICE=)

[[ $darm == true ]] \
  && cmd+=(ARM_DEVICE=/dev/null DISABLE_ARM=true) \
  || cmd+=(ARM_DEVICE=/dev/quac/arm_servos)

[[ " $@ " =~ " -dtcam"    ]] \
  && cmd+=(DISABLE_THERMAL_CAM=true)

[[ " $@ " =~ " -dimu "    ]] \
  && cmd+=(DISABLE_IMU=true)

[[ " $@ " =~ " -dmag "    ]] \
  && cmd+=(DISABLE_MAGNETOMETER=true)

cmd+=(docker compose -f docker-compose.yaml)

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose-files.remote.yaml) \
  || cmd+=(-f docker-compose-files.local.yaml)

cmd+=(up rsp twist_mux control inverse_kinematics)

[[ $dlidar == false ]] \
  && cmd+=(lidar)

[[ ! " $@ " =~ " -dsen "  ]] \
  && cmd+=(sensors)

([[ ! " $@ " =~ " -dvid " ]] && [[ ! " $@ " =~ " -dcamf " ]]) \
  && cmd+=(camera_front)

([[ ! " $@ " =~ " -dvid " ]] || [[ ! " $@ " =~ " -dcamb " ]]) \
  && cmd+=(camera_back)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -dnav " ]]) \
  && cmd+=(nav2)

([[ $dslam == false ]] && [[ ! " $@ " =~ " -ohm " ]]) \
  && cmd+=(slam_toolbox)

([[ $dslam == false ]] && [[ " $@ " =~ " -ohm " ]]) \
  && cmd+=(ohm_slam)

echo "${cmd[@]}"
"${cmd[@]}"