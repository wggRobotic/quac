import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import Command
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessStart
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import UnlessCondition

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch','rsp.launch.py')]),
        launch_arguments={ 
            'disable_wheels' : LaunchConfiguration('disable_wheels'),
            'disable_arm' : LaunchConfiguration('disable_arm'),
        }.items()
    )  

    robot_description = Command(['ros2 param get --hide-type /quac/robot_state_publisher robot_description'])

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            os.path.join(package_dir,'config','controllers.yaml'),
            {'robot_description': robot_description},           
        ],
        remappings=[
            ('/tf', 'tf'),
            ('diff_drive_controller/cmd_vel', 'cmd_vel'),
            ('diff_drive_controller/odom', 'odom')
        ],
    )

    delayed_controller_manager = TimerAction(period=3.0, actions=[controller_manager])

    delayed_controllers = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        [os.path.join(package_dir,'launch','controllers.launch.py')]
                    ),
                    launch_arguments={
                        'disable_wheels' : LaunchConfiguration('disable_wheels'),
                        'disable_arm' : LaunchConfiguration('disable_arm'),
                    }.items()
                )  
            ],
        )
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('rplidar_ros'),'launch','rplidar_a2m8_launch.py')
        ),
        launch_arguments={'frame_id': 'lidar'}.items(),
        condition=UnlessCondition(LaunchConfiguration('disable_lidar'))
    )

    return LaunchDescription([

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

        DeclareLaunchArgument(
            'disable_lidar',
            default_value='false',
            description='Whether to disable the lidar'
        ),
        
        PushRosNamespace('quac'),
        rsp,
        delayed_controller_manager,
        delayed_controllers,
        lidar
    ])
