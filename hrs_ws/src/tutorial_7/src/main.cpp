#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <ros/ros.h>
#include <sensor_msgs/image_encodings.h>
#include "sensor_msgs/JointState.h"
#include "message_filters/subscriber.h"
#include <string.h>
#include <naoqi_bridge_msgs/JointAnglesWithSpeed.h>
#include <naoqi_bridge_msgs/Bumper.h>
#include <naoqi_bridge_msgs/HeadTouch.h>
#include <naoqi_bridge_msgs/HandTouch.h>
#include <naoqi_bridge_msgs/JointAnglesWithSpeedAction.h>
#include <std_srvs/Empty.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <naoqi_bridge_msgs/SpeechWithFeedbackActionGoal.h>
#include <actionlib_msgs/GoalStatusArray.h>
#include <naoqi_bridge_msgs/BlinkActionGoal.h>
#include <naoqi_bridge_msgs/SetSpeechVocabularyActionGoal.h>
#include <std_msgs/ColorRGBA.h>
#include <naoqi_bridge_msgs/WordRecognized.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Pose2D.h>
#include <std_msgs/Bool.h>
#include "actionlib/client/simple_action_client.h"
#include <tutorial_7/BlinkAction.h>

using namespace std;

//Global variables
std::vector<string> words_recognized;
std::vector<float> confidence_levels;
string sentence;

bool stop_thread=false;
void spinThread()
{
  while(!stop_thread)
  {
    ros::spinOnce();
    //ROS_INFO_STREAM("Spinning the thing!!");
  }
}

class Nao_control
{

protected:

  // ROS node handler
  ros::NodeHandle nh_;

  // Subscriber to bumpers states
  ros::Subscriber bumper_sub;

  // Subscriber to head tactile states
  ros::Subscriber tactile_sub;

  // Publisher for nao speech
  ros::Publisher speech_pub;

  // Publisher for nao vocabulary parameters
  ros::Publisher voc_params_pub;

  // Client for starting speech recognition
  ros::ServiceClient recog_start_srv;

  // Client for stoping speech recognition
  ros::ServiceClient recog_stop_srv;

  // Client for going to suitable position before walking
  ros::ServiceClient walking_start_srv;

  // Client for stoping walking
  ros::ServiceClient walking_stop_srv;

  // Subscriber to speech recognition
  ros::Subscriber recog_sub;

  // Publisher to nao walking
  ros::Publisher walk_pub;

  // Subscriber for foot contact
  ros::Subscriber footContact_sub;


public:

  boost::thread *spin_thread;

  //Walking flag definitions
  bool ready_state = true;
  bool walking = false;

  //Walking movement definition
  //WALK IN STRAIGHT LINE
  double x = 1.0;
  double y = 0.0;
  double theta = 0.0;
  /*
  //SPINING AROUND - IMPLEMENT THESE TO MAKE A TURN
  double x = 1.0;
  double y = 0.0;
  double theta = 0.0;
  */
  // Create the action client
  actionlib::SimpleActionClient<tutorial_7::BlinkAction> my_actionClient;

