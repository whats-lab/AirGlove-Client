# AirGlove Client

Receive AirGlove hand tracking from the Spine app — 26 OpenXR hand joints per hand — and send finger haptics, from
Python, C/C++, Unity, Unreal and NVIDIA IsaacCapture.

```
AirGlove gloves ── Spine app ──(localhost)── airglove_client ── Python · C/C++ · Unity · Unreal · IsaacCapture
```

| Use | Install | Guide |
|---|---|---|
| Python | `pip install airglove-client` | [python/README.md](python/README.md) |
| Unity 2021.3+ / Unity 6 | Package Manager → *Add package from git URL* → `https://github.com/whats-lab/AirGlove-Client.git?path=/unity/com.whatslab.airgloveclient` | [unity/…/README.md](unity/com.whatslab.airgloveclient/README.md) |
| Unreal Engine 5 | copy `unreal/AirGloveClient` into your project's `Plugins/` | [unreal/AirGloveClient/README.md](unreal/AirGloveClient/README.md) |
| C / C++ | `include/airglove_client.h` + `native/<platform>/` | [below](#c--c) |
| NVIDIA IsaacCapture | `-DBUILD_PLUGIN_AIRGLOVE=ON` (fetches this repository) | IsaacCapture `docs/source/device/airglove.rst` |

The Spine app must be running with hand output enabled, on the same machine. The client listens on UDP 4040 and
talks to Spine on 4042; only one client per machine can listen on 4040.

**Compatibility:** airglove_client 0.4.0 speaks Spine's current OSC format — every message starts with its message
number, hand and device messages carry a `/pair/N` address prefix — reported as protocol **3**. It needs a Spine
release that sends this format; earlier Spine builds (protocol v2 and v1) do not interoperate with 0.4.0, and 0.3.x and
older clients do not work with the new Spine. The client sends a heartbeat every second; `spine_protocol()` (Python),
`TryGetSpineProtocol` (Unity), `GetSpineProtocol` (Unreal) and `agc_get_spine_protocol` (C) report the version from
Spine's last reply (`3`), or nothing when Spine does not answer.

**Pair:** the client talks to one Spine pair (default 1; Spine currently has only pair 1, both gloves). Choose another
with `Client(pair=N)` / `set_pair(N)` (Python), the `pair` constructor argument / `SetPair` (Unity, `AirGloveHandProvider.Pair`
for XR Hands), the *Pair* setting / `SetPair` (Unreal) or `agc_set_pair` (C). Changing the pair forgets the hand,
wrist, device status and haptics state received for the old one.

**Sequence and time:** Spine no longer sends a sequence number or a send time. `seq` is the client's count of joint
frames received for that hand (1, 2, …; the wrist has its own counter) and `sender_time_us` is the time the client
received the datagram (Unix epoch µs). `dropped_seq` in the stats is always 0 and the stale threshold setting
(`set_stale_threshold`, `SetStaleThreshold`, `agc_set_stale_threshold`) has no effect.

## Hand data

Each hand is 26 joints in OpenXR `XrHandJointEXT` order, each a pose plus a radius, **relative to the wrist**
(the wrist joint is the identity) and in OpenXR hand-joint axes: −Z along the bone towards the fingertip, +Y on the
back of the hand, right-handed, metres. Place the hand in your world with your own wrist pose (headset hand
tracking, a controller, a tracker). Engine integrations convert to their own space. Details, joint table and
engine conversions: [docs/hand_data.md](docs/hand_data.md).

**Wrist value:** `wrist()` / `TryGetWrist` / `GetWristOrientation` / `agc_get_wrist` return the glove's **raw back-of-hand
IMU quaternion** as Spine sends it — the sensor's wxyz reordered to `[y, x, z, -w]`, passed through unchanged. It is
not a wrist pose in any world or engine frame (and Unreal no longer converts it to Unreal axes); do not use it to place
the hand without your own sensor-to-world alignment.

## C / C++

```c
#include <airglove_client.h>

agc_client* client = agc_create("127.0.0.1", 4040, "127.0.0.1", 4042);
float joints[AGC_HAND_FLOATS];
uint8_t valid[AGC_JOINT_COUNT];
double age;
if (agc_get_hand(client, AGC_RIGHT, joints, AGC_HAND_FLOATS, valid, NULL, NULL, &age) == AGC_FRESH && age < 0.2) {
    /* joints[j * 8 + 0..3] = qx qy qz qw, [4..6] = px py pz, [7] = radius */
}
int strengths[5] = {0, 2, 0, 0, 0};
agc_set_haptics(client, AGC_RIGHT, strengths, 5);
agc_destroy(client);
```

Link `native/<platform>/` (`libairglove_client.so`, `airglove_client.dll` + `.lib`, `libairglove_client.dylib`).
Platforms: linux-x64, linux-arm64, windows-x64, windows-arm64, macos-universal. Also attached to each
[release](https://github.com/whats-lab/AirGlove-Client/releases).

## Haptics

`set_haptics(side, [thumb, index, middle, ring, pinky])` — exactly 5 values, each 0 (off), 1 (weak), 2 (medium),
3 (strong); any other count is rejected. The glove keeps vibrating until you send zeros, or until Spine has not heard
the client's heartbeat for 3 s (the client exited or hung), when Spine turns the haptics off.

## License

Source and documentation: Apache-2.0 ([LICENSE](LICENSE)). Prebuilt `airglove_client` binaries:
[LICENSE-BINARY.md](LICENSE-BINARY.md). The Meta OpenXR hand model under `python/airglove_client/viz/assets` is
derived from the Meta XR SDK hand (Oculus SDK License, see its NOTICE).

Copyright © 2026 WHATs LAB Corp. (주식회사 왓츠랩)
