docker build -t quac_pilot:dev -f docker/quac_pilot/dockerfile .

xhost +local:docker

cmd=(
  docker run
    -it --rm --network host
    -e DISPLAY=$DISPLAY
    -e QT_X11_NO_MITSHM=1
    -e ROS_DOMAIN_ID=187
    -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
    --volume /tmp/.X11-unix:/tmp/.X11-unix
    --device /dev/dri:/dev/dri
)

[[ " $@ " =~ " -sim "  ]] \
  && cmd+=(-e SIM_MODE="true")

[[ " $@ " =~ " -dnav " ]] \
  && cmd+=(-e DISABLE_NAV="true")

[[ " $@ " =~ " -ohm "    ]] \
  && cmd+=(-e OHM_SLAM="true")

[[ " $@ " =~ " -dslam "    ]] \
  && cmd+=(-e DISABLE_SLAM="true")

cmd+=(quac_pilot:dev)

"${cmd[@]}"
