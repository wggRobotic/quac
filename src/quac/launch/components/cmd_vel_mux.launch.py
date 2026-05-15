import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    cmd_vel_mux = Node(
        package='quac_cmd_vel_mux',
        executable='cmd_vel_mux',
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir, 'config', 'cmd_vel_mux.yaml'),
            {'use_sim_time': LaunchConfiguration('sim_mode')}
        ],
        remappings=[('/clock', 'clock')],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),

        cmd_vel_mux
    ])