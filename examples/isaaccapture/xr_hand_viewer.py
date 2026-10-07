import argparse
import sys
import time

import numpy as np
import viser
from isaaccapture.cloudxr import CloudXRLauncher
from isaaccapture.retargeting_engine.deviceio_source_nodes import HandsSource
from isaaccapture.retargeting_engine.interface import BaseRetargeter, OptionalType, TensorGroupType
from isaaccapture.retargeting_engine.tensor_types import FloatType, HandInput, HandInputIndex
from isaaccapture.teleop_session_manager import TeleopSession, TeleopSessionConfig

from airglove_client.viz import MetaHand

SIDES = ("left", "right")


class HandPoses(BaseRetargeter):
    def input_spec(self):
        return {HandsSource.LEFT: OptionalType(HandInput()), HandsSource.RIGHT: OptionalType(HandInput())}

    def output_spec(self):
        return {side: TensorGroupType(side, [FloatType(f"{side}_{k}") for k in range(26 * 7 + 1)]) for side in SIDES}

    def _compute_fn(self, inputs, outputs, context):
        for side, key in zip(SIDES, (HandsSource.LEFT, HandsSource.RIGHT)):
            hand = inputs[key]
            out = outputs[side]
            if hand.is_none:
                for k in range(26 * 7 + 1):
                    out[k] = 0.0
                continue
            pos = np.asarray(hand[HandInputIndex.JOINT_POSITIONS], dtype=np.float32)
            quat = np.asarray(hand[HandInputIndex.JOINT_ORIENTATIONS], dtype=np.float32)
            out[0] = 1.0
            flat = np.concatenate([quat, pos], axis=1).reshape(-1)
            for k, v in enumerate(flat):
                out[1 + k] = float(v)


def build_pipeline():
    hands = HandsSource(name="hands")
    return HandPoses(name="hand_poses").connect(
        {HandsSource.LEFT: hands.output(HandsSource.LEFT), HandsSource.RIGHT: hands.output(HandsSource.RIGHT)})


class SkinnedHand:
    def __init__(self, server, side, offset):
        self.hand = MetaHand(side)
        R, p = self.hand.bind_R, self.hand.bind_p
        self.offset = np.asarray(offset, dtype=float)
        self.handle = server.scene.add_mesh_skinned(
            f"/xr_hand/{side}", vertices=self.hand.vertices + self.offset, faces=self.hand.faces,
            bone_wxyzs=np.stack([_wxyz(r) for r in R]), bone_positions=p + self.offset,
            skin_weights=self.hand.weights, color=(220, 170, 150))

    def update(self, quat_xyzw, pos):
        R, p = self.hand.bones(quat_xyzw, pos)
        for k, bone in enumerate(self.handle.bones):
            bone.wxyz = _wxyz(R[k])
            bone.position = p[k] + self.offset


def _wxyz(R):
    w = np.sqrt(max(0.0, 1.0 + R[0, 0] + R[1, 1] + R[2, 2])) / 2.0
    if w > 1e-6:
        return np.array([w, (R[2, 1] - R[1, 2]) / (4 * w), (R[0, 2] - R[2, 0]) / (4 * w), (R[1, 0] - R[0, 1]) / (4 * w)])
    i = int(np.argmax(np.diag(R)))
    j, k = (i + 1) % 3, (i + 2) % 3
    s = np.sqrt(max(0.0, 1.0 + R[i, i] - R[j, j] - R[k, k])) * 2.0
    q = np.zeros(4)
    q[1 + i] = s / 4.0
    q[0] = (R[k, j] - R[j, k]) / s
    q[1 + j] = (R[j, i] + R[i, j]) / s
    q[1 + k] = (R[k, i] + R[i, k]) / s
    return q


def main(argv):
    parser = argparse.ArgumentParser(description="IsaacCapture OpenXR hands -> Meta XR hand mesh (viser)")
    parser.add_argument("--port", type=int, default=8081)
    parser.add_argument("--record", help="save received poses (npz) for comparison")
    parser.add_argument("--duration", type=float, default=0.0)
    CloudXRLauncher.add_launcher_arguments(parser)
    args = parser.parse_args(argv[1:])

    server = viser.ViserServer(host="127.0.0.1", port=args.port)
    server.scene.set_up_direction("+y")

    @server.on_client_connect
    def _(client):
        client.camera.position = (0.0, 0.22, 0.30)
        client.camera.look_at = (0.0, -0.02, -0.08)
        client.camera.up_direction = (0.0, 1.0, 0.0)

    hands = {"left": SkinnedHand(server, "left", (-0.12, 0.0, 0.0)), "right": SkinnedHand(server, "right", (0.12, 0.0, 0.0))}
    log = {s: [] for s in SIDES}
    with CloudXRLauncher.launch_context(args):
        with TeleopSession(TeleopSessionConfig(app_name="AirGloveXrHandViewer", pipeline=build_pipeline())) as session:
            print(f"[xr_hand] viser on http://localhost:{args.port}", flush=True)
            start = time.monotonic()
            try:
                while not args.duration or time.monotonic() - start < args.duration:
                    result = session.step()
                    for side in SIDES:
                        data = np.asarray(list(result[side]), dtype=np.float64)
                        if data[0] < 0.5:
                            continue
                        jp = data[1:].reshape(26, 7)
                        hands[side].update(jp[:, 0:4], jp[:, 4:7])
                        log[side].append((time.monotonic(), jp))
                    time.sleep(1 / 90)
            except KeyboardInterrupt:
                pass
    if args.record:
        np.savez(args.record, **{f"{s}_t": np.array([t for t, _ in log[s]]) for s in SIDES},
                 **{s: np.array([j for _, j in log[s]]) for s in SIDES})
        print(f"[xr_hand] recorded {[len(log[s]) for s in SIDES]} -> {args.record}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
