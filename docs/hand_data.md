# Hand data

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
  tracking, a controller, a tracker), or with the glove's wrist IMU orientation below.
- OpenXR hand-joint axes for both hands: −Z along the bone towards the fingertip, +Y on the back of the hand,
  X = Y × Z. Right-handed, metres.
- `radius` is the joint radius in metres (for capsule colliders or rendering).
- A joint can be reported invalid (`valid[j] = 0`); it is then the identity at the origin.

## Wrist orientation

The glove's IMU wrist orientation `[qx, qy, qz, qw]` in a world frame with +Y up. The heading (rotation about +Y) is
whatever the IMU reports and is not aligned to anything; use it for tilt, or align it yourself.

## Engines

| Target | Position | Rotation |
|---|---|---|
| OpenXR / WebXR / ROS (right-handed) | as is | as is |
| Unity (left-handed, +Y up) | `(x, y, -z)` | `(x, y, -z, -w)` |
| Unreal (left-handed, +Z up, cm) | `100 × (-z, x, y)` | `(-qz, qx, qy, -qw)` |

## Freshness

Every read returns the age of the sample. Treat a hand as lost when it is older than about 0.2 s; the client drops
out-of-order samples and recovers on its own when the sender restarts.

## Haptics

`set_haptics(side, [thumb, index, middle, ring, pinky])`, each 0 (off), 1 (weak), 2 (medium), 3 (strong). The glove
keeps vibrating until you send zeros. `haptics_result(side)` reports whether the last request was applied.
