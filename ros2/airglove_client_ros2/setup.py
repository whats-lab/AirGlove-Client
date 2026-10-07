from setuptools import setup

setup(
    name="airglove_client_ros2",
    version="0.1.0",
    packages=["airglove_client_ros2"],
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/airglove_client_ros2"]),
        ("share/airglove_client_ros2", ["package.xml"]),
        ("share/airglove_client_ros2/launch", ["launch/airglove_client.launch.py", "launch/xr_hand.launch.py"]),
        ("share/airglove_client_ros2/rviz", ["rviz/xr_hand.rviz"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    license="Apache-2.0",
    entry_points={"console_scripts": ["airglove_client_node = airglove_client_ros2.node:main", "airglove_client_xr_hand = airglove_client_ros2.xr_hand:main"]},
)
