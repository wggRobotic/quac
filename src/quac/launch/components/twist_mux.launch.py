import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    twist_mux = Node(
        package='twist_mux',
        executable='twist_mux',
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir, 'config', 'twist_mux.yaml'),
            {'use_sim_time': LaunchConfiguration('sim_mode')}
        ],
        remappings=[('cmd_vel_out', 'cmd_vel'), ('/clock', 'clock'),],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),

        twist_mux
    ])