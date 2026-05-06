xhost +local:docker

cmd=(env)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(SIM_MODE=true)

cmd+=(docker compose up rviz)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(gazebo twist_mux rsp inverse_kinematics)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dnav " ]] && [[ ! " $@ " =~ " -dslam " ]]) \
  && cmd+=(nav2)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dslam " ]] && [[ ! " $@ " =~ " -ohm " ]]) \
  && cmd+=(slam_toolbox)

([[ " $@ " =~ " -sim "  ]] && [[ ! " $@ " =~ " -dslam " ]] && [[ " $@ " =~ " -ohm " ]]) \
  && cmd+=(ohm_slam)

echo "${cmd[@]}"
"${cmd[@]}"

docker compose down