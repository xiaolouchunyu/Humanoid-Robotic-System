#!/usr/bin/env python
import rospy
import time
import almath
import sys
from naoqi import ALProxy
import argparse
from nao_control_tutorial_1.srv import (MoveJoints, MoveInterpol)


 
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

def usage():
    return "%s [x y]"%sys.argv[0]
   
if __name__ == "__main__":
    func_name = sys.argv[1]

    if len(sys.argv)>=3:    
        move_type = sys.argv[2]

    if len(sys.argv) == 6:
        jointname = sys.argv[3]
        angle = float(sys.argv[4])
        speed = float(sys.argv[5])
    
    if func_name == "MoveJoint":
        success = moveJoints_server_client(move_type, jointname, angle, speed)

    elif func_name == "ArmsUp":
        success = arms_up()

    elif func_name == "Task2":
        success = task2()
    
    else:
        print("No valid function selected")

    print("After service call")
    print("Success = ", success)