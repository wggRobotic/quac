import rclpy
from rclpy.node import Node
from sensor_msgs.msg import CompressedImage
from cv_bridge import CvBridge
import cv2
import numpy as np
import board
import busio
import adafruit_mlx90640

class ThermalCamPublisher(Node):
    def __init__(self):
        super().__init__('thermal_cam_publisher')
        
        # Publisher for compressed images
        self.publisher_ = self.create_publisher(CompressedImage, 'thermal_image/compressed', 10)

        self.bridge = CvBridge()

        # Initialize I2C bus and sensor.
        i2c_bus = busio.I2C(board.SCL, board.SDA, frequency=400000)
        self.sensor = adafruit_mlx90640.MLX90640(i2c_bus)
        self.sensor.refresh_rate = adafruit_mlx90640.RefreshRate.REFRESH_16_HZ
        
        self.timer = self.create_timer(0.1, self.timer_callback)  # Publish at 10 Hz

    def timer_callback(self):
        # Read the pixels from the thermal camera.
        frame = np.zeros((24*32,), dtype=np.float32)
        self.sensor.getFrame(frame)
        frame = np.reshape(frame, (24, 32))

        # Normalize the image to 0-255.
        thermal_image = cv2.normalize(frame, None, 0, 255, cv2.NORM_MINMAX)

        # Convert to uint8.
        thermal_image = np.uint8(thermal_image)

        # Resize the image to a standard size.
        thermal_image = cv2.resize(thermal_image, (320, 240), interpolation=cv2.INTER_CUBIC)

        # Apply a colormap to the thermal image.
        thermal_image_color = cv2.applyColorMap(thermal_image, cv2.COLORMAP_JET)

        # Convert the image to a ROS 2 CompressedImage message.
        msg = self.bridge.cv2_to_compressed_imgmsg(thermal_image_color, dst_format='jpg')

        # Publish the message.
        self.publisher_.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = ThermalCamPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()