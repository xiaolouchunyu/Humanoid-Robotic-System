#!/usr/bin/env python2
import rospy
import time
import almath
import sys
from naoqi import ALProxy
import argparse
from nao_control_tutorial_1.srv import (MoveJoints, MoveInterpol, KillTask)
motionProxy =0;
robotIP="10.152.246.134" 
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
    #pNames = "Body"
    #pStiffnessLists = 1.0
    #pTimesList = 0.3
    #motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    



    print("Moving with joint speed definition")
    joints = req.jointnames
    angles = req.angles
    speeds = req.speeds

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


    
    #pNames = "Body"
    #pStiffnessLists = 0.0
    #pTimesList = 0.3
    #motionProxy.stiffnessInterpolation(pNames,pStiffnessLists,pTimesList)
    return success

def handle_MoveInterpol(req):
    motionProxy = ALProxy("ALMotion", robotIP, PORT)

    print("Moving with time interpolation")
    joints = req.jointnames
    angles = req.angles
    times = req.times

    isAbsolute = True

    no_limit_violation=check_limits(joints, angles)
    if no_limit_violation==False:
        success = False
        return success

    try:
        for i in range(len(joints)):
            #motionProxy.setStiffnesses(joints[i], 1.0)
            
            motionProxy.angleInterpolation(joints[i],angles[i]*almath.TO_RAD,times[i],True)
            print('move ', joints[i],angles[i], 'angles', 'with speed:', times[i] )
            time.sleep(0.1)
            #motionProxy.setStiffnesses(joints[i], 0.0)
            
        success = True
    except Exception as e:
        print(e)
        success = False

    return success


def handle_KillTask(req):
    try:
        motionProxy = ALProxy("ALMotion", robotIP, PORT)
        taskList = motionProxy.getTaskList()
        print("Tasklist: ", taskList)
        uiMotion = taskList[0][1]
        motionProxy.killTask(uiMotion)
        success = True
    except Exception as e:
        print(e)
        success = False
    return success

if __name__ == '__main__':

    rospy.init_node('MoveJoints_server') #name of node
    speed_service = rospy.Service('MoveJoints', MoveJoints, handle_MoveJoints)
    time_service = rospy.Service('MoveInterpol', MoveInterpol, handle_MoveInterpol)
    killer_service = rospy.Service('KillTask', KillTask, handle_KillTask)
    rospy.spin()


