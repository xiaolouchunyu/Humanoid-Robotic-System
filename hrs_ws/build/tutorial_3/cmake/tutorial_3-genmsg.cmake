# generated from genmsg/cmake/pkg-genmsg.cmake.em

message(STATUS "tutorial_3: 1 messages, 0 services")

set(MSG_I_FLAGS "-Itutorial_3:/home/nao_a/hrs_ws/src/tutorial_3/msg;-Istd_msgs:/opt/ros/kinetic/share/std_msgs/cmake/../msg;-Inav_msgs:/opt/ros/kinetic/share/nav_msgs/cmake/../msg;-Igeometry_msgs:/opt/ros/kinetic/share/geometry_msgs/cmake/../msg;-Iactionlib_msgs:/opt/ros/kinetic/share/actionlib_msgs/cmake/../msg")

# Find all generators
find_package(gencpp REQUIRED)
find_package(geneus REQUIRED)
find_package(genlisp REQUIRED)
find_package(gennodejs REQUIRED)
find_package(genpy REQUIRED)

add_custom_target(tutorial_3_generate_messages ALL)

# verify that message/service dependencies have not changed since configure



get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_custom_target(_tutorial_3_generate_messages_check_deps_${_filename}
  COMMAND ${CATKIN_ENV} ${PYTHON_EXECUTABLE} ${GENMSG_CHECK_DEPS_SCRIPT} "tutorial_3" "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" ""
)

#
#  langs = gencpp;geneus;genlisp;gennodejs;genpy
#

### Section generating for lang: gencpp
### Generating Messages
_generate_msg_cpp(tutorial_3
  "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/tutorial_3
)

### Generating Services

### Generating Module File
_generate_module_cpp(tutorial_3
  ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/tutorial_3
  "${ALL_GEN_OUTPUT_FILES_cpp}"
)

