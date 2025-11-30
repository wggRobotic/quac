import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node


def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir,'launch','rsp.launch.py')]),
        launch_arguments={'use_sim_time': 'true'}.items()
    )

    default_world = os.path.join(
        package_dir,
        'worlds',
        'obstacles.world'
        )    
    
    gazebo = IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(
                    get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')]),
                    launch_arguments={'gz_args': f'-r -v4 {default_world}', 'on_exit_shutdown': 'true'}.items()
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

    # Spawn robot
    spawn_entity = Node(
        package='ros_gz_sim', 
        executable='create',
        arguments=['-topic', 'robot_description',
                    '-name', 'quac_robot',
                    '-z', '0.1'],
        output='screen'
    )

    controllers = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [os.path.join(package_dir,'launch','controllers.launch.py')]
        )
    )  

    return LaunchDescription([
        rsp,
        gazebo,
        ros_gz_bridge,
        spawn_entity,
        controllers
    ])
