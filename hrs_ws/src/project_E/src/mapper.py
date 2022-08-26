#!/usr/bin/env python

####################################################################################
#This node should handle tracking the positions of NAO, all known objects and obstacles
#
#Subscribes to:
#detected markers
#detected obstacles
#robot movements
#
#Published to:
#object positions

#from xmlrpc.server import DocXMLRPCRequestHandler
import rospy
import math
from std_msgs.msg import String
from nav_msgs.msg import Odometry
from geometry_msgs import Pose2D
from project_E.srv import (getMap, getNAOPosition)

obstacle_id = 0
reference_ids = [1,2,3,4]
nao_initial_pos = [0,1]
dist_threshold = 0.2    #Distance in meters below which 2 detected entities are determined to be the same

class Map:
    def __init__(self):
        #Initialize the lists containing objects and obstacles
        self.obstacles = []
        self.objects = []

        self.getNAOPosition_Service = rospy.Service('getNAOPosition', getNAOPosition, self.handle_getNAOPosition)
        self.getMap_Service = rospy.Service('getMap', getMap, self.handle_getMap)
        rospy.Subscriber("object_topic", Odometry, self.object_callback)
        rospy.Subscriber("aruco_topic", Odometry, self.aruco_callback)

        #Define initial map of the room - all object delivery bays and reference aruco markers to identify nao's position
        #For simplicity, coordinates floow a [x,y,heigth] scheme
        self.redbay = [0,0,0]
        self.greenbay = [1,1,1]
        self.bluebay = [0,0,1]

        self.ref1 = [1,2]
        self.ref2 = [2,2]
        self.ref3 = [2,2]
        self.ref4 = [2,2]

        self.nao_pos = nao_initial_pos
        self.nao_orientation = 0      #NAO anti-clockwise rotation angle around the z direction in rad

    def add_object(self,x,y,z,color):
        #Based on the given coordinates, checks if detected object corresponds to an existing one. If not, adds it to the map
        match = 0
        for object in self.objects:
            dist = self.get_distance(object, [x,y])
            if dist<dist_threshold:
                match = 1
                break

        if match == 0:
            #Adds new obstacle to the map
            object = Odometry()
            object.pose.pose.position.x = self.nao_pos[1] + math.sin(self.nao_orientation)*x
            object.pose.pose.position.y = self.nao_pos[2] + math.cos(self.nao_orientation)*y
            object.pose.pose.position.z = z
            object.child_frame_id = str(color)
            self.objects.append(object)
            return
        else:
            #Obstacle already exists and does not need to be added to the map
            return

    def add_obstacle(self,x,y):
        #Based on the given coordinates, checks if detected object corresponds to an existing one. If not, adds it to the map
        match = 0
        for obstacle in self.obstacles:
            dist = self.get_distance(obstacle, [x,y])
            if dist<dist_threshold:
                match = 1
                break

        if match == 0:
            #Adds new obstacle to the map
            obstacle = Odometry()
            obstacle.pose.pose.position.x = self.nao_pos[0] + math.sin(self.nao_orientation)*x
            obstacle.pose.pose.position.y = self.nao_pos[1] + math.cos(self.nao_orientation)*y
            self.obstacles.append(obstacle)
            return
        else:
            #Obstacle already exists and does not need to be added to the map
            return


    #remove_object()

    #remove_obstacle()

    def aruco_pos_update(self,marker_x,marker_y,marker_z,id):
    #Sets NAO's map position based on reference aruco markers
        if id == 1:
            naox = self.ref1[1] - marker_x
            naoy = self.ref1[2] - marker_y
        elif id == 2:
            naox = self.ref2[1] - marker_x
            naoy = self.ref2[2] - marker_y
        elif id == 3:
            naox = self.ref3[1] - marker_x
            naoy = self.ref3[2] - marker_y
        elif id == 4:
            naox = self.ref4[1] - marker_x
            naoy = self.ref4[2] - marker_y

        self.nao_pos = [naox, naoy]
    
    def move_nao(self,x,y):
        naox = self.nao_pos[1] + x
        naoy = self.nao_pos[2] + y
        self.nao_pos = self.nao_pos

    def get_distance(entity1, entity2):
        dx = entity1[1] - entity2[1]
        dy = entity1[2] - entity2[2]
        dist = math.sqrt(dx**2 + dy**2)
        return dist

    def handle_getMap(self, req):
        return self.objects, self.obstacles 



    def object_callback(self, odom):
        color = odom.pose.pose.orientation.x
        x = odom.pose.pose.position.x
        y = odom.pose.pose.position.y
        z = odom.pose.pose.position.z

        self.add_object(x,y,z,id)

    def aruco_callback(self, odom):
        print("Market detected")
        id = int(odom.child_frame_id)
        x = odom.pose.pose.position.x
        y = odom.pose.pose.position.y
        z = odom.pose.pose.position.z

        #Detected marker is a reference marker
        if id in reference_ids:
            self.aruco_pos_update(x,y,z,id)
        
        #Detected marker represents an obstacle
        elif id == obstacle_id:
            self.add_obstacle(x,y)

        else:
            print("Error, aruco ID does not match known markers")

    
    
def object_handler():
    rospy.init_node('mapper', anonymous=True)
    map = Map()

    #rospy.Subscriber("object_topic", Odometry, object_callback)
    #rospy.Subscriber("aruco_topic", Odometry, aruco_callback)

    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    object_handler()