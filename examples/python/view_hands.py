import argparse
import time

import numpy as np
import viser

from airglove_client import Client, Side
from airglove_client.viz import MetaHand, matrix_to_wxyz


class HandView:
    def __init__(self, server, side, offset):
        self.hand = MetaHand(side.name.lower())
        self.offset = np.asarray(offset, dtype=float)
        self.handle = server.scene.add_mesh_skinned(
            f"/{side.name.lower()}", vertices=self.hand.vertices + self.offset, faces=self.hand.faces,
            bone_wxyzs=np.stack([matrix_to_wxyz(r) for r in self.hand.bind_R]),
            bone_positions=self.hand.bind_p + self.offset, skin_weights=self.hand.weights, color=(220, 170, 150))

    def update(self, frame):
        R, p = self.hand.bones(frame.orientations, frame.positions)
        for k, bone in enumerate(self.handle.bones):
            bone.wxyz = matrix_to_wxyz(R[k])
            bone.position = p[k] + self.offset


def main():
    parser = argparse.ArgumentParser(description="Show AirGlove hands as Meta OpenXR hand meshes in the browser")
    parser.add_argument("--port", type=int, default=4040)
    parser.add_argument("--viewer-port", type=int, default=8080)
    args = parser.parse_args()

    server = viser.ViserServer(port=args.viewer_port)
    server.scene.set_up_direction("+y")
    views = {Side.LEFT: HandView(server, Side.LEFT, (-0.12, 0, 0)), Side.RIGHT: HandView(server, Side.RIGHT, (0.12, 0, 0))}
    with Client(listen_port=args.port) as client:
        print(f"open http://localhost:{args.viewer_port}")
        while True:
            for side, view in views.items():
                frame = client.hand(side)
                if frame is not None and frame.age_s < 0.2:
                    view.update(frame)
            time.sleep(1 / 60)


if __name__ == "__main__":
    main()
