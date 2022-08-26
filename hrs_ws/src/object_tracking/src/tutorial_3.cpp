#include "ros/ros.h"
#include "std_msgs/String.h"
#include "nav_msgs/Odometry.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>



using namespace cv;
using namespace std;
RNG rng(12345);
//static const std::string OPENCV_WINDOW = "Original image";
static const std::string OPENCV_FILTERED_WINDOW_RED = "Filtered image red";
static const std::string OPENCV_FILTERED_WINDOW_BLUE = "Filtered image blue";
static const std::string OPENCV_FILTERED_WINDOW_GREEN = "Filtered image green";
//static const std::string OPENCV_BINARY_WINDOW = "Binary image";
static const std::string BLOB_EXTRACTION_BLUE = "Filtered image blue - dilate";
static const std::string BLOB_EXTRACTION_GREEN = "Filtered image green - dilate";
static const std::string BLOB_EXTRACTION_RED = "Filtered image red - dilate";
//static const std::string OPENCV_BOTTOM_WINDOW= "Bottom camera";
//static const std::string OPENCV_BOTTOM_BLUR= "Blurred greyscale";
//static const std::string OPENCV_BOTTOM_CIRCLE= "Circle extraction";
//SET VALUES FOR THRESHOLD FUNCTIONS TO DETECT DIFFERENT COLORS
const int max_value_H = 360/2;
const int max_value = 255;
int low_H = 0, low_S = 70, low_V = 50;
int high_H = 10, high_S = 255, high_V = max_value;

//GET AREA OF BIGGEST BLOB
int getMaxAreaContourId(vector <vector<cv::Point> > contours, double & maxArea) {
    //double maxArea = 0;
    int maxAreaContourId = -1;
    for (int j = 0; j < contours.size(); j++) {
        double newArea = cv::contourArea(contours.at(j));
        if (newArea > maxArea) {
            maxArea = newArea;
            maxAreaContourId = j;
        } // End if
    } // End for
    return maxAreaContourId;       //RETURNS AN ID THAT IS THE INDEX OF CONTOURS VECTOR, THAT CONTAINS THE POINTS THAT DEFINE THE CONTOUR
} 



class ImageConverter
{
    ros::NodeHandle nh_;
    ros::NodeHandle na_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;   //CREATE SUB FOR TOP CAMERA
    image_transport::Publisher image_pub_;  //CREATE PUBLISHER 
    image_transport::Subscriber bottom_sub_;  //CREATE SUB FOR BOTTOM CAMERA
    ros::Publisher object_pub = na_.advertise<nav_msgs::Odometry>("object_topic", 3);
public:
    
    ImageConverter()
        : it_(nh_)
    {
        
        image_sub_ = it_.subscribe("/nao_robot/camera/top/camera/image_raw", 1, &ImageConverter::imageCb, this);  //SUBSCRIBE TO TOP CAMERA USING imageCB
        //bottom_sub_ = it_.subscribe("/nao_robot/camera/bottom/camera/image_raw", 1, &ImageConverter::imageCb1, this); //SUBSCRIBE TO BOTTOM CAMERA USING imageCB
        //image_pub_ = it_.advertise("/image_converter/output_video", 1);

        //CREATE 2 WINDOW INSTANCES
        //cv::namedWindow(OPENCV_WINDOW); 
        //cv::namedWindow(OPENCV_BOTTOM_WINDOW);
    }

