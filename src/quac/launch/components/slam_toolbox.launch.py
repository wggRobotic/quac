import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import UnlessCondition, IfCondition
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    remapping = [
        ('/tf', 'tf'),
        ('/tf_static', 'tf_static'),
        ('/clock', 'clock'),
        ('/scan', 'scan'),
        ('/map', 'map'),
        ('/map_metadata', 'map_metadata'),
    ]

    return LaunchDescription([

        DeclareLaunchArgument(
            'sim_mode',
            default_value='true',
            description='Use simulation/Gazebo clock'
        ),
        DeclareLaunchArgument(
            'sync',
            default_value='true',
            description='Use simulation/Gazebo clock'
        ),
        Node(
            package='slam_toolbox',
            executable='sync_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            namespace='quac',
            remappings=remapping,
            parameters=[
                os.path.join(package_dir, 'config', 'slam_toolbox_sync.yaml'),
                {'use_sim_time': LaunchConfiguration('sim_mode')}
            ],
            condition=IfCondition(LaunchConfiguration('sync'))
        ),
        Node(
            package='slam_toolbox',
            executable='async_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            namespace='quac',
            remappings=remapping,
            parameters=[
                os.path.join(package_dir, 'config', 'slam_toolbox_async.yaml'),
                {'use_sim_time': LaunchConfiguration('sim_mode')}
            ],
            condition=UnlessCondition(LaunchConfiguration('sync'))
        )
    ])