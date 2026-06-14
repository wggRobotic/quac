import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    remappings = [
        ('/tf_static', 'tf_static'),
        ('/tf', 'tf'),
        ('odom', 'odom_vo'),
        ('rgbd_image0', 'camera_back/rtabmap_rgbd'),
        ('rgbd_image1', 'camera_gripper/rtabmap_rgbd'),
    ]

    return LaunchDescription([

        Node(
            package='rtabmap_slam',
            executable='rtabmap',
            output='screen',
            namespace='quac',
            parameters=[os.path.join(package_dir, 'config', 'rtabmap.yaml')],
            remappings=remappings,
            arguments=['-d']
        ),

        Node(
            package='rtabmap_util',
            executable='obstacles_detection',
            output='screen',
            namespace='quac',
            parameters=[os.path.join(package_dir, 'config', 'rtabmap.yaml')],
            remappings=[
                ('cloud', 'camera_front/points'),
                ('obstacles', 'camera_front/obstacles'),
                ('ground', 'camera_front/ground'),
            ] + remappings
        )
    ])