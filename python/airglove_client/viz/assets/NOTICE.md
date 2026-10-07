# Meta OpenXR hand model — license notice

`left.glb`, `right.glb` in this directory are converted from `OpenXRLeftHand.fbx` /
`OpenXRRightHand.fbx` of the **Meta XR Core SDK** (`com.meta.xr.sdk.core@74.0.0`,
`Meshes/HandTracking/`), skeleton = OpenXR hand joints (`XRHand_*`).

Copyright (c) Meta Platforms, Inc. and affiliates. All rights reserved.

Licensed under the **Oculus SDK License Agreement**:
https://developer.oculus.com/licenses/oculussdk/

This is **not** a Creative Commons asset. Redistribution terms are those of the Oculus SDK
License Agreement; check them before shipping these files outside the team.

## Changes made

- Converted FBX → glTF binary (`.glb`) with `tools/meta_fbx_to_glb.py` (assimp export).
- Unit scale 0.01 on the root node (cm → m).
- Mesh, skin weights, bone hierarchy and bind poses are otherwise unmodified.

The same mesh and bind pose also exist as the CC BY 4.0 Oculus Hand Models 1.0
(`OculusHand_{L,R}.fbx`, legacy `b_{l,r}_*` skeleton); `tools/meta_fbx_to_glb.py` converts only the OpenXR
(`XRHand_*`) FBX used here.