    ~ImageConverter()
   {
     //cv::destroyWindow(OPENCV_WINDOW);
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
        //cv::imshow(OPENCV_WINDOW, cv_ptr->image);
        //cv::waitKey(3);

        
        cv::Mat hsv_img;
        cv::Mat hsv_filtered_red;
        cv::Mat hsv_filtered_blue;
        cv::Mat hsv_filtered_green;
        cv::Mat blue_dilated;
        cv::Mat green_dilated;
        cv::Mat red_dilated;
        //CONVERT TO HSV IMAGE
        cv::cvtColor(cv_ptr->image, hsv_img, cv::COLOR_BGR2HSV);
        //FILTER IMAGES FOR 3 DIFFERENT COLORS
        cv::inRange(hsv_img, Scalar(0, 70, 50), Scalar(20, 255, 255), hsv_filtered_red);
        cv::inRange(hsv_img, Scalar(110, 50, 50), Scalar(130, 255, 255), hsv_filtered_blue);
        cv::inRange(hsv_img, Scalar(36, 50, 70), Scalar(80, 255, 255), hsv_filtered_green);
        //DILATE IMAGE
        dilate(hsv_filtered_blue, blue_dilated, Mat(), Point(-1, -1), 2, 1, 1);
        dilate(hsv_filtered_green, green_dilated, Mat(), Point(-1, -1), 2, 1, 1);
        dilate(hsv_filtered_red, red_dilated, Mat(), Point(-1, -1), 2, 1, 1);
        
        //FIND CONTOURS OF DILATED IMAGE
        vector<vector<Point> > contours_blue;
        vector<vector<Point> > contours_green;
        vector<vector<Point> > contours_red;
        vector<Vec4i> hierarchy;
        findContours( hsv_filtered_blue, contours_blue, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
        findContours( hsv_filtered_red, contours_red, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
        findContours( hsv_filtered_green, contours_green, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
        //GET AREA OF BIGGEST BLOB
        double maxArea = 0;
        int blob_blue = getMaxAreaContourId(contours_blue, maxArea);
        int blob_green = getMaxAreaContourId(contours_green, maxArea);
        int blob_red = getMaxAreaContourId(contours_red, maxArea);
        cout << "last max area is " << maxArea << ".\n";
        //cout << "blob is:" << blob << ".\n";
        //cout << "blob is:" << contours[blob][1] << ".\n"; 
        
        
        // get the moments
        vector<Moments> mu_blue(contours_blue.size());
        vector<Moments> mu_green(contours_green.size());
        vector<Moments> mu_red(contours_red.size());
        for( int i = 0; i<contours_blue.size(); i++ )
        { mu_blue[i] = moments( contours_blue[i], false ); }
        for( int i = 0; i<contours_green.size(); i++ )
        { mu_green[i] = moments( contours_green[i], false ); }
        for( int i = 0; i<contours_red.size(); i++ )
        { mu_red[i] = moments( contours_red[i], false ); }

        // get the centroid of figures.
        vector<Point2f> mc_blue(contours_blue.size());
        for( int i = 0; i<contours_blue.size(); i++)
        { mc_blue[i] = Point2f( mu_blue[i].m10/mu_blue[i].m00 , mu_blue[i].m01/mu_blue[i].m00 ); }

        vector<Point2f> mc_green(contours_green.size());
        for( int i = 0; i<contours_green.size(); i++)
        { mc_green[i] = Point2f( mu_green[i].m10/mu_green[i].m00 , mu_green[i].m01/mu_green[i].m00 ); }

        vector<Point2f> mc_red(contours_red.size());
        for( int i = 0; i<contours_red.size(); i++)
        { mc_red[i] = Point2f( mu_red[i].m10/mu_red[i].m00 , mu_red[i].m01/mu_red[i].m00 ); }

        Scalar color = Scalar(0,0,255);
        circle( blue_dilated, mc_blue[blob_blue], 4, color, -1, 8, 0 ); //DRAW A CIRCLE AT THE MASS POINT OF THE BIGGEST CONTOUR
        circle( green_dilated, mc_green[blob_green], 4, color, -1, 8, 0 ); //DRAW A CIRCLE AT THE MASS POINT OF THE BIGGEST CONTOUR
        circle( red_dilated, mc_red[blob_red], 4, color, -1, 8, 0 ); //DRAW A CIRCLE AT THE MASS POINT OF THE BIGGEST CONTOUR

        //cout << "blob blue is:" << mc_blue[blob_blue].x << ".\n";
        //cout << "blob green is:" << mc_green[blob_green].y << ".\n";
        //cout << "blob red is:" << mc_red[blob_red] << ".\n";
        nav_msgs::Odometry odom;
        odom.pose.pose.position.x = mc_blue[blob_blue].x;
        odom.pose.pose.position.y = mc_blue[blob_blue].y;
        
        object_pub.publish(odom);
        
        // blue Window
        cv::imshow(OPENCV_FILTERED_WINDOW_BLUE, hsv_filtered_blue);
        cv::waitKey(3);

        // green Window
        cv::imshow(OPENCV_FILTERED_WINDOW_GREEN, hsv_filtered_green);
        cv::waitKey(3);
        
        // RED Window
        cv::imshow(OPENCV_FILTERED_WINDOW_RED, hsv_filtered_red);
        cv::waitKey(3);
        //dilate window
        
        //cv::imshow(BLOB_EXTRACTION, red_dilated);
        cv::imshow(BLOB_EXTRACTION_BLUE, blue_dilated);
        cv::waitKey(3);
        cv::imshow(BLOB_EXTRACTION_GREEN, green_dilated);
        cv::waitKey(3);
        cv::imshow(BLOB_EXTRACTION_RED, red_dilated);
        cv::waitKey(3);
        
        
        //Convert image to Binary
        //cv::Mat binary_img;
        //cv::threshold(grey_img, binary_img, 55, 255, 0);
        //cv::cvThreshold(cv_ptr->image, binary_img, cv::COLOR_RGB2GRAY);

        // Create Binary Window
        //cv::imshow(OPENCV_BINARY_WINDOW, binary_img);
        //cv::waitKey(3);

        // Output modified video stream
        //image_pub_.publish(cv_ptr->toImageMsg());
    }
    /*
    //FUNCTION FOR BOTTOM CAMERA
    void imageCb1(const sensor_msgs::ImageConstPtr& msg_bottom)
    {
        cv_bridge :: CvImagePtr cv_ptr;
        try
        {
            cv_ptr = cv_bridge::toCvCopy(msg_bottom, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception& e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }

        // Update GUI Window
        cv::imshow(OPENCV_BOTTOM_WINDOW, cv_ptr->image);
        cv::waitKey(3);

        //Convert image to Greyscale
        cv::Mat grey_img;
        cv::cvtColor(cv_ptr->image, grey_img, cv::COLOR_RGB2GRAY);

        //Blur image
        cv::Mat blur_img;
        medianBlur(grey_img, blur_img, 5);
        cv::imshow(OPENCV_BOTTOM_BLUR, blur_img);
        cv::waitKey(3);

        //GET CIRCLE OBJECTS WITH hOUGHCIRCLES
        vector<Vec3f> circles;
        HoughCircles(blur_img, circles, HOUGH_GRADIENT, 1,
                    blur_img.rows/16,  // change this value to detect circles with different distances to each other
                    100, 30, 1, 30 // change the last two parameters
                // (min_radius & max_radius) to detect larger circles
        );


        //Draw circles
        for( size_t i = 0; i < circles.size(); i++ )
        {
            Vec3i c = circles[i];
            Point center = Point(c[0], c[1]);
            // circle center
            circle( cv_ptr->image, center, 1, Scalar(0,100,100), 3, LINE_AA);
            // circle outline
            int radius = c[2];
            circle( cv_ptr->image, center, radius, Scalar(255,0,255), 3, LINE_AA);
        }

        cv::imshow(OPENCV_BOTTOM_CIRCLE, cv_ptr->image);
        cv::waitKey(3);
    }*/
};



int  main(int argc , char **argv){
	ros::init(argc , argv , "keyboard_node",ros::init_options:: AnonymousName);
    ImageConverter ic;
	ros::NodeHandle n;
	//ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);

	ros::spin();

	return 0;
}
