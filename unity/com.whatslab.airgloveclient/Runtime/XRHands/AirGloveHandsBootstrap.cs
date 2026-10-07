using System.Collections.Generic;
using UnityEngine;
using UnityEngine.XR.Hands;
using UnityEngine.XR.Hands.ProviderImplementation;

namespace WhatsLab.AirGloveClient.XRHands
{
    [DefaultExecutionOrder(-1000)]
    public class AirGloveHandsBootstrap : MonoBehaviour
    {
        public string listenAddress = "127.0.0.1";
        public int listenPort = 4040;
        public string spineAddress = "127.0.0.1";
        public int spinePort = 4042;
        public Transform leftRoot;
        public Transform rightRoot;

        XRHandSubsystem _subsystem;
        XRHandProviderUtility.SubsystemUpdater _updater;

        public XRHandSubsystem Subsystem => _subsystem;

        void OnEnable()
        {
            AirGloveHandProvider.ListenAddress = listenAddress;
            AirGloveHandProvider.ListenPort = listenPort;
            AirGloveHandProvider.SpineAddress = spineAddress;
            AirGloveHandProvider.SpinePort = spinePort;
            UpdateRoots();
            var descriptors = new List<XRHandSubsystemDescriptor>();
            SubsystemManager.GetSubsystemDescriptors(descriptors);
            foreach (var d in descriptors)
            {
                if (d.id != AirGloveHandSubsystem.Id) continue;
                _subsystem = d.Create();
                break;
            }
            if (_subsystem == null)
            {
                Debug.LogError("AirGloveClient: XR Hands descriptor not registered");
                return;
            }
            _subsystem.Start();
            _updater = new XRHandProviderUtility.SubsystemUpdater(_subsystem);
            _updater.Start();
        }

        void Update() => UpdateRoots();

        void UpdateRoots()
        {
            if (leftRoot) AirGloveHandProvider.LeftRoot = new Pose(leftRoot.position, leftRoot.rotation);
            if (rightRoot) AirGloveHandProvider.RightRoot = new Pose(rightRoot.position, rightRoot.rotation);
        }

        void OnDisable()
        {
            _updater?.Stop();
            _updater?.Destroy();
            _updater = null;
            if (_subsystem != null)
            {
                _subsystem.Stop();
                _subsystem.Destroy();
                _subsystem = null;
            }
        }
    }
}
