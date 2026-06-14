import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    return LaunchDescription([
        Node(
            package='quac_cam_streams',
            executable='depthai_streamer',
            name='camera_front',
            namespace='quac',
            output='screen',
            parameters=[os.path.join(package_dir, 'config', 'cam_streamers.yaml')]
        )
    ])
