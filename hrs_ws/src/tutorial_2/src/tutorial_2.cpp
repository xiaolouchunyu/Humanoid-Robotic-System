#include "ros/ros.h"
#include "std_msgs/String.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

static const std::string OPENCV_WINDOW = "Original image";
static const std::string OPENCV_GREY_WINDOW = "Greyscale image";
static const std::string OPENCV_BINARY_WINDOW = "Binary image";

class ImageConverter
{
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;
    image_transport::Publisher image_pub_;

public:
    ImageConverter()
        : it_(nh_)
    {
        
        image_sub_ = it_.subscribe("/nao_robot/camera/top/camera/image_raw", 1, &ImageConverter::imageCb, this);
        //image_pub_ = it_.advertise("/image_converter/output_video", 1);

        cv::namedWindow(OPENCV_WINDOW);
    }

    ~ImageConverter()
   {
     cv::destroyWindow(OPENCV_WINDOW);
   }

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

        //Convert image to Greyscale
        cv::Mat grey_img;
        cv::cvtColor(cv_ptr->image, grey_img, cv::COLOR_RGB2GRAY);

        // Create Greyscale Window
        cv::imshow(OPENCV_GREY_WINDOW, grey_img);
        cv::waitKey(3);

        //Convert image to Binary
        cv::Mat binary_img;
        cv::threshold(grey_img, binary_img, 55, 255, 0);
        //cv::cvThreshold(cv_ptr->image, binary_img, cv::COLOR_RGB2GRAY);

        // Create Binary Window
        cv::imshow(OPENCV_BINARY_WINDOW, binary_img);
        cv::waitKey(3);

        // Output modified video stream
        image_pub_.publish(cv_ptr->toImageMsg());
    }

};

void chatterCallback(const std_msgs::String::ConstPtr& msg){
	ROS_INFO("Keyboard input: [%s]", msg->data.c_str());
}


int  main(int argc , char **argv){
	ros::init(argc , argv , "keyboard_node",ros::init_options:: AnonymousName);
    	ImageConverter ic;
	ros::NodeHandle n;
	ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);

	ros::spin();

	return 0;
}
