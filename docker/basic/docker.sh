cd docker/basic

xhost +local:docker

docker build -t quac_basic:dev .

sudo docker run \
    -it --rm --network host \
    --env DISPLAY=$DISPLAY \
    --env QT_X11_NO_MITSHM=1 \
    --volume /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri:/dev/dri \
    quac_basic:dev
