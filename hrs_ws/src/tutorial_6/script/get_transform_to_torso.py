import motion
import argparse
from naoqi import ALProxy

def main(robotIP, PORT=9559):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)

    name  = 'CameraBottom'
    frame  = motion.FRAME_TORSO
    useSensorValues  = True
    result = motionProxy.getTransform(name, frame, useSensorValues)
    for i in range(0, 4):
        for j in range(0, 4):
            print result[4*i + j],
        print ''

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", type=str, default="10.152.246.134",
                        help="Robot ip address")
    parser.add_argument("--port", type=int, default=9559,
                        help="Robot port number")

    args = parser.parse_args()
    main(args.ip, args.port)