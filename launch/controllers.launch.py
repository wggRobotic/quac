from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    diff_drive = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['diff_drive_controller'],
        output='screen'
    )

    joint_state = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster'],
        output='screen',
    )

    position_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_position_controller"],
    )

    return LaunchDescription([
        diff_drive,
        joint_state,
        position_controller,
    ])
