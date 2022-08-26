#include "ros/ros.h"
#include "std_msgs/String.h"
#include "aruco/aruco.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include "opencv2/videoio.hpp"
#include <iostream>
#include "std_msgs/Float32.h"
#include "nav_msgs/Odometry.h"
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include "sensor_msgs/JointState.h"
#include "nao_control_tutorial_1/MoveJoints.h"
#include "nao_control_tutorial_1/MoveInterpol.h"
#include "nao_control_tutorial_1/KillTask.h"
#include <string.h>

using namespace cv;
using namespace std;
using namespace aruco;

RNG rng(12345);
static const std::string OPENCV_WINDOW = "Original image";
static const std::string OBJECT_SCREENSHOT = "Top camera";
static const std::string ROI_SELECTOR = "ROI selector";
static const std::string TASK5 = "task5";
static const std::string BACKPROJ = "backprojection";
static const std::string TRACKING = "Object tracking";
static const std::string CAMSHIFT = "Tracking with camshift";
static const std::string OPTICAL_FLOW = "OPTICAL FLOW";
//SET VALUES FOR THRESHOLD FUNCTIONS TO DETECT DIFFERENT COLORS
const int max_value_H = 360/2;
const int max_value = 255;
int low_H = 0, low_S = 70, low_V = 50;
int high_H = 10, high_S = 255, high_V = max_value;

Rect rect(169, 112, 51, 70);
Rect rect2(169, 112, 51, 70);
Mat ref_img;
Mat ref_gray;
Mat next_img;
Mat mask;
vector<Point2f> p0, p1;
vector<Scalar> colors;

float current_yaw;
float current_pitch;

class ImageConverter
{
    ros::NodeHandle nh_;
    ros::NodeHandle na_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;   //CREATE SUB FOR TOP CAMERA
    image_transport::Publisher image_pub_;  //CREATE PUBLISHER 
    image_transport::Subscriber bottom_sub_;  //CREATE SUB FOR BOTTOM CAMERA
    ros::Publisher aruco_pub = na_.advertise<nav_msgs::Odometry>("aruco_topic", 3);

public:
    
    Mat translation;
    Mat rotation;

    ImageConverter()
        : it_(nh_)
    {
        
        image_sub_ = it_.subscribe("/nao_robot/camera/top/camera/image_raw", 1, &ImageConverter::imageCb, this);  //SUBSCRIBE TO TOP CAMERA USING imageCB
        //image_pub_ = it_.advertise("/image_converter/output_video", 1);

        //CREATE 2 WINDOW INSTANCES
        cv::namedWindow(OPENCV_WINDOW);
        cv::namedWindow(OBJECT_SCREENSHOT);
    }

    ~ImageConverter()
   {
     cv::destroyWindow(OPENCV_WINDOW);
     cv::destroyWindow(OBJECT_SCREENSHOT);
   }

