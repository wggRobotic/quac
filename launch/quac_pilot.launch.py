import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    simulation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir,'launch','sim.launch.py')]),
        condition=IfCondition(LaunchConfiguration('sim_mode'))
    )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        output="screen",
        arguments=["-d", os.path.join(package_dir,'config','nav.rviz')],
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
            'use_sim_time': LaunchConfiguration('sim_mode'),
            'params_file': os.path.join(package_dir, 'config', 'mapper_params_online_async.yaml'),
        }.items()
    )

    nav_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(package_dir, 'launch', 'navigation_launch.py')
        ),
        launch_arguments={
            'use_sim_time': LaunchConfiguration('sim_mode'),
            'namespace': 'quac',
            'params_file': os.path.join(package_dir, 'config', 'nav2_params.yaml')
        }.items()
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Simulates the robot in Gazebo'
        ),

        PushRosNamespace('quac'),
        rviz,
        twist_mux,
        simulation,
        nav_server,
        map_server
    ])