import rclpy
from rclpy.node import Node
from sensor_msgs.msg import MagneticField
import board
import adafruit_tlv493d

class MagnetometerPublisher(Node):
    def __init__(self):
        super().__init__('magnetometer_publisher')
        
        self.publisher = self.create_publisher(MagneticField, 'magnetic_field', 10)

        i2c = board.I2C()
        self.tlv = adafruit_tlv493d.TLV493D(i2c)
        
        self.timer = self.create_timer(0.1, self.publish_field)

    def publish_field(self):
        msg = MagneticField()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'magnetometer'
        
        try:
            mag_x, mag_y, mag_z = self.tlv.magnetic
        except OSError as e:
            self.get_logger().error(f'OSError: {e}')
            return

        msg.magnetic_field.x = mag_x
        msg.magnetic_field.y = mag_y
        msg.magnetic_field.z = mag_z

        msg.magnetic_field_covariance[0] = -1 # signal "no orientation"
        
        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = MagnetometerPublisher()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("KeyboardInterrupt caught!")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()