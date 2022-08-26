#!/usr/bin/env python2
import rospy
import time
import almath
import sys
from naoqi import ALProxy
import motion 
import argparse
import numpy as np

from nao_control_tutorial_2.srv import (MoveJoints, MoveInterpol,getPositions,setPositions)
motionProxy =0;


def handle_MoveJoints(req):
    print("Moving with joint speed definition")
    joints = req.jointnames
    angles = req.angles
    speeds = req.speeds
    print(joints)
    print(angles)
    print(speeds)

    try:
        for i in range(len(joints)):
            motionProxy.setStiffnesses(joints[i], 1.0)
            motionProxy.setAngles(joints[i],angles[i]*almath.TO_RAD,speeds[i])
            motionProxy.setStiffnesses(joints[i], 0.0)
        success = True
    except:
        success = False
    time.sleep(3.0)
    
    return success

def handle_getPositions(req):
    print("Moving with joint speed definition")
    joint = req.jointnames
    name            = "CameraTop"
    frame           = motion.FRAME_WORLD
    useSensorValues = True
    req.ef          = motionProxy.getPosition(name, frame, useSensorValues)
    print("Position of", name, " in World is:")
    print(req.ef)
    return req.ef

def handle_MoveInterpol(req):
    print("Moving with time interpolation")
    joints = req.jointnames
    angles = req.angles
    times = req.times
    print(joints)
    print(angles)
    print(times)
    isAbsolute = True

    try:
        for i in range(len(joints)):
            motionProxy.setStiffnesses(joints[i], 1.0)
            motionProxy.angleInterpolation(joints[i],angles[i]*almath.TO_RAD, times[i],isAbsolute)
            motionProxy.setStiffnesses(joints[i], 0.0)
        success = True
    except:
        success = False
    time.sleep(0.5)
    
    return success

def handle_setPositions(req):
    success = 0
    frame     = motion.FRAME_TORSO
    if req.speed == True:
        endeffector = req.endeffector
        fractionMaxSpeed = req.speed
        position = req.position 
        if len(req.orientation)>0: 
            axisMask         = 63
            orientation = req.orientation
        else:
            axisMask = 7
            orientation = np.empty((3,1), float)
        target = np.concatenate((position, orientation), axis=0)
        motionProxy.setPositions(endeffector, frame, target, fractionMaxSpeed, axisMask)
    else: 
        endeffector = req.endeffector
        position = req.position
        time = req.time
        if len(req.orientation)>0: 
            axisMask         = 63
            orientation = req.orientation
        else:
            axisMask = 7
            orientation = np.empty((3,1), float)
        motionProxy.positionInterpolations(endeffector, frame, orientation,
                                 axisMask, time)    
    success = True
        

    
    return success

#def moveJoints_server():
#    rospy.init_node('MoveJoints_server') #name of node
#    speed_service = rospy.Service('MoveJoints', MoveJoints, handle_MoveJoints)
#    time_service = rospy.Service('MoveInterpol', MoveInterpol, handle_MoveInterpol)
#    rospy.spin()


if __name__ == '__main__':
    robotIP="10.152.246.188" 
    PORT=9559
    motionProxy = ALProxy("ALMotion", robotIP, PORT)

    rospy.init_node('MoveJoints_node') #name of node
    speed_service = rospy.Service('MoveJoints', MoveJoints, handle_MoveJoints)
    time_service = rospy.Service('MoveInterpol', MoveInterpol, handle_MoveInterpol)
    getpositions_service = rospy.Service('MoveInterpol', getPositions, handle_getPositions)
    setPositions_service = rospy.Service('MoveInterpol', setPositions, handle_setPositions)


    rospy.spin()


