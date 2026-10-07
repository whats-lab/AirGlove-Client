# AirGlove Client

Receive AirGlove hand tracking — 26 OpenXR hand joints per hand — from the Spine app and send finger haptics, from
Python, C/C++, C# (Unity), Unreal and ROS 2.

```
AirGlove gloves ── Spine app ── localhost ── libairglove_client ── Python · C/C++ · C#/Unity · Unreal · ROS 2
```

## Python

```bash
pip install airglove
```

```python
from airglove import Client, Side

with Client() as client:
    frame = client.hand(Side.RIGHT)
    if frame is not None and frame.age_s < 0.2:
        print(frame.positions[10])
    client.set_haptics(Side.RIGHT, [0, 2, 0, 0, 0])
```

Examples: `examples/python/`.

## C / C++

Header `include/airglove_client.h`, library from the
[Releases](https://github.com/whats-lab/AirGlove-Client/releases) (`linux-x64`, `linux-arm64`, `windows-x64`).

## Docs

- `docs/hand_data.md` — joint order, axes, engine conversions, haptics.

## License

Source and documentation: Apache-2.0 (`LICENSE`). The prebuilt `airglove_client` binaries: `LICENSE-BINARY.md`.
The hand model in `viz/` is derived from the Meta OpenXR hand (Oculus SDK License); see its notice.

Copyright © 2026 WHATs LAB Corp.
