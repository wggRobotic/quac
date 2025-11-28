import os
import sys

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace

def generate_launch_description():

    nav_flag = '-nav' in sys.argv

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", os.path.join(get_package_share_directory('quac'),'config','map.rviz' if nav_flag else 'default.rviz')]
    )

    # Full path to the params file
    params_file_path = os.path.join(
        os.getcwd(), 'src', 'quac', 'config', 'mapper_params_online_async.yaml'
    )

    map_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
        ),
        launch_arguments={
            'params_file': params_file_path
        }.items()
    )

    

    twist_mux = Node(
        package='twist_mux',
        executable='twist_mux',
        name='twist_mux',
        output='screen',
        parameters=[
            os.path.join(
                get_package_share_directory('quac'),
                'config',
                'twist_mux.yaml'
            )
        ],
        remappings=[
            ('cmd_vel_out', 'diff_drive_controller/cmd_vel')
        ]
    )

    return LaunchDescription([
        PushRosNamespace('quac'),
        rviz_node,
        map_server,
        twist_mux
    ])