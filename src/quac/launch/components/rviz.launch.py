import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        namespace='quac',
        output='screen',
        arguments=["-d", os.path.join(package_dir,'config','nav.rviz')],
        parameters=[{'use_sim_time': LaunchConfiguration('sim_mode')}],
        remappings=[
            ('/robot_description', 'robot_description'),
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
            ('/clicked_point', 'clicked_point'),
            ('/goal_pose', 'goal_pose'),
            ('/initialpose', 'initialpose')
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Simulates the robot in Gazebo'
        ),

        rviz
    ])