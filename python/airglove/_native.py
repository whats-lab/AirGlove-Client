import ctypes
import os
import platform
import sys
from pathlib import Path

ABI_VERSION = 1


def _platform_dir():
    machine = platform.machine().lower()
    arch = "arm64" if machine in ("aarch64", "arm64") else "x64"
    if sys.platform.startswith("linux"):
        return f"linux-{arch}", "libairglove_client.so"
    if sys.platform == "win32":
        return f"windows-{arch}", "airglove_client.dll"
    if sys.platform == "darwin":
        return "macos-universal", "libairglove_client.dylib"
    raise OSError(f"unsupported platform: {sys.platform}")


def _find_library():
    override = os.environ.get("AIRGLOVE_CLIENT_LIBRARY")
    if override:
        return Path(override)
    folder, name = _platform_dir()
    path = Path(__file__).parent / "_lib" / folder / name
    if not path.exists():
        raise OSError(f"AirGlove client library not found at {path}; set AIRGLOVE_CLIENT_LIBRARY")
    return path


def _bind(lib):
    c_int, c_double, c_float = ctypes.c_int, ctypes.c_double, ctypes.c_float
    p_int, p_double = ctypes.POINTER(c_int), ctypes.POINTER(c_double)
    p_i32, p_i64, p_u64 = ctypes.POINTER(ctypes.c_int32), ctypes.POINTER(ctypes.c_int64), ctypes.POINTER(ctypes.c_uint64)
    p_float, p_u8 = ctypes.POINTER(c_float), ctypes.POINTER(ctypes.c_uint8)
    handle = ctypes.c_void_p
    signatures = {
        "agc_abi_version": (c_int, []),
        "agc_version": (ctypes.c_char_p, []),
        "agc_last_error": (ctypes.c_char_p, []),
        "agc_create": (handle, [ctypes.c_char_p, c_int, ctypes.c_char_p, c_int]),
        "agc_destroy": (None, [handle]),
        "agc_get_hand": (c_int, [handle, c_int, p_float, c_int, p_u8, p_i32, p_i64, p_double]),
        "agc_get_wrist": (c_int, [handle, c_int, p_float, p_i32, p_i64, p_double]),
        "agc_get_device_status": (c_int, [handle, p_int, p_int, p_double]),
        "agc_get_spine_alive": (c_int, [handle, p_double]),
        "agc_set_haptics": (c_int, [handle, c_int, p_int, c_int]),
        "agc_request_haptics": (c_int, [handle, c_int]),
        "agc_get_haptics": (c_int, [handle, c_int, p_int, c_int, p_double]),
        "agc_get_haptics_result": (c_int, [handle, c_int, p_int, p_double]),
        "agc_poll_alarm": (c_int, [handle, ctypes.c_char_p, c_int, p_int]),
        "agc_set_stale_threshold": (None, [handle, c_double]),
        "agc_get_stats": (None, [handle, p_u64, p_u64, p_u64]),
    }
    for name, (restype, argtypes) in signatures.items():
        fn = getattr(lib, name)
        fn.restype = restype
        fn.argtypes = argtypes
    return lib


_lib = None


def lib():
    global _lib
    if _lib is None:
        loaded = _bind(ctypes.CDLL(str(_find_library())))
        abi = loaded.agc_abi_version()
        if abi != ABI_VERSION:
            raise OSError(f"AirGlove client library ABI {abi}, this package expects {ABI_VERSION}")
        _lib = loaded
    return _lib
