import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, RegisterEventHandler, DeclareLaunchArgument, GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch.event_handlers import OnProcessStart
from launch.conditions import UnlessCondition, IfCondition

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch','rsp.launch.py')]),
        launch_arguments={ 
            'disable_wheels' : LaunchConfiguration('disable_wheels'),
            'disable_arm' : LaunchConfiguration('disable_arm')
        }.items()
    )

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[os.path.join(package_dir,'config','control.yaml'),],
        remappings=[
            ('/tf', 'tf'),
            ('/trajectories', 'trajectories'),
            ('diff_drive_controller/cmd_vel_unstamped', 'cmd_vel'),
            ('diff_drive_controller/odom', 'odom'),
            ('arm_position_controller/joint_trajectory', 'arm_joint_trajectory'),
            ('controller_manager/robot_description', 'robot_description')
        ],
    )

    quac_common = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        [os.path.join(package_dir,'launch','quac_common.launch.py')]
                    ),
                    launch_arguments={ 
                        'disable_nav' : LaunchConfiguration('disable_nav'),
                        'ohm_slam' : LaunchConfiguration('ohm_slam'),
                        'disable_slam' : PythonExpression(["'",
                            LaunchConfiguration('disable_slam'), "' == 'true' or '",
                            LaunchConfiguration('disable_lidar'), "' == 'true'"
                        ])
                    }.items()
                )  
            ],
        )
    )

    wheel_monitor = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                Node(
                    package="controller_manager",
                    executable="spawner",
                    arguments=["wheel_monitor"],
                )
            ],
        )
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('rplidar_ros'),'launch','rplidar_a2m8_launch.py')
        ),
        launch_arguments={'frame_id': 'laser', 'serial_port': '/dev/quac/lidar_uart'}.items(),
        condition=UnlessCondition(LaunchConfiguration('disable_lidar'))
    )

    sensors = GroupAction(
        actions=[
            Node(
                package="quac_sensor",
                executable="thermal_cam",
                condition=UnlessCondition(LaunchConfiguration('disable_thermal_cam'))
            ),
            Node(
                package="quac_sensor",
                executable="imu",
                condition=UnlessCondition(LaunchConfiguration('disable_imu'))
            ),
            Node(
                package="quac_sensor",
                executable="magnetometer",
                condition=UnlessCondition(LaunchConfiguration('disable_magnetometer'))
            ),
        ],
        condition=UnlessCondition(LaunchConfiguration('disable_sensors'))
    )

    video_app = Node(
        package="quac_video",
        executable="video_app",
        parameters=[
            {'hazmat.model': os.path.join(package_dir, "models", "hazmat_yolo26.onnx")},
            {'hazmat.property_parameters.custom-lib-path': os.getenv("NVINVER_YOLO_LIB_PATH", "/invalid.so")},
            os.path.join(package_dir, "config", "video", "video_app_params.yaml"),
            os.path.join(package_dir, "config", "video", "nvinfer_params.yaml"),
            {'hazmat.regenerate': LaunchConfiguration('regenerate_hazmat')},
            {'paintroller.regenerate': LaunchConfiguration('regenerate_paintroller')}
        ],
        condition=UnlessCondition(LaunchConfiguration('disable_video'))
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
            description='Whether to disable the video streaming and processing'
        ),

        DeclareLaunchArgument(
            'regenerate_hazmat',
            default_value='false',
            description='Whether to regenerate the tensorrt hazmat engine'
        ),

        DeclareLaunchArgument(
            'regenerate_paintroller',
            default_value='false',
            description='Whether to regenerate the tensorrt paintroller engine'
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
            description='whether to disable all sensors'
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
        
        PushRosNamespace('quac'),
        rsp,
        controller_manager,
        quac_common,
        wheel_monitor,
        lidar,
        sensors,
        video_app
    ])
