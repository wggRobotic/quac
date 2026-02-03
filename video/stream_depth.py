import pyrealsense2 as rs
import numpy as np
import cv2
import socket
import struct
import lz4.frame

# ---------------- CONFIG ----------------
DEST_IP = "127.0.0.1"

RGB_PORT = 5000
DEPTH_PORT = 5001

WIDTH = 640
HEIGHT = 480
FPS = 60
BITRATE = 4000  # kbps
# ----------------------------------------

# ----------- RealSense setup ------------
pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.color, WIDTH, HEIGHT, rs.format.bgr8, FPS)
config.enable_stream(rs.stream.depth, WIDTH, HEIGHT, rs.format.z16, FPS)
pipeline.start(config)

# ----------- RGB GStreamer --------------
gst_pipeline = (
    f'appsrc ! videoconvert ! video/x-raw,format=I420 '
    f'! x264enc tune=zerolatency bitrate={BITRATE} speed-preset=ultrafast '
    f'key-int-max=30 '
    f'! rtph264pay config-interval=1 pt=96 '
    f'! udpsink host={DEST_IP} port={RGB_PORT} sync=false async=false'
)

rgb_out = cv2.VideoWriter(
    gst_pipeline,
    cv2.CAP_GSTREAMER,
    0,
    FPS,
    (WIDTH, HEIGHT),
    True
)

if not rgb_out.isOpened():
    raise RuntimeError("Failed to open RGB stream")

# ----------- Depth UDP socket ------------
depth_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

frame_id = 0
print("Streaming RGB + 16-bit depth...")

try:
    while True:
        frames = pipeline.wait_for_frames()
        color = frames.get_color_frame()
        depth = frames.get_depth_frame()

        if not color or not depth:
            continue

        # ---------- RGB ----------
        rgb_frame = np.asanyarray(color.get_data())
        rgb_out.write(rgb_frame)

        # ---------- Depth ----------
        depth_frame = np.asanyarray(depth.get_data())  # uint16

        raw_bytes = depth_frame.tobytes()
        compressed = lz4.frame.compress(raw_bytes, compression_level=0)

        header = struct.pack(
            "!IHH",
            frame_id,
            WIDTH,
            HEIGHT
        )

        depth_sock.sendto(header + compressed, (DEST_IP, DEPTH_PORT))

        frame_id += 1

except KeyboardInterrupt:
    print("Stopping")

finally:
    rgb_out.release()
    pipeline.stop()
    depth_sock.close()
