#!/usr/bin/env python3

import time

import rclpy
from rclpy.node import Node

from geometry_msgs.msg import TwistStamped, Pose


class CmdVelThenPose(Node):
    def __init__(self):
        super().__init__("sensor_box_autonomous")

        self.cmd_pub = self.create_publisher(TwistStamped, "/quac/cmd_vel_pilot", 10)
        self.pose_pub = self.create_publisher(Pose, "/quac/ee_pose", 10)

    def run(self):
        twist = TwistStamped()
        twist.header.frame_id = "base_link"
        twist.twist.linear.x = 0.2
        twist.twist.angular.z = 0.0

        pose = Pose()
        pose.position.x = 1.0
        pose.position.y = 0.0
        pose.position.z = 0.03
        pose.orientation.w = 1.0
        pose.orientation.y = 0.15

        self.get_logger().info("Publishing pose #1")
        self.pose_pub.publish(pose)

        time.sleep(1.0)

        self.get_logger().info("Publishing pose #2")
        self.pose_pub.publish(pose)

        time.sleep(2.0)

        self.get_logger().info("Publishing TwistStamped for 5 seconds...")

        start = time.time()
        while rclpy.ok() and (time.time() - start) < 5.0:
            twist.header.stamp = self.get_clock().now().to_msg()
            self.cmd_pub.publish(twist)
            time.sleep(0.1)

        # Stop the robot
        stop = TwistStamped()
        stop.header.stamp = self.get_clock().now().to_msg()
        stop.header.frame_id = "base_link"
        self.cmd_pub.publish(stop)

        self.get_logger().info("Done.")


def main():
    rclpy.init()
    node = CmdVelThenPose()

    try:
        node.run()
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()