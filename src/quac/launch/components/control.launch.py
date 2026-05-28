import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import RegisterEventHandler, DeclareLaunchArgument
from launch_ros.actions import Node
from launch.event_handlers import OnProcessStart
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir,'config','controllers.yaml'),
            {
                "diff_drive_controller": {
                    "ros__parameters": {
                        "enable_odom_tf": LaunchConfiguration('disable_ekf')
                    }
                }
            }
        ],
        remappings=[
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
            ('/trajectories', 'trajectories'),
            ('diff_drive_controller/cmd_vel_unstamped', 'cmd_vel'),
            ('diff_drive_controller/cmd_vel', 'cmd_vel'),
            ('diff_drive_controller/odom', 'odom'),
            ('arm_position_controller/joint_trajectory', 'arm_joint_trajectory'),
            ('controller_manager/robot_description', 'robot_description')
        ]
    )

    controllers = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                Node(
                    package='controller_manager',
                    executable='spawner',
                    namespace='quac',
                    output='screen',
                    arguments=['joint_state_broadcaster'],
                ),
                Node(
                    package='controller_manager',
                    executable='spawner',
                    namespace='quac',
                    output='screen',
                    arguments=['diff_drive_controller'],
                ),
                Node(
                    package="controller_manager",
                    executable="spawner",
                    namespace='quac',
                    output='screen',
                    arguments=["arm_position_controller"],
                ),
                Node(
                    package="controller_manager",
                    executable="spawner",
                    namespace='quac',
                    output='screen',
                    arguments=["wheel_monitor"],
                ),
            ],
        ),
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'disable_ekf',
            default_value='false'
        ),
        controller_manager,
        controllers
    ])
