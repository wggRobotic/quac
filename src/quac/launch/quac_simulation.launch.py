import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import UnlessCondition, IfCondition

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'gazebo.launch.py')]),
    )

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'rsp.launch.py')]),
        launch_arguments={'sim_mode' : 'true'}.items()
    )

    twist_mux = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'twist_mux.launch.py')]),
        launch_arguments={'sim_mode' : 'true'}.items()
    )

    inverse_kinematics = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'inverse_kinematics.launch.py')]),
        launch_arguments={'sim_mode' : 'true'}.items()
    )

    slam_nav = GroupAction(
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'slam_toolbox.launch.py')]),
                launch_arguments={'sim_mode' : 'true'}.items(),
                condition=UnlessCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'ohm_slam.launch.py')]),
                launch_arguments={'sim_mode' : 'true'}.items(),
                condition=IfCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'nav2.launch.py')]),
                launch_arguments={'sim_mode' : 'true'}.items(),
                condition=UnlessCondition(LaunchConfiguration('disable_nav'))
            )
        ],
        condition=UnlessCondition(LaunchConfiguration('disable_slam'))
    )

    rviz = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'rviz.launch.py')]),
        launch_arguments={'sim_mode' : 'true'}.items()
    )

    guiniverse = Node(
        package='guiniverse',
        executable='main',
        namespace='quac',
        output='screen',
        parameters=[{'use_sim_time': LaunchConfiguration('sim_mode')}],
        condition=UnlessCondition(LaunchConfiguration('disable_guiniverse')),
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
            description='whether to disable slam'
        ),
        DeclareLaunchArgument(
            'disable_guiniverse',
            default_value='true',
            description='whether to disable guiniverse'
        ),
        
        gazebo,
        rsp,
        twist_mux,
        inverse_kinematics,
        slam_nav,
        rviz,
        guiniverse
    ])
