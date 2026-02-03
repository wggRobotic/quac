import socket
import struct
import numpy as np
import lz4.frame
import cv2

PORT = 5001

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", PORT))

print("Listening for depth stream...")

while True:
    packet, _ = sock.recvfrom(65536)

    header_size = struct.calcsize("!IHH")
    frame_id, width, height = struct.unpack(
        "!IHH", packet[:header_size]
    )

    compressed = packet[header_size:]
    raw = lz4.frame.decompress(compressed)

    depth = np.frombuffer(raw, dtype=np.uint16).reshape((height, width))

    # Visualization only (scaled)
    depth_vis = cv2.convertScaleAbs(depth, alpha=0.03)
    cv2.imshow("Depth", depth_vis)

    if cv2.waitKey(1) == 27:
        break
