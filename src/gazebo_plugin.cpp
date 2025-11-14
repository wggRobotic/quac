#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <thread>
#include <mutex>

namespace gazebo
{
class TwistJointPlugin : public ModelPlugin
{
public:
  void Load(physics::ModelPtr model, sdf::ElementPtr sdf) override
  {
    // ROS 2 setup
    if (!rclcpp::ok())
      rclcpp::init(0, nullptr);

    node_ = std::make_shared<rclcpp::Node>("twist_joint_plugin_node");

    subscription_ = node_->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10,
        std::bind(&TwistJointPlugin::OnTwistMsg, this, std::placeholders::_1));

    joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    odom_pub_ = node_->create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(node_);

    ros_thread_ = std::thread([this]() { rclcpp::spin(node_); });

    // Connect update event
    update_connection_ = event::Events::ConnectWorldUpdateBegin(
        std::bind(&TwistJointPlugin::OnUpdate, this));

    model_ = model;

    // Read joint names from SDF
    for (int i = 1; i <= 4; ++i)
    {
      std::string jointParam = "joint" + std::to_string(i);

      if (sdf->HasElement(jointParam))
      {
        std::string jointName = sdf->Get<std::string>(jointParam);

        auto joint = model_->GetJoint(jointName);
        if (joint)
        {
          RCLCPP_INFO(node_->get_logger(), "%s is %s in model", jointParam.c_str(), jointName.c_str());
          joints_.push_back(joint);
        }
        else
        {
          gzerr << "Joint [" << jointName << "] not found in model.\n";
        }
      }
    }

    if (joints_.size() != 4)
      gzerr << "Expected 4 joints, got " << joints_.size() << "\n";

    last_cmd_time_ = node_->get_clock()->now();

    // Initialize odometry
    x_ = 0.0;
    y_ = 0.0;
    yaw_ = 0.0;
    prev_wheel_pos_ = {0.0, 0.0, 0.0, 0.0};

    RCLCPP_INFO(node_->get_logger(), "TwistJointPlugin loaded with %zu joints", joints_.size());
  }

  void OnTwistMsg(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    std::lock_guard<std::mutex> lock(cmd_mutex_);
    last_cmd_ = *msg;
    last_cmd_time_ = node_->get_clock()->now();
  }

  void OnUpdate()
  {
    std::lock_guard<std::mutex> lock(cmd_mutex_);

    double wheel_radius = 0.2;
    double wheel_separation = 0.18;
    double max_torque = 1.0;

    // Timeout stop: 0.5s
    auto now = node_->get_clock()->now();
    if ((now - last_cmd_time_).seconds() > 0.5)
    {
      last_cmd_.linear.x = 0.0;
      last_cmd_.angular.z = 0.0;
    }

    // --- 1️⃣ Apply velocity commands ---
    double v = last_cmd_.linear.x;
    double w = last_cmd_.angular.z;

    double v_left = v - (wheel_separation / 2.0) * w;
    double v_right = v + (wheel_separation / 2.0) * w;

    double left_vel = v_left / wheel_radius;
    double right_vel = v_right / wheel_radius;

    double target_velocities[4] = {left_vel, -right_vel, left_vel, -right_vel};

    for (size_t i = 0; i < joints_.size(); i++)
    {
      joints_[i]->SetParam("fmax", 0, max_torque);
      joints_[i]->SetVelocity(0, target_velocities[i]);
      RCLCPP_INFO(node_->get_logger(), "velocity joint%ld: %f", i, target_velocities[i]); 
    }

    // --- 2️⃣ Publish joint states ---
    auto joint_msg = sensor_msgs::msg::JointState();
    joint_msg.header.stamp = now;
    for (auto &joint : joints_)
    {
      joint_msg.name.push_back(joint->GetName());
      joint_msg.position.push_back(joint->Position(0));
      joint_msg.velocity.push_back(joint->GetVelocity(0));
      joint_msg.effort.push_back(0.0);
    }
    joint_state_pub_->publish(joint_msg);

    // --- 3️⃣ Compute odometry from wheel positions ---
    double left_front = joints_[0]->Position(0);
    double right_front = joints_[1]->Position(0);
    double left_rear = joints_[2]->Position(0);
    double right_rear = joints_[3]->Position(0);

    // average front and rear wheels
    double left = (left_front + left_rear) / 2.0;
    double right = (right_front + right_rear) / 2.0;

    double delta_left = left - prev_wheel_pos_[0];
    double delta_right = right - prev_wheel_pos_[1];

    prev_wheel_pos_[0] = left;
    prev_wheel_pos_[1] = right;

    double d_left = delta_left * wheel_radius;
    double d_right = delta_right * wheel_radius;

    double d_center = (d_right + d_left) / 2.0;
    double d_theta = (d_right - d_left) / wheel_separation;

    x_ += d_center * cos(yaw_ + d_theta / 2.0);
    y_ += d_center * sin(yaw_ + d_theta / 2.0);
    yaw_ += d_theta;

    // --- 4️⃣ Publish odometry ---
    auto odom_msg = nav_msgs::msg::Odometry();
    odom_msg.header.stamp = now;
    odom_msg.header.frame_id = "odom";
    odom_msg.child_frame_id = "base_link";

    odom_msg.pose.pose.position.x = x_;
    odom_msg.pose.pose.position.y = y_;
    odom_msg.pose.pose.position.z = 0.0;
    tf2::Quaternion q;
    q.setRPY(0, 0, yaw_);
    odom_msg.pose.pose.orientation.x = q.x();
    odom_msg.pose.pose.orientation.y = q.y();
    odom_msg.pose.pose.orientation.z = q.z();
    odom_msg.pose.pose.orientation.w = q.w();

    odom_msg.twist.twist.linear.x = d_center / 0.001; // approximate vel
    odom_msg.twist.twist.angular.z = d_theta / 0.001;

    odom_pub_->publish(odom_msg);

    // --- 5️⃣ Broadcast TF ---
    geometry_msgs::msg::TransformStamped odom_tf;
    odom_tf.header.stamp = now;
    odom_tf.header.frame_id = "odom";
    odom_tf.child_frame_id = "base_link";
    odom_tf.transform.translation.x = x_;
    odom_tf.transform.translation.y = y_;
    odom_tf.transform.translation.z = 0.0;
    odom_tf.transform.rotation.x = q.x();
    odom_tf.transform.rotation.y = q.y();
    odom_tf.transform.rotation.z = q.z();
    odom_tf.transform.rotation.w = q.w();

    tf_broadcaster_->sendTransform(odom_tf);
  }

  ~TwistJointPlugin()
  {
    rclcpp::shutdown();
    if (ros_thread_.joinable())
      ros_thread_.join();
  }

private:
  physics::ModelPtr model_;
  std::vector<physics::JointPtr> joints_;

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  std::thread ros_thread_;
  event::ConnectionPtr update_connection_;

  geometry_msgs::msg::Twist last_cmd_;
  rclcpp::Time last_cmd_time_;
  std::mutex cmd_mutex_;

  // odometry state
  double x_, y_, yaw_;
  std::vector<double> prev_wheel_pos_; // store previous left/right positions
};

GZ_REGISTER_MODEL_PLUGIN(TwistJointPlugin)
} // namespace gazebo
