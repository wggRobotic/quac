#!/usr/bin/env python3

import gi
import depthai as dai

gi.require_version("Gst", "1.0")
from gi.repository import Gst

Gst.init(None)

W, H, FPS = 640, 400, 30

pipe = Gst.parse_launch(f"""
appsrc name=src caps=video/x-raw,format=BGR,width={W},height={H},framerate={FPS}/1
! videoconvert
! x264enc tune=zerolatency speed-preset=ultrafast
! rtph264pay pt=96
! udpsink host=192.168.2.100 port=5000
""")

src = pipe.get_by_name("src")
pipe.set_state(Gst.State.PLAYING)

with dai.Pipeline() as pipeline:
    cam = pipeline.create(dai.node.Camera).build()
    videoQueue = cam.requestOutput((W, H)).createOutputQueue()

    pipeline.start()

    while pipeline.isRunning():
        frame = videoQueue.get().getCvFrame()

        src.emit(
            "push-buffer",
            Gst.Buffer.new_wrapped(frame.tobytes())
        )