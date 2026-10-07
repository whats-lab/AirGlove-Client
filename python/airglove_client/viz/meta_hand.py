import json
import struct
from pathlib import Path

import numpy as np

_COMPONENT = {5120: np.int8, 5121: np.uint8, 5122: np.int16, 5123: np.uint16, 5125: np.uint32, 5126: np.float32}
_WIDTH = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}
_FINGERS = (("Thumb", 2, ("Metacarpal", "Proximal", "Distal")),
            ("Index", 6, ("Metacarpal", "Proximal", "Intermediate", "Distal")),
            ("Middle", 11, ("Metacarpal", "Proximal", "Intermediate", "Distal")),
            ("Ring", 16, ("Metacarpal", "Proximal", "Intermediate", "Distal")),
            ("Little", 21, ("Metacarpal", "Proximal", "Intermediate", "Distal")))
DRIVEN_BONES = {"XRHand_Wrist": 1}
for _finger, _base, _segments in _FINGERS:
    for _k, _segment in enumerate(_segments):
        DRIVEN_BONES[f"XRHand_{_finger}{_segment}"] = _base + _k


def _read_glb(path):
    data = Path(path).read_bytes()
    gltf, binary, off = None, b"", 12
    while off < len(data):
        length, kind = struct.unpack_from("<II", data, off)
        chunk = data[off + 8:off + 8 + length]
        if kind == 0x4E4F534A:
            gltf = json.loads(chunk)
        elif kind == 0x004E4942:
            binary = chunk
        off += 8 + length
    return gltf, binary


def _accessor(gltf, binary, index):
    acc = gltf["accessors"][index]
    view = gltf["bufferViews"][acc["bufferView"]]
    dtype = np.dtype(_COMPONENT[acc["componentType"]])
    width = _WIDTH[acc["type"]]
    offset = view.get("byteOffset", 0) + acc.get("byteOffset", 0)
    stride = view.get("byteStride", 0) or dtype.itemsize * width
    raw = np.frombuffer(binary, dtype=np.uint8, count=stride * (acc["count"] - 1) + dtype.itemsize * width, offset=offset)
    rows = np.lib.stride_tricks.as_strided(raw, shape=(acc["count"], dtype.itemsize * width), strides=(stride, 1))
    arr = np.ascontiguousarray(rows).view(dtype).reshape(acc["count"], width)
    if acc.get("normalized") and dtype.kind in "ui":
        arr = arr.astype(np.float64) / np.iinfo(dtype).max
    return arr


def _quat_matrix(q):
    x, y, z, w = q
    return np.array([[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
                     [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
                     [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]])


def _local(node):
    if "matrix" in node:
        return np.asarray(node["matrix"], dtype=float).reshape(4, 4).T
    T = np.eye(4)
    T[:3, :3] = _quat_matrix(node.get("rotation", [0, 0, 0, 1])) * np.asarray(node.get("scale", [1, 1, 1]))
    T[:3, 3] = node.get("translation", [0, 0, 0])
    return T


def _rigid(T):
    U, _, Vt = np.linalg.svd(T[:3, :3])
    out = np.eye(4)
    out[:3, :3] = U @ Vt
    out[:3, 3] = T[:3, 3]
    return out


class MetaHand:
    def __init__(self, side, path=None):
        path = path or Path(__file__).parent / "assets" / f"{'left' if str(side).lower().endswith('left') or side == 0 else 'right'}.glb"
        gltf, binary = _read_glb(path)
        nodes = gltf["nodes"]
        parent = [-1] * len(nodes)
        for i, n in enumerate(nodes):
            for c in n.get("children", []):
                parent[c] = i
        world = {}

        def w(i):
            if i not in world:
                world[i] = _local(nodes[i]) if parent[i] < 0 else w(parent[i]) @ _local(nodes[i])
            return world[i]

        mesh_node = next(i for i, n in enumerate(nodes) if "mesh" in n and "skin" in n)
        skin = gltf["skins"][nodes[mesh_node]["skin"]]
        joints = skin["joints"]
        slot = {j: k for k, j in enumerate(joints)}
        prim = gltf["meshes"][nodes[mesh_node]["mesh"]]["primitives"][0]
        attrs = prim["attributes"]
        ibm = _accessor(gltf, binary, skin["inverseBindMatrices"]).reshape(-1, 4, 4).transpose(0, 2, 1)
        skin_mat = w(joints[0]) @ ibm[0]
        pos = _accessor(gltf, binary, attrs["POSITION"]).astype(np.float64) @ skin_mat[:3, :3].T + skin_mat[:3, 3]
        self.faces = _accessor(gltf, binary, prim["indices"]).reshape(-1, 3).astype(np.uint32)
        jidx = _accessor(gltf, binary, attrs["JOINTS_0"]).astype(np.int64)
        jw = _accessor(gltf, binary, attrs["WEIGHTS_0"]).astype(np.float64)
        weights = np.zeros((len(pos), len(joints)))
        np.add.at(weights, (np.arange(len(pos))[:, None].repeat(jidx.shape[1], 1), jidx), jw)
        self.weights = weights / np.maximum(weights.sum(axis=1, keepdims=True), 1e-12)
        bind = np.stack([_rigid(w(j)) for j in joints])
        self.bone_names = [nodes[j].get("name", f"bone{j}") for j in joints]
        self.bone_parents = []
        for j in joints:
            p = parent[j]
            while p >= 0 and p not in slot:
                p = parent[p]
            self.bone_parents.append(slot.get(p, -1))
        root = bind[self.bone_names.index("XRHand_Wrist")]
        C = root[:3, :3].T
        t = -C @ root[:3, 3]
        self.bind_R = np.einsum("ij,njk->nik", C, bind[:, :3, :3])
        self.bind_p = bind[:, :3, 3] @ C.T + t
        self.vertices = pos @ C.T + t
        self.joint_of = [DRIVEN_BONES.get(n, -1) for n in self.bone_names]
        order, seen = [], set()

        def visit(k):
            if k in seen:
                return
            if self.bone_parents[k] >= 0:
                visit(self.bone_parents[k])
            seen.add(k)
            order.append(k)

        for k in range(len(joints)):
            visit(k)
        self.order = order

    def bones(self, orientations, positions):
        n = len(self.bone_names)
        R = np.zeros((n, 3, 3))
        p = np.zeros((n, 3))
        for k in self.order:
            j, par = self.joint_of[k], self.bone_parents[k]
            if j >= 0:
                R[k] = np.stack([_quat_matrix(orientations[j])])[0]
                p[k] = positions[j]
            elif par < 0:
                R[k], p[k] = self.bind_R[k], self.bind_p[k]
            else:
                rel = R[par] @ self.bind_R[par].T
                R[k] = rel @ self.bind_R[k]
                p[k] = p[par] + rel @ (self.bind_p[k] - self.bind_p[par])
        return R, p

    def skin(self, orientations, positions):
        R, p = self.bones(orientations, positions)
        M = np.einsum("nij,nkj->nik", R, self.bind_R)
        shift = p - np.einsum("nij,nj->ni", M, self.bind_p)
        per_bone = np.einsum("nij,vj->nvi", M, self.vertices) + shift[:, None, :]
        return np.einsum("vn,nvi->vi", self.weights, per_bone)


def matrix_to_wxyz(R):
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