    //get "message" of images, by converting them from ROS to OpenCV images
    void imageCb(const sensor_msgs::ImageConstPtr& msg)
    {
        cv_bridge :: CvImagePtr cv_ptr;
        try
        {
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception& e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }

        // Update GUI Window

        cv::imshow(OPENCV_WINDOW, cv_ptr->image);
        cv::waitKey(3);

        //Set camera parameters
        // create a variable in the class
        CameraParameters TheCameraParameters;

        // load the parameter matrix in the constructor
        Mat dist(1,5,CV_32FC1);
        dist.at<float>(0,0)=-0.066494;
        dist.at<float>(0,1)=0.095481;
        dist.at<float>(0,2)=-0.000279;
        dist.at<float>(0,3)=0.002292;
        dist.at<float>(0,4)=0.000000;
        Mat cameraP(3,3,CV_32FC1);

        cameraP.at<float>(0,0)=551.543059;
        cameraP.at<float>(0,1)=0.000000;
        cameraP.at<float>(0,2)=327.382898;
        cameraP.at<float>(1,0)=0.000000;
        cameraP.at<float>(1,1)=553.736023;
        cameraP.at<float>(1,2)=225.026380;
        cameraP.at<float>(2,0)=0.000000;
        cameraP.at<float>(2,1)=0.000000;
        cameraP.at<float>(2,2)=1.000000;

        TheCameraParameters.setParams(cameraP,dist,Size(640,480));
        TheCameraParameters.resize( Size(640,480));


        //Aruco tracking
        Mat aruco_img;
        aruco_img = cv_ptr->image.clone();

        aruco::MarkerDetector MDetector;
        std::vector<aruco::Marker> markers;
        MDetector.detect(aruco_img, markers, TheCameraParameters,0.05);

        //2d marker draw
        /*for(size_t i=0;i<markers.size();i++){
            std::cout<<markers[i]<<std::endl;
            //draw in the image
            markers[i].draw(aruco_img,  180);
        }*/

        //3d marker draw  //Rvec rotation from camera to output aruco frame and Tvev the translation between them (3D)

        for(auto m:markers){
            aruco::CvDrawingUtils::draw3dAxis(aruco_img, m, TheCameraParameters);
            //cout<<"Rotation between camera and marker coord system: "<<m.Rvec<<endl;
            //cout<<"Translation between camera and marker coord system: "<<m.Tvec<<endl;
            rotation = m.Rvec;
            translation = m.Tvec;
            //cout<<"Translation between camera and marker coord system: "<<translation<<endl;
        }

        imshow("Marker tracker", aruco_img);

        //Calculate joint angle update
        uint8_t *translationData = translation.data;
        int width = translation.cols;
        int height = translation.rows;
        int _stride = translation.step;//in case cols != strides
        //cout<<"width: "<<std::to_string(width)<<endl;
        //cout<<"height: "<<std::to_string(height)<<endl;
        //cout<<"_stride: "<<std::to_string(_stride)<<endl;
        //cout<<"Number of markers: "<<markers.size()<<endl;


        if (markers.size() > 0){
            float x = translation.at<float>(0);
            float y = translation.at<float>(1);
            float z = translation.at<float>(2);
            //cout<<"x: "<<std::to_string(x)<<endl;
            //cout<<"y: "<<std::to_string(y)<<endl;
            //cout<<"z: "<<std::to_string(z)<<endl;

            float x_to_center = x + z*0.28;
            float y_to_center = y + z*0.2;

            //Center x and y coordinates on the center of the image
            //cout<<"x center: "<<std::to_string(x_to_center)<<endl;
            //cout<<"y center: "<<std::to_string(y_to_center)<<endl;

            //Calculate angle updates
            float d_yaw = -(x_to_center/z)*57.222787386;
            float d_pitch = (y_to_center/z)*57.222787386;
            //cout<<"d_yaw in degrees: "<<std::to_string(d_yaw)<<endl;
            //cout<<"d_pitch in degrees: "<<std::to_string(d_pitch)<<endl;


            //ROS_INFO("HeadYaw: %f, HeadPitch: %f", current_yaw, current_pitch);
            //TARGET ANGLES THAT ROBOT NEEDS TO MOVE
            float target_yaw = d_yaw + current_yaw*57.222787386;
            float target_pitch = d_pitch + current_pitch*57.222787386;
            //cout<<"current_yaw: "<<std::to_string(current_yaw*57.222787386)<<endl;
            //cout<<"current_pitch: "<<std::to_string(current_pitch*57.222787386)<<endl;

            cout<<"target_yaw: "<<std::to_string(target_yaw)<<endl;
            cout<<"target_pitch: "<<std::to_string(target_pitch)<<endl;

            
            ros::NodeHandle n;

            /*ADVERTISE ARUCO DATA WITH STD MSGS*/
            /*std_msgs::Float32 msg;
            std_msgs::Float32 msg2;
            std_msgs::Float32 msg3;
            msg.data = x;
            msg2.data = y;
            msg3.data = z;
            //std::stringstream ss;
            //ss << "hello world ";
            //msg.data = ss.str();

            aruco_pub.publish(msg);
            aruco_pub.publish(msg2);
            aruco_pub.publish(msg3);*/
            /*ADVERTISE ARUCO DATA WITH ODOMETRY*/
            nav_msgs::Odometry odom;
            odom.pose.pose.position.x = x;
            odom.pose.pose.position.y = y;
            odom.pose.pose.position.z = z;
            
            aruco_pub.publish(odom);
            //Tracking using set angles
            /*
            ros::ServiceClient client = n.serviceClient <nao_control_tutorial_1::MoveJoints>("MoveJoints");
            nao_control_tutorial_1::MoveJoints service;
            service.request.jointnames = {"HeadYaw", "HeadPitch"};
            service.request.speeds = {0.2, 0.2};
            service.request.angles = {target_yaw, target_pitch};
            cout<<""<<std::to_string(service.request.angles[0])<<endl;
            
            //Call set angles
            if (client.call(service)){
                bool success = service.response.success;
                cout<<"Service success status: "<<success<<endl;
            }*/

            //Tracking using interpolation
            ros::ServiceClient client = n.serviceClient <nao_control_tutorial_1::MoveInterpol>("MoveInterpol");
            nao_control_tutorial_1::MoveInterpol service;

            ros::ServiceClient killer = n.serviceClient <nao_control_tutorial_1::KillTask>("KillTask");
            nao_control_tutorial_1::KillTask killer_service;

            service.request.jointnames = {"HeadYaw", "HeadPitch"};
            service.request.times = {1, 1};
            service.request.angles = {target_yaw, target_pitch};            

            
            //Kill previous movement task
            if (killer.call(killer_service)){
                bool success = service.response.success;
                cout<<"Service success status: "<<success<<endl;
            }

            //Call interpolation
            if (client.call(service)){
                bool success = service.response.success;
                cout<<"Service success status: "<<success<<endl;
            }

        }
        
    }
};

class Nao_control
{
public:
    // ros handler
    ros::NodeHandle nh_;

    // subscriber to joint states
    ros::Subscriber sensor_data_sub;

    Nao_control()
    {
        sensor_data_sub=nh_.subscribe("/joint_states",1, &Nao_control::sensorCallback, this);
    }
    ~Nao_control()
    {
    }

  //handler for joint states
    void sensorCallback(const sensor_msgs::JointState::ConstPtr& jointState)
    {
        //float positions[] = jointState->position;
        //float positions[] = jointState->position;
        current_yaw = jointState->position[0];
        current_pitch = jointState->position[1];
        //ROS_INFO("HeadYaw: %f, HeadPitch: %f", current_yaw, current_pitch);
    }

// TODO: create function for each task

};
int main(int argc, char** argv)
{
    ros::init(argc, argv, "aruco_tracker");

    ImageConverter converter;
    Nao_control ic;

    ros::NodeHandle n;
    
    //ros::ServiceClient client = n.serviceClient <nao_control_tutorial_1::MoveJoints>("MoveJoints");
    //nao_control_tutorial_1::MoveJoints service;

    //cout<<"Translation between camera and marker coord system: "<<converter.translation<<endl;
    
    ros::spin();
    return 0;

}