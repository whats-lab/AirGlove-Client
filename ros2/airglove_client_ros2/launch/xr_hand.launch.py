import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

OPENXR_TO_ROS = ["--qx", "0.5", "--qy", "-0.5", "--qz", "-0.5", "--qw", "0.5"]


def generate_launch_description():
    share = get_package_share_directory("airglove_client_ros2")
    return LaunchDescription([
        DeclareLaunchArgument("listen_port", default_value="4040"),
        DeclareLaunchArgument("spine_port", default_value="4042"),
        Node(package="airglove_client_ros2", executable="airglove_client_node", name="airglove_client",
             parameters=[{"listen_port": LaunchConfiguration("listen_port"), "spine_port": LaunchConfiguration("spine_port")}]),
        Node(package="airglove_client_ros2", executable="airglove_client_xr_hand", name="airglove_client_xr_hand"),
        Node(package="tf2_ros", executable="static_transform_publisher", name="left_wrist_tf",
             arguments=["--y", "0.12"] + OPENXR_TO_ROS + ["--frame-id", "world", "--child-frame-id", "airglove_client_left_wrist"]),
        Node(package="tf2_ros", executable="static_transform_publisher", name="right_wrist_tf",
             arguments=["--y", "-0.12"] + OPENXR_TO_ROS + ["--frame-id", "world", "--child-frame-id", "airglove_client_right_wrist"]),
        Node(package="rviz2", executable="rviz2", name="rviz2", arguments=["-d", os.path.join(share, "rviz", "xr_hand.rviz")]),
    ])
