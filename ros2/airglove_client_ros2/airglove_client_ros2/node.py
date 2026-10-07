import rclpy
from geometry_msgs.msg import Pose, PoseArray, QuaternionStamped, TransformStamped
from rclpy.node import Node
from std_msgs.msg import Bool, Float32MultiArray, Int32MultiArray, String
from tf2_ros import TransformBroadcaster

from airglove_client import JOINT_NAMES, Client, Side


class AirGloveClientNode(Node):
    def __init__(self):
        super().__init__("airglove_client")
        self.declare_parameter("listen_port", 4040)
        self.declare_parameter("spine_port", 4042)
        self.declare_parameter("rate_hz", 90.0)
        self.declare_parameter("stale_s", 0.2)
        self.declare_parameter("frame_prefix", "airglove_client_")
        self.declare_parameter("publish_tf", True)
        p = lambda name: self.get_parameter(name).value
        self.client = Client(listen_port=p("listen_port"), spine_port=p("spine_port"))
        self.client.set_stale_threshold(p("stale_s"))
        self.stale = p("stale_s")
        self.prefix = p("frame_prefix")
        self.tf = TransformBroadcaster(self) if p("publish_tf") else None
        self.last_seq = {Side.LEFT: None, Side.RIGHT: None}
        self.pub = {}
        for side in Side:
            n = side.name.lower()
            self.pub[side] = {
                "joints": self.create_publisher(PoseArray, f"airglove_client/{n}/joints", 10),
                "radii": self.create_publisher(Float32MultiArray, f"airglove_client/{n}/radii", 10),
                "wrist": self.create_publisher(QuaternionStamped, f"airglove_client/{n}/wrist_orientation", 10),
                "connected": self.create_publisher(Bool, f"airglove_client/{n}/connected", 10),
            }
            self.create_subscription(Int32MultiArray, f"airglove_client/{n}/haptics",
                                     lambda msg, s=side: self.client.set_haptics(s, list(msg.data)[:5]), 10)
        self.alarm_pub = self.create_publisher(String, "airglove_client/alarm", 10)
        self.create_timer(1.0 / p("rate_hz"), self.tick)
        self.create_timer(1.0, self.publish_status)
        self.get_logger().info(f"airglove client {Client.version()} on UDP {p('listen_port')}")

    def frame(self, side, joint):
        return f"{self.prefix}{side.name.lower()}_{joint}"

    def tick(self):
        stamp = self.get_clock().now().to_msg()
        for side in Side:
            frame = self.client.hand(side)
            if frame is None or frame.age_s > self.stale or frame.seq == self.last_seq[side]:
                continue
            self.last_seq[side] = frame.seq
            wrist_frame = self.frame(side, "wrist")
            msg = PoseArray()
            msg.header.stamp = stamp
            msg.header.frame_id = wrist_frame
            transforms = []
            for j, name in enumerate(JOINT_NAMES):
                pose = Pose()
                (pose.position.x, pose.position.y, pose.position.z) = map(float, frame.positions[j])
                (pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w) = map(float, frame.orientations[j])
                msg.poses.append(pose)
                if self.tf is not None and name != "wrist":
                    t = TransformStamped()
                    t.header.stamp = stamp
                    t.header.frame_id = wrist_frame
                    t.child_frame_id = self.frame(side, name)
                    t.transform.translation.x, t.transform.translation.y, t.transform.translation.z = pose.position.x, pose.position.y, pose.position.z
                    t.transform.rotation = pose.orientation
                    transforms.append(t)
            self.pub[side]["joints"].publish(msg)
            self.pub[side]["radii"].publish(Float32MultiArray(data=[float(r) for r in frame.radii]))
            if transforms:
                self.tf.sendTransform(transforms)
            wrist = self.client.wrist(side)
            if wrist is not None and wrist.age_s <= self.stale:
                q = QuaternionStamped()
                q.header.stamp = stamp
                q.header.frame_id = f"{self.prefix}imu_world"
                q.quaternion.x, q.quaternion.y, q.quaternion.z, q.quaternion.w = map(float, wrist.orientation)
                self.pub[side]["wrist"].publish(q)
        for alarm in self.client.alarms():
            side = alarm.side.name.lower() if alarm.side is not None else "unknown"
            self.alarm_pub.publish(String(data=f"{alarm.code} {side}"))

    def publish_status(self):
        status = self.client.device_status()
        if status is None:
            return
        self.pub[Side.LEFT]["connected"].publish(Bool(data=status.left_connected))
        self.pub[Side.RIGHT]["connected"].publish(Bool(data=status.right_connected))

    def destroy_node(self):
        self.client.close()
        super().destroy_node()


def main():
    rclpy.init()
    node = AirGloveClientNode()
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
