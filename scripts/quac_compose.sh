#docker compose -f docker/quac.docker-compose.yaml --profile all build
#docker compose -f docker/quac.docker-compose.yaml --profile hardware up camera_front_app

cmd=(env)

([[ " $@ " =~ " -dwheels " ]] || [ ! -e /dev/quac/wheels_rs485 ]) \
  && cmd+=(WHEELS_DEVICE=/dev/null DISABLE_WHEELS=true) \
  || cmd+=(WHEELS_DEVICE=)

([[ " $@ " =~ " -darm "    ]] || [ ! -e /dev/quac/arm_servos ]) \
  && cmd+=(ARM_DEVICE=/dev/null DISABLE_ARM=true) \
  || cmd+=(ARM_DEVICE=/dev/quac/arm_servos)

[[ " $@ " =~ " -dtc"    ]] \
  && cmd+=(DISABLE_THERMAL_CAM=true)

[[ " $@ " =~ " -dimu "    ]] \
  && cmd+=(DISABLE_IMU=true)

[[ " $@ " =~ " -dmag "    ]] \
  && cmd+=(DISABLE_MAGNETOMETER=true)

cmd+=(docker compose -f docker/quac.docker-compose.yaml)

([[ " $@ " =~ " -dlidar "  ]] || [ ! -e /dev/quac/lidar_uart ]) \
  || cmd+=(--profile lidar_profile)

[[ " $@ " =~ " -dsen "  ]] \
  || cmd+=(--profile sensors_profile)

([[ " $@ " =~ " -dvid " ]] || [[ " $@ " =~ " -dcamf " ]]) \
  || cmd+=(--profile camera_front_profile)

([[ " $@ " =~ " -dvid " ]] || [[ " $@ " =~ " -dcamb " ]]) \
  || cmd+=(--profile camera_back_profile)

[[ " $@ " =~ " -dnav " ]] \
  || cmd+=(--profile nav2_profile)

[[ " $@ " =~ " -dslam " ]] \
  || cmd+=(--profile slam_toolbox_profile)

cmd+=(up)

echo "${cmd[@]}"
"${cmd[@]}"