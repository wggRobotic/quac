# Copyright (c) 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.descriptions import ParameterFile
from nav2_common.launch import RewrittenYaml


def generate_launch_description():
    package_dir = get_package_share_directory('quac')

    remappings = [('/tf', 'tf'), ('/tf_static', 'tf_static'), ('/clock', 'clock')]

    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=os.path.join(package_dir, 'config', 'nav2_params.yaml'),
            param_rewrites={
                'use_sim_time': LaunchConfiguration('sim_mode'),
                'autostart': 'true'
            },
            convert_types=True),
        allow_substs=True)

    load_nodes = GroupAction(
        actions=[
            Node(
                package='nav2_controller',
                executable='controller_server',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav')]),
            Node(
                package='nav2_smoother',
                executable='smoother_server',
                name='smoother_server',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav2')]),
            Node(
                package='nav2_planner',
                executable='planner_server',
                name='planner_server',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav2')]),
            Node(
                package='nav2_behaviors',
                executable='behavior_server',
                name='behavior_server',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav2')]),
            Node(
                package='nav2_bt_navigator',
                executable='bt_navigator',
                name='bt_navigator',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav2')]),
            Node(
                package='nav2_waypoint_follower',
                executable='waypoint_follower',
                name='waypoint_follower',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings + [('cmd_vel', 'cmd_vel_nav2')]),
            Node(
                package='nav2_velocity_smoother',
                executable='velocity_smoother',
                name='velocity_smoother',
                namespace='quac',
                output='screen',
                respawn=False,
                parameters=[configured_params],
                arguments=['--ros-args', '--log-level', 'info'],
                remappings=remappings +
                        [('cmd_vel', 'cmd_vel_nav'), ('cmd_vel_smoothed', 'cmd_vel_nav2')]),
            Node(
                package='nav2_lifecycle_manager',
                executable='lifecycle_manager',
                name='lifecycle_manager_navigation',
                namespace='quac',
                output='screen',
                arguments=['--ros-args', '--log-level', 'info'],
                parameters=[
                    {'use_sim_time': LaunchConfiguration('sim_mode'),},
                    {'autostart': True},
                    {'node_names': [
                        'controller_server',
                        'smoother_server',
                        'planner_server',
                        'behavior_server',
                        'bt_navigator',
                        'waypoint_follower',
                        'velocity_smoother'
                    ]}
                ]
            ),
        ]
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'sim_mode',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),
        SetEnvironmentVariable('RCUTILS_LOGGING_BUFFERED_STREAM', '1'),

        load_nodes,
    ])