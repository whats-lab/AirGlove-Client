from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("listen_port", default_value="4040"),
        DeclareLaunchArgument("rate_hz", default_value="90.0"),
        Node(package="airglove_client_ros2", executable="airglove_client_node", name="airglove_client", output="screen",
             parameters=[{"listen_port": LaunchConfiguration("listen_port"), "rate_hz": LaunchConfiguration("rate_hz")}]),
    ])
