import os
import sys

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    nav_flag = 'nav:=true' in sys.argv
    sim_flag = 'sim:=true' in sys.argv

    simulation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir,'launch','sim.launch.py')])
    )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        output="screen",
        arguments=["-d", os.path.join(package_dir,'config','map.rviz' if nav_flag else 'default.rviz')],
        remappings=[
            ('/robot_description', 'robot_description'),
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
            ('/clicked_point', 'clicked_point'),
            ('/goal_pose', 'goal_pose'),
            ('/initialpose', 'initialpose'),
        ],
    )

    twist_mux = Node(
        package='twist_mux',
        executable='twist_mux',
        output='screen',
        parameters=[
            os.path.join(
                package_dir,
                'config',
                'twist_mux.yaml'
            )
        ],
        remappings=[
            ('cmd_vel_out', 'cmd_vel')
        ],
    )

    map_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(package_dir, 'launch', 'online_async_launch.py')
        ),
        launch_arguments={
            'use_sim_time': 'true' if sim_flag else 'false',
            'params_file': os.path.join(package_dir, 'config', 'mapper_params_online_async.yaml'),
        }.items()
    )

    ld = [
        PushRosNamespace('quac'),
        rviz,
        twist_mux
    ]

    if sim_flag:
        ld.append(simulation)
        
    if nav_flag:
        ld.append(map_server)

    return LaunchDescription(ld)