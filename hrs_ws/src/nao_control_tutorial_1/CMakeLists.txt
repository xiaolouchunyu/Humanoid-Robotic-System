cmake_minimum_required(VERSION 3.0.2)
project(nao_control_tutorial_1)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  rospy
  message_generation
  sensor_msgs
  image_transport
  aruco
  cv_bridge
  nav_msgs
)

find_package(OpenCV REQUIRED COMPONENTS)

add_service_files(
    FILES
    MoveJoints.srv
    MoveInterpol.srv
    KillTask.srv
)



generate_messages(DEPENDENCIES std_msgs nav_msgs)

catkin_package(
  CATKIN_DEPENDS roscpp std_msgs nav_msgs sensor_msgs cv_bridge image_transport opencv2 aruco message_runtime
  DEPENDS system_lib
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  /home/nao_a/Downloads/aruco-3.1.12/aruco_src/include/
)

catkin_install_python(PROGRAMS ./script/move_service.py
    DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

add_executable(aruco_tracker src/client.cpp)

target_link_libraries(aruco_tracker 
${catkin_LIBRARIES}
${OpenCV_LIBRARIES}
)
