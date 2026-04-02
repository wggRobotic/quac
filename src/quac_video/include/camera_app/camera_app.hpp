#include <rclcpp/rclcpp.hpp>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <librealsense2/rs.hpp>
#include <image_transport/image_transport.hpp>
#include <std_msgs/msg/string.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <memory>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>

#include "yolos/tasks/detection.hpp"


class CameraApp : public rclcpp::Node
{
public:
  CameraApp();

  int init(image_transport::ImageTransport &it);

  void ip_callback(const std_msgs::msg::String::SharedPtr msg);

  void run();
  void send_gst_frame(const void* data);

  std::string ip;
  bool ip_set;
  int port;
  std::string serial_number;
  int width, height, fps;

  rs2::config rs_cfg;
  rs2::pipeline rs_pipeline;

  GstElement* gst_pipeline;
  GstElement* gst_appsrc;  

  std::vector<std::string> hazmat_labels;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr ip_subscriber;
};