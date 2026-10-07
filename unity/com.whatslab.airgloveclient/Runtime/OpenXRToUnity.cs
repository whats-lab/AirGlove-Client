using UnityEngine;

namespace WhatsLab.AirGloveClient
{
    public static class OpenXRToUnity
    {
        public static Vector3 Position(float x, float y, float z) => new Vector3(x, y, -z);

        public static Quaternion Rotation(float qx, float qy, float qz, float qw) => new Quaternion(-qx, -qy, qz, qw);

        public static Pose JointPose(float[] joints, int joint)
        {
            int o = joint * 8;
            return new Pose(Position(joints[o + 4], joints[o + 5], joints[o + 6]),
                Rotation(joints[o], joints[o + 1], joints[o + 2], joints[o + 3]));
        }
    }
}
