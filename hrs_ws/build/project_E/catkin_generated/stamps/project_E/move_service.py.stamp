#!/usr/bin/env python
import rospy
import time
import almath
import sys
from naoqi import ALProxy
import motion
import argparse
from project_E.srv import (MoveJoints, MoveInterpol, getPositions, getPositionsResponse, setPositions, getTransform, initPosition)
from geometry_msgs.msg import Pose2D

motionProxy =0
robotIP="10.152.246.151" 
PORT=9559

def check_limits(jointnames, angles):
    for id, jointname in enumerate(jointnames):
        if jointname == "HeadYaw":
            if abs(angles[id]) > 100:
                print("Joint limit violated on joint", jointname)
                return False
        if jointname == "HeadPitch":
            if angles[id] > 27 or angles[id] < -37:
                print("Joint limit violated on joint", jointname)
                return False

    return True


def handle_MoveJoints(req):
    
    motionProxy = ALProxy("ALMotion", robotIP, PORT)
    pNames = "Body"
    pStiffnessLists = 1.0
    pTimesList = 0.3
    motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    



    print("Moving with joint speed definition")
    joints = req.jointnames
    angles = req.angles
    speeds = req.speeds
    print(joints)
    print(angles)
    print(speeds)

    try:
        for i in range(len(joints)):
            #motionProxy.setStiffnesses(joints[i], 1.0)
            
            motionProxy.setAngles(joints[i],angles[i]*almath.TO_RAD,speeds[i])
            print('move ', joints[i],angles[i], 'angles', 'with speed:', speeds[i] )
            time.sleep(0.1)
            #motionProxy.setStiffnesses(joints[i], 0.0)
            
        success = True
    except Exception as e:
        print(e)
        success = False


    
    pNames = "Body"
    pStiffnessLists = 0.0
    pTimesList = 0.3
    motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    return success

def handle_MoveInterpol(req):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)
    pNames = "Body"
    pStiffnessLists = 1.0
    pTimesList = 0.5
    motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    print("Moving with time interpolation")
    joints = req.jointnames
    angles = req.angles
    times = req.times
    print(joints)
    print(angles)
    print(times)
    isAbsolute = True

    no_limit_violation=check_limits(joints, angles)
    if no_limit_violation==False:
        success = False
        return success

    try:
        for i in range(len(joints)):
            #motionProxy.setStiffnesses(joints[i], 1.0)
            
            print(joints[i])
            motionProxy.angleInterpolation(joints[i],angles[i]*almath.TO_RAD,times[i],True)
            print('move ', joints[i],angles[i], 'angles', 'with speed:', times[i] )
            print("almath.TO_RAD = ", almath.TO_RAD)
            time.sleep(0.1)
            #motionProxy.setStiffnesses(joints[i], 0.0)
            
        success = True
    except Exception as e:
        print(e)
        success = False
    pNames = "Body"
    pStiffnessLists = 0.0
    pTimesList = 0.5
    motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    return success


def handle_GetPosition(req):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)
    
    joint = req.jointname

    frame           = motion.FRAME_TORSO
    useSensorValues = True
    result          = motionProxy.getPosition(joint, frame, useSensorValues)
    print("Position of", joint, " in World is:" ,result)
    
    return getPositionsResponse(result)

def handle_SetPosition(req):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)
    
    joint = req.endeffector
    position = req.position
    orientation = req.orientation
    target = position + orientation
    speed = req.speed
    movement_time = req.time
    frame = motion.FRAME_TORSO
    success = True
    if orientation == (0,0,0):
        axisMask = 7
    else:
        print("Mask 63")
        axisMask = 63
    
    success == False
    if speed != 0 and movement_time == 0:
        motionProxy.setPositions(joint,frame, target, speed, axisMask)
        time.sleep(0.1)
        print("Moving based on speed")
        print(joint)
        print(frame)
        print(target)
        print(axisMask)
        print(movement_time)
        print("\n\n")
        success = True
    elif speed == 0 and movement_time != 0:
        motionProxy.positionInterpolations(joint,frame, target, axisMask, movement_time)
        time.sleep(0.1)
        print("Moving based on time")
        print(joint)
        print(frame)
        print(target)
        print(axisMask)
        print(movement_time)
        print("\n\n")
        success = True
    
    elif speed == 0 and movement_time == 0:
        print("Error: wrong input")
        return
    
    return success

def handle_GetTransform(req):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)

    jointname = req.jointname
    frame = req.reference
    useSensorValues  = True

    try:
        result = motionProxy.getTransform(jointname, frame, useSensorValues)
        for i in range(0, 4):
            for j in range(0, 4):
                print(result[4*i + j])
        success = True
    except Exception as e:
        print(e)
        success = False

    return result, success

def handle_InitPosition(req):
    #Initialize predefined postures proxy
    try:
        postureProxy = ALProxy("ALRobotPosture", robotIP, PORT)
    except Exception as e:
        print("Could not create proxy to ALRobotPosture")
        print("Error was: ", e)
        success = False
        return success

    #for i in range(5):
        #motion.setStiffnesses('Body', 1.0)

    postureProxy.goToPosture("StandInit", 1.0)
    success = True
    return success

if __name__ == '__main__':

    rospy.init_node('MoveJoints_server') #name of node
    speed_service = rospy.Service('MoveJoints', MoveJoints, handle_MoveJoints)
    time_service = rospy.Service('MoveInterpol', MoveInterpol, handle_MoveInterpol)
    position_service = rospy.Service('getPositions', getPositions, handle_GetPosition)
    set_position_service = rospy.Service('setPositions', setPositions, handle_SetPosition)
    get_transform_service = rospy.Service('getTransform', getTransform, handle_GetTransform)
    init_position = rospy.Service('initPosition', initPosition, handle_InitPosition)
    #walk_service = rospy.Service('Walk', Walk, handle_Walk)
    rospy.spin()

