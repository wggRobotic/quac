xhost +local:docker

cmd=(env)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(SIM_MODE=true)

cmd+=(docker compose up rviz)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(gazebo cmd_vel_mux rsp inverse_kinematics)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dnav " ]] && [[ ! " $@ " =~ " -dslam " ]]) \
  && cmd+=(nav2)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dslam " ]] && [[ ! " $@ " =~ " -rtab " ]] && [[ ! " $@ " =~ " -ohm " ]]) \
  && cmd+=(slam_toolbox)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dslam " ]] && [[ " $@ " =~ " -ohm " ]]) \
  && cmd+=(ohm_slam)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dslam " ]] && [[ " $@ " =~ " -rtab " ]] && [[ ! " $@ " =~ " -ohm " ]] ) \
  && cmd+=(rtabmap)

echo "${cmd[@]}"
"${cmd[@]}"

docker compose down