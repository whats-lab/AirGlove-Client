using System;
using System.Runtime.InteropServices;

namespace WhatsLab.AirGloveClient
{
    internal static class AirGloveClientNative
    {
        private const string Lib = "airglove_client";

        public const int AbiVersion = 1;
        public const int JointCount = 26;
        public const int JointFloats = 8;
        public const int HandFloats = JointCount * JointFloats;
        public const int FingerCount = 5;
        public const int Fresh = 1;

        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_abi_version();
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr agc_version();
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr agc_last_error();
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr agc_create(string listenAddress, int listenPort, string spineAddress, int spinePort);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern void agc_destroy(IntPtr client);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_get_hand(IntPtr client, int side, [Out] float[] joints, int capacity, [Out] byte[] valid, out int seq, out long senderTimeUs, out double ageS);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_get_wrist(IntPtr client, int side, [Out] float[] orientationXyzw, out int seq, out long senderTimeUs, out double ageS);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_get_device_status(IntPtr client, out int left, out int right, out double ageS);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_get_spine_alive(IntPtr client, out double ageS);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_set_haptics(IntPtr client, int side, int[] strengths, int count);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_get_haptics_result(IntPtr client, int side, out int success, out double ageS);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern int agc_poll_alarm(IntPtr client, byte[] code, int capacity, out int side);
        [DllImport(Lib, CallingConvention = CallingConvention.Cdecl)] public static extern void agc_set_stale_threshold(IntPtr client, double seconds);

        public static string Str(IntPtr p) => p == IntPtr.Zero ? string.Empty : Marshal.PtrToStringAnsi(p) ?? string.Empty;
    }
}
