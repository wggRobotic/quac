import pyrealsense2 as rs
import numpy as np
import cv2

# ---------------- CONFIG ----------------
DEST_IP = "127.0.0.1"   # main PC
DEST_PORT = 5000

WIDTH = 640
HEIGHT = 480
FPS = 60
BITRATE = 4000  # kbps (increase if artifacts appear)
# ----------------------------------------

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.color, WIDTH, HEIGHT, rs.format.bgr8, FPS)

pipeline.start(config)

gst_pipeline = (
    f'appsrc ! videoconvert ! video/x-raw,format=I420 '
    f'! x264enc tune=zerolatency bitrate={BITRATE} speed-preset=ultrafast '
    f'key-int-max=30 '
    f'! rtph264pay config-interval=1 pt=96 '
    f'! udpsink host={DEST_IP} port={DEST_PORT} sync=false async=false'
)

out = cv2.VideoWriter(
    gst_pipeline,
    cv2.CAP_GSTREAMER,
    0,
    FPS,
    (WIDTH, HEIGHT),
    True
)

if not out.isOpened():
    raise RuntimeError("Failed to open GStreamer pipeline")

print("Streaming started...")

try:
    while True:
        frames = pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        if not color_frame:
            continue

        frame = np.asanyarray(color_frame.get_data())
        out.write(frame)

except KeyboardInterrupt:
    print("Stopping stream")

finally:
    out.release()
    pipeline.stop()
