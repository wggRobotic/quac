import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition, UnlessCondition

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    simulation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir,'launch','sim.launch.py')]),
        condition=IfCondition(LaunchConfiguration('sim_mode')),
        launch_arguments={ 
            'disable_nav' : LaunchConfiguration('disable_nav'),
            'ohm_slam' : LaunchConfiguration('ohm_slam'),
            'disable_slam' : LaunchConfiguration('disable_slam')
        }.items()
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
            ('/initialpose', 'initialpose')
        ],
    )

    guiniverse = Node(
        package="guiniverse",
        executable="main",
        condition=UnlessCondition(LaunchConfiguration('disable_guiniverse')),
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Simulates the robot in Gazebo'
        ),
        DeclareLaunchArgument(
            'disable_nav',
            default_value='false',
            description='disables nav2'
        ),
        DeclareLaunchArgument(
            'ohm_slam',
            default_value='false',
            description='uses ohm_tsd_slam instead of slam_toolbox'
        ),
        DeclareLaunchArgument(
            'disable_slam',
            default_value='false',
            description='whether to diable slam'
        ),

        PushRosNamespace('quac'),
        rviz,
        simulation,
        guiniverse
    ])