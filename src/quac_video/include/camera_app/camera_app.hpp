#include <rclcpp/rclcpp.hpp>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <librealsense2/rs.hpp>
#include <image_transport/image_transport.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
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
#include <mutex>

#include "yolos/tasks/detection.hpp"
#include "quirc.h"


struct detection_state
{
  bool enabled;
  std::string model;
  std::string model_path;
  std::string engine_path;
};

class CameraApp : public rclcpp::Node
{
public:
  CameraApp();

  void init_detection_state(struct detection_state* info, const std::string& prefix);

  int init(image_transport::ImageTransport &it);
  int model_init(struct detection_state* info);

  void ip_callback(const std_msgs::msg::String::SharedPtr msg);
  void run();

  void qrcode_thread_function();

  std::string ip;
  bool ip_set;
  int port;
  std::string serial_number;
  int width, height, fps, bitrate, key_int_max;

  rs2::config rs_cfg;
  rs2::pipeline rs_pipeline;

  GstElement* gst_pipeline;
  GstElement* gst_appsrc;  

  std::string engines_dir;
  std::string models_dir;
  struct detection_state hazmat_info;
  struct detection_state paintroller_info;

  struct
  {
    std::mutex mutex;
    struct quirc* quirc_instance;
    void* bgr_data;
    void* depth_data;
    double depth_units;
    bool queued;
  } qr_code_state;

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr ip_subscriber;
};