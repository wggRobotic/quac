cmd=(env)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(SIM_MODE=true)

cmd+=(docker compose -f docker-compose.yaml)

[[ " $@ " =~ " -remote "    ]] \
  && cmd+=(-f docker-compose-files.remote.yaml) \
  || cmd+=(-f docker-compose-files.local.yaml)

cmd+=(up rviz)

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