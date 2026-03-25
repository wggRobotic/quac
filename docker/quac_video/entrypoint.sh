#!/bin/bash

HAZMAT_ENGINE_FILE="engines/hazmat_yolo26.engine"

if [ -f "$HAZMAT_ENGINE_FILE" ]; then
    echo "[INFO] $HAZMAT_ENGINE_FILE already exists, skipping build."
else
    echo "[INFO] $HAZMAT_ENGINE_FILE engine not found. Building engine from ONNX..."

    /usr/src/tensorrt/bin/trtexec \
        --onnx=src/quac/quac_video/quac_video/models/hazmat_yolo26.onnx \
        --saveEngine=engines/$HAZMAT_ENGINE_FILE \
        --verbose \
        --skipInference
fi

ros2 run quac_video stream_app