#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TwistStamped


class CmdVelDelay(Node):
    def __init__(self):
        super().__init__("cmd_vel_delay")

        self.create_subscription(
            TwistStamped,
            "/quac/cmd_vel_pilot",
            self.callback,
            10,
        )

    def callback(self, msg: TwistStamped):
        now = self.get_clock().now()
        stamp = rclpy.time.Time.from_msg(msg.header.stamp)

        age = (now - stamp).nanoseconds / 1e9

        self.get_logger().info(
            f"Age: {age:.6f} s "
            f"(stamp={msg.header.stamp.sec}.{msg.header.stamp.nanosec:09d})"
        )


def main():
    rclpy.init()
    node = CmdVelDelay()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
