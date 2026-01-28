from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import UnlessCondition
from launch_ros.actions import Node

def generate_launch_description():

    disable_wheels_arg = DeclareLaunchArgument(
        'disable_wheels',
        default_value='false',
        description='Whether to disable the wheels'
    )

    disable_arm_arg = DeclareLaunchArgument(
        'disable_arm',
        default_value='false',
        description='Whether to disable the robotic arm'
    )

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
        condition=UnlessCondition(LaunchConfiguration('disable_arm'))
    )

    arm_position_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_position_controller"],
        condition=UnlessCondition(LaunchConfiguration('disable_arm'))
    )

    return LaunchDescription([
        disable_wheels_arg,
        disable_arm_arg,
        diff_drive_controller,
        joint_state_controller,
        arm_position_controller,
    ])
