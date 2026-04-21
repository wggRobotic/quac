#include <rclcpp/callback_group.hpp>
#include <rclcpp/rclcpp.hpp>
#include <cstdlib>
#include <quac_interfaces/msg/image_bgrd.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <quac_interfaces/msg/detected_object_array.hpp>

#include <memory>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <pthread.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <mutex>
#include "yolos/tasks/detection.hpp"

struct TopicHandler
{
  std::shared_ptr<yolos::det::YOLODetector> detector;
  rclcpp::Subscription<quac_interfaces::msg::ImageBGRD>::SharedPtr subscriber;
  rclcpp::Publisher<quac_interfaces::msg::DetectedObjectArray>::SharedPtr object_publisher;
};

struct detection
{
  float x, y, z;
  int count;
  std::string type;
  std::string name;
};

class HazmatDetectionServer : public rclcpp::Node
{
public:
  HazmatDetectionServer();

  void run();
  void image_callback(const quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i);
  void publish_callback();

private:
  std::vector<std::unique_ptr<TopicHandler>> topic_handlers;
  rclcpp::CallbackGroup::SharedPtr callback_group;
  tf2_ros::Buffer tf_buffer;
  tf2_ros::TransformListener tf_listener;

  struct
  {
    rclcpp::Publisher<quac_interfaces::msg::DetectedObjectArray>::SharedPtr object_publisher;
    rclcpp::TimerBase::SharedPtr timer;
    
    std::mutex detections_mutex;
    std::vector<detection> detections;
    std::string reference_frame;
  } mapping;

  std::vector<std::string> topics;
  std::vector<std::string> camera_frames;
  std::string model_path;
  std::string engine_path;
  std::string labels_path;
};