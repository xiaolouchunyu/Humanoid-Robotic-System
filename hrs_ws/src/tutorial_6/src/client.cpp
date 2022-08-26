#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/String.h"
#include "aruco/aruco.h"
#include <image_transport/image_transport.h>
#include <tf/transform_broadcaster.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include "nav_msgs/Odometry.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include "opencv2/videoio.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include "sensor_msgs/JointState.h"
#include "tutorial_6/MoveJoints.h"
#include "tutorial_6/getTransform.h"
#include "tutorial_6/aruco_position.h"
#include "tutorial_6/setPositions.h"
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
int rest_counter_left = 0;
int rest_counter_right = 0;

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

        //Create publisher for aruco positions
        //ros::Publisher aruco_pub = nh_.advertise<tutorial_6::aruco_position>("/aruco_position", 1);

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
        /*
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
        TheCameraParameters.resize( Size(640,480));*/


        float datad[5] = {-0.0481869853715082,  0.0201858398559121,
                            0.0030362056699177, -0.00172241952442813, 0};
        float datac[9] = {278.236008818534, 0,                156.194471689706,
                        0,                279.380102992049, 126.007123836447,
                        0,                0,                1};

        cv::Mat cameraP(3,3,CV_32FC1, datac);
        cv::Mat dist(5,1,CV_32FC1, datad);

        TheCameraParameters.setParams(cameraP,dist,Size(640,480));
        TheCameraParameters.resize( Size(640,480));
        //Aruco tracking
        Mat aruco_img;
        aruco_img = cv_ptr->image.clone();

        aruco::MarkerDetector MDetector;
        std::vector<aruco::Marker> markers;
        MDetector.detect(aruco_img, markers, TheCameraParameters,0.05);

        //2d marker draw
        for(size_t i=0;i<markers.size();i++){
            std::cout<<markers[i]<<std::endl;
            //draw in the image
            markers[i].draw(aruco_img,  180);
        }

        //3d marker draw  //Rvec rotation from camera to output aruco frame and Tvev the translation between them (3D)
        int marker_id = 0;
        int id = 0;
        for(auto m:markers){
            marker_id++;
            aruco::CvDrawingUtils::draw3dAxis(aruco_img, m, TheCameraParameters);
            //cout<<"Rotation between camera and marker coord system: "<<m.Rvec<<endl;
            //cout<<"Translation between camera and marker coord system: "<<m.Tvec<<endl;
            rotation = m.Rvec;
            translation = m.Tvec;
            //cout<<"Translation between camera and marker coord system: "<<translation<<endl;


            //Publish aruco position to visualization_marker topic
            tf::TransformBroadcaster  my_transformBroadcaster;

            tf::Transform my_transform;
            tf::Quaternion q;

            double x = translation.at<float>(0);
            //cout<<"heeeeeeeeeeeeeerereeeeeeeeeeeeee1"<<x<<endl;
            double y = translation.at<float>(1);
            //cout<<"heeeeeeeeeeeeeeeeeeeeeeeeeere22222222 "<<y<<endl;
            double z = translation.at<float>(2);
            double  roll = rotation.at<float>(0);
            double  pitch = rotation.at<float>(1);
            double  yaw = rotation.at<float>(2);
            
            my_transform.setOrigin(tf:: Vector3(x, y, z));
            q.setRPY(roll , pitch , yaw);
            my_transform.setRotation(q);
            id = m.id ;
            my_transformBroadcaster.sendTransform(tf:: StampedTransform(my_transform ,ros::Time::now(), "CameraBottom_optical_frame", "marker"));
            
        }
    
        imshow("Marker tracker", aruco_img);

        //Update counters
        rest_counter_left++;
        rest_counter_right++;

        //Instanciate service
        ros::ServiceClient move_client = nh_.serviceClient <tutorial_6::setPositions>("setPositions");
        tutorial_6::setPositions move_service;

        if (markers.size() > 0){
            //Calculate joint angle update
            uint8_t *translationData = translation.data;
            int width = translation.cols;
            int height = translation.rows;
            int _stride = translation.step;//in case cols != strides
            cout<<"width: "<<std::to_string(width)<<endl;
            cout<<"height: "<<std::to_string(height)<<endl;
            //cout<<"_stride: "<<std::to_string(_stride)<<endl;
            //cout<<"Number of markers: "<<markers.size()<<endl;


            //CameraBottom_optical to CameraBottom transformation
            float data[9] = { 0, 0, 1, -1, 0, 0, 0, -1, 0};
            //float data[9] = { 0, -1, 0, 0, 0, -1, 1, 0, 0};
            cv::Mat transform_Optical_to_Camera = cv::Mat(3, 3, CV_32F, data);
            cout << "transform_Optical_to_Camera = " << endl << " "  << transform_Optical_to_Camera << endl << endl;

            Mat translation_bottom = transform_Optical_to_Camera*translation;

            //Get transformation from bottom camera to torso frame
            //Update translation matrix to homogeneous coordinates
            cv::Mat temp_mat = cv::Mat::ones(1, 1, CV_32F);
            translation_bottom.push_back(temp_mat);

            //Get transformation to torso
            ros::ServiceClient transform_client = nh_.serviceClient <tutorial_6::getTransform>("getTransform");
            tutorial_6::getTransform transfrom_service;
            transfrom_service.request.jointname = {"CameraBottom"};
            transfrom_service.request.reference = 0;
            
            if (transform_client.call(transfrom_service)){
                std::vector<float> tmp_torso_transform = transfrom_service.response.transform;
                bool success = transfrom_service.response.success;
                cout<<"Service success status: "<<success<<endl;

                cv::Mat transform_Camera_to_Torso = cv::Mat(4, 4, CV_32F);
                //transform_Camera_to_Torso = transform_Camera_to_Torso.inv();
                memcpy(transform_Camera_to_Torso.data, tmp_torso_transform.data(), 16*sizeof(float));
                //cv::Mat transform_Camera_to_Torso = cv::Mat(4, 4, CV_32F, tmp_torso_transform.data);
                cout << "transform_Camera_to_Torso = " << endl << " "  << transform_Camera_to_Torso << endl << endl;

                Mat translation_torso = transform_Camera_to_Torso*translation_bottom;

                //Calculate marker position on the torso frame
                float arm_length = 0.205;   //0.21558595861951066

                float marker_x = translation_torso.at<float>(0);
                float marker_y = translation_torso.at<float>(1);
                float marker_z = translation_torso.at<float>(2);
                cout<<"marker_x: "<<std::to_string(marker_x)<<endl;
                cout<<"marker_y: "<<std::to_string(marker_y)<<endl;
                cout<<"marker_z: "<<std::to_string(marker_z)<<endl;

                nav_msgs::Odometry odom;
                odom.pose.pose.position.x = 177.49*pow(marker_x,0.893); //DEPTH
                if (marker_y >= 0){
                    odom.pose.pose.position.y = 188.06*pow(marker_y,1.097); // WIDTH
                }
                if (marker_y < 0){
                    odom.pose.pose.position.y = (-65.98*pow(marker_y,2))+(125.62*marker_y) - 12.42;
                }
                odom.pose.pose.position.z = 0;

                //SEND ID OF MARKER UNDER ORIENTATION X
                odom.pose.pose.orientation.x = id ;
            
                aruco_pub.publish(odom);



                tf::TransformBroadcaster  my_transformBroadcaster;

                tf::Transform my_transform;
                tf::Quaternion q;

                double x = 177.49*pow(marker_x,0.893);
                //cout<<"heeeeeeeeeeeeeerereeeeeeeeeeeeee1"<<x<<endl;
                double y = 0.0;
                if (marker_y >= 0){
                    y = 188.06*pow(marker_y,1.097);
                }
                if (marker_y < 0){
                    y = (-65.98*pow(marker_y,2))+(125.62*marker_y) - 12.42;
                }
                //cout<<"heeeeeeeeeeeeeeeeeeeeeeeeeere22222222 "<<y<<endl;
                double z = marker_z;
                double  roll = rotation.at<float>(0);
                double  pitch = rotation.at<float>(1);
                double  yaw = rotation.at<float>(2);
                
                my_transform.setOrigin(tf:: Vector3(x, y, z));
                q.setRPY(roll , pitch , yaw);
                my_transform.setRotation(q);
                my_transformBroadcaster.sendTransform(tf:: StampedTransform(my_transform ,ros::Time::now(), "CameraBottom_optical_frame", "marker2"));



                //Determine arm activation
                bool LArm_on = 0;
                bool RArm_on = 0;
                if(marker_y > -0.035){
                    LArm_on = 1;
                    rest_counter_left = 0;
                }
                if(marker_y < 0.035){
                    RArm_on = 1;
                    rest_counter_right = 0;
                }


                //Move left arm
                if(LArm_on ==1){
                    float pos_shoulder[] = {0.0, 0.09000000357627869, 0.10599999874830246};

                    //Calculate arm position
                    float diff_x = marker_x-pos_shoulder[0];
                    float diff_y = marker_y-pos_shoulder[1];
                    float diff_z = marker_z-pos_shoulder[2];

                    float norm = sqrt(pow(diff_x,2)+pow(diff_y,2)+pow(diff_z,2));
                    float arm_ratio = arm_length/norm;

                    float ratio_x = diff_x*arm_ratio;
                    float ratio_y = diff_y*arm_ratio;
                    float ratio_z = diff_z*arm_ratio;

                    float target_x = ratio_x+pos_shoulder[0];
                    float target_y = ratio_y+pos_shoulder[1];
                    float target_z = ratio_z+pos_shoulder[2];
                    //float target_z = marker_z+0.2;

                    //Update arm position
                    move_service.request.endeffector = "LArm";
                    std::vector<float> pos{target_x, target_y, target_z};
                    move_service.request.position = pos;
                    //move_service.request.orientation = {-2.9246387481689453,-1.3527696132659912,-0.7539294362068176};
                    move_service.request.orientation = {0,0,0};
                    move_service.request.speed = 0.99;
                    move_service.request.time = 0;
                    
                    //Call set angles
                    if (move_client.call(move_service)){
                        bool success = move_service.response.success;
                        cout<<"Service success status: "<<success<<endl; 
                    }
                }

                //Move right arm
                if(RArm_on == 1){
                    float pos_shoulder[] = {0.0, -0.09000000357627869, 0.10599999874830246};

                    //Calculate arm position
                    float diff_x = marker_x-pos_shoulder[0];
                    float diff_y = marker_y-pos_shoulder[1];
                    float diff_z = marker_z-pos_shoulder[2];

                    float norm = sqrt(pow(diff_x,2)+pow(diff_y,2)+pow(diff_z,2));
                    float arm_ratio = arm_length/norm;

                    float ratio_x = diff_x*arm_ratio;
                    float ratio_y = diff_y*arm_ratio;
                    float ratio_z = diff_z*arm_ratio;

                    float target_x = ratio_x+pos_shoulder[0];
                    float target_y = ratio_y+pos_shoulder[1];
                    float target_z = ratio_z+pos_shoulder[2];

                    //Update arm position
                    std::vector<float> pos{target_x, target_y, target_z};
                    move_service.request.endeffector = "RArm";
                    move_service.request.position = pos;
                    //move_service.request.orientation = {-2.9246387481689453,-1.3527696132659912,-0.7539294362068176};
                    move_service.request.orientation = {0,0,0};
                    move_service.request.speed = 0.99;
                    move_service.request.time = 0;
                    
                    //Call set angles
                    if (move_client.call(move_service)){
                        bool success = move_service.response.success;
                        //cout<<"Service success status: "<<success<<endl; 
                    }                    
                }

            }
        }
    
        //cout<<"rest_counter_left: "<<rest_counter_left<<endl;
        //cout<<"rest_counter_right: "<<rest_counter_right<<endl;

        if(rest_counter_left>100){
            //Update arm position
            std::vector<float> pos{0.042516645044088364, 0.2545584440231323, -0.04622722044587135};
            std::vector<float> orientation{-0.7164158821105957, 0.7601354122161865, 1.3018254041671753};
            move_service.request.endeffector = "LArm";
            move_service.request.position = pos;
            //move_service.request.orientation = {0,0,0};
            move_service.request.orientation = orientation;
            move_service.request.speed = 0.99;
            move_service.request.time = 0;
            
            //Call set angles
            if (move_client.call(move_service)){
                bool success = move_service.response.success;
                //cout<<"Service success status: "<<success<<endl; 
            }  
        }

        if(rest_counter_right>100){
            //Update arm position
            std::vector<float> pos{0.10620994120836258, -0.24930402636528015, -0.013966461643576622};
            std::vector<float> orientation{1.8761860132217407, 0.521427571773529, -0.898587703704834};
            move_service.request.orientation = {0,0,0};//
            move_service.request.endeffector = "RArm";
            move_service.request.position = pos;
            move_service.request.orientation = orientation;
            move_service.request.speed = 0.99;
            move_service.request.time = 0;
            
            //Call set angles
            if (move_client.call(move_service)){
                bool success = move_service.response.success;
                //cout<<"Service success status: "<<success<<endl; 
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
        //ROS_INFO("HeadYclien  aw: %f, HeadPitch: %f", current_yaw, current_pitch);
    }

// TODO: create function for each task

};
int main(int argc, char** argv)
{
    ros::init(argc, argv, "aruco_tracker");

    ImageConverter converter;
    Nao_control ic;

    ros::NodeHandle n;

    //Publisher objects
    tf::TransformBroadcaster  my_transformBroadcaster;
    
    //ros::Publisher vis_pub = n.advertise<visualization_msgs::Marker>( "visualization_marker", 0 );

    //ros::ServiceClient client = n.serviceClient <tutorial_6::MoveJoints>("MoveJoints");
    //tutorial_6::MoveJoints service;

    //cout<<"Translation between camera and marker coord system: "<<converter.translation<<endl;
    
    ros::spin();
    return 0;

}