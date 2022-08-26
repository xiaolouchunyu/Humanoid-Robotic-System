#!/usr/bin/env python
import rospy
import time
import almath
import sys
from naoqi import ALProxy
import argparse
from tutorial_6.srv import (MoveJoints, MoveInterpol, getPositions, setPositions)

CLI = argparse.ArgumentParser()
CLI.add_argument(
    "--joint",
    nargs=1,
    default = [],
)
CLI.add_argument(
    "--position",
    nargs="*",
    type=float,
    default = [],
)
CLI.add_argument(
    "--orientation",
    nargs="*",
    type=float,
    default = [],
)
CLI.add_argument(
    "--speed",
    nargs=1,
    type=float,
    default = [],
)
CLI.add_argument(
    "--time",
    nargs=1,
    type=float,
    default = [],
)
 
def moveJoints_server_client(move_type,jointname,angle,speed):
    print("Moving joints")
    jointnames = [jointname]
    angles = [angle]
    speeds = [speed]
    rospy.wait_for_service('MoveJoints')
    rospy.wait_for_service('MoveInterpol')
    try:
        if move_type == "speeds":
            move_joints = rospy.ServiceProxy('MoveJoints', MoveJoints)
            resp1 = move_joints(jointnames,angles,speeds)
            return resp1.success

        elif move_type == "times":
            move_joints = rospy.ServiceProxy('MoveInterpol', MoveInterpol)
            resp1 = move_joints(jointnames,angles,speeds)
            return resp1.success
        
        else:
            print('No valid movement type')
            raise ValueError('No valid movement type')

    except rospy.ServiceException as e:
        print("Service call failed: %s"%e)
        return False

def arms_up():
    print("Moving arms up")
    jointnames = ["LShoulderPitch", "RShoulderPitch"]
    angles = [-90, -90]
    speed = [0.2, 0.2]
    #times = [2, 2]

    rospy.wait_for_service('MoveJoints')
    try:
        move_joints = rospy.ServiceProxy('MoveJoints', MoveJoints)
        #move_joints = rospy.ServiceProxy('MoveInterpol', MoveInterpol)
        resp1 = move_joints(jointnames,angles,speed)
        return resp1.success
    except rospy.ServiceException as e:
        print("Service call failed: %s"%e)
        return False

def task2():
    print("Updating left shoulder joints")
    jointnames = ["LShoulderRoll", "LShoulderPitch"]
    angles = [30, 0]
    times = [2.0, 1.0]

    #rospy.wait_for_service('MoveJoints')
    rospy.wait_for_service('MoveInterpol')
    try:
        move_joints = rospy.ServiceProxy('MoveInterpol', MoveInterpol)
        resp1 = move_joints(jointnames,angles,times)
        return resp1.success
    except rospy.ServiceException as e:
        print("Service call failed: %s"%e)
        return False

def setPositions(effector,position,orientation,speed,time):
    rospy.wait_for_service('setPositions')
    try:
        set_positions = rospy.ServiceProxy('setPositions',setPositions)
        resp1 = set_positions(effector,position,orientation,speed,time)
        return resp1.success
    except rospy.ServiceException as e:
        print("Service call failed: %s"%e)
        return False



def usage():
    return "%s [x y]"%sys.argv[0]
   
if __name__ == "__main__":
    args = CLI.parse_args()
 
    end = args.joint
    position = args.position
    orientation = args.orientation
    speed = args.speed
    time = args.time
    
    success = setPositions(end,position,orientation,speed,time)

    print("After service call")
    print("Success = ", success)