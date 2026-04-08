docker build -t quac_core:dev -f dockerfiles/core.dockerfile .

sudo docker run \
    -it --rm --network host \
    -e ROS_DOMAIN_ID=187 \
    -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
    -v "$(pwd)":/quac_host \
    quac_core:dev
