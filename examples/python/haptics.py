import argparse
import time

from airglove import Client, Side


def main():
    parser = argparse.ArgumentParser(description="Vibrate AirGlove fingers through Spine")
    parser.add_argument("--side", choices=["left", "right"], default="right")
    parser.add_argument("--strength", type=int, nargs=5, default=[2, 2, 2, 2, 2], metavar="S",
                        help="thumb index middle ring pinky, 0 (off) .. 3 (strong)")
    parser.add_argument("--duration", type=float, default=1.0)
    parser.add_argument("--port", type=int, default=4040)
    args = parser.parse_args()
    side = Side[args.side.upper()]

    with Client(listen_port=args.port) as client:
        client.set_haptics(side, args.strength)
        time.sleep(0.3)
        print("result:", client.haptics_result(side))
        time.sleep(max(0.0, args.duration - 0.3))
        client.set_haptics(side, [0, 0, 0, 0, 0])
        time.sleep(0.3)
        print("off result:", client.haptics_result(side))


if __name__ == "__main__":
    main()
