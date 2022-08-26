cmake_minimum_required(VERSION 2.8.3)
project(project_E)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED cmake_modules COMPONENTS
  roscpp
  cv_bridge
  std_msgs
  rospy
  naoqi_bridge_msgs
  std_srvs
  tf
  actionlib_msgs
  geometry_msgs
  message_generation
  sensor_msgs
  image_transport
  aruco
  cv_bridge
  nav_msgs
)

find_package(Boost REQUIRED COMPONENTS system thread)

add_message_files(
    FILES
    aruco_position.msg
)

add_service_files(
    FILES
    MoveJoints.srv
    MoveInterpol.srv
    getPositions.srv
    setPositions.srv
    getTransform.srv
    initPosition.srv
    getMap.srv
    getNAOPosition
)

add_action_files(
    FILES
    Blink.action
    )

generate_messages(
    DEPENDENCIES
    actionlib_msgs
    std_msgs
    nav_msgs
    )

catkin_package(
  CATKIN_DEPENDS actionlib_msgs roscpp std_msgs sensor_msgs nav_msgs geometry_msgs cv_bridge image_transport opencv2 aruco message_runtime
  DEPENDS system_lib
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  /home/nao_a/Downloads/aruco-3.1.12/aruco_src/include/
)

catkin_install_python(
  PROGRAMS ./script/move_service.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

catkin_install_python(
  PROGRAMS ./src/controller.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

##add_executable()

include_directories(${Boost_INCLUDE_DIRS})

##target_link_libraries()

