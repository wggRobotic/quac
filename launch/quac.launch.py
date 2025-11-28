import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import Command
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessStart

import time

def generate_launch_description():
    package_name = 'quac'

    # Robot State Publisher
    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory(package_name),'launch','rsp.launch.py'
        )]), launch_arguments={'use_sim_time': 'false'}.items()
    )

    time.sleep(5)    

    robot_description = Command(['ros2 param get --hide-type /quac/robot_state_publisher robot_description'])

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[{'robot_description': robot_description},
                    os.path.join(get_package_share_directory(package_name),'config','controllers.yaml')
        ]
    )

    delayed_controller_manager = TimerAction(period=3.0, actions=[controller_manager])

    delayed_diff_drive_controller = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                Node(
                    package='controller_manager',
                    executable='spawner',
                    arguments=['diff_drive_controller'],
                    output='screen',
                    remappings=[
                        ('diff_cont/cmd_vel', 'cmd_vel')
                    ]
                )
            ],
        )
    )

    delayed_joint_state_broadcaster = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                Node(
                    package='controller_manager',
                    executable='spawner',
                    arguments=['joint_state_broadcaster'],
                    output='screen',
                )
            ],
        )
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('rplidar_ros'),'launch','rplidar_a2m8_launch.py')
        ),
        launch_arguments={
            'frame_id': 'laser_frame'
        }.items()
    )

    return LaunchDescription([
        PushRosNamespace('quac'),
        rsp,
        delayed_controller_manager,
        delayed_diff_drive_controller,
        delayed_joint_state_broadcaster,
        lidar
    ])