  Nao_control(): my_actionClient(nh_, "blink", true) // Init the action client: target the /blink action server. Here "true" causes the action client to spin its own thread
  {
    // Subscribe to topic bumper and specify that all data will be processed by function bumperCallback
    bumper_sub=nh_.subscribe("/bumper",1, &Nao_control::bumperCallback, this);

    // Subscribe to topic tactile_touch and specify that all data will be processed by function tactileCallback
    tactile_sub=nh_.subscribe("/tactile_touch",1, &Nao_control::tactileCallback, this);

    speech_pub = nh_.advertise<naoqi_bridge_msgs::SpeechWithFeedbackActionGoal>("/speech_action/goal", 1);

    voc_params_pub= nh_.advertise<naoqi_bridge_msgs::SetSpeechVocabularyActionGoal>("/speech_vocabulary_action/goal", 1);

    recog_start_srv=nh_.serviceClient<std_srvs::Empty>("/start_recognition");

    recog_stop_srv=nh_.serviceClient<std_srvs::Empty>("/stop_recognition");

    walking_start_srv=nh_.serviceClient<std_srvs::Empty>("/start_walk_srv");

    walking_stop_srv=nh_.serviceClient<std_srvs::Empty>("/stop_walk_srv");

    recog_sub=nh_.subscribe("/word_recognized",1, &Nao_control::speechRecognitionCallback, this);

    footContact_sub = nh_.subscribe<std_msgs::Bool>("/foot_contact", 1, &Nao_control::footContactCallback, this);

    walk_pub=nh_.advertise<geometry_msgs::Pose2D>("/cmd_pose", 1);

    // Vocabulary definition
    ros::Duration(1).sleep();
    naoqi_bridge_msgs::SetSpeechVocabularyActionGoal vocab_msg;

    std::vector<string> vocabulary = {"banana", "apple", "pineapple", "pear", "lemon", "toy"};
    string msg_id = "vocabulary";

    vocab_msg.goal_id.id = msg_id;
    vocab_msg.goal.words = vocabulary;

    voc_params_pub.publish(vocab_msg);


    stop_thread=false;
    spin_thread=new boost::thread(&spinThread);

    // Define that the robot needs to assume a suitable posture before starting to walk
    std_srvs::Empty filler;
    if (walking_start_srv.call(filler)){
      ROS_INFO("Configuring the robot to assume proper position before walking");
    }
  }
  ~Nao_control()
  {
    stop_thread=true;
    sleep(1);
    spin_thread->join();
  }

  void footContactCallback(const std_msgs::BoolConstPtr& contact)
  {
    /*
     * TODO control tutorial 3
     */
    if(contact == false){
      ROS_INFO("Foot contact with the ground lost");
      stopWalk();
    }
  }

  void speechRecognitionCallback(const naoqi_bridge_msgs::WordRecognized::ConstPtr& msg)
  {
    //Save recognized words and their confidence level
   for (string word:msg->words){
     words_recognized.push_back(word);
     ROS_INFO_STREAM("Word recognized: "<< word);
     sentence = sentence + " " + word;
   }
   for (float confidence_lvl:msg->confidence_values){
     confidence_levels.push_back(confidence_lvl);
     ROS_INFO_STREAM("Confidence level: "<< confidence_lvl);
   }
  }

  void bumperCallback(const naoqi_bridge_msgs::Bumper::ConstPtr& bumperState)
  {
    tutorial_7::BlinkGoal blinkGoal;
    std_msgs::ColorRGBA color;
    std::vector<std_msgs::ColorRGBA> colors;
    bool blinkFlag = false;
    double time(0.0);

    if (bumperState->bumper==bumperState->right)
    {
      if (bumperState->state == bumperState->statePressed){
        blinkFlag = true;
        ROS_WARN_STREAM("Right Bumper has been Pressed");
        color.r = 0;
        color.g = 1;
        color.b = 0;
        color.a = 1;
        colors.push_back(color);
        time = 4.0;
      }
    }
    else if(bumperState->bumper==bumperState->left)
    {
      if (bumperState->state == bumperState->statePressed){
        blinkFlag = true;
        ROS_WARN_STREAM("Left Bumper has been Pressed");
        color.r = 1;
        color.g = 0;
        color.b = 0;
        color.a = 1;
        colors.push_back(color);
        time = 2.0;
      }
    }
    else
    {
      blinkFlag = false;
    }

    if(blinkFlag == true)
    {
      blinkGoal.colors = colors;
      blinkGoal.blink_duration = ros::Duration(time);
      blinkGoal.blink_rate_mean = 1.0;
      blinkGoal.blink_rate_sd = 0.1;
      my_actionClient.sendGoal(blinkGoal, this->doneBlinkCallback, this->activeBlinkCallback, this->feedbackBlinkCallback);
    }
    else
    {
      // Must be called to preempt the current action otherwise NAO stays in an infinite blinking state...
      my_actionClient.cancelAllGoals();
    }
  }

