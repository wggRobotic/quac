import cv2

gst = (
    'udpsrc port=5000 caps="application/x-rtp,media=video,encoding-name=H264,payload=96" '
    '! rtph264depay ! avdec_h264 ! videoconvert ! appsink'
)

cap = cv2.VideoCapture(gst, cv2.CAP_GSTREAMER)

while True:
    ret, frame = cap.read()
    if not ret:
        continue
    cv2.imshow("Stream", frame)
    if cv2.waitKey(1) == 27:
        break
