#include <ros/ros.h>

#include <functional>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "enshu3/detection_camera.hpp"
#include "enshu3/myrobot.h"
#include "enshu3/utils.hpp"

int main(int argc, char** argv)
{
  // ROS node
  ros::init(argc, argv, "enshu3_driving");
  ros::NodeHandle n;
  ros::Rate rate(MAIN_RATE);
  DetectionCamera camera(n, rate);
  MyRobot robot(n, rate);

  // Main loop
  while (ros::ok() && !robot.is_finished() && !camera.is_finished())
  {
    //////////// <write your code from here> /////////////

    int ls = robot.get_ls();
    int rs = robot.get_rs();

    ROS_INFO("Left Sonar %i cm", ls);
    ROS_INFO("Right Sonar %i cm", rs);

    cv::Mat img = camera.get_img();
    if (!img.empty())
    {
      // Command for robot
      double v = 0;
      double omega = 0;

      // Set robot velocity according to the detected object
      // (Only the following types of objects can be detected)
      // - stop sign
      // - person
      // - cat
      // - traffic light
      std::vector<BBox> detection = camera.get_detection();
      for (int i = 0; i < detection.size(); i++)
      {
        BBox bbox = detection[i];
        int center_x = (bbox.ul.x + bbox.br.x) / 2;
        int center_y = (bbox.ul.y + bbox.br.y) / 2;
        int width = (bbox.br.x - bbox.ul.x);
        int height = (bbox.br.y - bbox.ul.y);
        ROS_INFO("[%s] center:(%d, %d), width:%d, height:%d", bbox.label.c_str(), center_y, center_x, width, height);

        if (bbox.label == "stop sign")
        {
          if (center_x < (640 / 2) - 50)
          {
            v = 0.02;
            omega = 0.2;
          }
          else if (center_x > (640 / 2) + 50)
          {
            v = 0.02;
            omega = -0.2;
          }
          else
          {
            v = 0.04;
            omega = 0.0;
          }
        }
      }

      // Send command to robot
      robot.move(v, omega);

      /// Display robot command
      camera.add_command(v, omega);
      /// Display detections
      camera.add_detection();
      /// Show image
      camera.show_img();
    }

    ////////////////////////////////////////////////////////
    ros::spinOnce();
    rate.sleep();
  }

  return 0;
}
