import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import RegisterEventHandler, DeclareLaunchArgument
from launch_ros.actions import Node
from launch.event_handlers import OnProcessStart
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    ekf = Node(
        package="robot_localization",
        executable="ekf_node",
        output="screen",
        parameters=[
            os.path.join(package_dir,'config','ekf.yaml'),
            {'use_sim_time': LaunchConfiguration('sim_mode')},
        ],
        remappings=[
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
            ('/clock', 'clock')
        ]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),

        ekf,
    ])