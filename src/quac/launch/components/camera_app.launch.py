import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    camera = Node(
        package='quac_video',
        executable='camera_app',
        name=LaunchConfiguration('camera_name'),
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir, 'config', 'camera_params.yaml'),
            {
                'engines_dir': os.environ.get('QUAC_ENGINES_DIR', '/null/'),
                'models_dir': os.path.join(package_dir, 'models/')
            },
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'camera_name',
            default_value='cam',
            description='name of the camera'
        ),
        camera
    ])
