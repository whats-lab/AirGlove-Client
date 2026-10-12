# Hand data

The client returns hands in the OpenXR hand-joint convention (`XR_EXT_hand_tracking`: joint order, joint axes,
radius) **relative to the wrist** instead of in a reference space: the glove measures the fingers, not where the wrist
is. World pose = your wrist pose × joint pose.

## Joints

26 joints in OpenXR `XrHandJointEXT` order:

| # | joint | # | joint | # | joint |
|---|---|---|---|---|---|
| 0 | palm | 9 | index_distal | 18 | ring_intermediate |
| 1 | wrist | 10 | index_tip | 19 | ring_distal |
| 2 | thumb_metacarpal | 11 | middle_metacarpal | 20 | ring_tip |
| 3 | thumb_proximal | 12 | middle_proximal | 21 | little_metacarpal |
| 4 | thumb_distal | 13 | middle_intermediate | 22 | little_proximal |
| 5 | thumb_tip | 14 | middle_distal | 23 | little_intermediate |
| 6 | index_metacarpal | 15 | middle_tip | 24 | little_distal |
| 7 | index_proximal | 16 | ring_metacarpal | 25 | little_tip |
| 8 | index_intermediate | 17 | ring_proximal | | |

Each joint is `[qx, qy, qz, qw, px, py, pz, radius]` (C layout: 26 × 8 floats).

- Relative to the wrist: `wrist` is the identity. Place the hand in your world with your own wrist pose (headset hand
  tracking, a controller, a tracker).
- OpenXR hand-joint axes for both hands: −Z along the bone towards the fingertip, +Y on the back of the hand,
  X = Y × Z. Right-handed, metres.
- `radius` is the joint radius in metres (for capsule colliders or rendering).
- A joint can be reported invalid (`valid[j] = 0`); it is then the identity at the origin.

## Wrist orientation

**Not a world wrist pose.** Since airglove_client 0.4.0 the wrist value is the raw quaternion of the glove's
back-of-hand (dorsum) IMU exactly as Spine sends it: the sensor's `[w, x, y, z]` reordered to `[y, x, z, -w]`, not
normalised and not converted to OpenXR or engine axes (Unreal's `GetWristOrientation` returns these four values as
`FQuat(X, Y, Z, W)` without conversion). Its frame is the IMU's own; it arrives on every raw glove frame, independently
of the joints. Use it only together with your own sensor-to-world alignment.

## Engines

| Target | Position | Rotation |
|---|---|---|
| OpenXR / WebXR / ROS (right-handed) | as is | as is |
| Unity (left-handed, +Y up) | `(x, y, -z)` | `(x, y, -z, -w)` |
| Unreal (left-handed, +Z up, cm) | `100 × (-z, x, y)` | `(-qz, qx, qy, -qw)` |
| Unreal Live Link → imported Meta hand FBX bones | `(x, -y, z)` (m) | `(x, -y, z, -w)` |

## Freshness

Every read returns the age of the sample. Treat a hand as lost when it is older than about 0.2 s. `seq` counts the
frames this client received for that hand and `sender_time_us` is when it received them (Unix epoch µs); Spine sends
neither, so there is no reordering or loss detection.

## Haptics

`set_haptics(side, [thumb, index, middle, ring, pinky])` — exactly 5 values, each 0 (off), 1 (weak), 2 (medium),
3 (strong). The glove keeps vibrating until you send zeros, or until Spine has not heard the client's heartbeat for
3 s, when Spine turns the haptics off. `haptics_result(side)` reports whether the last request was applied (`False`
when that hand's glove is not connected or still busy with the previous command).
