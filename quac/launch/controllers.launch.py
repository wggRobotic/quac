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
    )

    diff_drive_controller = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['diff_drive_controller'],
    )

    wheel_monitor = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["wheel_monitor"],
    )

    arm_position_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_position_controller"],
    )

    return LaunchDescription([
        joint_state_controller,
        diff_drive_controller,
        wheel_monitor,
        arm_position_controller,
    ])
