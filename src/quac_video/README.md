# JPEG

## TCP
```
gst-launch-1.0 v4l2src device=/dev/video4 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! jpegenc quality=85 ! tcpserversink host=0.0.0.0 port=5000
gst-launch-1.0 -v nvv4l2camerasrc device=/dev/video4 ! nvvidconv ! nvjpegenc quality=85 ! tcpserversink host=0.0.0.0 port=5000 sync=false (format error)
gst-launch-1.0 tcpclientsrc host=192.168.1.58 port=5000 ! jpegdec ! videoconvert ! autovideosink
```

## UDP
```
gst-launch-1.0 v4l2src device=/dev/video4 ! video/x-raw,width=1920,height=1080,framerate=30/1 ! videoconvert ! jpegenc quality=85 ! rtpjpegpay ! udpsink host=192.168.1.136 port=5000 sync=false
gst-launch-1.0 udpsrc port=5000 caps="application/x-rtp, media=video, encoding-name=JPEG, payload=26" ! rtpjpegdepay ! jpegdec ! videoconvert ! autovideosink
```

# H264
```
```