import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction, RegisterEventHandler, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import Command, LaunchConfiguration
from launch.event_handlers import OnProcessStart
from launch.conditions import UnlessCondition

def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(package_dir, 'launch','rsp.launch.py')]),
        launch_arguments={ 
            'disable_wheels' : LaunchConfiguration('disable_wheels'),
            'disable_arm' : LaunchConfiguration('disable_arm'),
        }.items()
    )

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[os.path.join(package_dir,'config','controllers.yaml'),],
        remappings=[
            ('/tf', 'tf'),
            ('/trajectories', 'trajectories'),
            ('diff_drive_controller/cmd_vel', 'cmd_vel'),
            ('diff_drive_controller/odom', 'odom'),
            ('arm_position_controller/joint_trajectory', 'arm_joint_trajectory'),
            ('controller_manager/robot_description', 'robot_description')
        ],
    )

    delayed_controllers = RegisterEventHandler(
        event_handler=OnProcessStart(
            target_action=controller_manager,
            on_start=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        [os.path.join(package_dir,'launch','controllers.launch.py')]
                    ),
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

    arm_ik = Node(
        package="quac_control",
        executable="quac_ik_node",
        remappings=[
            ('/ee_pos', 'ee_pos'),
            ('/joint_commands', 'arm_joint_trajectory')
        ],
    )

    video_app = Node(
        package="quac_video",
        executable="video_app",
        parameters=[
            {'hazmat.model': os.path.join(package_dir, "models", "hazmat_yolo26.onnx")},
            {'hazmat.property_parameters.custom-lib-path': os.getenv("NVINVER_YOLO_LIB_PATH", "/invalid.so")},
            os.path.join(package_dir, "config", "video", "video_app_params.yaml"),
            os.path.join(package_dir, "config", "video", "nvinfer_params.yaml")
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
        
        PushRosNamespace('quac'),
        #rsp,
        #controller_manager,
        #delayed_controllers,
        #lidar,
        #arm_ik,
        video_app
    ])
