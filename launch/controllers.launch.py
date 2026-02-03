from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import UnlessCondition
from launch_ros.actions import Node

def generate_launch_description():

    joint_state_controller = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster'],
        output='screen',
    )

    diff_drive_controller = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['diff_drive_controller'],
        output='screen',
    )

    arm_position_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_position_controller"],
    )

    return LaunchDescription([
        diff_drive_controller,
        joint_state_controller,
        arm_position_controller,
    ])
