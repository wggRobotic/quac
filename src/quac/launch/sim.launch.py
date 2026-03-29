import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, TimerAction, RegisterEventHandler, IncludeLaunchDescription, DeclareLaunchArgument
from launch.event_handlers import OnProcessStart
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir,'launch','rsp.launch.py')]),
        launch_arguments={'sim_mode': 'true'}.items()
    )
    
    generate_sdf = ExecuteProcess(
        cmd=['xacro', os.path.join(package_dir,'worlds','obstacles.sdf.xacro'),'-o', '/tmp/world.sdf'],
        output='screen'
    )
    
    gazebo = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=generate_sdf,
            on_start=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource([os.path.join(
                        get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
                        launch_arguments={'gz_args': f'-r -v4 /tmp/world.sdf', 'on_exit_shutdown': 'true'}.items()
                )
            ],
        )
    )

    bridge_params = os.path.join(package_dir,'config','gz_bridge.yaml')
    
    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            '--ros-args',
            '-p',
            f'config_file:={bridge_params}',
        ]
    )

    spawn_entity = Node(
        package='ros_gz_sim', 
        executable='create',
        arguments=['-topic', 'robot_description',
                    '-name', 'quac_robot',
                    '-z', '0.1'],
        output='screen'
    )

    quac_common = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [os.path.join(package_dir,'launch','quac_common.launch.py')]
        ),
        launch_arguments={
            'use_sim_time': 'true',
            'disable_nav' : LaunchConfiguration('disable_nav'),
            'ohm_slam' : LaunchConfiguration('ohm_slam'),
            'disable_slam' : LaunchConfiguration('disable_slam')
        }.items()
    )  

    delayed_arm_position_reset = TimerAction(
        period=7.0,
        actions=[ExecuteProcess(
                    cmd=['ros2',' topic pub --once /quac/arm_joint_trajectory trajectory_msgs/msg/JointTrajectory "{joint_names: [\'arm_servo_0_joint\',\'arm_servo_1_joint\',\'gripper_servo_joint\'], points: [{positions: [-1.57, 1.57, 0.0], time_from_start: {sec: 5, nanosec: 0}}]}"'],
                    output='screen',
                    shell = True, 
                )]
    )


    return LaunchDescription([
        DeclareLaunchArgument(
            'disable_nav',
            default_value='false',
            description='disables nav2'
        ),
        DeclareLaunchArgument(
            'ohm_slam',
            default_value='false',
            description='uses ohm_tsd_slam instead of slam_toolbox'
        ),
        DeclareLaunchArgument(
            'disable_slam',
            default_value='false',
            description='whether to diable slam'
        ),
        generate_sdf,
        rsp,
        gazebo,
        ros_gz_bridge,
        spawn_entity,
        quac_common,
        #delayed_arm_position_reset,
    ])
