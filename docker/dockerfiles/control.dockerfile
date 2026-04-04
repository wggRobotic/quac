FROM ros:humble

WORKDIR /quac

COPY ./src/quac_control src/quac_control
RUN colcon build --packages-select quac_control

COPY ./src/quac_hardware src/quac_hardware
RUN colcon build --packages-select quac_hardware

RUN rosdep init && rosdep update
RUN rosdep install --from-paths src --ignore-src -y

COPY src/quac_control