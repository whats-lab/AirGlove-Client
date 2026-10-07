import ctypes
from dataclasses import dataclass
from enum import IntEnum

import numpy as np

from ._native import lib

JOINT_COUNT = 26
FINGER_COUNT = 5

JOINT_NAMES = (
    "palm", "wrist",
    "thumb_metacarpal", "thumb_proximal", "thumb_distal", "thumb_tip",
    "index_metacarpal", "index_proximal", "index_intermediate", "index_distal", "index_tip",
    "middle_metacarpal", "middle_proximal", "middle_intermediate", "middle_distal", "middle_tip",
    "ring_metacarpal", "ring_proximal", "ring_intermediate", "ring_distal", "ring_tip",
    "little_metacarpal", "little_proximal", "little_intermediate", "little_distal", "little_tip",
)

FINGER_NAMES = ("thumb", "index", "middle", "ring", "pinky")


class Side(IntEnum):
    LEFT = 0
    RIGHT = 1


class AirGloveError(RuntimeError):
    pass


@dataclass(frozen=True)
class HandFrame:
    orientations: np.ndarray
    positions: np.ndarray
    radii: np.ndarray
    valid: np.ndarray
    seq: int
    sender_time_us: int
    age_s: float


@dataclass(frozen=True)
class WristFrame:
    orientation: np.ndarray
    seq: int
    sender_time_us: int
    age_s: float


@dataclass(frozen=True)
class DeviceStatus:
    left_connected: bool
    right_connected: bool
    age_s: float


@dataclass(frozen=True)
class Alarm:
    code: str
    side: "Side | None"


def _side(side):
    return int(Side(side))


def _error():
    return AirGloveError(lib().agc_last_error().decode(errors="replace"))


class Client:
    def __init__(self, listen_address="127.0.0.1", listen_port=4040, spine_address="127.0.0.1", spine_port=4042):
        self._lib = lib()
        handle = self._lib.agc_create(listen_address.encode(), listen_port, spine_address.encode(), spine_port)
        if not handle:
            raise _error()
        self._handle = ctypes.c_void_p(handle)
        self._joints = (ctypes.c_float * (JOINT_COUNT * 8))()
        self._valid = (ctypes.c_uint8 * JOINT_COUNT)()

    def close(self):
        if getattr(self, "_handle", None):
            self._lib.agc_destroy(self._handle)
            self._handle = None

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()

    def __del__(self):
        self.close()

    @staticmethod
    def version():
        return lib().agc_version().decode()

    def hand(self, side):
        seq, t_us, age = ctypes.c_int32(), ctypes.c_int64(), ctypes.c_double()
        rc = self._lib.agc_get_hand(self._handle, _side(side), self._joints, len(self._joints), self._valid,
                                    ctypes.byref(seq), ctypes.byref(t_us), ctypes.byref(age))
        if rc < 0:
            raise _error()
        if rc == 0:
            return None
        data = np.ctypeslib.as_array(self._joints).reshape(JOINT_COUNT, 8).copy()
        return HandFrame(data[:, 0:4], data[:, 4:7], data[:, 7], np.ctypeslib.as_array(self._valid).astype(bool),
                         seq.value, t_us.value, age.value)

    def wrist(self, side):
        q = (ctypes.c_float * 4)()
        seq, t_us, age = ctypes.c_int32(), ctypes.c_int64(), ctypes.c_double()
        rc = self._lib.agc_get_wrist(self._handle, _side(side), q, ctypes.byref(seq), ctypes.byref(t_us),
                                     ctypes.byref(age))
        if rc < 0:
            raise _error()
        if rc == 0:
            return None
        return WristFrame(np.array(q, dtype=np.float32), seq.value, t_us.value, age.value)

    def device_status(self):
        left, right, age = ctypes.c_int(), ctypes.c_int(), ctypes.c_double()
        rc = self._lib.agc_get_device_status(self._handle, ctypes.byref(left), ctypes.byref(right), ctypes.byref(age))
        if rc < 0:
            raise _error()
        return DeviceStatus(bool(left.value), bool(right.value), age.value) if rc else None

    def spine_alive_age(self):
        age = ctypes.c_double()
        rc = self._lib.agc_get_spine_alive(self._handle, ctypes.byref(age))
        if rc < 0:
            raise _error()
        return age.value if rc else None

    def set_haptics(self, side, strengths):
        values = list(strengths)
        array = (ctypes.c_int * len(values))(*values)
        if self._lib.agc_set_haptics(self._handle, _side(side), array, len(values)) < 0:
            raise _error()

    def request_haptics(self, side):
        if self._lib.agc_request_haptics(self._handle, _side(side)) < 0:
            raise _error()

    def haptics(self, side):
        values, age = (ctypes.c_int * FINGER_COUNT)(), ctypes.c_double()
        rc = self._lib.agc_get_haptics(self._handle, _side(side), values, FINGER_COUNT, ctypes.byref(age))
        if rc < 0:
            raise _error()
        return (tuple(values), age.value) if rc else None

    def haptics_result(self, side):
        success, age = ctypes.c_int(), ctypes.c_double()
        rc = self._lib.agc_get_haptics_result(self._handle, _side(side), ctypes.byref(success), ctypes.byref(age))
        if rc < 0:
            raise _error()
        return (bool(success.value), age.value) if rc else None

    def alarms(self):
        out = []
        code, side = ctypes.create_string_buffer(64), ctypes.c_int()
        while self._lib.agc_poll_alarm(self._handle, code, len(code), ctypes.byref(side)) == 1:
            out.append(Alarm(code.value.decode(errors="replace"), Side(side.value) if side.value >= 0 else None))
        return out

    def set_stale_threshold(self, seconds):
        self._lib.agc_set_stale_threshold(self._handle, float(seconds))

    def stats(self):
        d, m, s = ctypes.c_uint64(), ctypes.c_uint64(), ctypes.c_uint64()
        self._lib.agc_get_stats(self._handle, ctypes.byref(d), ctypes.byref(m), ctypes.byref(s))
        return {"datagrams": d.value, "malformed": m.value, "dropped_seq": s.value}
