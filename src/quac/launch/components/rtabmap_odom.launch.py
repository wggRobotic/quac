import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    return LaunchDescription([
        Node(
            package='rtabmap_odom',
            executable='rgbd_odometry',
            namespace='quac',
            output='screen',
            parameters=[os.path.join(package_dir, 'config', 'rtabmap.yaml')],
            remappings=[
                ('/tf_static', 'tf_static'),
                ('/tf', 'tf'),
                ('odom', 'odom_vo'),
                ('rgbd_image0', 'camera_back/rtabmap_rgbd'),
                ('rgbd_image', 'camera_gripper/rtabmap_rgbd'),
            ]
        )
    ])