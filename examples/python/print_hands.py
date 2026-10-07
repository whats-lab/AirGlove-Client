import argparse
import time

from airglove_client import JOINT_NAMES, Client, Side


def main():
    parser = argparse.ArgumentParser(description="Print AirGlove hand joints received from Spine")
    parser.add_argument("--port", type=int, default=4040)
    parser.add_argument("--rate", type=float, default=2.0)
    args = parser.parse_args()

    with Client(listen_port=args.port) as client:
        print(f"airglove client {Client.version()} listening on UDP {args.port}")
        while True:
            status = client.device_status()
            if status:
                print(f"glove connected: left={status.left_connected} right={status.right_connected}")
            for side in Side:
                frame = client.hand(side)
                if frame is None or frame.age_s > 0.2:
                    print(f"{side.name.lower():5s}: no data")
                    continue
                tip = JOINT_NAMES.index("index_tip")
                x, y, z = frame.positions[tip]
                print(f"{side.name.lower():5s}: seq={frame.seq} age={frame.age_s * 1000:.0f}ms "
                      f"index_tip=({x:+.3f}, {y:+.3f}, {z:+.3f}) m")
            for alarm in client.alarms():
                print(f"alarm: {alarm.code} ({alarm.side.name.lower() if alarm.side is not None else '-'})")
            time.sleep(1.0 / args.rate)


if __name__ == "__main__":
    main()
