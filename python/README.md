# airglove-client

Receive AirGlove hand tracking — 26 OpenXR hand joints per hand, wrist-relative — from the Spine app and send finger
haptics. The wheel bundles the native `airglove_client` library for linux-x64/arm64, windows-x64/arm64 and macOS.

```bash
pip install airglove-client
```

```python
from airglove_client import Client, Side

with Client() as client:
    frame = client.hand(Side.RIGHT)
    if frame is not None and frame.age_s < 0.2:
        print(frame.positions[10])
    client.set_haptics(Side.RIGHT, [0, 2, 0, 0, 0])
```

Hand data conventions: https://github.com/whats-lab/AirGlove-Client/blob/master/docs/hand_data.md
