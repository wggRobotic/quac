#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <mutex>
#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/subscription.hpp>

class topic_handler
{
  public:
    bool stamped;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr unstamped_subscriber;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr stamped_subscriber;

    double priority;
    double timeout;
};

class CmdVelMux : public rclcpp::Node
{
public:
  CmdVelMux() : Node("cmd_vel_mux")
  {
    declare_parameter<std::string>("out.topic", "cmd_vel");
    std::string out_topic = get_parameter("out.topic").as_string();

    declare_parameter<bool>("out.stamped_flag", false);
    out.stamped = get_parameter("out.stamped_flag").as_bool();

    declare_parameter<std::string>("out.stamp_frame", "base_link");
    out.frame = get_parameter("out.stamp_frame").as_string();

    if (out.stamped) out.stamped_publisher = create_publisher<geometry_msgs::msg::TwistStamped>(out_topic, 10);
    else out.unstamped_publisher = create_publisher<geometry_msgs::msg::Twist>(out_topic, 10);

    declare_parameter<std::vector<std::string>>("in.topics", {"cmd_vel_unmuxed"});
    std::vector<std::string> topics = get_parameter("in.topics").as_string_array();

    declare_parameter<std::vector<double>>("in.timeouts", {0.5});
    std::vector<double> timeouts = get_parameter("in.timeouts").as_double_array();

    declare_parameter<std::vector<bool>>("in.stamped_flags", {false});
    std::vector<bool> stamped_flags = get_parameter("in.stamped_flags").as_bool_array();

    declare_parameter<std::vector<double>>("in.priorities", {100.});
    std::vector<double> priorities = get_parameter("in.priorities").as_double_array();

    for (int i = 0; i < topics.size() && i < timeouts.size() && i < stamped_flags.size() && i < priorities.size(); i++)
    {
      topic_handler h;
      h.stamped = stamped_flags[i];
      h.priority = priorities[i];
      h.timeout = timeouts[i];

      if (h.stamped) h.stamped_subscriber = create_subscription<geometry_msgs::msg::TwistStamped>(
        topics[i], 
        10, 
        [this, i](geometry_msgs::msg::TwistStamped::ConstSharedPtr msg) {
          callback(msg->twist, msg->header.stamp, i);
        }
      );
      else h.unstamped_subscriber = create_subscription<geometry_msgs::msg::Twist>(
        topics[i], 
        10, [this, i](geometry_msgs::msg::Twist::ConstSharedPtr msg) {
          callback(*msg, now(), i);
        }
      );

      RCLCPP_INFO(
        get_logger(), 
        "Listening on topic '%s' : %s with timeout %f and priority %f", 
        topics[i].c_str(), 
        stamped_flags[i] ? "stamed" : "unstamped", 
        timeouts[i], 
        priorities[i]
      );

      handlers.push_back(h);
    }

    handler_i = -1;
  }

  void callback(const geometry_msgs::msg::Twist& msg, rclcpp::Time stamp, int i)
  {
    bool valid = false;
    if (handler_i == -1) valid = true;
    if (handler_i == i) valid = true;
    if (!valid) if(handlers[i].priority > handlers[handler_i].priority) valid = true;
    if (!valid) if ((now() - last_time).seconds() >= handlers[handler_i].timeout) valid = true;
    
    if (!valid) return;

    handler_i = i;
    last_time = now();

    if (out.stamped)
    {
      geometry_msgs::msg::TwistStamped out_msg;
      out_msg.twist = msg;
      out_msg.header.frame_id = out.frame;
      out_msg.header.stamp = stamp;
      out.stamped_publisher->publish(out_msg);
    }
    else out.unstamped_publisher->publish(msg);
  }

private:

  struct
  {
    bool stamped;
    std::string frame;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr unstamped_publisher;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr stamped_publisher;
  } out;

  std::vector<topic_handler> handlers;
  int handler_i;
  rclcpp::Time last_time;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CmdVelMux>());
  rclcpp::shutdown();
  return 0;
}