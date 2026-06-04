import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    return LaunchDescription([
        Node(
            package='quac_detection',
            executable='landolt_c_detection_server',
            namespace='quac',
            output='screen',
            remappings=[
                ('/tf', 'tf'),
                ('/tf_static', 'tf_static'),
            ],
            parameters=[os.path.join(package_dir, 'config', 'detection_servers.yaml')],
        )
    ])
