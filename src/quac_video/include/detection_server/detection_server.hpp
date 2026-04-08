#include <rclcpp/rclcpp.hpp>
#include <image_transport/image_transport.hpp>
#include <tf2_msgs/msg/detail/tf_message__struct.hpp>
#include <tf2_msgs/msg/tf_message.hpp>
#include <std_msgs/msg/string.hpp>
#include <cstdlib>



class DetectionServer : public rclcpp::Node
{
public:
  DetectionServer();

  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr detection_subscriber;
};