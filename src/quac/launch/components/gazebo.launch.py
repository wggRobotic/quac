import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, RegisterEventHandler, IncludeLaunchDescription
from launch.event_handlers import OnProcessExit

def generate_launch_description():
    package_dir = get_package_share_directory('quac')
    
    generate_sdf = ExecuteProcess(cmd=['xacro', os.path.join(package_dir,'worlds','obstacles.sdf.xacro'),'-o', '/tmp/world.sdf'])
    
    gazebo = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=generate_sdf,
            on_exit=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource([os.path.join(
                        get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
                        launch_arguments={'gz_args': f'-r -v4 /tmp/world.sdf', 'on_exit_shutdown': 'true'}.items()
                )
            ],
        )
    )
    
    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        namespace='quac',
        output='screen',
        arguments=[
            '--ros-args',
            '-p',
            f"config_file:={os.path.join(package_dir,'config','gz_bridge.yaml')}",
        ]
    )

    spawn_entity = Node(
        package='ros_gz_sim', 
        executable='create',
        namespace='quac',
        output='screen',
        arguments=['-topic', 'robot_description',
                    '-name', 'quac_robot',
                    '-z', '0.1'],
    )

    joint_state_broadcaster = Node(
        package='controller_manager',
        executable='spawner',
        namespace='quac',
        output='screen',
        arguments=['joint_state_broadcaster'],
    )
    diff_drive_controller = Node(
        package='controller_manager',
        executable='spawner',
        namespace='quac',
        output='screen',
        arguments=['diff_drive_controller'],
    )
    arm_position_controller = Node(
        package="controller_manager",
        executable="spawner",
        namespace='quac',
        output='screen',
        arguments=["arm_position_controller"],
    )

    return LaunchDescription([
        generate_sdf,
        gazebo,
        ros_gz_bridge,
        spawn_entity,
        joint_state_broadcaster,
        diff_drive_controller,
        arm_position_controller
    ])
