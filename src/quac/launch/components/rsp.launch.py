import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration, Command
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    robot_description_config = Command([
        'xacro ', os.path.join(package_dir,'description','robot.urdf.xacro'), 
        ' sim_mode:=', LaunchConfiguration('sim_mode'),
        ' disable_wheels:=', LaunchConfiguration('disable_wheels'),
        ' disable_arm:=', LaunchConfiguration('disable_arm'),
    ])
    
    rsp = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        namespace='quac',
        output='screen',
        parameters=[{
            'robot_description': robot_description_config, 
            'use_sim_time': LaunchConfiguration('sim_mode')
        }],
        remappings=[
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
            ('/clock', 'clock'),
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Starts with gazebo version of description'
        ),
        DeclareLaunchArgument(
            'disable_wheels',
            default_value='false',
            description='Whether to disable the wheels'
        ),
        DeclareLaunchArgument(
            'disable_arm',
            default_value='false',
            description='Whether to disable the robotic arm'
        ),

        rsp
    ])