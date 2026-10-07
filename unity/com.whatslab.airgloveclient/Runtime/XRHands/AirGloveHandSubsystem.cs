using Unity.Collections;
using UnityEngine;
using UnityEngine.XR;
using UnityEngine.XR.Hands;
using UnityEngine.XR.Hands.ProviderImplementation;

namespace WhatsLab.AirGloveClient.XRHands
{
    public class AirGloveHandSubsystem : XRHandSubsystem
    {
        public const string Id = "AirGloveClient-Hands";

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.SubsystemRegistration)]
        static void RegisterDescriptor()
        {
            XRHandSubsystemDescriptor.Register(new XRHandSubsystemDescriptor.Cinfo
            {
                id = Id,
                providerType = typeof(AirGloveHandProvider),
                subsystemTypeOverride = typeof(AirGloveHandSubsystem),
            });
        }
    }

    public class AirGloveHandProvider : XRHandSubsystemProvider
    {
        public static string ListenAddress = "127.0.0.1";
        public static int ListenPort = 4040;
        public static string SpineAddress = "127.0.0.1";
        public static int SpinePort = 4042;
        public static float StaleSeconds = 0.2f;
        public static Pose LeftRoot = new Pose(new Vector3(-0.12f, 1.2f, 0.35f), Quaternion.identity);
        public static Pose RightRoot = new Pose(new Vector3(0.12f, 1.2f, 0.35f), Quaternion.identity);

        public static AirGloveClient Client { get; private set; }

        readonly HandFrame _frame = new HandFrame();

        public override void Start()
        {
            if (Client != null) return;
            Client = new AirGloveClient(ListenAddress, ListenPort, SpineAddress, SpinePort);
            Client.SetStaleThreshold(StaleSeconds);
        }

        public override void Stop()
        {
            Client?.Dispose();
            Client = null;
        }

        public override void Destroy() => Stop();

        public override void GetHandLayout(NativeArray<bool> handJointsInLayout)
        {
            for (int i = 0; i < handJointsInLayout.Length; i++) handJointsInLayout[i] = true;
        }

        public override XRHandSubsystem.UpdateSuccessFlags TryUpdateHands(
            XRHandSubsystem.UpdateType updateType,
            ref Pose leftHandRootPose, NativeArray<XRHandJoint> leftHandJoints,
            ref Pose rightHandRootPose, NativeArray<XRHandJoint> rightHandJoints)
        {
            var flags = XRHandSubsystem.UpdateSuccessFlags.None;
            if (Client == null) return flags;
            if (Fill(HandSide.Left, Handedness.Left, LeftRoot, ref leftHandRootPose, leftHandJoints))
                flags |= XRHandSubsystem.UpdateSuccessFlags.LeftHandRootPose | XRHandSubsystem.UpdateSuccessFlags.LeftHandJoints;
            if (Fill(HandSide.Right, Handedness.Right, RightRoot, ref rightHandRootPose, rightHandJoints))
                flags |= XRHandSubsystem.UpdateSuccessFlags.RightHandRootPose | XRHandSubsystem.UpdateSuccessFlags.RightHandJoints;
            return flags;
        }

        bool Fill(HandSide side, Handedness handedness, Pose root, ref Pose rootPose, NativeArray<XRHandJoint> joints)
        {
            if (!Client.TryGetHand(side, _frame) || _frame.AgeSeconds > StaleSeconds) return false;
            for (int j = 0; j < AirGloveClient.JointCount; j++)
            {
                Pose local = OpenXRToUnity.JointPose(_frame.Joints, j);
                Pose world = local.GetTransformedBy(root);
                var id = j == 0 ? XRHandJointID.Palm : j == 1 ? XRHandJointID.Wrist : (XRHandJointID)(j + 1);
                var state = _frame.Valid[j] ? XRHandJointTrackingState.Pose | XRHandJointTrackingState.Radius : XRHandJointTrackingState.None;
                joints[id.ToIndex()] = XRHandProviderUtility.CreateJoint(handedness, state, id, world, _frame.Joints[j * 8 + 7]);
            }
            rootPose = OpenXRToUnity.JointPose(_frame.Joints, 1).GetTransformedBy(root);
            return true;
        }
    }
}
