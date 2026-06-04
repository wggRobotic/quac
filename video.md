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

gst-launch-1.0 v4l2src device=/dev/video4 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! x264enc tune=zerolatency speed-preset=superfast bitrate=3000 key-int-max=30 ! rtph264pay pt=96 config-interval=1 ! udpsink host=192.168.1.136 port=5000 sync=false async=false

gst-launch-1.0 -v udpsrc port=5000 buffer-size=1048576 caps="application/x-rtp, media=video, encoding-name=H264, payload=96" ! rtph264depay ! h264parse config-interval=1 ! avdec_h264 ! videoconvert ! autovideosink sync=false
```

# set ip
```
ros2 topic pub /quac/video_target_ip std_msgs/msg/String "{data: '192.168.1.136'}" -1
```

# publish dummy frame
```
ros2 run tf2_ros static_transform_publisher 0 0 0 0 0 0 map odom --ros-args -r /tf:=/quac/tf -r /tf_static:=/quac/tf_static
```