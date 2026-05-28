import rclpy
from rclpy.node import Node

from geometry_msgs.msg import TransformStamped
from tf2_ros.static_transform_broadcaster import StaticTransformBroadcaster


class DummyTfPublisher(Node):

    def __init__(self):
        super().__init__('dummy_tf_publisher')

        self.broadcaster = StaticTransformBroadcaster(self)

        transforms = []

        # map -> camera_back_color_optical_frame
        back_tf = TransformStamped()
        back_tf.header.stamp = self.get_clock().now().to_msg()
        back_tf.header.frame_id = 'map'
        back_tf.child_frame_id = 'camera_back_color_optical_frame'

        back_tf.transform.translation.x = 0.0
        back_tf.transform.translation.y = 0.0
        back_tf.transform.translation.z = 0.0

        back_tf.transform.rotation.x = 0.0
        back_tf.transform.rotation.y = 0.0
        back_tf.transform.rotation.z = 0.0
        back_tf.transform.rotation.w = 1.0

        transforms.append(back_tf)

        # map -> camera_front_color_optical_frame
        front_tf = TransformStamped()
        front_tf.header.stamp = self.get_clock().now().to_msg()
        front_tf.header.frame_id = 'map'
        front_tf.child_frame_id = 'camera_front_color_optical_frame'

        front_tf.transform.translation.x = 0.0
        front_tf.transform.translation.y = 0.0
        front_tf.transform.translation.z = 0.0

        front_tf.transform.rotation.x = 0.0
        front_tf.transform.rotation.y = 0.0
        front_tf.transform.rotation.z = 0.0
        front_tf.transform.rotation.w = 1.0

        transforms.append(front_tf)

        self.broadcaster.sendTransform(transforms)

        self.get_logger().info('Published dummy static TFs')


def main():
    rclpy.init(args=['--ros-args', '-r', '/tf:=/quac/tf', '-r', '/tf_static:=/quac/tf_static'])

    node = DummyTfPublisher()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()