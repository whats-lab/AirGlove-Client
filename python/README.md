# airglove-client

Receive AirGlove hand tracking — 26 OpenXR hand joints per hand, wrist-relative — from the Spine app and send finger
haptics. The wheel bundles the native library for linux-x64/arm64, windows-x64/arm64 and macOS.

```bash
pip install airglove-client          # client
pip install "airglove-client[viz]"   # + browser hand viewer (viser)
```

```python
from airglove_client import Client, Side, JOINT_NAMES

with Client() as client:                       # UDP 4040, Spine requests on 4042
    frame = client.hand(Side.RIGHT)            # None until the first sample
    if frame is not None and frame.age_s < 0.2:
        tip = JOINT_NAMES.index("index_tip")
        print(frame.positions[tip], frame.orientations[tip])   # metres, quaternion xyzw
    client.set_haptics(Side.RIGHT, [0, 2, 0, 0, 0])
```

| Call | Returns |
|---|---|
| `hand(side)` | `HandFrame`: `orientations (26,4)` xyzw, `positions (26,3)` m, `radii (26,)`, `valid (26,)`, `seq`, `age_s` — or `None` |
| `wrist(side)` | glove IMU wrist orientation (world, +Y up, unaligned heading) — or `None` |
| `device_status()` | `left_connected`, `right_connected` |
| `set_haptics(side, [5 × 0..3])`, `haptics_result(side)` | send / last result |
| `alarms()` | device alarms since the last call (`RESTART`, `ATLAS_DEVICE_ERROR`) |
| `stats()` | datagram counters |

`airglove_client.viz.MetaHand` skins the Meta OpenXR hand mesh from a `HandFrame` (numpy only).

Examples: [`examples/python`](https://github.com/whats-lab/AirGlove-Client/tree/master/examples/python) —
`print_hands.py`, `haptics.py`, `view_hands.py` (browser 3D view).
