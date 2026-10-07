import numpy as np
import rclpy
from geometry_msgs.msg import Point, PoseArray
from rclpy.node import Node
from std_msgs.msg import ColorRGBA
from visualization_msgs.msg import Marker

from airglove_client.viz import MetaHand


class XrHandMarkers(Node):
    def __init__(self):
        super().__init__("airglove_client_xr_hand")
        self.declare_parameter("topic_prefix", "airglove_client")
        prefix = self.get_parameter("topic_prefix").value
        self.hands = {}
        self.pubs = {}
        for side in ("left", "right"):
            self.hands[side] = MetaHand(side)
            self.pubs[side] = self.create_publisher(Marker, f"{prefix}/{side}/xr_hand", 10)
            self.create_subscription(PoseArray, f"{prefix}/{side}/joints", lambda m, s=side: self.on_joints(s, m), 10)

    def on_joints(self, side, msg):
        if len(msg.poses) != 26:
            return
        q = np.array([[p.orientation.x, p.orientation.y, p.orientation.z, p.orientation.w] for p in msg.poses])
        x = np.array([[p.position.x, p.position.y, p.position.z] for p in msg.poses])
        hand = self.hands[side]
        v = hand.skin(q, x)[hand.faces.reshape(-1)]
        m = Marker()
        m.header = msg.header
        m.ns = "xr_hand"
        m.id = 0 if side == "left" else 1
        m.type = Marker.TRIANGLE_LIST
        m.action = Marker.ADD
        m.pose.orientation.w = 1.0
        m.scale.x = m.scale.y = m.scale.z = 1.0
        m.color = ColorRGBA(r=0.85, g=0.62, b=0.52, a=1.0)
        m.points = [Point(x=float(a), y=float(b), z=float(c)) for a, b, c in v]
        self.pubs[side].publish(m)


def main():
    rclpy.init()
    node = XrHandMarkers()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
