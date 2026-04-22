#!/usr/bin/env python3

import rclpy
from rclpy.node import Node

from std_msgs.msg import Header
from builtin_interfaces.msg import Time
from geometry_msgs.msg import Pose

from quac_interfaces.msg import DetectedObject, DetectedObjectArray


class DetectedObjectPublisher(Node):

    def __init__(self):
        super().__init__('detected_object_publisher')

        self.pub = self.create_publisher(
            DetectedObjectArray,
            '/quac/camera_front/bgrd/hazmat_signs',
            10
        )

        self.timer = self.create_timer(1.0, self.publish_msg)

    def publish_msg(self):
        msg = DetectedObjectArray()

        # Header
        msg.header = Header()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "camera_front"

        # Object 1
        obj1 = DetectedObject()
        obj1.name = "object_0"
        obj1.type = "car"
        obj1.last_detection_stamp = self.get_clock().now().to_msg()

        obj1.pose = Pose()
        obj1.pose.position.x = 0.1
        obj1.pose.position.y = 0.0
        obj1.pose.position.z = 0.0
        obj1.pose.orientation.w = 1.0

        # Object 2
        obj2 = DetectedObject()
        obj2.name = "object_1"
        obj2.type = "person"
        obj2.last_detection_stamp = self.get_clock().now().to_msg()

        obj2.pose = Pose()
        obj2.pose.position.x = 0.0
        obj2.pose.position.y = 0.2
        obj2.pose.position.z = 0.1
        obj2.pose.orientation.w = 1.0

        msg.objects = [obj1, obj2]

        self.pub.publish(msg)


def main():
    rclpy.init()
    node = DetectedObjectPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()