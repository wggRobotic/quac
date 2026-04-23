FROM ros:humble
SHELL ["/bin/bash", "-c"]
WORKDIR /quac

RUN apt update
RUN apt install -y ros-humble-rmw-cyclonedds-cpp
RUN apt install -y ros-humble-image-transport ros-humble-image-transport-plugins

RUN apt install -y python3-pip
RUN pip3 install adafruit-circuitpython-tlv493d
RUN pip3 install adafruit-circuitpython-lsm6ds
RUN pip3 install adafruit-circuitpython-mlx90640
RUN pip3 install 'numpy<2'
RUN pip3 install Jetson.GPIO

COPY ./src/quac_sensor src/quac_sensor
RUN . /opt/ros/humble/setup.bash && colcon build --packages-select quac_sensor