add_custom_target(tutorial_3_generate_messages_cpp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_cpp}
)
add_dependencies(tutorial_3_generate_messages tutorial_3_generate_messages_cpp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_dependencies(tutorial_3_generate_messages_cpp _tutorial_3_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(tutorial_3_gencpp)
add_dependencies(tutorial_3_gencpp tutorial_3_generate_messages_cpp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS tutorial_3_generate_messages_cpp)

### Section generating for lang: geneus
### Generating Messages
_generate_msg_eus(tutorial_3
  "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/tutorial_3
)

### Generating Services

### Generating Module File
_generate_module_eus(tutorial_3
  ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/tutorial_3
  "${ALL_GEN_OUTPUT_FILES_eus}"
)

add_custom_target(tutorial_3_generate_messages_eus
  DEPENDS ${ALL_GEN_OUTPUT_FILES_eus}
)
add_dependencies(tutorial_3_generate_messages tutorial_3_generate_messages_eus)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_dependencies(tutorial_3_generate_messages_eus _tutorial_3_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(tutorial_3_geneus)
add_dependencies(tutorial_3_geneus tutorial_3_generate_messages_eus)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS tutorial_3_generate_messages_eus)

### Section generating for lang: genlisp
### Generating Messages
_generate_msg_lisp(tutorial_3
  "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/tutorial_3
)

### Generating Services

### Generating Module File
_generate_module_lisp(tutorial_3
  ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/tutorial_3
  "${ALL_GEN_OUTPUT_FILES_lisp}"
)

add_custom_target(tutorial_3_generate_messages_lisp
  DEPENDS ${ALL_GEN_OUTPUT_FILES_lisp}
)
add_dependencies(tutorial_3_generate_messages tutorial_3_generate_messages_lisp)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_dependencies(tutorial_3_generate_messages_lisp _tutorial_3_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(tutorial_3_genlisp)
add_dependencies(tutorial_3_genlisp tutorial_3_generate_messages_lisp)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS tutorial_3_generate_messages_lisp)

### Section generating for lang: gennodejs
### Generating Messages
_generate_msg_nodejs(tutorial_3
  "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/tutorial_3
)

### Generating Services

### Generating Module File
_generate_module_nodejs(tutorial_3
  ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/tutorial_3
  "${ALL_GEN_OUTPUT_FILES_nodejs}"
)

add_custom_target(tutorial_3_generate_messages_nodejs
  DEPENDS ${ALL_GEN_OUTPUT_FILES_nodejs}
)
add_dependencies(tutorial_3_generate_messages tutorial_3_generate_messages_nodejs)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_dependencies(tutorial_3_generate_messages_nodejs _tutorial_3_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(tutorial_3_gennodejs)
add_dependencies(tutorial_3_gennodejs tutorial_3_generate_messages_nodejs)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS tutorial_3_generate_messages_nodejs)

### Section generating for lang: genpy
### Generating Messages
_generate_msg_py(tutorial_3
  "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg"
  "${MSG_I_FLAGS}"
  ""
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/tutorial_3
)

### Generating Services

### Generating Module File
_generate_module_py(tutorial_3
  ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/tutorial_3
  "${ALL_GEN_OUTPUT_FILES_py}"
)

add_custom_target(tutorial_3_generate_messages_py
  DEPENDS ${ALL_GEN_OUTPUT_FILES_py}
)
add_dependencies(tutorial_3_generate_messages tutorial_3_generate_messages_py)

# add dependencies to all check dependencies targets
get_filename_component(_filename "/home/nao_a/hrs_ws/src/tutorial_3/msg/mymsg.msg" NAME_WE)
add_dependencies(tutorial_3_generate_messages_py _tutorial_3_generate_messages_check_deps_${_filename})

# target for backward compatibility
add_custom_target(tutorial_3_genpy)
add_dependencies(tutorial_3_genpy tutorial_3_generate_messages_py)

# register target for catkin_package(EXPORTED_TARGETS)
list(APPEND ${PROJECT_NAME}_EXPORTED_TARGETS tutorial_3_generate_messages_py)



if(gencpp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/tutorial_3)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gencpp_INSTALL_DIR}/tutorial_3
    DESTINATION ${gencpp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_cpp)
  add_dependencies(tutorial_3_generate_messages_cpp std_msgs_generate_messages_cpp)
endif()
if(TARGET nav_msgs_generate_messages_cpp)
  add_dependencies(tutorial_3_generate_messages_cpp nav_msgs_generate_messages_cpp)
endif()

if(geneus_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/tutorial_3)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${geneus_INSTALL_DIR}/tutorial_3
    DESTINATION ${geneus_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_eus)
  add_dependencies(tutorial_3_generate_messages_eus std_msgs_generate_messages_eus)
endif()
if(TARGET nav_msgs_generate_messages_eus)
  add_dependencies(tutorial_3_generate_messages_eus nav_msgs_generate_messages_eus)
endif()

if(genlisp_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/tutorial_3)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genlisp_INSTALL_DIR}/tutorial_3
    DESTINATION ${genlisp_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_lisp)
  add_dependencies(tutorial_3_generate_messages_lisp std_msgs_generate_messages_lisp)
endif()
if(TARGET nav_msgs_generate_messages_lisp)
  add_dependencies(tutorial_3_generate_messages_lisp nav_msgs_generate_messages_lisp)
endif()

if(gennodejs_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/tutorial_3)
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${gennodejs_INSTALL_DIR}/tutorial_3
    DESTINATION ${gennodejs_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_nodejs)
  add_dependencies(tutorial_3_generate_messages_nodejs std_msgs_generate_messages_nodejs)
endif()
if(TARGET nav_msgs_generate_messages_nodejs)
  add_dependencies(tutorial_3_generate_messages_nodejs nav_msgs_generate_messages_nodejs)
endif()

if(genpy_INSTALL_DIR AND EXISTS ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/tutorial_3)
  install(CODE "execute_process(COMMAND \"/usr/bin/python2\" -m compileall \"${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/tutorial_3\")")
  # install generated code
  install(
    DIRECTORY ${CATKIN_DEVEL_PREFIX}/${genpy_INSTALL_DIR}/tutorial_3
    DESTINATION ${genpy_INSTALL_DIR}
  )
endif()
if(TARGET std_msgs_generate_messages_py)
  add_dependencies(tutorial_3_generate_messages_py std_msgs_generate_messages_py)
endif()
if(TARGET nav_msgs_generate_messages_py)
  add_dependencies(tutorial_3_generate_messages_py nav_msgs_generate_messages_py)
endif()
