from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from pathlib import Path


def generate_launch_description():
    params_file = Path.home() / "workspace/openarm_ws/src/dxl_wt_openarm_leader/config/params.yaml" #sorce側から読む
    # params_file = Path(get_package_share_directory("dxl_wt_openarm_leader")) / "config" / "params.yaml" #install側から読む

    return LaunchDescription([
        Node(
            package="dxl_wt_openarm_leader",
            executable="read_write_wt_position_node",
            name="read_write_wt_node",
            parameters=[str(params_file)],
            output="screen",
        )
    ])
