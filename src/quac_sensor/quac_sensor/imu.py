import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import board
from adafruit_lsm6ds.lsm6dsox import LSM6DSOX

class ImuPublisher(Node):
    def __init__(self):
        super().__init__('imu_publisher')
        self.publisher = self.create_publisher(Imu, 'imu/data_raw', 10)

        i2c = board.I2C()
        self.sox = LSM6DSOX(i2c)

        self.timer = self.create_timer(0.1, self.publish_imu)

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

        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = ImuPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()