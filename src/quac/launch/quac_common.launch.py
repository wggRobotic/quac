import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.substitutions import LaunchConfiguration
from launch.conditions import UnlessCondition, IfCondition
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

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

    arm_position_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_position_controller"],
    )

    arm_ik = Node(
        package="quac_control",
        executable="quac_ik_node",
        parameters=[
            os.path.join(package_dir, 'config', 'control.yaml'),
            {'use_sim_time': LaunchConfiguration('use_sim_time')}    
        ],
        remappings=[('/clock', 'clock'),],
    )

    twist_mux = Node(
        package='twist_mux',
        executable='twist_mux',
        output='screen',
        parameters=[
            os.path.join(package_dir, 'config', 'twist_mux.yaml'),
            {'use_sim_time': LaunchConfiguration('use_sim_time')}
        ],
        remappings=[('cmd_vel_out', 'cmd_vel'), ('/clock', 'clock'),],
    )

    slam_nav = GroupAction(
        actions=[
            Node(
                package='ohm_tsd_slam',
                executable='slam_node',
                name='tsd_slam',
                remappings=[
                    ('tsd_slam/laser', 'scan'),
                    ('/clock', 'clock'),
                    ('tsd_slam/map', 'map'),
                    ('tsd_slam/estimated_pose', 'estimated_pose'),
                    ('tsd_slam/map/image', 'map/image'),
                    ('tsd_slam/start_stop_slam', 'start_stop_slam'),
                    ('tsd_slam/get_map', 'get_map'),
                    ('/tf', 'tf'),
                    ('/tf_static', 'tf_static'),
                ],
                parameters=[
                    os.path.join(get_package_share_directory('ohm_tsd_slam'), 'config', 'single-laser.yaml'),
                    {'use_sim_time': LaunchConfiguration('use_sim_time')}
                ],
                condition=IfCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(package_dir, 'launch', 'online_async_launch.py')
                ),
                launch_arguments={
                    'use_sim_time': LaunchConfiguration('use_sim_time'),
                    'params_file': os.path.join(package_dir, 'config', 'mapper_params_online_async.yaml'),
                }.items(),
                condition=UnlessCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(os.path.join(package_dir, 'launch', 'navigation_launch.py')),
                launch_arguments={
                    'use_sim_time': LaunchConfiguration('use_sim_time'),
                    'namespace': 'quac',
                    'params_file': os.path.join(package_dir, 'config', 'nav2_params.yaml')
                }.items(),
                condition=UnlessCondition(LaunchConfiguration('disable_nav'))
            )
        ],
        condition=UnlessCondition(LaunchConfiguration('disable_slam'))
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
            'use_sim_time',
            default_value='false',
            description='if simulation'
        ),
        DeclareLaunchArgument(
            'disable_slam',
            default_value='false',
            description='whether to diable slam'
        ),
        arm_ik,
        joint_state_controller,
        diff_drive_controller,
        arm_position_controller,
        twist_mux,
        slam_nav
    ])
