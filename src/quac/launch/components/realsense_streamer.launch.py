import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    camera = Node(
        package='quac_realsense_streamer',
        executable='realsense_streamer',
        name=LaunchConfiguration('camera_name'),
        namespace='quac',
        output='screen',
        parameters=[os.path.join(package_dir, 'config', 'cam_streamers.yaml')],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'camera_name',
            default_value='cam',
            description='name of the camera'
        ),
        camera
    ])
