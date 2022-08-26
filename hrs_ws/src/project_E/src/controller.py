#!/usr/bin/env python
import rospy
from project_E.srv import (MoveJoints, MoveInterpol, getPositions, getPositionsResponse, setPositions, getTransform, initPosition)
from mapper import Map
from collections import namedtuple

STANDBY_STATE = 0
MAPPING_STATE = 1
PLANNING_STATE = 2
COLLECTION_STATE = 3
GRASPING_STATE = 4
DELIVERY_STATE = 5
STATION_STATE = 6


class Controller():
    def __init__(self):
        self.state = 1
        self.task_finished = False

        #Set robot to initial position
        rospy.wait_for_service('initPosition')
        try:
            initClient = rospy.ServiceProxy('initPosition', initPosition)
            ready = initClient()
            if ready:
                print("Robot in the initial position")
            else:
                print("Failed to go to initial position")
        except rospy.ServiceException as e:
            print("Service call failed: %s"%e)

    def run_current_state(self):
        if self.state == MAPPING_STATE:
            self.mapping_task() 

        if self.state == PLANNING_STATE:
            self.planning_task()

    #def mapping_task(self):
    
    def planning_task(self):
        Distances = namedtuple('Distances',['distane' 'color'])
        distances = []
        full_distances = []
        naos_pos = map.aruco_pos_update(map.self,map.marker_x,map.marker_y,map.marker_z,map.id) #map.id doesnt exist, need to find a way so that map gets ids of references
        for object in map.objects:
            obj_coord = [object.pose.pose.position.x,object.pose.pose.position.y]
            d1 = 0
            d2 = 0
            d3 = 0
            if object.child_frame == 'red':
                d1 = Distances(map.get_distance(obj_coord,naos_pos)+map.get_distances(map.redbay,naos_pos),'red')
                if d1 > d2 & d1 > d3:
                    full_distances.append(d1)
                elif d1 > d2 & d1 < d3 or d1 > d3 & d1 < d2:
                    full_distances.insert(1,d1)
                else:    
                    full_distances.insert(0,d1)
            if object.child_frame == 'blue':
                d2 = Distances(map.get_distance(obj_coord,naos_pos)+map.get_distances(map.bluebay,naos_pos),'blue')
                if d2 > d1 & d2 > d3:
                    full_distances.append(d2)
                elif d2 > d1 & d2 < d3 or d2 > d3 & d2 < d1:
                    full_distances.insert(1,d2)
                else:
                    full_distances.insert(0,d2)
            if object.child_frame == 'green':
                d3 = Distances(map.get_distance(obj_coord,naos_pos)+map.get_distances(map.greenbay,naos_pos),'green')
                if d3 > d1 & d3 > d2:
                    full_distances.append(d3)
                elif d3 > d1 & d3 < d2 or d3 > d2 & d2 < d1:
                    full_distances.insert(1,d3)
                else:
                    full_distances.insert(0,d3)
        self.state = COLLECTION_STATE

        #CALL WALKING STATE TO GO TO DESIRED PLACES
        return full_distances #FULL DISTANCES IS A STRUCT THAT HAS IN ASCENDING ORDER TOTAL DISTANCES FOR EACH ONJECT AS WELL AS THE COLOR OF THE OBJECT!
            
    def avoid_obstacles(self):
        distances = []
        naos_pos = map.aruco_pos_update(map.self,map.marker_x,map.marker_y,map.marker_z,map.id) #map.id doesnt exist, need to find a way so that map gets ids of references
        for obstacle in map.obstacles:
            obst_coord = [obstacle.pose.pose.position.x,obstacle.pose.pose.position.y]
            distances.append(map.get_distance(obst_coord,naos_pos))
        return distances.sort() #now array distances have as first index the obstacle closest to nao and we can keep the distance to it more than a threshold

       

    #def grasping_task(self):

    #def walking_task(self):

def nao_controller():
    rospy.init_node('controller', anonymous=True)
    controller = Controller()
    #Loop until objective is achieved
    while(controller.task_finished == False):
        controller.run_current_state()
    #rospy.Subscriber("object_topic", Odometry, object_callback)
    #rospy.Subscriber("aruco_topic", Odometry, aruco_callback)

    # spin() simply keeps python from exiting until this node is stopped
        rospy.spin()

if __name__ == '__main__':
    nao_controller()