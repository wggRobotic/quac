import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    return LaunchDescription([
        Node(
            package="quac_arm_target_controller",
            executable="arm_controller",
            namespace='quac',
            output='screen',
            parameters=[os.path.join(package_dir, 'config', 'arm_target_controller.yaml')],
            remappings=[
                ('/tf', 'tf'),
                ('/tf_static', 'tf_static'),
            ],
        )
    ])
