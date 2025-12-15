import os
import sys

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace

def generate_launch_description():

    package_dir = get_package_share_directory('quac')

    nav_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(package_dir, 'launch', 'navigation_launch.py')
        ),
        launch_arguments={
            'use_sim_time': 'true',
            'namespace': 'quac',
            'params_file': os.path.join(package_dir, 'config', 'nav2_params.yaml')
        }.items()
    )

    ld = [
        PushRosNamespace('quac'),
        nav_server
    ]
    return LaunchDescription(ld)