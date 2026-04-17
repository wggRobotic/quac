import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    return LaunchDescription([
        Node(
            package='quac_video',
            executable='hazmat_detection_server',
            namespace='quac',
            output='screen',
            parameters=[os.path.join(package_dir, 'config', 'detection_servers.yaml')],
            {
                'engines_dir': os.environ.get('QUAC_ENGINES_DIR', '/null/'),
                'models_dir': os.path.join(package_dir, 'models/')
            },
        )
    ])
