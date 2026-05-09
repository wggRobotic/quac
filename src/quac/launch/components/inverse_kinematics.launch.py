import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    inverse_kinematics = Node(
        package="quac_inverse_kinematics",
        executable="ik_node",
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir, 'config', 'inverse_kinematics.yaml'),
            {'use_sim_time': LaunchConfiguration('sim_mode')}    
        ],
        remappings=[('/clock', 'clock')],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Starts with gazebo version of description'
        ),

        inverse_kinematics
    ])
