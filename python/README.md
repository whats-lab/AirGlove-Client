# airglove-client

Receive AirGlove hand tracking — 26 OpenXR hand joints per hand, wrist-relative — from the Spine app and send finger
haptics. The wheel bundles the native library for linux-x64/arm64, windows-x64/arm64 and macOS.

Needs a Spine that sends its current OSC format (message-number header, `/pair/N`), reported as protocol 3
(`airglove_client.SPINE_PROTOCOL`); older Spine builds (protocol v2, v1) are not supported. Set
`AIRGLOVE_CLIENT_LIBRARY` to use another build of the native library.

```bash
pip install airglove-client          # client
pip install "airglove-client[viz]"   # + browser hand viewer (viser)
```

```python
from airglove_client import Client, Side, JOINT_NAMES

with Client() as client:                       # UDP 4040, Spine requests on 4042, pair 1
    frame = client.hand(Side.RIGHT)            # None until the first sample
    if frame is not None and frame.age_s < 0.2:
        tip = JOINT_NAMES.index("index_tip")
        print(frame.positions[tip], frame.orientations[tip])   # metres, quaternion xyzw
    client.set_haptics(Side.RIGHT, [0, 2, 0, 0, 0])
```

| Call | Returns |
|---|---|
| `Client(..., pair=1)`, `set_pair(n)`, `pair` | Spine pair (`/pair/N`, ≥ 1); changing it forgets the old pair's data |
| `hand(side)` | `HandFrame`: `orientations (26,4)` xyzw, `positions (26,3)` m, `radii (26,)`, `valid (26,)`, `seq` (frames received for this hand), `sender_time_us` (client receive time, epoch µs), `age_s` — or `None` |
| `wrist(side)` | `WristFrame`: raw back-of-hand IMU quaternion as Spine sends it, `[y, x, z, -w]` of the sensor's wxyz — **not** a world wrist pose — or `None` |
| `device_status()` | `left_connected`, `right_connected` |
| `spine_protocol()` | `(version, age_s)` from Spine's last heartbeat reply (`3`) — or `None` before the first |
| `spine_alive_age()` | seconds since Spine's last heartbeat reply — or `None` |
| `set_haptics(side, [5 × 0..3])`, `haptics_result(side)` | send exactly 5 strengths (else `ValueError`) / last result |
| `alarms()` | device alarms since the last call: `code` (`RESTART`, `ATLAS_DEVICE_ERROR`), `side` (`Side`, or `None` when Spine names neither hand) |
| `stats()` | datagram counters (`dropped_seq` is always 0) |

`airglove_client.viz.MetaHand` skins the Meta OpenXR hand mesh from a `HandFrame` (numpy only).

Examples: [`examples/python`](https://github.com/whats-lab/AirGlove-Client/tree/master/examples/python) —
`print_hands.py`, `haptics.py`, `view_hands.py` (browser 3D view).
