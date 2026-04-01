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
        
        self.publisher = self.create_publisher(CompressedImage, 'thermal_image/compressed', 10)

        self.bridge = CvBridge()

        i2c_bus = busio.I2C(board.SCL, board.SDA, frequency=400000)
        self.sensor = adafruit_mlx90640.MLX90640(i2c_bus)
        self.sensor.refresh_rate = adafruit_mlx90640.RefreshRate.REFRESH_16_HZ
        
        self.timer = self.create_timer(0.1, self.timer_callback)

    def timer_callback(self):
        frame = np.zeros((24*32,), dtype=np.float32)
        try:
            self.sensor.getFrame(frame)
        except OSError as e:
            if e.errno == 121:
                self.get_logger().warn('I2C read error')
            else:
                self.get_logger().error(f'OSError: {e}')
            return
        except ValueError as e:
            self.get_logger().warn(f'Thermal frame calculation error: {e}')
            return
        
        frame = np.reshape(frame, (24, 32))

        thermal_image = cv2.normalize(frame, None, 0, 255, cv2.NORM_MINMAX)
        thermal_image = np.uint8(thermal_image)
        thermal_image = cv2.resize(thermal_image, (320, 240), interpolation=cv2.INTER_NEAREST)
        thermal_image_color = cv2.applyColorMap(thermal_image, cv2.COLORMAP_JET)

        msg = self.bridge.cv2_to_compressed_imgmsg(thermal_image_color, dst_format='jpg')

        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = ThermalCamPublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("KeyboardInterrupt caught!")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()