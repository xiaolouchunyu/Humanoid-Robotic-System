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


//GET AREA OF BIGGEST BLOB
int getMaxAreaContourId(vector <vector<cv::Point> > contours) {
    double maxArea = 0;
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
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;   //CREATE SUB FOR TOP CAMERA
    image_transport::Publisher image_pub_;  //CREATE PUBLISHER 
    image_transport::Subscriber bottom_sub_;  //CREATE SUB FOR BOTTOM CAMERA

public:
    
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

        
        //Get reference image
        if (ref_img.empty()){
            ref_img = cv_ptr->image;
            cvtColor(ref_img, ref_gray, COLOR_BGR2GRAY);

            goodFeaturesToTrack(ref_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

            mask = Mat::zeros(ref_img.size(), ref_img.type());
        }

        // Update GUI Window

        cv::imshow(OPENCV_WINDOW, cv_ptr->image);
        cv::waitKey(3);

        //Add rectangle to img
        Mat roi = imread("/home/nao_a/hrs_ws/src/tutorial_4/TemplateImg.jpg");
        cv::imshow(OBJECT_SCREENSHOT, roi);

        //Rect_( cv_ptr->image, center, radius, Scalar(255,0,255), 3, LINE_AA);
        //Rect rect(169, 112, 51, 70);
        extern Rect rect;
        rectangle(roi, rect, cv::Scalar(0, 255, 0));

        // Show screenshot with ROI

        cv::imshow(ROI_SELECTOR, roi);
        cv::waitKey(3);

        //CONVERT TO HSV IMAGES
        cv::Mat3b hsv_roi;
        cv::Mat3b hsv_camera;
        cv::cvtColor(roi, hsv_roi, cv::COLOR_BGR2HSV);
        cv::cvtColor(cv_ptr->image, hsv_camera, cv::COLOR_BGR2HSV);

        //Split imagescvCalcOpticalFlowPyrLK
        Mat roi_channels[3];
        Mat camera_channels[3];
        split(hsv_roi, roi_channels);
        split(hsv_camera, camera_channels);
        
        //Mask low saturated pixels
        Mat1b mask_roi;
        Mat1b mask_camera;
        threshold(roi_channels[1], mask_roi, 150, 255,0);
        threshold(camera_channels[1], mask_camera, 150, 255,0);

        //Set histogram setting and create histogram
        MatND hist;
        float hranges[] = { 0, 180 };
        float sranges[] = { 0, 255 };
        const float* ranges[] = {hranges};
        int hbins = 30, sbins = 32;
        int histSize[] = {sbins};
        cv::calcHist(&roi_channels[0], 1, 0, mask_roi, hist, 1, histSize, ranges , true, false);
        normalize(hist, hist, 0, 255, NORM_MINMAX);

        //Do back projection
        Mat backproj;
        calcBackProject( &camera_channels[0], 1, 0, hist, backproj, ranges, 1, true );
        Mat bitwise;
        bitwise_and(backproj, camera_channels[1], bitwise);

        cv::imshow(BACKPROJ, backproj);
        cv::waitKey(3);

        cv::imshow(TASK5, bitwise);
        cv::waitKey(3);

        TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);


        //Tracking with meanShift

        //Shift rectangle to match object
        //meanShift(bitwise, rect , term_crit);

        //Show camera tracking object
        //rectangle(cv_ptr->image, rect, 255, 2);
        //cv::imshow(TRACKING, cv_ptr->image);

        //Tracking with CamShift

        RotatedRect camshift_rect = CamShift(bitwise, rect2, term_crit);

        //Show camera tracking object
        Mat camshift_img;
        camshift_img = cv_ptr->image.clone();

        Point2f points[4];
        camshift_rect.points(points);
        for (int i = 0; i < 4; i++)
            line(camshift_img, points[i], points[(i+1)%4], 255, 2);
        cv::imshow(CAMSHIFT, camshift_img);
        
        Mat next_gray;
        
        next_img = cv_ptr->image;
        cvtColor(next_img, next_gray, COLOR_BGR2GRAY);



        // calculate optical flow
        vector<uchar> status;
        vector<float> err;
        TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
        calcOpticalFlowPyrLK(ref_gray, next_gray, p0, p1, status, err, Size(15,15), 2, criteria);

        //Colors for optical flow
        for(int i = 0; i < 100; i++)
        {
            int r = rng.uniform(0, 256);
            int g = rng.uniform(0, 256);
            int b = rng.uniform(0, 256);
            colors.push_back(Scalar(r,g,b));
        }

        vector<Point2f> good_new;
        for(uint i = 0; i < p0.size(); i++)
        {
            // Select good points
            if(status[i] == 1) {
                good_new.push_back(p1[i]);
                // draw the tracks
                line(mask,p1[i], p0[i], colors[i], 2);
                circle(next_gray, p1[i], 5, colors[i], -1);
            }
        }

        //Combine mask and latest camera input, show result
        Mat flow;
        add(next_img, mask, flow);
        imshow("Optical flow", flow);

        ref_gray = next_gray.clone();
        p0 = good_new;



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
            cout<<"Rotation between camera and marker coord system: "<<m.Rvec<<endl;
            cout<<"Translation between camera and marker coord system: "<<m.Tvec<<endl;
        }

        imshow("Marker tracker", aruco_img);
    }
};



int  main(int argc , char **argv){
	ros::init(argc , argv , "keyboard_node",ros::init_options:: AnonymousName);
    ImageConverter ic;
	ros::NodeHandle n;
	//ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);

	ros::spin();

	return 0;
}
