import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch.conditions import UnlessCondition, IfCondition

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'rsp.launch.py')]),
        launch_arguments={ 
            'disable_wheels' : LaunchConfiguration('disable_wheels'),
            'disable_arm' : LaunchConfiguration('disable_arm')
        }.items()
    )

    twist_mux = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'twist_mux.launch.py')]),
    )

    control = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'control.launch.py')]),
    )

    inverse_kinematics = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'inverse_kinematics.launch.py')]),
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'lidar.launch.py')]),
        condition=UnlessCondition(LaunchConfiguration('disable_lidar'))
    )

    sensors = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'sensors.launch.py')]),
        launch_arguments={ 
            'disable_imu': LaunchConfiguration('disable_imu'),
            'disable_magnetometer': LaunchConfiguration('disable_magnetometer'),
            'disable_thermal_cam': LaunchConfiguration('disable_thermal_cam'),
        }.items(),
        condition=UnlessCondition(LaunchConfiguration('disable_sensors'))
    )

    video = GroupAction(
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'realsense_streamer.launch.py')]),
                launch_arguments={ 'camera_name': 'camera_front'}.items(),
                condition=UnlessCondition(LaunchConfiguration('disable_realsense_front'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'realsense_streamer.launch.py')]),
                launch_arguments={'camera_name': 'camera_back'}.items(),
                condition=UnlessCondition(LaunchConfiguration('disable_realsense_back'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'qrcode_detection_server.launch.py')]),
                condition=UnlessCondition(LaunchConfiguration('disable_qrcode_detection'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'hazmat_detection_server.launch.py')]),
                condition=UnlessCondition(LaunchConfiguration('disable_hazmat_detection'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'paintroller_detection_server.launch.py')]),
                condition=UnlessCondition(LaunchConfiguration('disable_paintroller_detection'))
            )
        ],
        condition=UnlessCondition(LaunchConfiguration('disable_video'))
    )

    slam_nav = GroupAction(
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'slam_toolbox.launch.py')]),
                condition=UnlessCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'ohm_slam.launch.py')]),
                condition=IfCondition(LaunchConfiguration('ohm_slam'))
            ),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch', 'components', 'nav2.launch.py')]),
                condition=UnlessCondition(LaunchConfiguration('disable_nav'))
            )
        ],
        condition=IfCondition(PythonExpression([
            "'", LaunchConfiguration('disable_slam'), "' != 'true'",
            " and ",
            "'", LaunchConfiguration('disable_lidar'), "' != 'true'"
        ]))
    )

    return LaunchDescription([

        DeclareLaunchArgument(
            'disable_wheels',
            default_value='false',
            description='Whether to disable the wheels'
        ),
        DeclareLaunchArgument(
            'disable_arm',
            default_value='false',
            description='Whether to disable the robotic arm'
        ),
        DeclareLaunchArgument(
            'disable_lidar',
            default_value='false',
            description='Whether to disable the lidar'
        ),
        DeclareLaunchArgument(
            'disable_video',
            default_value='false',
            description='Whether to disable all video streaming and processing'
        ),
        DeclareLaunchArgument(
            'disable_realsense_front',
            default_value='false',
            description='Whether to disable the front camera'
        ),
        DeclareLaunchArgument(
            'disable_realsense_back',
            default_value='false',
            description='Whether to disable the back camera'
        ),
        DeclareLaunchArgument(
            'disable_qrcode_detection',
            default_value='false',
            description='Whether to disable qrcode detection'
        ),
        DeclareLaunchArgument(
            'disable_hazmat_detection',
            default_value='false',
            description='Whether to disable hazmat detection'
        ),
        DeclareLaunchArgument(
            'disable_paintroller_detection',
            default_value='false',
            description='Whether to disable paintroller detection'
        ),
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
            'disable_sensors',
            default_value='false',
            description='whether to disable all sensors'
        ),
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
        
        rsp,
        twist_mux,
        control,
        inverse_kinematics,
        lidar,
        sensors,
        video,
        slam_nav
    ])