  // Called once when the blink goal completes
  static void doneBlinkCallback(const actionlib::SimpleClientGoalState& state, const tutorial_7::BlinkResultConstPtr& result)
  {
    ROS_INFO_STREAM("Finished in state: "<< state.toString().c_str());
  }

  // Called once when the blink goal becomes active
  static void activeBlinkCallback()
  {
    ROS_INFO("Blink goal just went active");
  }

  // Called every time feedback is received for the blink goal
  static void feedbackBlinkCallback(const tutorial_7::BlinkFeedbackConstPtr& feedback)
  {
    ROS_INFO_STREAM("Got the following Feedback: "<< feedback->last_color);
  }

  void tactileCallback(const naoqi_bridge_msgs::HeadTouch::ConstPtr& tactileState)
  {
    //tutorial_7::TactileGoal tactileGoal;
    std_srvs::Empty filler;

    //Front button
    if(tactileState->button == tactileState->buttonFront){
      if(tactileState->state == tactileState->statePressed){        
        //Call speech recognition service
        if (recog_start_srv.call(filler)){
          ROS_INFO("Starting speech recognition");
          sentence = "";
        }
      }
    }

    //Middle button
    if(tactileState->button == tactileState->buttonMiddle){
      if(tactileState->state == tactileState->statePressed){
        //Call to stop speech recognition service
        if (recog_stop_srv.call(filler)){
          ROS_INFO("Stoping speech recognition");
        }

        //Call to speak vector of recognized words
        naoqi_bridge_msgs::SpeechWithFeedbackActionGoal speech_msg;

        string msg_id = "speech";

        speech_msg.goal_id.id = msg_id;
        speech_msg.goal.say = sentence;

        ROS_INFO_STREAM("Saying sentence: " << sentence);
        speech_pub.publish(speech_msg);
      }
    }    
    //Rear button
    if(tactileState->button == tactileState->buttonRear){
      if(tactileState->state == tactileState->statePressed){
        switch(walking){
          //Robot still ready to walk
          case false :
            ROS_INFO("Starting to walk");
            ready_state = false;
            walking = true;

            walker(x,y,theta);
            break;

          //Robot walking
          case true:
            stopWalk();
            walking = false;
            break;


            
        }
      }
    }
    

   //ROUTINE TO MOVE THE ROBOT AND MAKE A TURN WHEN PRESSING THE REAR BUTTON AND WHEN POSITION IS FIXED PRESS AGAIN FOR STRAIGHT WALK
    if(tactileState->button == tactileState->buttonRear){
      if(tactileState->state == tactileState->statePressed){
        switch(walking){
          //Robot still ready to walk
          case false :
            ROS_INFO("Starting to walk");
            ready_state = false;
            walking = true;

            walker(1.0,0.5,0.0);
            break;

          //Robot walking
          case true:
            walker(0.0,0.0,10.0);
            walking = false;
            break;


            
        }
      }
    }


  }


  void main_loop(ros::Rate rate_sleep)
  {

    while(nh_.ok())
    {
      rate_sleep.sleep();
    }
  }

  void walker(double x, double y, double theta)
  {
    geometry_msgs::Pose2D walk_msg;
    walk_msg.x = x;
    walk_msg.y = y;
    walk_msg.theta = theta;
    walk_pub.publish(walk_msg);
  }

  void stopWalk()
  {
    std_srvs::Empty filler;
    if (walking_stop_srv.call(filler)){
      ROS_INFO("Stopping walk");
      walking = false;
    }
  }

};
int main(int argc, char** argv)
{
  ros::init(argc, argv, "tutorial_7");
  Nao_control TermiNAOtor;
  ros::Rate rate_sleep(10);

  ROS_INFO_STREAM("Started the Client");
  // Waiting for NAO blink action server to start (roslaunch nao_apps leds.launch):
  TermiNAOtor.my_actionClient.waitForServer();

  // Get into the main loop:
  TermiNAOtor.main_loop(rate_sleep);

  return 0;
}
