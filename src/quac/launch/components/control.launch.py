import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import RegisterEventHandler, DeclareLaunchArgument, OpaqueFunction
from launch_ros.actions import Node
from launch.event_handlers import OnProcessStart
from launch.substitutions import LaunchConfiguration

def launch_setup(context, *args, **kwargs):
    odom_tf = context.launch_configurations.get('odom_tf', 'true').lower() == 'true'
    old_arm = context.launch_configurations.get('old_arm', 'false').lower() == 'true'

    package_dir = get_package_share_directory('quac')

    joints = [
        "arm_segment_0_joint",
        "arm_segment_1_joint",
        "gripper_joint"
    ]

    if not old_arm:
        joints.append("arm_segment_2_joint")

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        namespace='quac',
        output='screen',
        parameters=[
            os.path.join(package_dir,'config','controllers.yaml'),
            {
                "enable_odom_tf": odom_tf,
                "joints": joints    
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

    return [
        controller_manager,
        RegisterEventHandler(
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
    ]

def generate_launch_description():

    return LaunchDescription([
        DeclareLaunchArgument(
            'odom_tf',
            default_value='true'
        ),
        DeclareLaunchArgument(
            'old_arm',
            default_value='false'
        ),
        OpaqueFunction(function=launch_setup)
    ])
