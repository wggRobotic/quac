xhost +local:docker

docker build -t quac_basic:dev -f docker/basic/dockerfile .

sudo docker run \
    -it --rm --network host \
    -e DISPLAY=$DISPLAY \
    -e QT_X11_NO_MITSHM=1 \
    -e ROS_DOMAIN_ID=187 \
    -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
    --volume /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri:/dev/dri \
    -v "$(pwd)":/quac_host \
    quac_basic:dev
