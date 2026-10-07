using System;
using System.Collections.Generic;
using System.Text;

namespace WhatsLab.AirGloveClient
{
    public enum HandSide
    {
        Left = 0,
        Right = 1,
    }

    public sealed class AirGloveClientException : Exception
    {
        public AirGloveClientException(string message) : base(message) { }
    }

    public sealed class HandFrame
    {
        public readonly float[] Joints = new float[AirGloveClientNative.HandFloats];
        public readonly bool[] Valid = new bool[AirGloveClientNative.JointCount];
        public int Sequence;
        public long SenderTimeUs;
        public double AgeSeconds;
    }

    public readonly struct Alarm
    {
        public readonly string Code;
        public readonly HandSide? Side;
        public Alarm(string code, HandSide? side) { Code = code; Side = side; }
    }

    public sealed class AirGloveClient : IDisposable
    {
        public const int JointCount = AirGloveClientNative.JointCount;
        public const int FingerCount = AirGloveClientNative.FingerCount;

        public static readonly string[] JointNames =
        {
            "Palm", "Wrist",
            "ThumbMetacarpal", "ThumbProximal", "ThumbDistal", "ThumbTip",
            "IndexMetacarpal", "IndexProximal", "IndexIntermediate", "IndexDistal", "IndexTip",
            "MiddleMetacarpal", "MiddleProximal", "MiddleIntermediate", "MiddleDistal", "MiddleTip",
            "RingMetacarpal", "RingProximal", "RingIntermediate", "RingDistal", "RingTip",
            "LittleMetacarpal", "LittleProximal", "LittleIntermediate", "LittleDistal", "LittleTip",
        };

        public static readonly int[] JointParents =
        {
            1, -1, 1, 2, 3, 4, 1, 6, 7, 8, 9, 1, 11, 12, 13, 14, 1, 16, 17, 18, 19, 1, 21, 22, 23, 24,
        };

        private IntPtr _handle;
        private readonly byte[] _valid = new byte[AirGloveClientNative.JointCount];

        public AirGloveClient(string listenAddress = "127.0.0.1", int listenPort = 4040, string spineAddress = "127.0.0.1", int spinePort = 4042)
        {
            int abi = AirGloveClientNative.agc_abi_version();
            if (abi != AirGloveClientNative.AbiVersion)
                throw new AirGloveClientException($"airglove_client ABI {abi}, this wrapper expects {AirGloveClientNative.AbiVersion}");
            _handle = AirGloveClientNative.agc_create(listenAddress, listenPort, spineAddress, spinePort);
            if (_handle == IntPtr.Zero)
                throw new AirGloveClientException(AirGloveClientNative.Str(AirGloveClientNative.agc_last_error()));
        }

        public static string Version => AirGloveClientNative.Str(AirGloveClientNative.agc_version());

        public bool TryGetHand(HandSide side, HandFrame frame)
        {
            int rc = AirGloveClientNative.agc_get_hand(_handle, (int)side, frame.Joints, frame.Joints.Length, _valid,
                out frame.Sequence, out frame.SenderTimeUs, out frame.AgeSeconds);
            if (rc < 0) throw Error();
            if (rc != AirGloveClientNative.Fresh) return false;
            for (int j = 0; j < _valid.Length; j++) frame.Valid[j] = _valid[j] != 0;
            return true;
        }

        public bool TryGetWrist(HandSide side, float[] orientationXyzw, out double ageSeconds)
        {
            int rc = AirGloveClientNative.agc_get_wrist(_handle, (int)side, orientationXyzw, out _, out _, out ageSeconds);
            if (rc < 0) throw Error();
            return rc == AirGloveClientNative.Fresh;
        }

        public bool TryGetDeviceStatus(out bool leftConnected, out bool rightConnected)
        {
            int rc = AirGloveClientNative.agc_get_device_status(_handle, out int l, out int r, out _);
            if (rc < 0) throw Error();
            leftConnected = l != 0;
            rightConnected = r != 0;
            return rc == AirGloveClientNative.Fresh;
        }

        public void SetHaptics(HandSide side, IReadOnlyList<int> strengths)
        {
            var values = new int[Math.Min(strengths.Count, FingerCount)];
            for (int i = 0; i < values.Length; i++) values[i] = strengths[i];
            if (AirGloveClientNative.agc_set_haptics(_handle, (int)side, values, values.Length) < 0) throw Error();
        }

        public bool TryGetHapticsResult(HandSide side, out bool success)
        {
            int rc = AirGloveClientNative.agc_get_haptics_result(_handle, (int)side, out int ok, out _);
            if (rc < 0) throw Error();
            success = ok != 0;
            return rc == AirGloveClientNative.Fresh;
        }

        public IEnumerable<Alarm> PollAlarms()
        {
            var code = new byte[64];
            while (AirGloveClientNative.agc_poll_alarm(_handle, code, code.Length, out int side) == AirGloveClientNative.Fresh)
            {
                int n = Array.IndexOf(code, (byte)0);
                yield return new Alarm(Encoding.ASCII.GetString(code, 0, n < 0 ? code.Length : n), side < 0 ? null : (HandSide?)side);
            }
        }

        public void SetStaleThreshold(double seconds) => AirGloveClientNative.agc_set_stale_threshold(_handle, seconds);

        private static AirGloveClientException Error() => new AirGloveClientException(AirGloveClientNative.Str(AirGloveClientNative.agc_last_error()));

        public void Dispose()
        {
            if (_handle != IntPtr.Zero)
            {
                AirGloveClientNative.agc_destroy(_handle);
                _handle = IntPtr.Zero;
            }
        }
    }
}
