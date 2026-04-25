#include <rclcpp/callback_group.hpp>
#include <rclcpp/rclcpp.hpp>
#include <cstdlib>
#include <quac_interfaces/msg/image_bgrd.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <quac_interfaces/msg/detected_object.hpp>
#include <quac_interfaces/msg/detected_object_array.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <opencv2/opencv.hpp>
#include <memory>
#include <cstdlib>
#include <mutex>

struct TopicHandler
{
  rclcpp::Subscription<quac_interfaces::msg::ImageBGRD>::SharedPtr subscriber;
  rclcpp::Publisher<quac_interfaces::msg::DetectedObjectArray>::SharedPtr object_publisher;
};

struct detection
{
  struct
  {
    int x, y;
  } corners[4];
  std::string data;
  double confidence;
};

struct mapped_object_data
{
  int times_detected;
  sensor_msgs::msg::Image::SharedPtr image;
};

using DetectionCallback = std::function<void(
    const quac_interfaces::msg::ImageBGRD::SharedPtr,
    int,
    std::vector<detection>&
)>;

class DetectionServer : public rclcpp::Node
{
public:
  DetectionServer(const std::string& name, DetectionCallback callback);

  void run();
  void draw_bounding_box(cv::Mat& image, const detection& detection, const std::string name);
  void image_callback(const quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i);
  void publish_objects_callback();
  void publish_images_callback();

protected:
  std::vector<std::unique_ptr<TopicHandler>> topic_handlers;
  rclcpp::CallbackGroup::SharedPtr callback_group;
  tf2_ros::Buffer tf_buffer;
  tf2_ros::TransformListener tf_listener;

  DetectionCallback detection_callback;

  struct
  {
    rclcpp::Publisher<quac_interfaces::msg::DetectedObjectArray>::SharedPtr object_publisher;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_publisher;
    rclcpp::TimerBase::SharedPtr object_timer;
    rclcpp::TimerBase::SharedPtr image_timer;
    
    std::mutex detections_mutex;
    std::vector<quac_interfaces::msg::DetectedObject> objects;
    std::vector<mapped_object_data> object_datas;
    std::string reference_frame;
  } mapping;

  std::vector<std::string> camera_names;
  std::string object_name;
  double consideration_radius;
};