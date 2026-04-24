import rclpy
from rclpy.node import Node

from sensor_msgs.msg import Imu
from sensor_msgs.msg import MagneticField
from sensor_msgs.msg import CompressedImage

from cv_bridge import CvBridge
import cv2
import numpy as np

import board
import busio
from adafruit_lsm6ds.lsm6dsox import LSM6DSOX
import adafruit_tlv493d
import adafruit_mlx90640

class SensorPublisher(Node):
    def __init__(self):
        super().__init__('sensor_publisher')

        i2c = busio.I2C(board.SCL, board.SDA)
        self.timer = self.create_timer(0.1, self.publish_sensor)

        # imu
        self.declare_parameter('disable_imu', False)
        self.disable_imu = self.get_parameter('disable_imu').value

        if not self.disable_imu:
            self.imu_publisher = self.create_publisher(Imu, 'imu', 10)
            self.sox = LSM6DSOX(i2c)

        # magnetometer
        self.declare_parameter('disable_magnetometer', False)
        self.disable_magnetometer = self.get_parameter('disable_magnetometer').value

        if not self.disable_magnetometer:
            self.magnetometer_publisher = self.create_publisher(MagneticField, 'magnetic_field', 10)
            self.tlv = adafruit_tlv493d.TLV493D(i2c)

        # thermal cam
        self.declare_parameter('disable_thermal_cam', False)
        self.disable_thermal_cam = self.get_parameter('disable_thermal_cam').value

        if not self.disable_thermal_cam:
            self.thermal_cam_publisher = self.create_publisher(CompressedImage, 'thermal_image/compressed', 10)
            self.bridge = CvBridge()
            self.mlx = adafruit_mlx90640.MLX90640(i2c)
        
    def publish_imu(self):
        msg = Imu()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'imu'

        acc_x, acc_y, acc_z = self.sox.acceleration
        msg.linear_acceleration.x = acc_x
        msg.linear_acceleration.y = acc_y
        msg.linear_acceleration.z = acc_z

        ang_x, ang_y, ang_z = self.sox.gyro
        msg.angular_velocity.x = ang_x
        msg.angular_velocity.y = ang_y
        msg.angular_velocity.z = ang_z

        msg.orientation_covariance[0] = -1  # signal "no orientation"

        self.imu_publisher.publish(msg)

    def publish_magnetometer(self):
        msg = MagneticField()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'magnetometer'
        
        mag_x, mag_y, mag_z = self.tlv.magnetic
        msg.magnetic_field.x = mag_x
        msg.magnetic_field.y = mag_y
        msg.magnetic_field.z = mag_z

        msg.magnetic_field_covariance[0] = -1 # signal "no orientation"
        
        self.magnetometer_publisher.publish(msg)

    def publish_thermal_cam(self):
        frame = np.zeros((24*32,), dtype=np.float32)
        self.mlx.getFrame(frame)
        frame = np.reshape(frame, (24, 32))

        thermal_image = cv2.normalize(frame, None, 0, 255, cv2.NORM_MINMAX)
        thermal_image = np.uint8(thermal_image)
        thermal_image = cv2.resize(thermal_image, (320, 240), interpolation=cv2.INTER_NEAREST)
        thermal_image_color = cv2.applyColorMap(thermal_image, cv2.COLORMAP_JET)

        msg = self.bridge.cv2_to_compressed_imgmsg(thermal_image_color, dst_format='jpg')

        self.thermal_cam_publisher.publish(msg)

    def publish_sensor(self):

        try:
            if not self.disable_imu:
                self.publish_imu()
            if not self.disable_magnetometer:
                self.publish_magnetometer()
            if not self.disable_thermal_cam:
                self.publish_thermal_cam()

        except OSError as e:
            self.get_logger().error(f'OSError: {e}')
        except ValueError as e:
            self.get_logger().warn(f'calculation error: {e}')

def main(args=None):
    rclpy.init(args=args)
    node = SensorPublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("KeyboardInterrupt caught!")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()