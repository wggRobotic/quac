#!/bin/bash
cd /workspace/ros2_ws/engines

HAZMAT_ONNX_FILE="hazmat_yolo26.onnx"
HAZMAT_ENGINE_FILE="hazmat_yolo26.engine"

if [ -f "$HAZMAT_ONNX_FILE" ]; then
    echo "[INFO] $HAZMAT_ONNX_FILE already exists, skipping build."
else
    echo "[INFO] $HAZMAT_ONNX_FILE not found. Building ONNX from Pytorch..."

    python3 ../../DeepStream-Yolo/utils/export_yolo26.py -w ../src/quac/quac_video/infer/models/hazmat_yolo26.pt --opset 18
    mv labels.txt hazmat_labels.txt
fi

if [ -f "$HAZMAT_ENGINE_FILE" ]; then
    echo "[INFO] $HAZMAT_ENGINE_FILE already exists, skipping build."
else
    echo "[INFO] $HAZMAT_ENGINE_FILE engine not found. Building engine from ONNX..."

    /usr/src/tensorrt/bin/trtexec \
        --onnx=infer/models/hazmat_yolo26.onnx \
        --saveEngine=infer/engines/hazmat_yolo26.engine \
        --verbose \
        --skipInference
fi

cd /workspace/ros2_ws

./src/quac/quac_video/application/build/application 192.168.137.26 5000 6000

#gst-launch-1.0 v4l2src device=/dev/video4 ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' ! mux.sink_0 nvstreammux name=mux batch-size=1 width=640 height=480 live-source=1 ! nvinfer config-file-path=infer/config/hazmat_config.txt ! nvvideoconvert ! nvdsosd ! nvvideoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay ! udpsink host=192.168.137.26 port=5000