import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    sensors = Node(
        package="quac_sensor",
        executable="sensor_publisher",
        namespace='quac',
        output='screen',
        parameters=[{
            'disable_imu': LaunchConfiguration('disable_imu'),
            'disable_magnetometer': LaunchConfiguration('disable_magnetometer'),
            'disable_thermal_cam': LaunchConfiguration('disable_thermal_cam'),
        }]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'disable_thermal_cam',
            default_value='false',
            description='whether to disable the thermal cam'
        ),
        DeclareLaunchArgument(
            'disable_imu',
            default_value='false',
            description='whether to disable the imu'
        ),
        DeclareLaunchArgument(
            'disable_magnetometer',
            default_value='false',
            description='whether to disable the magnetometer'
        ),
        
        sensors,
    ])