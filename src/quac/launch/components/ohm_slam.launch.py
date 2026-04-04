import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    ohm_slam = Node(
        package='ohm_tsd_slam',
        executable='slam_node',
        name='tsd_slam',
        namespace='quac',
        output='screen',
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
            {'use_sim_time': LaunchConfiguration('sim_mode')}
        ],
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),

        ohm_slam
    ])